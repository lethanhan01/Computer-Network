#include "network_utils.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>

int testTcpConnect(const char* serverIp, int serverPort, int timeoutSeconds) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return 0;
    }

    // Chuyen socket sang non-blocking de kiem soat timeout bang select
    fcntl(sock, F_SETFL, O_NONBLOCK);

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)serverPort);
    if (inet_pton(AF_INET, serverIp, &addr.sin_addr) <= 0) {
        close(sock);
        return 0;
    }

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0 && errno != EINPROGRESS) {
        close(sock);
        return 0;
    }

    fd_set writeFds;
    FD_ZERO(&writeFds);
    FD_SET(sock, &writeFds);

    struct timeval tv = {.tv_sec = timeoutSeconds, .tv_usec = 0};
    int res = select(sock + 1, NULL, &writeFds, NULL, &tv);

    int err = 0;
    socklen_t len = sizeof(err);
    if (res > 0) {
        getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len);
    }
    close(sock);

    return (res > 0 && err == 0) ? 1 : 0;
}

int checkInternetConnection(void) {
    return testTcpConnect(PRIMARY_DNS_SERVER_IP, DNS_TCP_PORT, CONNECT_TIMEOUT_SECONDS) ||
           testTcpConnect(SECONDARY_DNS_SERVER_IP, DNS_TCP_PORT, CONNECT_TIMEOUT_SECONDS);
}
