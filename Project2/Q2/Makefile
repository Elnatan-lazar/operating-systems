all: supplier_molecule requester_molecule

supplier_molecule: supplier_molecule.cpp
	g++ -Wall -o supplier_molecule supplier_molecule.cpp

requester_molecule: requester_molecule.cpp
	g++ -Wall -o requester_molecule requester_molecule.cpp

clean:
	rm -f supplier_molecule requester_molecule
