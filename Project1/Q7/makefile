CC = gcc
CFLAGS = -Wall -Wextra

all: add2PB findPhone

add2PB: add2PB.c
	$(CC) $(CFLAGS) add2PB.c -o add2PB

findPhone: findPhone.c
	$(CC) $(CFLAGS) findPhone.c -o findPhone

clean:
	rm -f add2PB findPhone

.PHONY: all clean
