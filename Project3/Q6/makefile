CXX        = g++
CXXFLAGS   = -Wall -std=c++14 -pthread
INCLUDES   = -I../Q5         # הוספת נתיב לחיפוש reactor.h
LIBS       = ../Q5/libreactor.a
TARGET     = server_reactor
SRCS       = server_reactor.cpp

all: $(TARGET)

$(TARGET): $(SRCS) ../Q5/libreactor.a
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(SRCS) $(LIBS)

clean:
	rm -f $(TARGET)
