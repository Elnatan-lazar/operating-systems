#all: supplier_molecule requester_molecule

#supplier_molecule: supplier_molecule.cpp
#	g++ -Wall -o supplier_molecule supplier_molecule.cpp

#requester_molecule: requester_molecule.cpp
#	g++ -Wall -o requester_molecule requester_molecule.cpp

#clean:
#	rm -f supplier_molecule requester_molecule

CXXFLAGS = -Wall -fprofile-arcs -ftest-coverage
LDFLAGS = -lgcov

all: supplier_molecule requester_molecule

supplier_molecule: supplier_molecule.cpp
	g++ $(CXXFLAGS) -o supplier_molecule supplier_molecule.cpp $(LDFLAGS)

requester_molecule: requester_molecule.cpp
	g++ $(CXXFLAGS) -o requester_molecule requester_molecule.cpp $(LDFLAGS)

clean:
	rm -f supplier_molecule requester_molecule *.gcno *.gcda *.gcov

.PHONY: all clean