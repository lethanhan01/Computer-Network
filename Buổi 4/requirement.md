# BÀI TẬP TUẦN 4 – IT4062 / IT4062E: ỨNG DỤNG UDP SOCKET CHAT 2 CLIENTS

## 1. Yêu cầu chung

Xây dựng ứng dụng truyền thông mạng sử dụng giao thức **UDP Socket (`SOCK_DGRAM`)** theo mô hình Client - Server với các yêu cầu chi tiết như sau:

### Phía Server:
- **Giới hạn số lượng Client**: Phục vụ tối đa đồng thời **2 clients** (Client 1 và Client 2). Nếu có client thứ 3 gửi tin nhắn/kết nối trong lúc đã đủ 2 client, server gửi phản hồi thông báo phòng đã đầy.
- **Tham số dòng lệnh**: Chạy ở số hiệu cổng bất kỳ thông qua tham số dòng lệnh:
  ```bash
  ./server <PortNumber>
  # Ví dụ:
  ./server 5500
  ```
  Kiểm tra tính hợp lệ của tham số: nếu thiếu tham số hoặc cổng không hợp lệ (không phải số nguyên dương trong dải `1 - 65535`), hiển thị hướng dẫn sử dụng, thông báo lỗi và thoát với mã lỗi khác 0 (`exit code != 0`).
- **Xử lý tính hợp lệ của chuỗi nhận được**:
  - Khi nhận chuỗi từ một client, kiểm tra xem toàn bộ các ký tự trong xâu có thuộc bảng chữ cái (`a-z`, `A-Z`) hoặc chữ số (`0-9`) hay không (dùng hàm tương đương `isalnum()`).
  - **Trường hợp hợp lệ**: Hiển thị chuỗi lên màn hình server kèm thông tin `[IP:Port]` của client gửi, đồng thời chuyển tiếp chuỗi đó sang client còn lại.
  - **Trường hợp không hợp lệ (chứa dấu cách, ký tự đặc biệt...)**: Gửi lại thông báo lỗi về client gửi. **TUYỆT ĐỐI KHÔNG hiển thị kết quả chuỗi không hợp lệ phía server**.
- **Giải phóng kết nối**: Khi một client gửi tín hiệu thoát (`@` hoặc `#`), server giải phóng vị trí (slot) của client đó để client mới có thể tham gia vào hệ thống.

### Phía Client:
- **Tham số dòng lệnh**: Kết nối tới server bằng địa chỉ IP và số hiệu cổng truyền qua tham số dòng lệnh:
  ```bash
  ./client <IPAddress> <PortNumber>
  # Ví dụ:
  ./client 127.0.0.1 5500
  ```
  Kiểm tra tính hợp lệ của tham số: nếu thiếu tham số hoặc IP/Port không hợp lệ, hiển thị thông báo lỗi và thoát với mã lỗi khác 0.
- **Khởi tạo kết nối**: Ngay khi khởi chạy, client gửi gói tin đăng ký khởi tạo (ví dụ token `__CONNECT__`) lên server để server nhận diện và ghi nhận slot hoạt động.
- **Gửi và nhận dữ liệu bất đồng bộ**:
  - Cho phép người dùng nhập xâu bất kỳ từ bàn phím (`stdin`) và gửi tới server qua UDP.
  - Nhận kết quả do server chuyển tiếp từ client khác (client thứ 2) và hiển thị nội dung kèm địa chỉ `[IP:Port]` của client gửi.
  - Hiển thị thông báo lỗi nhận về từ server nếu xâu vừa gửi chứa ký tự không hợp lệ.
- **Điều kiện kết thúc**: Vòng lặp gửi/nhận tiếp diễn cho tới khi người dùng nhập xâu `@` hoặc `#`. Khi đó client gửi mã thoát lên server, in thông báo kết thúc và đóng chương trình an toàn (`exit code = 0`).

---

## 2. Bảng tổng hợp Đầu vào & Đầu ra

### 2.1. Phía Server (`./server <PortNumber>`)

| STT | Kịch bản / Tình huống | Đầu vào (Input) | Đầu ra màn hình (Console Output / Log) | Dữ liệu gửi qua mạng (Network Out) |
| :---: | :--- | :--- | :--- | :--- |
| **1** | Khởi động Server thành công | CLI: `./server 5500` | `[Server] UDP Server is running on port 5500...` | *None* |
| **2** | Tham số CLI thiếu hoặc cổng không hợp lệ | CLI: `./server`<br>hoặc `./server abc`<br>hoặc `./server 99999` | `Usage: ./server <PortNumber>`<br>`Error: Invalid port number! (Exit code != 0)` | *None* |
| **3** | Client 1 gửi tín hiệu kết nối | Gói tin UDP init từ `127.0.0.1:50001` (Token `__CONNECT__`) | `[Server] Client 1 connected from 127.0.0.1:50001` | *None* |
| **4** | Client 2 gửi tín hiệu kết nối | Gói tin UDP init từ `127.0.0.1:60089` (Token `__CONNECT__`) | `[Server] Client 2 connected from 127.0.0.1:60089` | *None* |
| **5** | Nhận chuỗi hợp lệ (chỉ gồm chữ cái và số) | Từ Client 1 (`127.0.0.1:50001`):<br>`Hello123Vietnam` | `[Client 1 (127.0.0.1:50001)]: Hello123Vietnam` | Chuyển tiếp tới Client 2 (`127.0.0.1:60089`):<br>`[127.0.0.1:50001]: Hello123Vietnam` |
| **6** | Nhận chuỗi không hợp lệ (chứa dấu cách, ký tự đặc biệt) | Từ Client 1 (`127.0.0.1:50001`):<br>`hello world` hoặc `Test!@#` | **(NO DISPLAY ON SERVER)**<br>*Tuyệt đối không hiển thị kết quả lên console* | Phản hồi lại cho Client 1:<br>`Error: String contains invalid characters! Only alphanumeric characters are allowed.`<br>*(Không chuyển tiếp sang Client 2)* |
| **7** | Client ngắt kết nối (nhập `@` hoặc `#`) | Từ Client 1 (`127.0.0.1:50001`):<br>`@` hoặc `#` | `[Server] Client 1 (127.0.0.1:50001) disconnected (exit token '@').`<br>*(Slot Client 1 được giải phóng)* | *None* |
| **8** | Client thứ 3 gửi tin khi server đã đủ 2 clients | Gói tin từ Client 3 (`127.0.0.1:65000`) | *None* (hoặc log cảnh báo từ chối kết nối) | Phản hồi lại cho Client 3:<br>`Error: Server is full (maximum 2 clients reached).` |

---

### 2.2. Phía Client (`./client <IPAddress> <PortNumber>`)

| STT | Kịch bản / Tình huống | Đầu vào (Input) | Đầu ra màn hình (Console Output / Log) | Dữ liệu gửi qua mạng (Network Out) |
| :---: | :--- | :--- | :--- | :--- |
| **1** | Khởi động Client và kết nối tới Server | CLI: `./client 127.0.0.1 5500` | `Connected to server 127.0.0.1 at port 5500`<br>`[You]: ` | Gói tin đăng ký kết nối (`__CONNECT__`) gửi tới Server |
| **2** | Tham số CLI thiếu hoặc IP/Port không hợp lệ | CLI: `./client`<br>hoặc `./client 127.0.0.1`<br>hoặc `./client 999.999.999.999 5500` | `Usage: ./client <IPAddress> <PortNumber>`<br>`Error: Invalid IP address or port number! (Exit code != 0)` | *None* |
| **3** | Người dùng nhập chuỗi hợp lệ | Bàn phím: `Hi2026Vietnam` + `Enter` | `[You]: ` *(tiếp tục hiển thị prompt chờ lượt nhập tiếp theo)* | Gói tin UDP gửi lên Server:<br>`Hi2026Vietnam` |
| **4** | Nhận chuỗi chuyển tiếp từ client khác | Gói tin UDP đến từ Server:<br>`[127.0.0.1:60089]: SocketUDP456` | `[127.0.0.1:60089]: SocketUDP456`<br>`[You]: ` | *None* |
| **5** | Người dùng nhập chuỗi không hợp lệ | Bàn phím: `hello world` + `Enter`<br>*(chứa dấu cách/ký tự lạ)* | `Error: String contains invalid characters! Only alphanumeric characters are allowed.`<br>`[You]: ` | Gói tin UDP gửi lên Server:<br>`hello world` |
| **6** | Người dùng gửi dòng trống | Bàn phím: Nhấn `Enter` trực tiếp | `[You]: ` *(bỏ qua dòng trống, nhắc lại prompt)* | *None* |
| **7** | Người dùng thoát ứng dụng | Bàn phím: `@` hoặc `#` + `Enter` | `Exiting program...`<br>*(Chương trình thoát sạch sẽ với exit code = 0)* | Gói tin UDP gửi lên Server thông báo ngắt kết nối: `@` hoặc `#` |

---

## 3. Quy chuẩn biên dịch & Đóng gói nộp bài

### 3.1. Quy chuẩn biên dịch với Makefile
- Makefile phải được đặt tại thư mục gốc của bài tập.
- Biên dịch đồng thời cả hai chương trình `server` và `client` bằng lệnh:
  ```bash
  make
  # hoặc
  make all
  ```
- Tên file thực thi sau khi biên dịch phải chính xác là:
  - `server`
  - `client`
- Cung cấp target `make clean` để dọn dẹp các tệp tin đối tượng (`*.o`), file thực thi và file nén:
  ```bash
  make clean
  ```

### 3.2. Cấu trúc đóng gói bài nộp
- Quy chuẩn đặt tên file nén bài nộp: `HotenSV_MSSV_HW4.zip` (Ví dụ: `NguyenVanA_20261234_HW4.zip`).
- File nén chỉ bao gồm các mã nguồn C (`*.c`, `*.h`) và `Makefile`, không nộp các file thực thi nhị phân.