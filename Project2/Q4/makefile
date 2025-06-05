#all: drinks_bar supplier_atom supplier_molecule

#drinks_bar: drinks_bar.cpp
#	g++ -Wall -std=c++11 -o drinks_bar drinks_bar.cpp

#supplier_atom: supplier_atom.cpp
#	g++ -Wall -std=c++11 -o supplier_atom supplier_atom.cpp

#supplier_molecule: supplier_molecule.cpp
#	g++ -Wall -std=c++11 -o supplier_molecule supplier_molecule.cpp

#clean:
#	rm -f drinks_bar supplier_atom supplier_molecule

CXXFLAGS = -Wall -std=c++11 -fprofile-arcs -ftest-coverage
LDFLAGS = -lgcov

all: drinks_bar supplier_atom supplier_molecule

drinks_bar: drinks_bar.cpp
	g++ $(CXXFLAGS) -o drinks_bar drinks_bar.cpp $(LDFLAGS)

supplier_atom: supplier_atom.cpp
	g++ -Wall -std=c++11 -o supplier_atom supplier_atom.cpp

supplier_molecule: supplier_molecule.cpp
	g++ -Wall -std=c++11 -o supplier_molecule supplier_molecule.cpp

clean:
	rm -f drinks_bar supplier_atom supplier_molecule *.gcno *.gcda *.gcov

.PHONY: all clean