# Makefile for drinks_bar project

CXX = g++
CXXFLAGS = -Wall -std=c++11

all: drinks_bar supplier_atom supplier_molecule

drinks_bar: drinks_bar.cpp
	$(CXX) $(CXXFLAGS) -o drinks_bar drinks_bar.cpp

supplier_atom: supplier_atom.cpp
	$(CXX) $(CXXFLAGS) -o supplier_atom supplier_atom.cpp

supplier_molecule: supplier_molecule.cpp
	$(CXX) $(CXXFLAGS) -o supplier_molecule supplier_molecule.cpp

clean:
	rm -f drinks_bar supplier_atom supplier_molecule
