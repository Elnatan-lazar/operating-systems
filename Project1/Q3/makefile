CC = gcc
CFLAGS = -Wall -std=c99 -fPIC
LDFLAGS = -lm -lmandelbrot
LIB_NAME = libmandelbrot.so
LIB_OBJ = mandelbrot.o
TARGET = main

.PHONY: all clean run

all: $(LIB_NAME) $(TARGET)

# Compile mandelbrot.o with -fPIC for shared lib
$(LIB_OBJ): mandelbrot.c mandelbrot.h
	$(CC) $(CFLAGS) -c mandelbrot.c -o $(LIB_OBJ)

# Create shared object
$(LIB_NAME): $(LIB_OBJ)
	$(CC) -shared -o $(LIB_NAME) $(LIB_OBJ)

# Compile main program and link with shared lib
$(TARGET): main.c mandelbrot.h
	$(CC) -Wall -std=c99 -o $(TARGET) main.c -L. -lmandelbrot -lm

run: all
	LD_LIBRARY_PATH=. ./$(TARGET)


clean:
	rm -f *.o *.so $(TARGET)
