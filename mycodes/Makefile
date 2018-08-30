CFLAGS =-ggdb3 -std=gnu++11 -pthread -pedantic -Wall -Werror
CC = g++
FLAGS=-I/usr/lib/x86_64-linux-gnu/
EXTRAFLAGS=-lpqxx -lpq

all: myServer

myServer: main.cpp main.h database.cpp database.h parser.cpp parser.h
	$(CC) $(CFLAGS) -o myServer main.cpp database.cpp parser.cpp $(EXTRAFLAGS)

.PHONY: clean
clean:
	rm -f *.o myServer
clobber:
	rm -f *~ *.o
