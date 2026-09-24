# HƯỚNG DẪN TỰ CHẤM ĐIỂM BÀI TẬP HW2 (IT4062 - LẬP TRÌNH MẠNG)

Gói công cụ này giúp sinh viên tự kiểm thử bài tập HW2 (chương trình `resolver`) trước khi nộp bài.

---

## 1. Thành phần trong gói tự chấm
- `grade_hw2.sh`: Script bash tự động kiểm thử và chấm điểm theo thang điểm 10.
- `resolver_sample`: File thực thi mẫu chuẩn của đề bài để sinh viên đối chiếu output.
- `HUONG_DAN_TU_CHAM.md`: Tài liệu hướng dẫn sử dụng.

---

## 2. Cách sử dụng

### Cách 1: Tự động chấm bài làm trong thư mục hiện tại
Copy file `grade_hw2.sh` vào thư mục chứa bài làm (chứa file `resolver` hoặc file `.zip` của bạn) và chạy:
```bash
chmod +x grade_hw2.sh
./grade_hw2.sh
```
Script sẽ tự động tìm kiếm file nén `HotenSV_MSSV_HW2.zip` hoặc file thực thi `./resolver` để chấm điểm.

---

### Cách 2: Chấm trực tiếp file thực thi `./resolver`
Sau khi bạn đã gõ `make` để biên dịch ra file `./resolver`, chạy lệnh:
```bash
./grade_hw2.sh ./resolver
```

---

### Cách 3: Chấm file zip nộp bài của bạn (Khuyên dùng trước khi nộp)
Đóng gói bài làm theo quy định: `HotenSV_MSSV_HW2.zip` (Ví dụ: `NguyenVanA_20261234_HW2.zip`).
Chạy lệnh:
```bash
./grade_hw2.sh NguyenVanA_20261234_HW2.zip
```
Script sẽ:
1. Kiểm tra định dạng tên file zip.
2. Giải nén vào thư mục tạm.
3. Chạy `make` để kiểm tra khả năng biên dịch độc lập.
4. Chạy toàn bộ 6 test case chức năng và xuất bảng điểm chi tiết.

---

### Cách 4: Chạy thử file thực thi mẫu (resolver_sample)
Để kiểm tra xem chương trình chuẩn chạy và hiển thị như thế nào:
```bash
# Xem kết quả tự chấm của file mẫu
./grade_hw2.sh --demo

# Hoặc chạy thử trực tiếp từng lệnh:
./resolver_sample google.com
./resolver_sample pornhub.com
./resolver_sample aznsc.test.com
```

---

## 3. Thang điểm đánh giá (Tổng 10.0 điểm)

| Thành phần | Điểm | Tiêu chí đánh giá |
| :--- | :---: | :--- |
| **Makefile & Cấu trúc** | **2.0đ** | Định dạng tên zip chuẩn, Makefile biên dịch ra đúng file `resolver` không có lỗi. |
| **Test 1: google.com** | **2.0đ** | Phân giải đúng Official IP, danh sách Alias IP, không bị chặn nhầm. |
| **Test 2: cloudflare.com** | **1.5đ** | Phân giải chính xác tên miền an toàn khác. |
| **Test 3: aznsc.test.com** | **1.5đ** | Tên miền không tồn tại, in chính xác thông báo: `Not found information`. |
| **Test 4: pornhub.com** | **1.5đ** | Nhận diện trang người lớn qua Cloudflare 1.1.1.3, in thông báo: `This site is not for you!`. |
| **Test 5: xvideos.com** | **1.5đ** | Kiểm tra chéo phát hiện trang người lớn khác, in: `This site is not for you!`. |
| **Test 6: Validation** | *(Đạt)* | Xử lý thoát với mã lỗi khác 0 khi người dùng không truyền tham số. |

> **Lưu ý**: Đạt từ **9.0 / 10.0** trở lên là bài làm đạt chất lượng xuất sắc, sẵn sàng nộp bài theo quy định môn học.
