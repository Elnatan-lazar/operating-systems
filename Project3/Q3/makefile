CXX = g++
CXXFLAGS = -Wall -std=c++11
TARGET = interactive_ch
SRC = interactive_ch.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run:
	./$(TARGET)

clean:
	rm -f $(TARGET)
