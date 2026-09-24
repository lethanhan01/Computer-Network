#include "cloudflare_filter.h"

#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <strings.h>

int queryDnsUdpFilter(const char* dnsServerIp, const char* domainName) {
    if (dnsServerIp == NULL || domainName == NULL || domainName[0] == '\0') {
        return 0;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        return 0;
    }

    // Timeout 2 giay de tranh treo khi rớt gói UDP
    struct timeval tv = {.tv_sec = DNS_QUERY_TIMEOUT_SECONDS, .tv_usec = 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in dest = {0};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(DNS_UDP_PORT);
    if (inet_pton(AF_INET, dnsServerIp, &dest.sin_addr) <= 0) {
        close(sock);
        return 0;
    }

    // Xay dung goi tin DNS query (RFC 1035)
    unsigned char buf[DNS_PACKET_MAX_SIZE] = {0};
    buf[0] = 0x12; buf[1] = 0x34; // Transaction ID
    buf[2] = 0x01; buf[3] = 0x00; // Flags: RD = 1 (Recursion Desired)
    buf[4] = 0x00; buf[5] = 0x01; // QDCOUNT = 1 (1 question)

    // Chuyen domain thanh chuoi nhan DNS (vi du: google.com -> 6google3com0)
    int idx = 12;
    const char* cur = domainName;
    while (*cur) {
        const char* dot = strchr(cur, '.');
        int len = dot ? (int)(dot - cur) : (int)strlen(cur);
        if (len <= 0 || len > 63 || idx + len + 1 >= 500) {
            close(sock);
            return 0;
        }
        buf[idx++] = (unsigned char)len;
        memcpy(&buf[idx], cur, (size_t)len);
        idx += len;
        if (!dot) {
            break;
        }
        cur = dot + 1;
    }
    buf[idx++] = 0x00; // Ket thuc QNAME

    // QTYPE = A (1), QCLASS = IN (1)
    buf[idx++] = 0x00; buf[idx++] = 0x01;
    buf[idx++] = 0x00; buf[idx++] = 0x01;

    // Gui query va nhan response
    sendto(sock, buf, (size_t)idx, 0, (struct sockaddr*)&dest, sizeof(dest));
    ssize_t recvd = recvfrom(sock, buf, sizeof(buf), 0, NULL, NULL);
    close(sock);

    if (recvd < 12) {
        return 0;
    }

    int ancount = (buf[6] << 8) | buf[7];
    if (ancount <= 0) {
        return 0;
    }

    // Phan tich Answer Section bat dau ngay sau Question Section (tai vi tri idx)
    int p = idx;
    while (p < recvd) {
        // Bo qua truong Name (pointer 2 bytes hoac chuoi nhan)
        if ((buf[p] & 0xC0) == 0xC0) {
            p += 2;
        } else {
            while (p < recvd && buf[p] != 0) {
                p += buf[p] + 1;
            }
            p++;
        }
        if (p + 10 > recvd) {
            break;
        }

        uint16_t type = (uint16_t)((buf[p] << 8) | buf[p + 1]);
        uint16_t rdlen = (uint16_t)((buf[p + 8] << 8) | buf[p + 9]);
        p += 10; // Type(2) + Class(2) + TTL(4) + RdLen(2)

        // Neu la A record va IP tra ve la sinkhole 0.0.0.0 thi da bi chan
        if (type == 1 && rdlen == 4 && p + 4 <= recvd) {
            if (buf[p] == 0 && buf[p + 1] == 0 && buf[p + 2] == 0 && buf[p + 3] == 0) {
                return 1;
            }
        }
        p += rdlen;
    }

    return 0;
}

int isBlockedByCloudflare(const char* domainName) {
    if (domainName == NULL || domainName[0] == '\0') {
        return 0;
    }

    if (queryDnsUdpFilter(CLOUDFLARE_PRIMARY_DNS_IP, domainName) ||
        queryDnsUdpFilter(CLOUDFLARE_SECONDARY_DNS_IP, domainName)) {
        return 1;
    }

    // Thu them ten mien goc neu co tien to www.
    if (strncasecmp(domainName, "www.", 4) == 0 && domainName[4] != '\0') {
        return queryDnsUdpFilter(CLOUDFLARE_PRIMARY_DNS_IP, domainName + 4) ||
               queryDnsUdpFilter(CLOUDFLARE_SECONDARY_DNS_IP, domainName + 4);
    }

    return 0;
}
