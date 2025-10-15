# make run
# another WSL - make client
# EXIT --> at the clients and the server

CXX = g++
CXXFLAGS = -Wall -std=c++11
TARGET = server
SRC = server.cpp
PORT = 9034

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	@echo "Running TCP server on port $(PORT)..."
	./$(TARGET)

client:
	@echo "Connecting to localhost on port $(PORT)..."
	nc localhost $(PORT)

clean:
	rm -f $(TARGET)
