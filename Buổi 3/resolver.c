#include "resolver.h"

#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>

int resolveDomain(const char* domainName, char* officialIp,
                  char (*aliasIps)[IPV4_STRING_BUFFER_SIZE],
                  int maxAliases, int* aliasCount) {
    if (domainName == NULL || officialIp == NULL || aliasIps == NULL || aliasCount == NULL) {
        return 0;
    }

    *aliasCount = 0;
    officialIp[0] = '\0';

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* resList = NULL;
    if (getaddrinfo(domainName, NULL, &hints, &resList) != 0 || resList == NULL) {
        return 0;
    }

    for (struct addrinfo* cur = resList; cur != NULL; cur = cur->ai_next) {
        if (cur->ai_family != AF_INET || cur->ai_addr == NULL) {
            continue;
        }

        struct sockaddr_in* ipv4 = (struct sockaddr_in*)cur->ai_addr;
        char ipStr[IPV4_STRING_BUFFER_SIZE] = {0};
        if (inet_ntop(AF_INET, &ipv4->sin_addr, ipStr, sizeof(ipStr)) == NULL) {
            continue;
        }

        // Kiem tra trung lap voi Official IP
        if (officialIp[0] != '\0' && strcmp(officialIp, ipStr) == 0) {
            continue;
        }

        // Kiem tra trung lap voi cac Alias IP da luu
        int isDup = 0;
        for (int i = 0; i < *aliasCount; i++) {
            if (strcmp(aliasIps[i], ipStr) == 0) {
                isDup = 1;
                break;
            }
        }
        if (isDup) {
            continue;
        }

        // Luu Official IP neu chua co, nguoc lai luu vao Alias IP
        if (officialIp[0] == '\0') {
            strncpy(officialIp, ipStr, IPV4_STRING_BUFFER_SIZE - 1);
            officialIp[IPV4_STRING_BUFFER_SIZE - 1] = '\0';
        } else if (*aliasCount < maxAliases) {
            strncpy(aliasIps[*aliasCount], ipStr, IPV4_STRING_BUFFER_SIZE - 1);
            aliasIps[*aliasCount][IPV4_STRING_BUFFER_SIZE - 1] = '\0';
            (*aliasCount)++;
        }
    }

    freeaddrinfo(resList);
    return (officialIp[0] != '\0') ? 1 : 0;
}

void printDomainResult(const char* officialIp,
                       char (*aliasIps)[IPV4_STRING_BUFFER_SIZE],
                       int aliasCount) {
    if (officialIp == NULL) {
        return;
    }

    printf("Official IP: %s\n", officialIp);
    if (aliasCount > 0 && aliasIps != NULL) {
        puts("Alias IP:");
        for (int i = 0; i < aliasCount; i++) {
            puts(aliasIps[i]);
        }
    }
}
