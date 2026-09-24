# 1. Naming & Constants

- Đặt tên giúp người đọc hiểu ngay vai trò của đối tượng
    - Hằng số: UPPER_SNAKE_CASE
    - Biến/hàm: camelCase; tên thể hiện rõ loại tài nguyên hoặc hành động.
    - Tránh tên ngắn, mơ hồ: s, tmp, x, n nếu không có ngữ cảnh rõ ràng.
    - Tên socket nên phân biệt rõ listening socket và connected socket. 

```
// Nên
#define SERVER_PORT 5550
#define LISTEN_BACKLOG 2
#define BUFFER_SIZE 1024

int listenSocket;
int connectionSocket;
int bytesReceived;
int bytesSent;

---

// Không nên
#define PORT 5550
#define BACKLOG 2
int s, c, n;
// Nên
#define SERVER_PORT 5550
int listenSocket;
int connectionSocket;

```



# 2. Formatting & Comments

Định dạng nhất quán làm nổi bật cấu trúc xử lý socket

- Dùng 4 spaces; không trộn tab và spaces.
- Dấu { đặt cùng dòng với câu lệnh điều khiển/hàm.
- Mỗi dòng lệnh nên ngắn, tránh biểu thức quá dài.
- Comment giải thích “tại sao”, không chỉ lặp lại “làm gì”.
- Comment theo từng bước xử lý mạng: socket → bind → listen → accept → recv/send → close.

```
if ((listenSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    return EXIT_FAILURE;
}

// Bind the server address to the listening socket.
if (bind(listenSocket, 
        (struct sockaddr *)&serverAddress, 
        sizeof(serverAddress)) == -1) {
    perror("bind");
    return EXIT_FAILURE;
}
```

# 3. Socket Programming Convention

- Quy ước quan trọng khi viết code LT mạng
    - Luôn kiểm tra giá trị trả về của socket(), bind(), listen(), accept(), recv() và send().
    - Dùng tên biến thể hiện rõ trạng thái: listenSocket, connectionSocket, clientAddress.
    - Dùng htons()/htonl() khi ghi port/địa chỉ theo network byte order.
    - Không dùng magic number: khai báo SERVER_PORT, BUFFER_SIZE, LISTEN_BACKLOG.
    - Phân biệt rõ:
        - listenSocket: chỉ nhận connection mới.
        - connectionSocket: giao tiếp với một client sau accept().
        - recv()/send(): có thể trả về 0 hoặc số byte nhỏ hơn mong đợi.

# 4. Khi nào nên viết Comment?

Comment để giải thích những điều code không thể tự thể hiện rõ ràng

### NÊN COMMENT KHI:
- Giải thích lý do hoặc quyết định thiết kế không hiển nhiên.
- Mô tả giao thức, byte order, timeout, blocking/non-blocking.
- Giải thích giới hạn, workaround hoặc hành vi đặc biệt của OS/API.
- Cảnh báo điều kiện quan trọng:
    - partial send/recv
    - retry
    - resource cleanup
- Đánh dấu TODO/FIXME kèm vấn đề và hướng xử lý cụ thể.

### KHÔNG NÊN COMMENT KHI:
- Chỉ lặp lại đúng tên hàm hoặc câu lệnh.
- Giải thích điều quá hiển nhiên: i++; hoặc socket = socket(...).
- Comment đã lỗi thời hoặc không còn đúng với code.
- Dùng comment thay cho việc đặt tên biến/hàm rõ nghĩa.
- Viết đoạn comment dài nhưng không nêu được mục đích.


# Quy tắc ngắn gọn

- Comment the why, not the what.    
- Comment đầu hàm: mô tả mục đích, tham số, giá trị trả về và các điều kiện quan trọng.
- Comment bên trong: giải thích lý do thiết kế, hành vi đặc biệt của API, giao thức, byte order, blocking/non-blocking, hoặc workaround.
- Không comment những câu lệnh quá hiển nhiên.
- Nếu code cần quá nhiều comment để hiểu, hãy cân nhắc tách hàm hoặc đổi tên biến/hàm cho rõ nghĩa.
- Tài liệu chính thức: Microsoft C++ Core Guidelines
    https://microsoft.github.io/OfficeCppGuidelines/CppCoreGuidelines.html

# Ví dụ về comment Doxygen / Docstring

```

/**
* @brief Xử lý đăng nhập tài khoản người dùng.

* Hàm kiểm tra thông tin tai khoản dua tren tên đang nhap va mat khẩu.
* Quản lý trạng thái và đếm số lan sai mật khẩu; tự động khóa tài khoản
* và lưu lại file nếu vượt quá MAX_FAILED_ATTEMPTS.

* @param[in] head Con tro toi nut dau cua danh sach lien ket tai khoản.
* @param[in] username Tên đăng nhập cần đối chiếu.
* @param[in] password Mật khẩu người dùng nhập vào.
* @param[out] signed_in_user Con tro bac hai nhan dia chi của node tai khoản vừa đăng

* @return Mã trạng thái kết quả đăng nhập (int):
- `0`: Đăng nhập thành công.
- `1`: Không tìm thấy tai khoản.
- '2: Tai khoản đang bi khoa (STATUS_BLOCKED).
- '3: Sai mật khẩu.
- `4`: Sai mật khẩu lien tiếp đạt giới hạn, tài khoản đã bị khóa.
*/

int sign_in_account(AccountNode* head, const char* username, const char* password, AccountNode** signed_in_user);

* Xử lý đăng nhập tài khoản người dùng.
* Hàm kiểm tra thông tin tài khoản dựa trên tên đăng nhập và mật khẩu. Quản lý trạng thái và đếm số lần sai mật khẩu; tự động khóa tài khoản và lưu lại file nếu vượt quá MAX_FAILED_ATTEMPTS.
* Parameters:
    - head - [in] Con trỏ tới nút đầu của danh sách liên kết tài khoản.
    - username - [in] Tên đăng nhập cần đối chiếu.
    - password - [in] Mật khẩu người dùng nhập vào.
    - signed_in_user - [out] Con trỏ bậc hai nhận địa chỉ của node tài khoản vừa đăng nhập thành công (truyền NULL nếu không cần lấy).

```