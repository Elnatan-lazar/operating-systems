CXX       = g++
CXXFLAGS  = -Wall -std=c++11 -pthread
TARGET    = server_proactor

INCLUDES  = -I../Q8
LIBS      = -L../Q5 -lreactor

all: $(TARGET)

$(TARGET): server_proactor.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(LIBS) -o $@

clean:
	rm -f $(TARGET)