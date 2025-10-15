# Q5 - make
# Q6:
# make run
# another WSL - make client
# EXIT
# Ctrl+C

CXX        = g++
CXXFLAGS   = -Wall -std=c++14 -pthread
INCLUDES   = -I../Q5        
LIBS       = ../Q5/libreactor.a
TARGET     = server_reactor
SRCS       = server_reactor.cpp
PORT       = 9034

all: $(TARGET)

$(TARGET): $(SRCS) $(LIBS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(SRCS) $(LIBS)

run: $(TARGET)
	./$(TARGET)

client:
	@echo "Connecting to localhost on port $(PORT)..."
	nc localhost $(PORT)

clean:
	rm -f $(TARGET)
