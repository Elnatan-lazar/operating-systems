CXX = g++
CXXFLAGS = -Wall -std=c++11
TARGET = convex_hull
SRC = convex_hull.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Run using input from file (default: input.txt)
run-file: $(TARGET)
	@echo "Running with input from input.txt"
	./$(TARGET) < input.txt

# Run with manual console input (interactive)
run-console: $(TARGET)
	@echo "Enter input manually (Ctrl+D to end):"
	./$(TARGET)

clean:
	rm -f $(TARGET)
