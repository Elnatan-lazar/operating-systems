CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -shared
TARGET_LIB = libmandelbrot.so
HEADER = mandelbrot.h

all: dynamic test_runner

# יצירת ספריה דינמית
dynamic: mandelbrot.o
	$(CC) $(LDFLAGS) -o $(TARGET_LIB) mandelbrot.o

mandelbrot.o: mandelbrot.c $(HEADER)
	$(CC) $(CFLAGS) -c mandelbrot.c

# תוכנית שמשתמשת בספריה
test_runner: main.c $(HEADER)
	$(CC) -o test_runner main.c -L. -lmandelbrot -lm

# ניקוי
clean:
	rm -f *.o *.so test_runner
