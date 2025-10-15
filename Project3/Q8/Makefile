# make

CXX       = g++
CXXFLAGS  = -Wall -std=c++11 -pthread
LIB_PATH  = libproactor.a
PROACTOR_OBJ = proactor.o

all: $(LIB_PATH)

$(LIB_PATH): $(PROACTOR_OBJ)
	ar rcs $@ $^

proactor.o: proactor.cpp ../Q5/reactor.h
	$(CXX) $(CXXFLAGS) -c proactor.cpp

clean:
	rm -f *.o $(LIB_PATH)