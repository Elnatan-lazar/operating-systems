CXX = g++
CXXFLAGS = -std=c++11 -Wall -pthread

TARGET = server_threaded
SRC = server_threaded.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)
