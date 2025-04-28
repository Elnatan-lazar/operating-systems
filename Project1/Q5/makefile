CC = gcc
CFLAGS = -Wall -Wextra -g -pg
TARGET = sum_array_sub_max

all: $(TARGET)

$(TARGET): sum_array_sub_max.c
	$(CC) $(CFLAGS) sum_array_sub_max.c -o $(TARGET)

run: $(TARGET)
	./$(TARGET) 42 10000

gprof: run
	gprof $(TARGET) gmon.out > analysis.txt
	cat analysis.txt

clean:
	rm -f $(TARGET) *.o gmon.out analysis.txt

.PHONY: all run gprof clean
