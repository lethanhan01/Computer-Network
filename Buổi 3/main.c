#include "network_utils.h"
#include "resolver.h"
#include "cloudflare_filter.h"
#include "crawler.h"
#include "csv_exporter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REQUIRED_ARG_COUNT 2

int main(int argc, char* argv[]) {
    // 1. Kiem tra tham so dong lenh
    if (argc != REQUIRED_ARG_COUNT || argv[1] == NULL || argv[1][0] == '\0') {
        fprintf(stderr, "Cú pháp: %s <tên miền>\n", argv[0]);
        fprintf(stderr, "Ví dụ:   %s google.com\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* domainName = argv[1];

    // 2. Kiem tra ket noi Internet
    if (!checkInternetConnection()) {
        puts("No internet connection");
        return EXIT_FAILURE;
    }

    // 3. Phan giai ten mien sang dia chi IP
    int aliasCount = 0;
    char officialIp[IPV4_STRING_BUFFER_SIZE] = {0};
    char aliasIps[MAX_ALIASES][IPV4_STRING_BUFFER_SIZE] = {{0}};

    if (!resolveDomain(domainName, officialIp, aliasIps, MAX_ALIASES, &aliasCount)) {
        puts("Not found information");
        return EXIT_SUCCESS;
    }

    // 4. In ket qua IP chinh thuc va danh sach IP phu
    printDomainResult(officialIp, aliasIps, aliasCount);

    // 5. Kiem tra bo loc Cloudflare 1.1.1.3 voi ten mien nguoi lon
    if (isBlockedByCloudflare(domainName)) {
        puts("This site is not for you!");
        return EXIT_SUCCESS;
    }

    // 6. Thu thap du lieu tu dong (crawler) voi cac ten mien an toan
    CrawlDataSet* crawlData = (CrawlDataSet*)malloc(sizeof(CrawlDataSet));
    if (crawlData != NULL) {
        if (crawlDomain(domainName, crawlData)) {
            exportCrawlDataToCsv(crawlData);
            printf("[+] Đã thu thập và lưu: %d links -> links.csv, %d texts -> texts.csv, %d videos -> videos.csv\n",
                   crawlData->linkCount, crawlData->textCount, crawlData->videoCount);
        }
        free(crawlData);
    }

    return EXIT_SUCCESS;
}
