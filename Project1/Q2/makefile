# Compiler and flags
CC = gcc
CFLAGS = -Wall -std=c99
LDFLAGS = -lm

# Target executable
TARGET = main

# Source files
SRC = main.c

# Build target
main: main.c
	$(CC) $(CFLAGS) -o main main.c $(LDFLAGS)

# Clean build files
clean:
	rm -f main
