CC = gcc
CFLAGS = -Wall -Wextra -O2

# Thông tin sinh viên để đóng gói nộp bài
# Có thể ghi đè qua lệnh: make zip NAME=HoVaTen MSSV=MaSoSinhVien
NAME = LeThanhAn
MSSV = 20235631
ZIP_NAME = $(NAME)_$(MSSV)_HW4.zip

all: server client

server: server.c
	$(CC) $(CFLAGS) server.c -o server

client: client.c
	$(CC) $(CFLAGS) client.c -o client

clean:
	rm -f server client *.o *.zip

zip: clean
	zip -r $(ZIP_NAME) *.c Makefile

.PHONY: all clean zip
