# Bài tập tuần 1 — IT4062/4062E

> **Hạn nộp:** Cuối buổi học. Sinh viên nộp bài đúng và sớm nhất được cộng điểm.

## Yêu cầu nộp bài

- [ ] Tạo `Makefile` để biên dịch chương trình.
- [ ] Đóng gói toàn bộ mã nguồn và `Makefile` vào một tệp nén theo định dạng:

  ```text
  HotenSV_MSSV_HW1.zip
  ```

  Ví dụ: `NguyenVanA_20161234_HW1.zip`.
- [ ] Nộp bài theo quy định.

---

## Chương trình quản lý tài khoản người dùng

Thông tin tài khoản được lưu trong tệp văn bản `user.txt`. Mỗi tài khoản chiếm một dòng, theo định dạng:

```text
username:password:status:score
```

| Trường | Ý nghĩa |
| --- | --- |
| `username` | Tên tài khoản |
| `password` | Mật khẩu |
| `status` | Trạng thái tài khoản |
| `score` | Điểm số của tài khoản |

Giá trị của `status`:

| Giá trị | Trạng thái |
| --- | --- |
| `0` | Đang hoạt động (`active`) |
| `1` | Đang bị khóa (`blocked`) |

### Yêu cầu chung

- Sử dụng cấu trúc **danh sách liên kết** (*linked list*) để lưu các tài khoản đọc từ tệp dữ liệu.
- Sau mỗi chức năng, phải hiển thị lại menu.
- Tệp `user.txt` phải nằm cùng thư mục với tệp thực thi.
- Kiểm soát và xử lý lỗi vào/ra tệp.
- Trước khi nộp, cần kiểm tra và sửa toàn bộ lỗi biên dịch.

---

## Menu chương trình

```text
USER MANAGEMENT PROGRAM
-----------------------------------
1. Register
2. Sign in
3. Search
4. Sort
Your choice (1-4, other to quit):
```

---

## Chức năng

### 1. Register — Đăng ký tài khoản

Người dùng nhập từ bàn phím:

- `username`
- `password`
- `score`

Quy tắc xử lý:

- Nếu tên tài khoản đã tồn tại, thông báo lỗi.
- Nếu tên tài khoản chưa tồn tại:
  - Tạo tài khoản mới với trạng thái `active`.
  - Thêm thông tin tài khoản vào `user.txt`.

### 2. Sign in — Đăng nhập

Người dùng nhập từ bàn phím:

- `username`
- `password`

Quy tắc xử lý:

- Nếu tài khoản tồn tại và mật khẩu đúng, thông báo đăng nhập thành công.
- Nếu tài khoản không tồn tại hoặc mật khẩu sai, hiển thị thông báo lỗi tương ứng.
- Nếu nhập sai mật khẩu quá 3 lần, khóa tài khoản và cập nhật trạng thái mới vào `user.txt`.

### 3. Search — Tìm kiếm tài khoản

Người dùng nhập `username` từ bàn phím.

- Nếu đã đăng nhập và tìm thấy tài khoản, hiển thị tên tài khoản cùng trạng thái `active` hoặc `blocked`.
- Nếu chưa đăng nhập hoặc không tìm thấy tài khoản, hiển thị thông báo lỗi tương ứng.

### 4. Sort & Search — Sắp xếp và tìm kiếm

1. Sắp xếp danh sách tài khoản theo `score` giảm dần bằng thuật toán **bubble sort**.
2. Hiển thị danh sách đã sắp xếp, gồm tên tài khoản và điểm số.
3. Sử dụng thuật toán **tìm kiếm nhị phân** để tìm các tài khoản có điểm lớn hơn `5`.
4. Hiển thị tên tài khoản và điểm số tương ứng của các tài khoản tìm được.

---

## Ví dụ tệp `user.txt`

```text
Hedspi:hedpsi2026:1:10
Hust:hust123:0:4
soict:soictfit:0:8
```
