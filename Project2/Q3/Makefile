all: drinks_bar client_bar

drinks_bar: drinks_bar.cpp
	g++ -Wall -o drinks_bar drinks_bar.cpp

client_bar: client_bar.cpp
	g++ -Wall -o client_bar client_bar.cpp

clean:
	rm -f drinks_bar client_bar
.PHONY: all clean