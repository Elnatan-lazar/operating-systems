# Q8 - make
# Q9:
# make run
# another WSL - make client
# EXIT --> at the clients and the server

CXX       = g++
CXXFLAGS  = -Wall -std=c++11 -pthread
TARGET    = server

INCLUDES  = -I../Q8
LIBS      = -L../Q8 -lproactor
PORT      = 9034

all: $(TARGET)

$(TARGET): server.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(LIBS) -o $@

run: $(TARGET)
	./$(TARGET)

client:
	@echo "Connecting to localhost on port $(PORT)..."
	nc localhost $(PORT)

clean:
	rm -f $(TARGET)
