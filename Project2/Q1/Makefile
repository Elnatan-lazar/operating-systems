CXX = g++
CXXFLAGS = -Wall -g -fprofile-arcs -ftest-coverage

all: atom_warehouse supplier_atom

atom_warehouse: atom_warehouse.cpp
	$(CXX) $(CXXFLAGS) -o atom_warehouse atom_warehouse.cpp

supplier_atom: supplier_atom.cpp
	$(CXX) $(CXXFLAGS) -o supplier_atom supplier_atom.cpp

clean:
	rm -f atom_warehouse supplier_atom *.gcda *.gcno *.gcov coverage.info -r out

.PHONY: all clean