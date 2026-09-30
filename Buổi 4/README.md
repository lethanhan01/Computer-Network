# BÀI TẬP TUẦN 4 – IT4062 / IT4062E: ỨNG DỤNG UDP SOCKET CHAT 2 CLIENTS

## 1. Giới thiệu
Bài tập yêu cầu xây dựng ứng dụng mạng mô hình Client - Server sử dụng **UDP Socket (`SOCK_DGRAM`)** trong ngôn ngữ C (tuân thủ chuẩn POSIX/Linux).

Ứng dụng gồm hai chương trình thực thi:
1. **`server`**: Lắng nghe và tiếp nhận thông điệp từ tối đa **2 clients**, chuyển tiếp chuỗi nhận được từ Client này sang Client kia (kèm theo thông tin IP và Port của Client gửi). Đồng thời kiểm tra tính hợp lệ của chuỗi: nếu chứa ký tự không phải chữ cái hoặc chữ số thì phản hồi thông báo lỗi và **không hiển thị kết quả phía server**.
2. **`client`**: Cho phép người dùng nhập xâu ký tự từ bàn phím gửi lên server, nhận và hiển thị nội dung từ Client khác do server chuyển tiếp. Lặp lại cho đến khi người dùng nhập chuỗi `@` hoặc `#`.

---

## 2. Cấu trúc thư mục mã nguồn
```
HW4/
├── Makefile             # Cấu hình biên dịch server, client và đóng gói bài nộp
├── protocol.h           # Định nghĩa hằng số, cấu trúc dữ liệu và khai báo hàm tiện ích
├── protocol.c           # Cài đặt hàm kiểm tra chuỗi (isalnum), validate IP/Port, định dạng địa chỉ
├── server.c             # Cài đặt UDP Server: quản lý 2 client slots, lọc ký tự, chuyển tiếp tin nhắn
├── client.c             # Cài đặt UDP Client: multiplexing I/O với select(), nhập/xuất bất đồng bộ
├── test_hw4.py          # Bộ kiểm thử tự động toàn diện bằng Python
├── grade_hw4.sh         # Script bash chấm điểm và đánh giá tự động theo thang 10.0
├── HUONG_DAN_TU_CHAM.md # Hướng dẫn sinh viên tự kiểm thử và chấm điểm
└── README.md            # Tài liệu kỹ thuật chi tiết của bài tập
```

---

## 3. Hướng dẫn biên dịch và dọn dẹp
Để biên dịch cả 2 chương trình `server` và `client`:
```bash
make
# hoặc
make all
```
Trình biên dịch `gcc` sẽ chạy với các cờ nghiêm ngặt: `-Wall -Wextra -std=c99 -D_DEFAULT_SOURCE -D_GNU_SOURCE`.

Để dọn dẹp các file đối tượng (`*.o`), file thực thi và file nén zip:
```bash
make clean
```

---

## 4. Hướng dẫn chạy chương trình

### 4.1. Khởi chạy Server
Cú pháp:
```bash
./server <PortNumber>
```
Ví dụ:
```bash
./server 5500
```
Server sẽ bind tới cổng UDP 5500 trên mọi giao diện mạng (`INADDR_ANY`) và sẵn sàng phục vụ 2 clients.

### 4.2. Khởi chạy Client 1 và Client 2
Mở 2 cửa sổ terminal riêng biệt cho 2 clients:
```bash
./client <IPAddress> <PortNumber>
```
Ví dụ:
```bash
# Terminal Client 1:
./client 127.0.0.1 5500

# Terminal Client 2:
./client 127.0.0.1 5500
```

### 4.3. Kịch bản hoạt động

#### Trường hợp 1: Gửi chuỗi hợp lệ (chỉ gồm chữ cái và chữ số)
- **Client 1 nhập**: `Hello123Vietnam`
- **Phía Server**:
  - Kiểm tra thấy toàn bộ ký tự thuộc bảng chữ cái hoặc chữ số.
  - Hiển thị kết quả trên màn hình:
    ```
    [Client 1 (127.0.0.1:41234)]: Hello123Vietnam
    ```
  - Chuyển tiếp xâu cho Client 2 kèm IP và Port của Client 1.
- **Phía Client 2**:
  - Nhận và hiển thị:
    ```
    [127.0.0.1:41234]: Hello123Vietnam
    ```

- **Client 2 nhập phản hồi**: `SocketUDP2026`
- **Phía Server**:
  - Hiển thị:
    ```
    [Client 2 (127.0.0.1:52345)]: SocketUDP2026
    ```
- **Phía Client 1**:
  - Nhận và hiển thị:
    ```
    [127.0.0.1:52345]: SocketUDP2026
    ```

#### Trường hợp 2: Gửi chuỗi không hợp lệ (chứa dấu cách, ký tự đặc biệt)
- **Client 1 nhập**: `hello world` (chứa dấu cách) hoặc `tin_nhan!@#`
- **Phía Server**:
  - Nhận diện chuỗi chứa ký tự không phải chữ cái/chữ số.
  - **TUYỆT ĐỐI KHÔNG hiển thị kết quả lên màn hình server** (theo đúng yêu cầu đề bài).
  - Gửi trả thông báo lỗi ngược lại cho Client 1:
    ```
    Loi: Xau chua ky tu khong hop le! Chi chap nhan chu cai va chu so.
    ```
  - Không chuyển tiếp chuỗi lỗi cho Client 2.
- **Phía Client 1**:
  - Hiển thị thông báo lỗi nhận được từ server.
- **Phía Client 2**:
  - Không nhận được dữ liệu gì.

#### Trường hợp 3: Kết thúc chương trình
- Người dùng tại Client 1 hoặc Client 2 nhập `@` hoặc `#`:
  - Client gửi tín hiệu ngắt kết nối đến Server, in thông báo thoát và kết thúc tiến trình sạch sẽ (returncode 0).
  - Server giải phóng vị trí (slot) của Client đó và thông báo cho Client còn lại.
  - Một Client mới có thể kết nối vào slot trống và tiếp tục trò chuyện.

---

## 5. Đóng gói bài nộp
Quy chuẩn nộp bài môn học: File nén có tên theo định dạng `HotenSV_MSSV_HW4.zip` (Ví dụ: `NguyenVanA_20161234_HW4.zip`).

Chạy lệnh `make zip` với họ tên và MSSV của bạn:
```bash
make zip NAME=NguyenVanA MSSV=20161234
```
Lệnh sẽ tự động dọn dẹp và nén các file mã nguồn (`*.c`, `*.h`) cùng `Makefile`.

---

## 6. Tự chấm điểm với grade_hw4.sh
Bạn có thể tự chấm bài trước khi nộp để đảm bảo đạt điểm tối đa (10.0/10.0):
```bash
# Cách 1: Tự động chấm mã nguồn trong thư mục hiện tại
./grade_hw4.sh

# Cách 2: Chấm file zip nộp bài
./grade_hw4.sh NguyenVanA_20161234_HW4.zip

# Cách 3: Chạy test suite Python
python3 test_hw4.py
```
