all: bar_drinks client_bar

bar_drinks: bar_drinks.cpp
	g++ -Wall -o bar_drinks bar_drinks.cpp

client_bar: client_bar.cpp
	g++ -Wall -o client_bar client_bar.cpp

clean:
	rm -f bar_drinks client_bar
.PHONY: all clean