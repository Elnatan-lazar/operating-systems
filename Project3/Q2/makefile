CXX = g++
CXXFLAGS = -Wall -std=c++11
TARGET = ch_profile
SRC = ch_profile.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) timing_results.txt
