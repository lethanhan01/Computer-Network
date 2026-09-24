#ifndef CRAWLER_H
#define CRAWLER_H

#include <stddef.h>

#define MAX_CRAWL_ITEMS 500
#define MAX_ITEM_STRING_LENGTH 1024
#define MAX_HTML_BUFFER_SIZE (1024 * 1024)
#define HTTP_TIMEOUT_SECONDS 10
#define HTTP_USER_AGENT "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"

/**
 * @brief Cấu trúc dữ liệu lưu trữ các mục thông tin thu thập được từ trang web.
 *
 * Chứa danh sách các liên kết (links), các đoạn văn bản (texts) và
 * các đường dẫn video (videos), kèm số lượng phần tử tương ứng.
 */
typedef struct {
    char links[MAX_CRAWL_ITEMS][MAX_ITEM_STRING_LENGTH];
    int linkCount;
    char texts[MAX_CRAWL_ITEMS][MAX_ITEM_STRING_LENGTH];
    int textCount;
    char videos[MAX_CRAWL_ITEMS][MAX_ITEM_STRING_LENGTH];
    int videoCount;
} CrawlDataSet;

/**
 * @brief Tải mã nguồn HTML của trang web từ URL chỉ định qua giao thức HTTPS.
 *
 * @param[in] url Đường dẫn URL trang web cần tải (ví dụ: https://youtube.com).
 * @param[out] htmlBuffer Vùng đệm nhận chuỗi HTML tải về.
 * @param[in] bufferSize Kích thước tối đa của vùng đệm htmlBuffer.
 *
 * @return Số byte dữ liệu HTML đã tải về thành công, hoặc 0 nếu thất bại.
 */
int fetchHtmlContent(const char* url, char* htmlBuffer, size_t bufferSize);

/**
 * @brief Bóc tách dữ liệu links, texts, videos từ chuỗi HTML.
 *
 * @param[in] domainName Tên miền (ví dụ: youtube.com) dùng để chuẩn hóa các liên kết tương đối.
 * @param[in] html Chuỗi mã nguồn HTML của trang web.
 * @param[out] dataSet Con trỏ tới cấu trúc CrawlDataSet nhận dữ liệu bóc tách được.
 */
void parseHtmlData(const char* domainName, const char* html, CrawlDataSet* dataSet);

/**
 * @brief Điều phối toàn bộ quy trình thu thập dữ liệu tự động cho một tên miền an toàn.
 *
 * @param[in] domainName Tên miền cần thu thập (ví dụ: youtube.com).
 * @param[out] dataSet Con trỏ tới cấu trúc CrawlDataSet để lưu toàn bộ dữ liệu cào được.
 *
 * @return 1 nếu thu thập thành công, 0 nếu không thể tải hoặc phân tích nội dung.
 */
int crawlDomain(const char* domainName, CrawlDataSet* dataSet);

#endif // CRAWLER_H
