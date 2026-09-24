#ifndef CSV_EXPORTER_H
#define CSV_EXPORTER_H

#include "crawler.h"

#define DEFAULT_LINKS_CSV_FILENAME "links.csv"
#define DEFAULT_TEXTS_CSV_FILENAME "texts.csv"
#define DEFAULT_VIDEOS_CSV_FILENAME "videos.csv"

/**
 * @brief Sắp xếp mảng chuỗi theo thứ tự xuất hiện của các chữ cái đầu (Case-insensitive A-Z).
 *
 * Sử dụng hàm qsort của thư viện chuẩn C với hàm so sánh ký tự đầu tiên
 * không phân biệt chữ hoa, chữ thường. Nếu ký tự đầu trùng nhau, sắp xếp theo từ điển.
 *
 * @param[in,out] items Mảng 2 chiều chứa danh sách các chuỗi cần sắp xếp.
 * @param[in] count Số lượng phần tử trong mảng items.
 */
void sortItemsByFirstLetter(char items[][MAX_ITEM_STRING_LENGTH], int count);

/**
 * @brief Ghi danh sách các mục dữ liệu ra tệp tin CSV theo chuẩn RFC 4180.
 *
 * Tự động bao bọc trong dấu ngoặc kép và escape ký tự nếu mục dữ liệu
 * chứa dấu phẩy, dấu nháy kép hoặc ký tự ngắt dòng.
 *
 * @param[in] filePath Đường dẫn tệp tin CSV đích (ví dụ: links.csv).
 * @param[in] items Mảng các chuỗi cần ghi.
 * @param[in] count Số lượng chuỗi cần ghi.
 *
 * @return 1 nếu ghi thành công, 0 nếu mở file hoặc ghi thất bại.
 */
int exportListToCsv(const char* filePath, char items[][MAX_ITEM_STRING_LENGTH], int count);

/**
 * @brief Điều phối sắp xếp và xuất toàn bộ CrawlDataSet ra 3 file links.csv, texts.csv, videos.csv.
 *
 * @param[in,out] dataSet Con trỏ tới cấu trúc CrawlDataSet chứa dữ liệu cào được.
 *
 * @return 1 nếu xuất thành công cả 3 tệp tin, 0 nếu có lỗi xảy ra.
 */
int exportCrawlDataToCsv(CrawlDataSet* dataSet);

#endif // CSV_EXPORTER_H
