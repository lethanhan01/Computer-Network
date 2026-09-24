#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

/**
 * @file network_utils.h
 * @brief Khai bao cac tien ich kiem tra ket noi mang va may chu.
 */

#define PRIMARY_DNS_SERVER_IP "8.8.8.8"
#define SECONDARY_DNS_SERVER_IP "1.1.1.1"
#define DNS_TCP_PORT 53
#define CONNECT_TIMEOUT_SECONDS 2

/**
 * @brief Thu nghiem ket noi TCP toi mot dia chi IP va cong xac dinh kem timeout.
 *
 * Ham khoi tao mot socket TCP o che do non-blocking, gui yeu cau ket noi
 * toi server, su dung select() de cho doi ket qua ket noi voi thoi gian timeout.
 * Sau khi kiem tra xong, socket luon duoc dong de giai phong tai nguyen.
 *
 * @param[in] serverIp Dia chi IPv4 cua may chu can kiem tra.
 * @param[in] serverPort Cong dich vu can kiem tra (vi du: 53 cho DNS).
 * @param[in] timeoutSeconds Thoi gian cho toi da (tinh bang giay).
 *
 * @return Trang thai ket noi (int):
 * - `1`: Ket noi thanh cong toi may chu.
 * - `0`: Ket noi that bai hoac timeout.
 */
int testTcpConnect(const char* serverIp, int serverPort, int timeoutSeconds);

/**
 * @brief Kiem tra ket noi Internet tong the cua he thong.
 *
 * Ham kiem tra ket noi lan luot toi 2 DNS server uy tin (8.8.8.8 va 1.1.1.1).
 * Neu it nhat mot trong hai server phan hoi, ket luan co ket noi Internet.
 *
 * @return Trang thai mang (int):
 * - `1`: Co ket noi Internet hoat dong.
 * - `0`: Khong co ket noi Internet.
 */
int checkInternetConnection(void);

#endif /* NETWORK_UTILS_H */
