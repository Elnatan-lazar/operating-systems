CXX       = g++
CXXFLAGS  = -Wall -std=c++11 -pthread
LIB       = libreactor.a
OBJS      = reactor.o

all: $(LIB)

reactor.o: reactor.cpp reactor.h
	$(CXX) $(CXXFLAGS) -c reactor.cpp

$(LIB): $(OBJS)
	ar rcs $(LIB) $(OBJS)

clean:
	rm -f $(OBJS) $(LIB)
