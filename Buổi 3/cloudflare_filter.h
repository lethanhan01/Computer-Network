#ifndef CLOUDFLARE_FILTER_H
#define CLOUDFLARE_FILTER_H

/**
 * @file cloudflare_filter.h
 * @brief Khai bao module loc ten mien doc hai va noi dung nguoi lon qua Cloudflare 1.1.1.3.
 */

#define CLOUDFLARE_PRIMARY_DNS_IP "1.1.1.3"
#define CLOUDFLARE_SECONDARY_DNS_IP "1.0.0.3"
#define DNS_UDP_PORT 53
#define DNS_QUERY_TIMEOUT_SECONDS 2
#define DNS_PACKET_MAX_SIZE 512

/**
 * @brief Gui truy van DNS UDP toi mot may chu DNS cu the de phat hien phan hoi bi chan.
 *
 * Ham tao goi tin DNS truy van ban ghi loai A cho ten mien duoc chi dinh,
 * gui qua UDP toi cong 53 cua may chu Cloudflare for Families. Neu may chu phan hoi
 * dia chi IPv4 la 0.0.0.0 (hoac IPv6 ::), ten mien duoc xac dinh la bi chan.
 *
 * @param[in] dnsServerIp Dia chi IP may chu DNS can truy van (1.1.1.3 hoac 1.0.0.3).
 * @param[in] domainName Ten mien can kiem tra.
 *
 * @return Ket qua kiem tra (int):
 * - `1`: Ten mien bi chan boi Cloudflare (la trang web doc hai/nguoi lon).
 * - `0`: Ten mien an toan hoac khong nhan duoc phan hoi chan.
 */
int queryDnsUdpFilter(const char* dnsServerIp, const char* domainName);

/**
 * @brief Kiem tra xem ten mien co bi chan boi bo loc Cloudflare 1.1.1.3 hay khong.
 *
 * Ham lan luot truy van may chu primary (1.1.1.3) va secondary (1.0.0.3)
 * de dam bao tinh san sang truong hop mot server bi mat ket noi.
 *
 * @param[in] domainName Ten mien can kiem tra.
 *
 * @return Ket qua kiem tra (int):
 * - `1`: Ten mien bi chan (can in thong bao 'This site is not for you!').
 * - `0`: Ten mien binh thuong / an toan.
 */
int isBlockedByCloudflare(const char* domainName);

#endif /* CLOUDFLARE_FILTER_H */
