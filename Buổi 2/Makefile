CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_DEFAULT_SOURCE -D_GNU_SOURCE
TARGET = resolver
SRCS = main.c network_utils.c resolver.c cloudflare_filter.c
OBJS = $(SRCS:.c=.o)
HEADERS = network_utils.h resolver.h cloudflare_filter.h

# Thông tin sinh viên để đóng gói nộp bài
# Có thể ghi đè qua lệnh: make zip NAME=HoVaTen MSSV=MaSoSinhVien
NAME = LeThanhAn
MSSV = 20235631
ZIP_NAME = $(NAME)_$(MSSV)_HW2.zip

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) google.com

clean:
	rm -f $(OBJS) $(TARGET) *.zip

zip: clean
	zip -r $(ZIP_NAME) *.c *.h Makefile

test: $(TARGET)
	bash grade_hw2.sh ./$(TARGET)

.PHONY: all run clean zip test
