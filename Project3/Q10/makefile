CXX = g++
CXXFLAGS = -Wall -std=c++11 -pthread -I../Q8

all: server

server: server.cpp ../Q8/proactor.cpp
	$(CXX) $(CXXFLAGS) -o $@ server.cpp ../Q8/proactor.cpp

clean:
	rm -f server
