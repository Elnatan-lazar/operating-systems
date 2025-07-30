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

help:
	@echo "Available commands:"
	@echo "  Newgraph n      - create a new graph and enter n points"
	@echo "  Newpoint x,y    - add a new point"
	@echo "  Removepoint x,y - remove a point"
	@echo "  CH              - compute convex hull and print area"
	@echo "  EXIT            - disconnect from server"

clean:
	rm -f $(TARGET)
