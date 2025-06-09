CC = gcc
CFLAGS = -Wall -Wextra -O2

all: Receiver Sender

Receiver: Receiver.c
	$(CC) $(CFLAGS) -o Receiver Receiver.c

Sender: Sender.c
	$(CC) $(CFLAGS) -o Sender Sender.c

clean:
	rm -f Receiver Sender

