CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -Iinclude
SRC      = src/Conge.cpp src/GestionnaireConges.cpp src/GestionnaireFichiersConge.cpp src/MenuConge.cpp

all: gestion_conges test_conges

gestion_conges: main.cpp $(SRC)
	$(CXX) $(CXXFLAGS) main.cpp $(SRC) -o $@

test_conges: test_conges.cpp $(SRC)
	$(CXX) $(CXXFLAGS) test_conges.cpp $(SRC) -o $@

run: gestion_conges
	./gestion_conges

test: test_conges
	./test_conges

clean:
	rm -f gestion_conges test_conges gestion_conges.exe test_conges.exe

.PHONY: all run test clean
