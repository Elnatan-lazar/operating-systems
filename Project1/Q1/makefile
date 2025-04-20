# Makefile

CC = gcc
CFLAGS = -Wall -g

all: StackOverflow DivisionByZero InvalidMemory

StackOverflow: StackOverflow.c
	$(CC) $(CFLAGS) StackOverflow.c -o StackOverflow

DivisionByZero: DivisionByZero.c
	$(CC) $(CFLAGS) DivisionByZero.c -o DivisionByZero

InvalidMemory: InvalidMemory.c
	$(CC) $(CFLAGS) InvalidMemory.c -o InvalidMemory

run-stack:
	./StackOverflow

run-zero:
	./DivisionByZero

run-invalid:
	./InvalidMemory

.PHONY: all clean run-stack run-zero run-invalid

clean:
	rm -f StackOverflow DivisionByZero InvalidMemory
