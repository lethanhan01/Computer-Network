/*
 * UDP Echo Client / UDP Chat Client
 * Học phần: IT4062 - Lập trình mạng
 * 
 * Mô tả:
 *   Chương trình UDP Client gửi chuỗi văn bản do người dùng nhập từ bàn phím
 *   đến UDP Server theo địa chỉ IP và Port từ tham số dòng lệnh,
 *   nhận phản hồi lỗi từ server hoặc tin nhắn chuyển tiếp từ Client khác qua cơ chế select().
 */

#include <stdio.h>          /* Thư viện vào/ra chuẩn: printf, perror, fgets */
#include <stdlib.h>         /* Thư viện chuẩn: exit, malloc, free */
#include <sys/types.h>      /* Các kiểu dữ liệu hệ thống */
#include <sys/socket.h>     /* Các hàm và cấu trúc socket: socket, sendto, recvfrom */
#include <netinet/in.h>     /* Các cấu trúc địa chỉ Internet: sockaddr_in, in_addr */
#include <arpa/inet.h>      /* Các hàm chuyển đổi địa chỉ: htons, inet_addr, inet_pton */
#include <string.h>         /* Thao tác chuỗi và bộ nhớ: memset, bzero, strlen */
#include <unistd.h>         /* Các lời gọi hệ thống chuẩn: close */
#include <sys/select.h>     /* I/O Multiplexing: select, fd_set */
#include <errno.h>          /* Mã lỗi hệ thống: errno */

#define SERV_PORT 5550      /* Cổng mặc định nếu cần tham khảo */
#define SERV_IP "127.0.0.1" /* Địa chỉ IP mặc định nếu cần tham khảo */
#define BUFF_SIZE 1024      /* Kích thước bộ đệm chứa dữ liệu */
#define MIN_PORT 1          /* Số hiệu cổng nhỏ nhất */
#define MAX_PORT 65535      /* Số hiệu cổng lớn nhất */

#define CONNECT_TOKEN "__CONNECT__"
#define EXIT_TOKEN_AT "@"
#define EXIT_TOKEN_HASH "#"

/**
 * @brief Kiểm tra tính hợp lệ của địa chỉ IPv4.
 */
int parse_and_validate_ip(const char *ip_str, struct in_addr *out_addr)
{
	if (ip_str == NULL || *ip_str == '\0') {
		return 0;
	}
	if (inet_pton(AF_INET, ip_str, out_addr) <= 0) {
		return 0;
	}
	return 1;
}

/**
 * @brief Kiểm tra và chuyển đổi số hiệu cổng.
 */
int parse_and_validate_port(const char *port_str, int *out_port)
{
	if (port_str == NULL || *port_str == '\0') {
		return 0;
	}
	char *end_ptr = NULL;
	errno = 0;
	long val = strtol(port_str, &end_ptr, 10);
	if (errno != 0 || *end_ptr != '\0' || val < MIN_PORT || val > MAX_PORT) {
		return 0;
	}
	if (out_port != NULL) {
		*out_port = (int)val;
	}
	return 1;
}

/**
 * @brief Cắt bỏ ký tự xuống dòng (\r, \n) ở cuối chuỗi.
 */
void trim_crlf(char *str)
{
	if (str == NULL) {
		return;
	}
	size_t len = strlen(str);
	while (len > 0 && (str[len - 1] == '\r' || str[len - 1] == '\n')) {
		str[len - 1] = '\0';
		len--;
	}
}

/**
 * @brief Hàm chính thực thi UDP Client.
 * 
 * Chi tiết luồng xử lý:
 *   - Bước 1: Tạo socket UDP phía client với domain AF_INET và type SOCK_DGRAM.
 *   - Bước 2: Thiết lập thông tin địa chỉ server đích (IP và Port từ tham số dòng lệnh).
 *   - Bước 3: Giao tiếp với Server bằng cơ chế I/O multiplexing select() trên bàn phím và socket.
 */
int main(int argc, char *argv[])
{
	/* Kiểm tra tham số dòng lệnh */
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <IPAddress> <PortNumber>\n", argv[0]);
		fprintf(stderr, "Example: %s 127.0.0.1 5500\n", argv[0]);
		return EXIT_FAILURE;
	}

	struct in_addr server_ip_addr;
	if (!parse_and_validate_ip(argv[1], &server_ip_addr)) {
		fprintf(stderr, "Error: Invalid IP address '%s'.\n", argv[1]);
		return EXIT_FAILURE;
	}

	int port = 0;
	if (!parse_and_validate_port(argv[2], &port)) {
		fprintf(stderr, "Error: Invalid port number '%s'. Port must be between %d and %d.\n",
				argv[2], MIN_PORT, MAX_PORT);
		return EXIT_FAILURE;
	}

	int client_sock;                 /* File descriptor của socket client */
	char buff[BUFF_SIZE];            /* Bộ đệm chứa thông điệp gửi và nhận */
	struct sockaddr_in server_addr;  /* Cấu trúc lưu thông tin địa chỉ server đích */
	int bytes_sent, bytes_received;  /* Số byte thực tế đã gửi hoặc nhận */
	socklen_t sin_size;              /* Kích thước của struct sockaddr_in */
	
	/* =========================================================================
	 * Step 1: Khởi tạo socket UDP
	 * =========================================================================
	 * Hàm socket():
	 *   - Vào: 
	 *       + AF_INET: Giao thức mạng IPv4.
	 *       + SOCK_DGRAM: Giao thức truyền gói tin UDP không kết nối.
	 *       + 0: Giao thức mặc định của SOCK_DGRAM (IPPROTO_UDP).
	 *   - Ra: File descriptor của socket nếu thành công, hoặc < 0 nếu thất bại.
	 */
	if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("\nError: ");
		exit(0);
	}

	/* =========================================================================
	 * Step 2: Xác định thông tin địa chỉ của Server đích
	 * =========================================================================
	 * Cấu hình struct server_addr:
	 *   - bzero: Xóa sạch toàn bộ cấu trúc về 0 để tránh dữ liệu rác.
	 *   - sin_family = AF_INET (IPv4).
	 *   - sin_port = htons(SERV_PORT): Chuyển port từ định dạng máy chủ sang mạng.
	 *   - sin_addr.s_addr = inet_addr(SERV_IP): Chuyển chuỗi IP "127.0.0.1" sang số nguyên 32-bit (network byte order).
	 */
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr = server_ip_addr;
	
	/* =========================================================================
	 * Step 3: Giao tiếp với Server (Gửi chuỗi và nhận lại phản hồi)
	 * =========================================================================
	 */
	sin_size = sizeof(struct sockaddr_in);

	/* Gửi gói tin đăng ký ban đầu (__CONNECT__) lên Server */
	sendto(client_sock, CONNECT_TOKEN, strlen(CONNECT_TOKEN), 0, (struct sockaddr *)&server_addr, sin_size);

	printf("Connected to server %s at port %d\n", argv[1], port);
	printf("[You]: ");
	fflush(stdout);

	while (1) {
		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(STDIN_FILENO, &read_fds);
		FD_SET(client_sock, &read_fds);

		int max_fd = (client_sock > STDIN_FILENO) ? client_sock : STDIN_FILENO;

		int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
		if (activity < 0) {
			if (errno == EINTR) {
				continue;
			}
			perror("Error in select");
			break;
		}

		/*
		 * Nhánh A: Nhận gói tin phản hồi hoặc tin nhắn chuyển tiếp từ Server
		 */
		if (FD_ISSET(client_sock, &read_fds)) {
			/*
			 * Hàm recvfrom(): Chờ nhận gói tin phản hồi từ Server.
			 *   - Vào:
			 *       + client_sock: Socket descriptor của client.
			 *       + buff: Con trỏ vùng đệm lưu dữ liệu nhận về.
			 *       + BUFF_SIZE - 1: Số byte tối đa đọc vào (để chừa 1 byte kết thúc chuỗi '\0').
			 *       + 0: Cờ nhận tin mặc định.
			 *       + (struct sockaddr*)&server_addr: Nơi lưu địa chỉ server phản hồi.
			 *       + &sin_size: Con trỏ kích thước của struct server_addr.
			 *   - Ra: 
			 *       + Số byte nhận được thực tế.
			 *       + < 0 nếu có lỗi khi nhận.
			 */
			bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&server_addr, &sin_size);
			if (bytes_received < 0) {
				if (errno != EINTR) {
					perror("Error: ");
				}
			} else {
				buff[bytes_received] = '\0';        /* Đảm bảo chuỗi nhận được kết thúc an toàn */
				if (isatty(STDIN_FILENO)) {
					printf("\r\033[K%s", buff);
					if (buff[bytes_received - 1] != '\n') {
						printf("\n");
					}
					printf("[You]: ");
					fflush(stdout);
				} else {
					printf("%s", buff);
					if (buff[bytes_received - 1] != '\n') {
						printf("\n");
					}
					fflush(stdout);
				}
			}
		}

		/*
		 * Nhánh B: Người dùng nhập dữ liệu từ bàn phím và gửi lên Server
		 */
		if (FD_ISSET(STDIN_FILENO, &read_fds)) {
			memset(buff, 0, sizeof(buff));       /* Khởi tạo toàn bộ bộ đệm về ký tự null '\0' */
			if (fgets(buff, BUFF_SIZE, stdin) == NULL) {
				close(client_sock);
				return 0;
			}

			trim_crlf(buff);

			/* Bỏ qua dòng trống */
			if (strlen(buff) == 0) {
				if (isatty(STDIN_FILENO)) {
					printf("[You]: ");
					fflush(stdout);
				}
				continue;
			}

			/* Kiểm tra tín hiệu thoát: @ hoặc # */
			if (strcmp(buff, EXIT_TOKEN_AT) == 0 || strcmp(buff, EXIT_TOKEN_HASH) == 0) {
				sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *)&server_addr, sin_size);
				break;
			}

			/*
			 * Hàm sendto(): Gửi gói tin dữ liệu tới server qua UDP.
			 * (Lưu ý: UDP là giao thức phi kết nối nên sử dụng sendto thay vì send thông thường).
			 *   - Vào:
			 *       + client_sock: Socket descriptor của client.
			 *       + buff: Con trỏ vùng đệm chứa nội dung cần gửi.
			 *       + strlen(buff): Số lượng byte cần gửi đi.
			 *       + 0: Cờ gửi tin mặc định.
			 *       + (struct sockaddr*)&server_addr: Địa chỉ đích của server.
			 *       + sin_size: Kích thước của struct server_addr.
			 *   - Ra: 
			 *       + Số byte gửi thành công.
			 *       + < 0 nếu có lỗi khi gửi.
			 */
			bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *)&server_addr, sin_size);
			if (bytes_sent < 0) {
				perror("Error: ");
				close(client_sock);
				return 0;
			}

			if (isatty(STDIN_FILENO)) {
				printf("[You]: ");
				fflush(stdout);
			}
		}
	}
		
	/* Đóng socket sau khi trao đổi dữ liệu hoàn tất */
	close(client_sock);
	return 0;
}
