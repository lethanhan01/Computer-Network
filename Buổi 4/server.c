/*
 * UDP Echo Server / UDP 2-Client Chat Server
 * Học phần: IT4062 - Lập trình mạng
 * 
 * Mô tả:
 *   Chương trình UDP Server lắng nghe các gói tin từ client tại một cổng xác định (PORT),
 *   quản lý tối đa 2 clients trao đổi chuỗi ký tự.
 *   - Nhận diện client kết nối mới thông qua token "__CONNECT__" hoặc gói tin đầu tiên.
 *   - Kiểm tra tính hợp lệ của chuỗi nhận được:
 *       + Chuỗi hợp lệ (chỉ gồm chữ cái và chữ số): In ra màn hình server và
 *         chuyển tiếp kèm thông tin IP:Port tới client còn lại.
 *       + Chuỗi không hợp lệ: Tuyệt đối KHÔNG hiển thị tại server, gửi lại
 *         thông báo lỗi cho client gửi.
 *   - Xử lý ngắt kết nối an toàn khi client gửi '@' hoặc '#' và giải phóng slot.
 *   - Bắt tín hiệu SIGINT (Ctrl+C) để đóng socket và thoát an toàn.
 */

#include <stdio.h>          /* Thư viện vào/ra chuẩn: printf, perror, fgets */
#include <stdlib.h>         /* Thư viện chuẩn: exit, malloc, free */
#include <sys/types.h>      /* Các kiểu dữ liệu hệ thống */
#include <sys/socket.h>     /* Các hàm và cấu trúc socket: socket, bind, recvfrom, sendto */
#include <netinet/in.h>     /* Các cấu trúc địa chỉ Internet: sockaddr_in, in_addr */
#include <arpa/inet.h>      /* Các hàm chuyển đổi địa chỉ mạng: htons, ntohs, inet_ntoa */
#include <string.h>         /* Thao tác chuỗi và bộ nhớ: memset, bzero */
#include <unistd.h>         /* Các lời gọi hệ thống chuẩn: close */
#include <ctype.h>          /* Kiểm tra ký tự: isalnum */
#include <signal.h>         /* Bắt tín hiệu hệ thống: signal, SIGINT */
#include <errno.h>          /* Mã lỗi hệ thống: errno */

#define PORT 5550          /* Cổng mặc định nếu cần tham khảo */ 
#define BUFF_SIZE 1024     /* Kích thước bộ đệm nhận/gửi dữ liệu */
#define MAX_CLIENTS 2      /* Giới hạn tối đa 2 clients */
#define MIN_PORT 1         /* Số hiệu cổng nhỏ nhất */
#define MAX_PORT 65535     /* Số hiệu cổng lớn nhất */

#define CONNECT_TOKEN "__CONNECT__"
#define EXIT_TOKEN_AT "@"
#define EXIT_TOKEN_HASH "#"

/**
 * @brief Cấu trúc lưu thông tin phiên kết nối của một Client.
 */
typedef struct {
	struct sockaddr_in address; /* Địa chỉ mạng IP và Port của Client */
	int active;                 /* Cờ trạng thái: 1 = đang kết nối, 0 = slot trống */
} ClientSession;

/* Biến toàn cục lưu socket server để giải phóng khi nhận tín hiệu ngắt */
static int g_server_sock = -1;

/**
 * @brief Hàm xử lý tín hiệu ngắt SIGINT (Ctrl+C) để đóng socket và thoát an toàn.
 */
void handle_signal(int sig)
{
	(void)sig;
	printf("\n[Server] Shutting down server...\n");
	if (g_server_sock >= 0) {
		close(g_server_sock);
		g_server_sock = -1;
	}
	exit(0);
}

/**
 * @brief Kiểm tra và chuyển đổi chuỗi cổng sang số nguyên hợp lệ.
 * 
 * @param[in] port_str Chuỗi ký tự biểu diễn số hiệu cổng.
 * @param[out] out_port Con trỏ lưu giá trị cổng nguyên sau khi kiểm tra.
 * @return int 1 nếu cổng hợp lệ (1 - 65535), 0 nếu không hợp lệ.
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
 * @brief Kiểm tra xem toàn bộ các ký tự trong chuỗi có phải chữ cái hoặc chữ số.
 * 
 * @param[in] str Chuỗi ký tự cần kiểm tra.
 * @return int 1 nếu toàn bộ ký tự hợp lệ, 0 nếu chuỗi rỗng hoặc có ký tự lạ.
 */
int is_alphanumeric_str(const char *str)
{
	if (str == NULL || *str == '\0') {
		return 0;
	}
	for (size_t i = 0; str[i] != '\0'; i++) {
		if (!isalnum((unsigned char)str[i])) {
			return 0;
		}
	}
	return 1;
}

/**
 * @brief So sánh hai cấu trúc sockaddr_in xem có cùng IP và Port không.
 */
int are_sockaddrs_equal(const struct sockaddr_in *addr_a, const struct sockaddr_in *addr_b)
{
	if (addr_a == NULL || addr_b == NULL) {
		return 0;
	}
	return (addr_a->sin_family == addr_b->sin_family &&
			addr_a->sin_port == addr_b->sin_port &&
			addr_a->sin_addr.s_addr == addr_b->sin_addr.s_addr);
}

/**
 * @brief Hàm chính thực thi UDP Server.
 * 
 * Chi tiết luồng xử lý:
 *   - Bước 1: Tạo socket UDP với domain AF_INET và type SOCK_DGRAM.
 *   - Bước 2: Thiết lập cấu trúc sockaddr_in của server (IP INADDR_ANY, cổng từ tham số dòng lệnh) và gọi bind().
 *   - Bước 3: Vào vòng lặp để nhận gói tin từ client qua recvfrom(), xử lý kiểm tra xâu và chuyển tiếp qua sendto().
 */
int main(int argc, char *argv[])
{
	/* Kiểm tra tham số dòng lệnh */
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <PortNumber>\n", argv[0]);
		fprintf(stderr, "Example: %s 5500\n", argv[0]);
		return EXIT_FAILURE;
	}

	int port = 0;
	if (!parse_and_validate_port(argv[1], &port)) {
		fprintf(stderr, "Error: Invalid port number '%s'. Port must be between %d and %d.\n",
				argv[1], MIN_PORT, MAX_PORT);
		return EXIT_FAILURE;
	}

	int server_sock;                  /* File descriptor của socket server */
	char buff[BUFF_SIZE];             /* Bộ đệm lưu trữ dữ liệu nhận và gửi */
	int bytes_sent, bytes_received;   /* Số lượng byte thực tế đã gửi hoặc nhận */
	struct sockaddr_in server;        /* Cấu trúc lưu thông tin địa chỉ của server */
	struct sockaddr_in client;        /* Cấu trúc lưu thông tin địa chỉ của client gửi đến */
	socklen_t sin_size;               /* Kích thước của cấu trúc sockaddr_in */

	/* =========================================================================
	 * Step 1: Khởi tạo socket UDP
	 * =========================================================================
	 * Hàm socket():
	 *   - Vào: 
	 *       + AF_INET: Giao thức IPv4.
	 *       + SOCK_DGRAM: Giao thức UDP (hướng datagram, không kết nối).
	 *       + 0: Giao thức mặc định tương ứng với SOCK_DGRAM (IPPROTO_UDP).
	 *   - Ra: File descriptor của socket (số nguyên không âm nếu thành công, -1 nếu lỗi).
	 */
	if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("\nError: ");
		exit(0);
	}
	g_server_sock = server_sock;

	/* Đăng ký xử lý tín hiệu SIGINT (Ctrl+C) để đóng socket sạch sẽ */
	signal(SIGINT, handle_signal);
	
	/* =========================================================================
	 * Step 2: Gán (bind) địa chỉ IP và Port cho socket
	 * =========================================================================
	 * Thiết lập thông tin server:
	 *   - sin_family = AF_INET (IPv4)
	 *   - sin_port = htons(PORT): Chuyển port từ Host Byte Order sang Network Byte Order.
	 *   - sin_addr.s_addr = INADDR_ANY: Lắng nghe trên tất cả các card mạng của máy.
	 *   - bzero: Xóa phần đệm sin_zero của struct sockaddr_in về 0.
	 */
	server.sin_family = AF_INET;         
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server.sin_zero), 8);

	/* Tái sử dụng cổng tránh lỗi port bị chiếm dụng tạm thời */
	int reuse_opt = 1;
	setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuse_opt, sizeof(reuse_opt));

	/*
	 * Hàm bind():
	 *   - Vào:
	 *       + server_sock: Socket descriptor cần gắn địa chỉ.
	 *       + (struct sockaddr*)&server: Con trỏ trỏ tới cấu trúc địa chỉ server.
	 *       + sizeof(struct sockaddr): Kích thước của cấu trúc địa chỉ.
	 *   - Ra: 0 nếu thành công, -1 nếu thất bại (ví dụ: port đã bị chiếm dụng).
	 */
	if (bind(server_sock, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
		perror("\nError: ");
		exit(0);
	}     
	
	/* =========================================================================
	 * Step 3: Giao tiếp và trao đổi dữ liệu với các Client
	 * =========================================================================
	 */
	printf("[SERVER] UDP Server is running and listening at port %d...\n", port);
	printf("[Server] Receives string from one client, forwards to the other\n");
	printf("         (with sender's IP and Port) and vice versa.\n");
	fflush(stdout);

	/* Khởi tạo danh sách 2 slots client */
	ClientSession clients[MAX_CLIENTS];
	for (int i = 0; i < MAX_CLIENTS; i++) {
		clients[i].active = 0;
		memset(&clients[i].address, 0, sizeof(clients[i].address));
	}

	while (1) {
		sin_size = sizeof(struct sockaddr_in);
    		
		/*
		 * Hàm recvfrom(): Chờ và nhận dữ liệu từ một client bất kỳ gửi tới.
		 *   - Vào:
		 *       + server_sock: Socket descriptor nhận dữ liệu.
		 *       + buff: Con trỏ vùng đệm lưu dữ liệu nhận được.
		 *       + BUFF_SIZE - 1: Kích thước tối đa có thể đọc (chừa 1 byte cho '\0').
		 *       + 0: Cờ điều khiển (flags), 0 là chế độ nhận dữ liệu mặc định.
		 *       + (struct sockaddr*)&client: Con trỏ lưu thông tin IP/port của client gửi tới.
		 *       + &sin_size: Con trỏ chứa kích thước ban đầu của struct client.
		 *   - Ra: 
		 *       + Số bytes nhận được thực tế nếu thành công.
		 *       + -1 nếu có lỗi xảy ra.
		 */
		bytes_received = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&client, &sin_size);
		
		if (bytes_received < 0) {
			if (errno == EINTR) {
				continue;
			}
			perror("\nError: ");
			continue;
		}

		buff[bytes_received] = '\0'; /* Thêm ký tự kết thúc chuỗi an toàn */
		trim_crlf(buff);

		char client_ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(client.sin_addr), client_ip, sizeof(client_ip));
		int client_port = ntohs(client.sin_port);

		/* Xác định client gửi thuộc slot nào */
		int sender_idx = -1;
		for (int i = 0; i < MAX_CLIENTS; i++) {
			if (clients[i].active && are_sockaddrs_equal(&clients[i].address, &client)) {
				sender_idx = i;
				break;
			}
		}

		/* Tìm slot trống nếu cần đăng ký */
		int free_idx = -1;
		for (int i = 0; i < MAX_CLIENTS; i++) {
			if (!clients[i].active) {
				free_idx = i;
				break;
			}
		}

		/* 1. Xử lý gói tin đăng ký ban đầu __CONNECT__ */
		if (strcmp(buff, CONNECT_TOKEN) == 0) {
			if (sender_idx == -1) {
				if (free_idx != -1) {
					clients[free_idx].address = client;
					clients[free_idx].active = 1;
					printf("[Server] Client %d connected from %s:%d\n", free_idx + 1, client_ip, client_port);
					fflush(stdout);
				} else {
					const char *full_msg = "Error: Server is full (maximum 2 clients reached).\n";
					sendto(server_sock, full_msg, strlen(full_msg), 0, (struct sockaddr *)&client, sizeof(client));
				}
			}
			continue;
		}

		/* 2. Xử lý tín hiệu thoát (@ hoặc #) */
		if (strcmp(buff, EXIT_TOKEN_AT) == 0 || strcmp(buff, EXIT_TOKEN_HASH) == 0) {
			if (sender_idx != -1) {
				clients[sender_idx].active = 0;
				printf("[Server] Client %d (%s:%d) disconnected (exit token '%s').\n",
				       sender_idx + 1, client_ip, client_port, buff);
				fflush(stdout);
			}
			continue;
		}

		/* 3. Xử lý trường hợp client chưa đăng ký gửi chuỗi */
		if (sender_idx == -1) {
			if (free_idx != -1) {
				clients[free_idx].address = client;
				clients[free_idx].active = 1;
				sender_idx = free_idx;
				printf("[Server] Client %d connected from %s:%d\n", sender_idx + 1, client_ip, client_port);
				fflush(stdout);
			} else {
				const char *full_msg = "Error: Server is full (maximum 2 clients reached).\n";
				sendto(server_sock, full_msg, strlen(full_msg), 0, (struct sockaddr *)&client, sizeof(client));
				continue;
			}
		}

		/* 4. Kiểm tra tính hợp lệ của chuỗi ký tự nhận được */
		if (!is_alphanumeric_str(buff)) {
			/*
			 * YÊU CẦU NGHIÊM NGẶT: Tuyệt đối KHÔNG hiển thị kết quả chuỗi lỗi lên màn hình server.
			 * Gửi thông báo lỗi ngược lại cho client gửi.
			 */
			const char *err_msg = "Error: String contains invalid characters! Only alphanumeric characters are allowed.\n";
			/*
			 * Hàm sendto(): Gửi phản hồi lỗi lại cho client gửi.
			 */
			bytes_sent = sendto(server_sock, err_msg, strlen(err_msg), 0, (struct sockaddr *)&client, sin_size);
			if (bytes_sent < 0) {
				perror("\nError: ");
			}
		} else {
			/* In ra địa chỉ IP (inet_ntoa), Port (ntohs) của client và nội dung nhận được */
			printf("[Client %d (%s:%d)]: %s\n", sender_idx + 1, client_ip, client_port, buff);
			fflush(stdout);

			/* Chuyển tiếp xâu cho Client còn lại kèm địa chỉ IP và Port của Client gửi */
			for (int i = 0; i < MAX_CLIENTS; i++) {
				if (i != sender_idx && clients[i].active) {
					char forward_buff[BUFF_SIZE + 64];
					snprintf(forward_buff, sizeof(forward_buff), "[%s:%d]: %s\n", client_ip, client_port, buff);

					/*
					 * Hàm sendto(): Chuyển tiếp dữ liệu sang client còn lại.
					 */
					bytes_sent = sendto(server_sock, forward_buff, strlen(forward_buff), 0,
					                    (struct sockaddr *)&(clients[i].address), sizeof(clients[i].address));
					if (bytes_sent < 0) {
						perror("\nError: ");
					}
				}
			}
		}
	}
	
	/* Đóng socket khi hoàn tất */
	close(server_sock);
	return 0;
}
