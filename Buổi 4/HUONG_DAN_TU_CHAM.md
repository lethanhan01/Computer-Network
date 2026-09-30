# HƯỚNG DẪN TỰ CHẤM ĐIỂM BÀI TẬP HW4 (IT4062 - LẬP TRÌNH MẠNG)

Gói công cụ này giúp sinh viên tự kiểm thử bài tập HW4 (ứng dụng mạng UDP Socket Chat 2 Clients) trước khi nộp bài.

---

## 1. Thành phần trong gói tự chấm
- `grade_hw4.sh`: Script bash tự động kiểm thử và chấm điểm theo thang điểm 10.0.
- `test_hw4.py`: Bộ kiểm thử tự động toàn diện bằng Python (sử dụng sockets và IPC subprocess).
- `server_sample` & `client_sample`: File thực thi mẫu chuẩn của đề bài để đối chiếu.
- `HUONG_DAN_TU_CHAM.md`: Tài liệu hướng dẫn sử dụng công cụ chấm.
- `README.md`: Tài liệu mô tả kỹ thuật và kiến trúc hệ thống.

---

## 2. Cách sử dụng

### Cách 1: Tự động chấm bài làm trong thư mục hiện tại
```bash
chmod +x grade_hw4.sh
./grade_hw4.sh
```

### Cách 2: Chấm trực tiếp 2 file thực thi `./server` và `./client`
```bash
./grade_hw4.sh ./server ./client
```

### Cách 3: Chấm file zip nộp bài của bạn (Khuyên dùng trước khi nộp)
```bash
./grade_hw4.sh NguyenVanA_20161234_HW4.zip
```

### Cách 4: Chạy bộ kiểm thử chuyên sâu với Python
```bash
python3 test_hw4.py
```

---

## 3. Thang điểm đánh giá (Tổng 10.0 điểm)

| Thành phần | Điểm | Tiêu chí đánh giá |
| :--- | :---: | :--- |
| **Makefile & Cấu trúc** | **2.0đ** | Định dạng tên zip chuẩn `HotenSV_MSSV_HW4.zip`. Makefile biên dịch ra đúng 2 file `server` và `client` không có cảnh báo/lỗi (`-Wall -Wextra`). |
| **Test 1: Tham số dòng lệnh** | **1.0đ** | Xử lý đúng số lượng tham số, kiểm tra cổng hợp lệ (1-65535) và địa chỉ IPv4 hợp lệ. Thoát với mã lỗi khác 0 khi tham số sai. |
| **Test 2: Giao tiếp 2 Clients** | **2.5đ** | 2 Client gửi chuỗi hợp lệ (chỉ gồm chữ cái và chữ số). Server hiển thị nội dung và chuyển tiếp xâu sang Client đối diện kèm IP và Port của Client gửi. |
| **Test 3: Lọc ký tự không hợp lệ** | **2.0đ** | Chuỗi chứa dấu cách, ký tự đặc biệt: Server gửi thông báo lỗi về Client gửi, Client đối diện không nhận được, và **tuyệt đối không hiển thị kết quả phía Server**. |
| **Test 4: Kết thúc với @ và #** | **1.5đ** | Client ngắt kết nối và thoát an toàn sạch sẽ (returncode 0) ngay khi người dùng nhập chuỗi `@` hoặc `#`. |
| **Test 5: Quản lý vị trí & Tái kết nối** | **1.0đ** | Server giải phóng slot khi Client ngắt kết nối, cho phép Client mới kết nối vào slot trống và tiếp tục hoạt động bình thường. |

> **Lưu ý**: Đạt từ **9.0 / 10.0** trở lên là bài làm đạt chất lượng xuất sắc, sẵn sàng nộp bài theo quy định môn học.
