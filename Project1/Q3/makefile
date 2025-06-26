CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -shared
TARGET_LIB = libmandelbrot.so
HEADER = mandelbrot.h

all: dynamic main

# יצירת ספריה דינמית
dynamic: mandelbrot.o
	$(CC) $(LDFLAGS) -o $(TARGET_LIB) mandelbrot.o

mandelbrot.o: mandelbrot.c $(HEADER)
	$(CC) $(CFLAGS) -c mandelbrot.c

# קימפול של הקובץ הקורא לספרייה
main: main.c $(HEADER)
	$(CC) -o main main.c -L. -lmandelbrot -lm

# הרצה עם הגדרת משתנה ספרייה
run: main
	LD_LIBRARY_PATH=. ./main

# ניקוי
clean:
	rm -f *.o *.so main
