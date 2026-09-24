#include "csv_exporter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>

static int compareByFirstLetter(const void* a, const void* b) {
    const char* strA = (const char*)a;
    const char* strB = (const char*)b;

    // Bo qua khoang trang dau chuoi neu co
    while (*strA != '\0' && isspace((unsigned char)*strA)) {
        strA++;
    }
    while (*strB != '\0' && isspace((unsigned char)*strB)) {
        strB++;
    }

    if (*strA == '\0' && *strB == '\0') {
        return 0;
    }
    if (*strA == '\0') {
        return 1;
    }
    if (*strB == '\0') {
        return -1;
    }

    int diff = tolower((unsigned char)*strA) - tolower((unsigned char)*strB);
    if (diff != 0) {
        return diff;
    }

    return strcasecmp(strA, strB);
}

void sortItemsByFirstLetter(char items[][MAX_ITEM_STRING_LENGTH], int count) {
    if (items == NULL || count <= 1) {
        return;
    }

    qsort(items, (size_t)count, MAX_ITEM_STRING_LENGTH, compareByFirstLetter);
}

int exportListToCsv(const char* filePath, char items[][MAX_ITEM_STRING_LENGTH], int count) {
    if (filePath == NULL) {
        return 0;
    }

    FILE* fp = fopen(filePath, "w");
    if (fp == NULL) {
        return 0;
    }

    for (int i = 0; i < count; i++) {
        const char* text = items[i];
        if (text[0] == '\0') {
            continue;
        }

        // Kiem tra ky tu can escape trong dinh dang CSV RFC 4180
        int needsQuote = 0;
        if (strpbrk(text, ",\"\n\r") != NULL) {
            needsQuote = 1;
        }

        if (needsQuote) {
            fputc('"', fp);
            for (const char* p = text; *p != '\0'; p++) {
                if (*p == '"') {
                    fputs("\"\"", fp);
                } else {
                    fputc(*p, fp);
                }
            }
            fputs("\"\n", fp);
        } else {
            fprintf(fp, "%s\n", text);
        }
    }

    fclose(fp);
    return 1;
}

int exportCrawlDataToCsv(CrawlDataSet* dataSet) {
    if (dataSet == NULL) {
        return 0;
    }

    // 1. Sap xep tung danh sach theo chu cai dau A-Z
    sortItemsByFirstLetter(dataSet->links, dataSet->linkCount);
    sortItemsByFirstLetter(dataSet->texts, dataSet->textCount);
    sortItemsByFirstLetter(dataSet->videos, dataSet->videoCount);

    // 2. Xuat ra cac file CSV doc lap
    int success = 1;
    if (!exportListToCsv(DEFAULT_LINKS_CSV_FILENAME, dataSet->links, dataSet->linkCount)) {
        success = 0;
    }
    if (!exportListToCsv(DEFAULT_TEXTS_CSV_FILENAME, dataSet->texts, dataSet->textCount)) {
        success = 0;
    }
    if (!exportListToCsv(DEFAULT_VIDEOS_CSV_FILENAME, dataSet->videos, dataSet->videoCount)) {
        success = 0;
    }

    return success;
}
