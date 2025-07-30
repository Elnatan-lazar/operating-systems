CXX = g++
CXXFLAGS = -Wall -std=c++11 -pthread -I../Q8

all: server_proactor

server_proactor: server_proactor.cpp ../Q8/proactor.cpp
	$(CXX) $(CXXFLAGS) -o $@ server_proactor.cpp ../Q8/proactor.cpp

clean:
	rm -f server_proactor
