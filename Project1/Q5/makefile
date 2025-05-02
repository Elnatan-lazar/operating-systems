CC = gcc
CFLAGS = -Wall -Wextra -g -pg
TARGET = sum_array_sub_max
SRC = max_sub_array_sum.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run10000: $(TARGET)
	./$(TARGET) 42 10000

run100: $(TARGET)
	./$(TARGET) 42 100

run1000: $(TARGET)
	./$(TARGET) 42 1000

gprof10000: run10000
	gprof $(TARGET) gmon.out > analysis10000.txt


gprof1000: run1000
	gprof $(TARGET) gmon.out > analysis1000.txt


gprof100: run100
	gprof $(TARGET) gmon.out > analysis100.txt



clean:
	rm -f $(TARGET) *.o gmon.out analysis.txt

.PHONY: all run gprof clean
