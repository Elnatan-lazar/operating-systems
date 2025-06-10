CXX       = g++
CXXFLAGS  = -Wall -std=c++11 -pthread
TARGET    = example_main
SRC       = reactor.cpp example_main.cpp

all: $(TARGET)

$(TARGET): $(SRC) reactor.h
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
