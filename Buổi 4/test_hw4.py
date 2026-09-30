#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Test Suite tự động cho bài tập HW4 (IT4062 - Lập trình mạng)
Ứng dụng Socket UDP Chat 2 Clients với kiểm tra tính hợp lệ xâu ký tự
"""

import subprocess
import os
import sys
import time
import socket
import signal

WORK_DIR = os.path.dirname(os.path.abspath(__file__))
SERVER_BIN = os.path.join(WORK_DIR, "server")
CLIENT_BIN = os.path.join(WORK_DIR, "client")

def get_free_port():
    """Tìm một cổng UDP còn trống trong hệ thống"""
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.bind(('', 0))
        return s.getsockname()[1]

def test_compilation():
    print("\n--- Test 1: Kiểm tra biên dịch Makefile (2.0đ) ---")
    proc_clean = subprocess.run(["make", "clean"], cwd=WORK_DIR, capture_output=True, text=True)
    assert proc_clean.returncode == 0, f"make clean thất bại: {proc_clean.stderr}"

    proc_make = subprocess.run(["make", "all"], cwd=WORK_DIR, capture_output=True, text=True)
    assert proc_make.returncode == 0, f"make thất bại:\n{proc_make.stderr}"
    assert os.path.exists(SERVER_BIN), "File thực thi 'server' không tồn tại sau khi make!"
    assert os.path.exists(CLIENT_BIN), "File thực thi 'client' không tồn tại sau khi make!"
    print("✔ Makefile biên dịch thành công tạo cả 'server' và 'client'.")

def test_argument_handling():
    print("\n--- Test 2: Kiểm tra xử lý đối số dòng lệnh & Cổng (1.0đ) ---")
    # Server thieu tham so
    p = subprocess.run([SERVER_BIN], capture_output=True, text=True)
    assert p.returncode != 0, "Server không truyền đối số phải thoát với mã lỗi khác 0!"

    # Server cong khong hop le
    p = subprocess.run([SERVER_BIN, "99999"], capture_output=True, text=True)
    assert p.returncode != 0, "Server cổng > 65535 phải thoát với mã lỗi!"

    p = subprocess.run([SERVER_BIN, "abc"], capture_output=True, text=True)
    assert p.returncode != 0, "Server cổng không phải là số phải thoát với mã lỗi!"

    # Client thieu tham so
    p = subprocess.run([CLIENT_BIN], capture_output=True, text=True)
    assert p.returncode != 0, "Client không truyền đối số phải thoát với mã lỗi!"

    p = subprocess.run([CLIENT_BIN, "127.0.0.1"], capture_output=True, text=True)
    assert p.returncode != 0, "Client thiếu cổng phải thoát với mã lỗi!"

    # Client IP khong hop le
    p = subprocess.run([CLIENT_BIN, "999.999.999.999", "5500"], capture_output=True, text=True)
    assert p.returncode != 0, "Client IP không hợp lệ phải thoát với mã lỗi!"

    print("✔ Xử lý tham số dòng lệnh và tính hợp lệ IP/Port chính xác.")

def test_two_clients_communication():
    print("\n--- Test 3: Giao tiếp giữa 2 Clients với xâu ký tự hợp lệ (2.5đ) ---")
    port = get_free_port()
    
    server_proc = subprocess.Popen(
        [SERVER_BIN, str(port)],
        cwd=WORK_DIR,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    time.sleep(0.3)

    try:
        # Khoi dong Client 1
        c1 = subprocess.Popen(
            [CLIENT_BIN, "127.0.0.1", str(port)],
            cwd=WORK_DIR,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        time.sleep(0.2)

        # Khoi dong Client 2
        c2 = subprocess.Popen(
            [CLIENT_BIN, "127.0.0.1", str(port)],
            cwd=WORK_DIR,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        time.sleep(0.2)

        # Client 1 gui: Hello123Vietnam
        msg1 = "Hello123Vietnam"
        c1.stdin.write(msg1 + "\n")
        c1.stdin.flush()
        time.sleep(0.3)

        # Client 2 gui: SocketUDP456
        msg2 = "SocketUDP456"
        c2.stdin.write(msg2 + "\n")
        c2.stdin.flush()
        time.sleep(0.3)

        # Dong hai client bang lenh @ va #
        c1.stdin.write("@\n")
        c1.stdin.flush()
        c2.stdin.write("#\n")
        c2.stdin.flush()

        c1_out, _ = c1.communicate(timeout=2)
        c2_out, _ = c2.communicate(timeout=2)

        # Kiem tra Client 2 da nhan duoc msg1 tu Client 1 (kem IP va Port)
        assert msg1 in c2_out, f"Client 2 không nhận được tin nhắn '{msg1}'!\nOutput:\n{c2_out}"
        assert "127.0.0.1" in c2_out, f"Client 2 không thấy địa chỉ IP của Client 1 trong tin nhắn nhận được!\nOutput:\n{c2_out}"
        print(f"✔ Client 2 đã nhận xâu '{msg1}' kèm IP:Port của Client 1.")

        # Kiem tra Client 1 da nhan duoc msg2 tu Client 2 (kem IP va Port)
        assert msg2 in c1_out, f"Client 1 không nhận được tin nhắn '{msg2}'!\nOutput:\n{c1_out}"
        assert "127.0.0.1" in c1_out, f"Client 1 không thấy địa chỉ IP của Client 2 trong tin nhắn nhận được!\nOutput:\n{c1_out}"
        print(f"✔ Client 1 đã nhận xâu '{msg2}' kèm IP:Port của Client 2.")

    finally:
        server_proc.send_signal(signal.SIGINT)
        server_out, _ = server_proc.communicate(timeout=2)
        assert msg1 in server_out, f"Server không hiển thị xâu hợp lệ '{msg1}'!"
        assert msg2 in server_out, f"Server không hiển thị xâu hợp lệ '{msg2}'!"
        print("✔ Phía Server hiển thị đầy đủ các xâu hợp lệ kèm thông tin Client.")

def test_invalid_characters_handling():
    print("\n--- Test 4: Kiểm tra xâu chứa ký tự không phải chữ cái hoặc chữ số (2.0đ) ---")
    port = get_free_port()
    
    server_proc = subprocess.Popen(
        [SERVER_BIN, str(port)],
        cwd=WORK_DIR,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    time.sleep(0.3)

    try:
        c1 = subprocess.Popen(
            [CLIENT_BIN, "127.0.0.1", str(port)],
            cwd=WORK_DIR,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        c2 = subprocess.Popen(
            [CLIENT_BIN, "127.0.0.1", str(port)],
            cwd=WORK_DIR,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        time.sleep(0.2)

        # Client 1 gui xau co dau cach: "hello world"
        invalid_msg_1 = "hello world"
        c1.stdin.write(invalid_msg_1 + "\n")
        c1.stdin.flush()
        time.sleep(0.3)

        # Client 1 gui xau co ky tu dac biet: "ha_noi!@2026"
        invalid_msg_2 = "ha_noi!@2026"
        c1.stdin.write(invalid_msg_2 + "\n")
        c1.stdin.flush()
        time.sleep(0.3)

        # Dong hai client
        c1.stdin.write("@\n")
        c1.stdin.flush()
        c2.stdin.write("@\n")
        c2.stdin.flush()

        c1_out, _ = c1.communicate(timeout=2)
        c2_out, _ = c2.communicate(timeout=2)

        # 1. Client 1 phai nhan duoc thong bao loi
        c1_lower = c1_out.lower()
        assert "loi" in c1_lower or "error" in c1_lower or "invalid" in c1_lower, \
            f"Client 1 không nhận được thông báo lỗi khi gửi ký tự không hợp lệ!\nOutput:\n{c1_out}"
        print("✔ Server đã phản hồi thông báo lỗi cho Client 1.")

        # 2. Client 2 TUYET DOI KHONG nhan duoc cac xau khong hop le
        assert invalid_msg_1 not in c2_out, f"Lỗi: Client 2 vẫn nhận được xâu không hợp lệ '{invalid_msg_1}'!"
        assert invalid_msg_2 not in c2_out, f"Lỗi: Client 2 vẫn nhận được xâu không hợp lệ '{invalid_msg_2}'!"
        print("✔ Client 2 không bị chuyển tiếp xâu chứa ký tự không hợp lệ.")

    finally:
        server_proc.send_signal(signal.SIGINT)
        server_out, _ = server_proc.communicate(timeout=2)
        # 3. Yeu cau de bai: (Khong hien thi ket qua phia server)
        assert invalid_msg_1 not in server_out, f"Lỗi: Server vẫn hiển thị xâu không hợp lệ '{invalid_msg_1}' trên console!"
        assert invalid_msg_2 not in server_out, f"Lỗi: Server vẫn hiển thị xâu không hợp lệ '{invalid_msg_2}' trên console!"
        print("✔ Server tuân thủ nghiêm ngặt: KHÔNG hiển thị xâu chứa ký tự không hợp lệ.")

def test_exit_conditions():
    print("\n--- Test 5: Thoát chương trình khi nhập '@' hoặc '#' (1.5đ) ---")
    port = get_free_port()
    
    server_proc = subprocess.Popen(
        [SERVER_BIN, str(port)],
        cwd=WORK_DIR,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    time.sleep(0.3)

    try:
        # Test thoat voi '@'
        c_at = subprocess.Popen(
            [CLIENT_BIN, "127.0.0.1", str(port)],
            cwd=WORK_DIR,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        time.sleep(0.2)
        c_at.stdin.write("@\n")
        c_at.stdin.flush()
        ret_at = c_at.wait(timeout=2)
        assert ret_at == 0, f"Client thoát bằng '@' nhưng trả về mã lỗi {ret_at}"
        print("✔ Client thoát thành công và kết thúc an toàn khi nhập '@'.")

        # Test thoat voi '#'
        c_hash = subprocess.Popen(
            [CLIENT_BIN, "127.0.0.1", str(port)],
            cwd=WORK_DIR,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        time.sleep(0.2)
        c_hash.stdin.write("#\n")
        c_hash.stdin.flush()
        ret_hash = c_hash.wait(timeout=2)
        assert ret_hash == 0, f"Client thoát bằng '#' nhưng trả về mã lỗi {ret_hash}"
        print("✔ Client thoát thành công và kết thúc an toàn khi nhập '#'.")

    finally:
        server_proc.send_signal(signal.SIGINT)
        server_proc.communicate(timeout=2)

def test_client_reconnect_multisession():
    print("\n--- Test 6: Quản lý 2 Clients & Kết nối mới sau khi Client cũ thoát (1.0đ) ---")
    port = get_free_port()
    
    server_proc = subprocess.Popen(
        [SERVER_BIN, str(port)],
        cwd=WORK_DIR,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    time.sleep(0.3)

    try:
        # Client 1 va Client 2 ket noi
        c1 = subprocess.Popen([CLIENT_BIN, "127.0.0.1", str(port)], cwd=WORK_DIR, stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
        c2 = subprocess.Popen([CLIENT_BIN, "127.0.0.1", str(port)], cwd=WORK_DIR, stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
        time.sleep(0.2)

        # Client 1 thoat
        c1.stdin.write("@\n")
        c1.stdin.flush()
        c1.wait(timeout=2)
        time.sleep(0.3)

        # Client 3 ket noi vao slot trong
        c3 = subprocess.Popen([CLIENT_BIN, "127.0.0.1", str(port)], cwd=WORK_DIR, stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
        time.sleep(0.2)

        msg3 = "Client3JoinedSuccess"
        c3.stdin.write(msg3 + "\n")
        c3.stdin.flush()
        time.sleep(0.3)

        # Dong c2 va c3
        c2.stdin.write("#\n")
        c2.stdin.flush()
        c3.stdin.write("#\n")
        c3.stdin.flush()

        c2_out, _ = c2.communicate(timeout=2)
        c3_out, _ = c3.communicate(timeout=2)

        assert msg3 in c2_out, f"Client 2 không nhận được tin từ Client 3 mới kết nối!\nOutput:\n{c2_out}"
        print("✔ Server quản lý vị trí thông minh: Client mới tái sử dụng slot trống và tiếp tục giao tiếp.")

    finally:
        server_proc.send_signal(signal.SIGINT)
        server_proc.communicate(timeout=2)

def main():
    print("======================================================================")
    print("      BỘ KIỂM THỬ TỰ ĐỘNG BÀI TẬP HW4 (IT4062 - UDP SOCKET CHAT)      ")
    print("======================================================================")
    try:
        test_compilation()
        test_argument_handling()
        test_two_clients_communication()
        test_invalid_characters_handling()
        test_exit_conditions()
        test_client_reconnect_multisession()
        print("\n======================================================================")
        print("🎉 TẤT CẢ CÁC BÀI TEST ĐỀU THÀNH CÔNG! ĐIỂM DỰ KIẾN: 10.0 / 10.0")
        print("======================================================================")
        return 0
    except AssertionError as e:
        print(f"\n❌ KIỂM THỬ THẤT BẠI: {e}")
        return 1
    except Exception as e:
        print(f"\n❌ LỖI HỆ THỐNG: {e}")
        return 2

if __name__ == "__main__":
    sys.exit(main())
