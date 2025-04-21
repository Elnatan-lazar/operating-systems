CC = gcc
CFLAGS = -Wall -Wextra -O2

all: receiver sender

receiver: Receiver.c
	$(CC) $(CFLAGS) -o Receiver Receiver.c

sender: Sender.c
	$(CC) $(CFLAGS) -oSsender Sender.c

clean:
	rm -f Receiver Sender
