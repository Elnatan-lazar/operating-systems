CXX = g++
CXXFLAGS = -Wall -std=c++11 -fprofile-arcs -ftest-coverage
LDFLAGS = -lgcov

all: drinks_bar supplier_atom supplier_molecule

drinks_bar: drinks_bar.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

supplier_atom: supplier_atom.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

supplier_molecule: supplier_molecule.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f drinks_bar supplier_atom supplier_molecule *.gcno *.gcda *.gcov

.PHONY: all clean
