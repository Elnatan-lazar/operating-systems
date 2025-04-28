CC = gcc
CFLAGS = -Wall -Wextra -g -coverage
TARGET = dijkstra

all: $(TARGET)

$(TARGET): dijkstra.c
	$(CC) $(CFLAGS) dijkstra.c -o $(TARGET)

clean:
	rm -f $(TARGET) *.o *.gcno *.gcda *.gcov

gcov: $(TARGET)
	./$(TARGET)
	gcov dijkstra.c

.PHONY: all clean gcov
