
CXX = g++
CXXFLAGS = -Wall -std=c++11 -pthread

all: libproactor.a

libproactor.a: proactor.o
	ar rcs $@ $^

proactor.o: proactor.cpp proactor.h
	$(CXX) $(CXXFLAGS) -c proactor.cpp

clean:
	rm -f *.o *.a
