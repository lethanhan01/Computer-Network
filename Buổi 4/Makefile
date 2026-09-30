CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -D_DEFAULT_SOURCE -D_GNU_SOURCE

all: server client

server: server.c
	$(CC) $(CFLAGS) server.c -o server

client: client.c
	$(CC) $(CFLAGS) client.c -o client

clean:
	rm -f server client *.o *.zip

zip: clean
	@if [ -z "$(NAME)" ] || [ -z "$(MSSV)" ]; then \
		echo "Lỗi: Vui lòng truyền NAME và MSSV. Ví dụ: make zip NAME=NguyenVanA MSSV=20161234"; \
		exit 1; \
	fi
	zip -r $(NAME)_$(MSSV)_HW4.zip *.c Makefile

.PHONY: all clean zip
