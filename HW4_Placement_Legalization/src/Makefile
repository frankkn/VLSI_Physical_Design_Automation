CC = g++
OPT = -O3
LIBS = -std=c++17
UDLIBS = ./ArgParser/ArgParser.cc ./Abacus/Abacus.cc ./Module/Module.cc ./OutputWriter/OutputWriter.cc ./Clock/Clock.cc

all: main

main: main.cpp
	rm -f ../bin/hw4
	rm -f ../output/*
	$(CC) -o ../bin/hw4 main.cpp $(UDLIBS) $(OPT) $(LIBS)

clean:
	rm -f ../bin/hw4
	rm -f ../output/*