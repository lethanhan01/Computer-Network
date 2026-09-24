CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
TARGET = user_manager
SRCS = main.c
OBJS = $(SRCS:.c=.o)

# Thong tin sinh vien nop bai
override NAME := LeThanhAn
override MSSV := 20235631
ZIP_NAME = $(NAME)_$(MSSV)_HW1.zip

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET) *.zip

zip: clean
	zip -r $(ZIP_NAME) main.c Makefile user.txt

.PHONY: all run clean zip
