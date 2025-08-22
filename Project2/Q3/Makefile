CXXFLAGS = -Wall -fprofile-arcs -ftest-coverage
LDFLAGS = -lgcov

all: drinks_bar client_bar

drinks_bar: drinks_bar.cpp
	g++ $(CXXFLAGS) -o drinks_bar drinks_bar.cpp $(LDFLAGS)

client_bar: client_bar.cpp
	g++ $(CXXFLAGS) -o client_bar client_bar.cpp $(LDFLAGS)

clean:
	rm -f drinks_bar client_bar *.gcno *.gcda *.gcov

.PHONY: all clean