CXX        = g++
CXXFLAGS   = -Wall -std=c++14 -pthread
INCLUDES   = -I../Q5         # The path of reactor.h
LIBS       = ../Q5/libreactor.a
TARGET     = server_reactor
SRCS       = server_reactor.cpp

all: $(TARGET)

$(TARGET): $(SRCS) $(LIBS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(SRCS) $(LIBS)

clean:
	rm -f $(TARGET)
