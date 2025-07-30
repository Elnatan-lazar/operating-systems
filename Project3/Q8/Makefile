CXX       = g++
CXXFLAGS  = -Wall -std=c++11 -pthread
LIB_PATH  = ../Q5/libreactor.a
PROACTOR_OBJ = proactor.o

all: $(LIB_PATH)

$(LIB_PATH): $(PROACTOR_OBJ)
	ar rcs $@ $^

proactor.o: proactor.cpp ../Q5/reactor.h
	$(CXX) $(CXXFLAGS) -c proactor.cpp

clean:
	rm -f *.o