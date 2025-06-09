# Compiler and flags
CC = gcc
CFLAGS = -Wall -std=c99
LDFLAGS = -lm

# Target executable
TARGET = main

# Source files
SRC = main.c

# Build target
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

# Clean build files
clean:
	rm -f $(TARGET)
