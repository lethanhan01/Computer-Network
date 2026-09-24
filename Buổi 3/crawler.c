#include "crawler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <strings.h>

/**
 * @brief Tải mã nguồn HTML của trang web qua cURL (HTTPS/HTTP).
 * Chống Command Injection bằng cách kiểm tra ký tự điều khiển trong URL.
 */
int fetchHtmlContent(const char* url, char* htmlBuffer, size_t bufferSize) {
    if (url == NULL || url[0] == '\0' || htmlBuffer == NULL || bufferSize < 1024 ||
        strpbrk(url, " \"'`$;|&\n\r\t") != NULL) {
        return 0;
    }

    char command[2048];
    snprintf(command, sizeof(command),
             "curl -sL -k --max-time %d -A \"%s\" \"%s\"",
             HTTP_TIMEOUT_SECONDS, HTTP_USER_AGENT, url);

    FILE* pipe = popen(command, "r");
    if (pipe == NULL) {
        return 0;
    }

    size_t totalBytes = 0;
    while (totalBytes < bufferSize - 1) {
        size_t bytesRead = fread(htmlBuffer + totalBytes, 1, bufferSize - 1 - totalBytes, pipe);
        if (bytesRead == 0) {
            break;
        }
        totalBytes += bytesRead;
    }
    htmlBuffer[totalBytes] = '\0';
    pclose(pipe);

    return (int)totalBytes;
}

/**
 * @brief Thêm phần tử chuỗi vào danh sách, khử trùng lặp và kiểm tra giới hạn.
 */
static void addItem(char list[][MAX_ITEM_STRING_LENGTH], int* count, const char* value) {
    if (list == NULL || count == NULL || value == NULL || value[0] == '\0' || *count >= MAX_CRAWL_ITEMS) {
        return;
    }

    for (int i = 0; i < *count; i++) {
        if (strcmp(list[i], value) == 0) {
            return;
        }
    }

    strncpy(list[*count], value, MAX_ITEM_STRING_LENGTH - 1);
    list[*count][MAX_ITEM_STRING_LENGTH - 1] = '\0';
    (*count)++;
}

/**
 * @brief Nhận diện đường dẫn video qua định dạng file hoặc pattern của các nền tảng video.
 */
static int isVideoUrl(const char* url) {
    if (url == NULL || url[0] == '\0') {
        return 0;
    }

    // Phần mở rộng tệp video phổ biến
    static const char* const EXTENSIONS[] = {
        ".mp4", ".webm", ".ogg", ".m3u8", ".mov", ".avi", ".mkv", ".flv", ".mpd"
    };
    for (size_t i = 0; i < sizeof(EXTENSIONS) / sizeof(EXTENSIONS[0]); i++) {
        const char* p = strcasestr(url, EXTENSIONS[i]);
        if (p != NULL) {
            size_t extLen = strlen(EXTENSIONS[i]);
            char nextChar = p[extLen];
            if (nextChar == '\0' || nextChar == '?' || nextChar == '#' || nextChar == '&' || nextChar == '/') {
                return 1;
            }
        }
    }

    // Pattern nhận diện video của YouTube và các dịch vụ chia sẻ video
    static const char* const VIDEO_PATTERNS[] = {
        "watch?v=", "/watch/", "/shorts/", "youtu.be/", "/embed/",
        "/video/", "/videos/", "vimeo.com/", "dailymotion.com/video/"
    };
    for (size_t i = 0; i < sizeof(VIDEO_PATTERNS) / sizeof(VIDEO_PATTERNS[0]); i++) {
        if (strcasestr(url, VIDEO_PATTERNS[i]) != NULL) {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Thêm liên kết web hợp lệ vào tập dữ liệu (bỏ qua liên kết neo và giao thức phi web).
 */
static void addLink(CrawlDataSet* dataSet, const char* link) {
    if (dataSet == NULL || link == NULL || link[0] == '\0') {
        return;
    }
    if (link[0] == '#' || strncmp(link, "javascript:", 11) == 0 ||
        strncmp(link, "mailto:", 7) == 0 || strncmp(link, "tel:", 4) == 0 ||
        strstr(link, "android-app://") != NULL || strstr(link, "ios-app://") != NULL) {
        return;
    }
    addItem(dataSet->links, &dataSet->linkCount, link);
}

/**
 * @brief Thêm đường dẫn video vào tập dữ liệu.
 */
static void addVideo(CrawlDataSet* dataSet, const char* video) {
    if (dataSet != NULL && video != NULL && video[0] != '\0') {
        addItem(dataSet->videos, &dataSet->videoCount, video);
    }
}

/**
 * @brief Làm sạch văn bản HTML: giải mã entity và thu gọn khoảng trắng liên tiếp.
 */
static void cleanHtmlText(char* str) {
    if (str == NULL || str[0] == '\0') {
        return;
    }

    // Giải mã các thực thể HTML thường gặp
    static const struct { const char* entity; const char* rep; } ENTITIES[] = {
        {"&amp;", "&"}, {"&quot;", "\""}, {"&apos;", "'"}, {"&#39;", "'"},
        {"&lt;", "<"}, {"&gt;", ">"}, {"&nbsp;", " "}, {"&copy;", "(c)"}
    };
    for (size_t i = 0; i < sizeof(ENTITIES) / sizeof(ENTITIES[0]); i++) {
        char* pos = strstr(str, ENTITIES[i].entity);
        size_t entLen = strlen(ENTITIES[i].entity);
        size_t repLen = strlen(ENTITIES[i].rep);
        while (pos != NULL) {
            memmove(pos + repLen, pos + entLen, strlen(pos + entLen) + 1);
            memcpy(pos, ENTITIES[i].rep, repLen);
            pos = strstr(pos + repLen, ENTITIES[i].entity);
        }
    }

    // Thu gọn khoảng trắng dư thừa
    char* src = str;
    char* dst = str;
    int inSpace = 1;
    while (*src != '\0') {
        unsigned char c = (unsigned char)*src++;
        if (isspace(c) || c < 32) {
            if (!inSpace) {
                *dst++ = ' ';
                inSpace = 1;
            }
        } else {
            *dst++ = (char)c;
            inSpace = 0;
        }
    }
    if (dst > str && *(dst - 1) == ' ') {
        dst--;
    }
    *dst = '\0';
}

/**
 * @brief Thêm đoạn văn bản vào tập dữ liệu (loại bỏ chuỗi rác hoặc mã nguồn rò rỉ).
 */
static void addText(CrawlDataSet* dataSet, const char* text) {
    if (dataSet == NULL || text == NULL) {
        return;
    }

    size_t len = strlen(text);
    if (len < 2 || len > 256) {
        return;
    }

    // Bỏ qua mã nguồn script/JSON rò rỉ
    if (strchr(text, '{') != NULL || strchr(text, '}') != NULL ||
        strstr(text, "\":") != NULL || text[0] == '"' || text[0] == '[' || text[0] == ']') {
        return;
    }

    // Yêu cầu chứa ít nhất 1 chữ cái hoặc ký tự UTF-8
    for (size_t i = 0; i < len; i++) {
        if (isalpha((unsigned char)text[i]) || (unsigned char)text[i] >= 128) {
            addItem(dataSet->texts, &dataSet->textCount, text);
            return;
        }
    }
}

/**
 * @brief Chuẩn hóa URL tương đối dựa trên tên miền đầu vào (ví dụ: youtube.com).
 */
static void normalizeUrl(const char* domainName, const char* rawUrl, char* normalizedUrl, size_t maxLen) {
    if (rawUrl == NULL || normalizedUrl == NULL || maxLen == 0) {
        return;
    }

    if (strncmp(rawUrl, "http://", 7) == 0 || strncmp(rawUrl, "https://", 8) == 0) {
        snprintf(normalizedUrl, maxLen, "%s", rawUrl);
    } else if (strncmp(rawUrl, "//", 2) == 0) {
        snprintf(normalizedUrl, maxLen, "https:%s", rawUrl);
    } else if (rawUrl[0] == '/') {
        snprintf(normalizedUrl, maxLen, "https://%s%s", domainName, rawUrl);
    } else {
        snprintf(normalizedUrl, maxLen, "https://%s/%s", domainName, rawUrl);
    }
}

/**
 * @brief Trích xuất giá trị thuộc tính HTML ngắn gọn (hỗ trợ ngoặc kép, ngoặc đơn và unquoted).
 */
static int extractAttribute(const char* tagStart, const char* tagEnd, const char* attrName, char* outVal, size_t outSize) {
    if (tagStart == NULL || tagEnd == NULL || attrName == NULL || outVal == NULL || outSize == 0) {
        return 0;
    }

    size_t nameLen = strlen(attrName);
    const char* cur = tagStart;

    while (cur < tagEnd) {
        const char* match = strcasestr(cur, attrName);
        if (match == NULL || match >= tagEnd) {
            return 0;
        }

        // Ký tự trước attrName phải là khoảng trắng hoặc '<'
        if (match > tagStart && !isspace((unsigned char)*(match - 1)) && *(match - 1) != '<') {
            cur = match + 1;
            continue;
        }

        const char* p = match + nameLen;
        while (p < tagEnd && isspace((unsigned char)*p)) p++;

        if (p >= tagEnd || *p != '=') {
            cur = match + 1;
            continue;
        }

        p++; // Bỏ qua dấu '='
        while (p < tagEnd && isspace((unsigned char)*p)) p++;
        if (p >= tagEnd) return 0;

        char quote = *p;
        const char* valStart = p;
        const char* valEnd = NULL;

        if (quote == '"' || quote == '\'') {
            valStart = p + 1;
            valEnd = memchr(valStart, quote, (size_t)(tagEnd - valStart));
            if (valEnd == NULL) return 0;
        } else {
            valEnd = valStart;
            while (valEnd < tagEnd && !isspace((unsigned char)*valEnd) && *valEnd != '>') {
                valEnd++;
            }
        }

        size_t len = (size_t)(valEnd - valStart);
        if (len == 0 || len >= outSize) return 0;

        strncpy(outVal, valStart, len);
        outVal[len] = '\0';
        return 1;
    }

    return 0;
}

/**
 * @brief Quét tìm các pattern video YouTube (/watch?v=..., /shorts/...) xuất hiện trong trang.
 * Giúp thu thập đầy đủ video trên YouTube ngay cả khi nội dung nằm trong cấu trúc JSON/JavaScript.
 */
static void scanVideoPatterns(const char* domainName, const char* html, CrawlDataSet* dataSet) {
    if (html == NULL || dataSet == NULL) {
        return;
    }

    const char* p = html;
    while (*p != '\0') {
        const char* matchWatch = strstr(p, "/watch?v=");
        const char* matchShorts = strstr(p, "/shorts/");
        const char* match = NULL;

        if (matchWatch != NULL && matchShorts != NULL) {
            match = (matchWatch < matchShorts) ? matchWatch : matchShorts;
        } else if (matchWatch != NULL) {
            match = matchWatch;
        } else {
            match = matchShorts;
        }

        if (match == NULL) {
            break;
        }

        // Đọc ID video (thường gồm 11 ký tự alnum, '-', '_')
        const char* valStart = match;
        const char* valEnd = valStart;
        while (*valEnd != '\0' && *valEnd != '"' && *valEnd != '\'' && *valEnd != '\\' &&
               *valEnd != '&' && *valEnd != ' ' && *valEnd != '<' && (valEnd - valStart) < 64) {
            valEnd++;
        }

        size_t len = (size_t)(valEnd - valStart);
        if (len > 9) {
            char rawPath[128];
            char fullUrl[MAX_ITEM_STRING_LENGTH];
            strncpy(rawPath, valStart, len);
            rawPath[len] = '\0';

            normalizeUrl(domainName, rawPath, fullUrl, sizeof(fullUrl));
            addVideo(dataSet, fullUrl);
            addLink(dataSet, fullUrl);
        }

        p = valEnd;
    }
}

/**
 * @brief Bóc tách dữ liệu links, texts, videos từ chuỗi HTML chuẩn và loại bỏ script/style.
 */
void parseHtmlData(const char* domainName, const char* html, CrawlDataSet* dataSet) {
    if (domainName == NULL || html == NULL || dataSet == NULL) {
        return;
    }

    const char* p = html;
    while (*p != '\0') {
        // 1. Bỏ qua các khối script, style và chú thích HTML để tránh text rác
        if (strncasecmp(p, "<script", 7) == 0) {
            const char* end = strcasestr(p, "</script>");
            if (end == NULL) break;
            p = end + 9;
            continue;
        }
        if (strncasecmp(p, "<style", 6) == 0) {
            const char* end = strcasestr(p, "</style>");
            if (end == NULL) break;
            p = end + 8;
            continue;
        }
        if (strncmp(p, "<!--", 4) == 0) {
            const char* end = strstr(p, "-->");
            if (end == NULL) break;
            p = end + 3;
            continue;
        }

        // 2. Xử lý các thẻ mở hoặc đóng HTML
        if (*p == '<') {
            const char* tagEnd = strchr(p, '>');
            if (tagEnd == NULL) break;

            char rawVal[MAX_ITEM_STRING_LENGTH];
            char normUrl[MAX_ITEM_STRING_LENGTH];

            // 2.1. Thẻ liên kết href (<a>, <link>)
            if (extractAttribute(p, tagEnd, "href", rawVal, sizeof(rawVal))) {
                normalizeUrl(domainName, rawVal, normUrl, sizeof(normUrl));
                addLink(dataSet, normUrl);
                if (isVideoUrl(normUrl)) {
                    addVideo(dataSet, normUrl);
                }
            }

            // 2.2. Thẻ nguồn nhúng src hoặc data-src (<video>, <source>, <iframe>, <embed>)
            if (extractAttribute(p, tagEnd, "src", rawVal, sizeof(rawVal)) ||
                extractAttribute(p, tagEnd, "data-src", rawVal, sizeof(rawVal))) {
                normalizeUrl(domainName, rawVal, normUrl, sizeof(normUrl));
                addLink(dataSet, normUrl);
                if (isVideoUrl(normUrl) || strncasecmp(p, "<video", 6) == 0 || strncasecmp(p, "<source", 7) == 0) {
                    addVideo(dataSet, normUrl);
                }
            }

            // 2.3. Thẻ siêu dữ liệu meta (og:video, twitter:player)
            if (strncasecmp(p, "<meta", 5) == 0) {
                char propVal[64] = {0};
                if ((extractAttribute(p, tagEnd, "property", propVal, sizeof(propVal)) && strstr(propVal, "video") != NULL) ||
                    (extractAttribute(p, tagEnd, "name", propVal, sizeof(propVal)) && strstr(propVal, "player") != NULL)) {
                    if (extractAttribute(p, tagEnd, "content", rawVal, sizeof(rawVal))) {
                        normalizeUrl(domainName, rawVal, normUrl, sizeof(normUrl));
                        addVideo(dataSet, normUrl);
                        addLink(dataSet, normUrl);
                    }
                }
            }

            p = tagEnd + 1;
            continue;
        }

        // 3. Bóc tách nội dung văn bản (text) giữa các thẻ HTML
        const char* nextTag = strchr(p, '<');
        size_t textLen = nextTag ? (size_t)(nextTag - p) : strlen(p);
        if (textLen > 0) {
            char rawText[MAX_ITEM_STRING_LENGTH] = {0};
            size_t copyLen = (textLen < sizeof(rawText)) ? textLen : sizeof(rawText) - 1;
            strncpy(rawText, p, copyLen);
            rawText[copyLen] = '\0';
            cleanHtmlText(rawText);
            if (rawText[0] != '\0') {
                addText(dataSet, rawText);
            }
        }

        p = nextTag ? nextTag : (p + textLen);
    }
}

/**
 * @brief Điều phối toàn bộ quy trình thu thập dữ liệu tự động cho một tên miền an toàn.
 */
int crawlDomain(const char* domainName, CrawlDataSet* dataSet) {
    if (domainName == NULL || domainName[0] == '\0' || dataSet == NULL) {
        return 0;
    }

    dataSet->linkCount = dataSet->textCount = dataSet->videoCount = 0;

    char* htmlBuffer = (char*)malloc(MAX_HTML_BUFFER_SIZE);
    if (htmlBuffer == NULL) {
        return 0;
    }

    // Tải trang chủ theo giao thức HTTPS chuẩn
    char targetUrl[512];
    snprintf(targetUrl, sizeof(targetUrl), "https://%s", domainName);
    int bytesRead = fetchHtmlContent(targetUrl, htmlBuffer, MAX_HTML_BUFFER_SIZE);

    if (bytesRead > 0) {
        parseHtmlData(domainName, htmlBuffer, dataSet);
        scanVideoPatterns(domainName, htmlBuffer, dataSet);
    }

    // Đối với YouTube: trang chủ là SPA không chứa video tĩnh cho khách chưa đăng nhập,
    // tự động lấy thêm danh sách video xu hướng thực tế của YouTube nếu videoCount == 0
    if (strstr(domainName, "youtube") != NULL && dataSet->videoCount == 0) {
        snprintf(targetUrl, sizeof(targetUrl), "https://www.youtube.com/results?search_query=trending");
        int extraBytes = fetchHtmlContent(targetUrl, htmlBuffer, MAX_HTML_BUFFER_SIZE);
        if (extraBytes > 0) {
            scanVideoPatterns(domainName, htmlBuffer, dataSet);
        }
    }

    free(htmlBuffer);
    return (bytesRead > 0 || dataSet->linkCount > 0) ? 1 : 0;
}
