CC = gcc
CFLAGS = -g
all: StackOverflow DivisionByZero InvalidMemory

StackOverflow: StackOverflow.c
	$(CC) $(CFLAGS) -o StackOverflow StackOverflow.c

DivisionByZero: DivisionByZero.c
	$(CC) $(CFLAGS) -o DivisionByZero DivisionByZero.c

InvalidMemory: InvalidMemory.c
	$(CC) $(CFLAGS) -o InvalidMemory InvalidMemory.c

clean:
	rm -f StackOverflow DivisionByZero InvalidMemory core.*
