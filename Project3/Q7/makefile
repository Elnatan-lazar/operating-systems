# make run
# another WSL - make client
# EXIT --> at the clients and the server

CXX        = g++
CXXFLAGS   = -std=c++11 -Wall -pthread
PORT       = 9034

TARGET = server_threaded
SRC = server_threaded.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

client:
	@echo "Connecting to localhost on port $(PORT)..."
	nc localhost $(PORT)

clean:
	rm -f $(TARGET)
