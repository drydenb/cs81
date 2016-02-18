# A makefile for the entire go engine 
CC = g++
CFLAGS = -std=c++11 -Wall -g
EXEC = izumi
all: 
	cd ../capture; make
	cd ../gtp; make
	$(EXEC)

$(EXEC): main.o ../capture/*.o ../gtp/*.o 
	$(CC) $(CFLAGS) -o izumi main.o

main.o: main.cpp 
	$(CC) $(CFLAGS) -c main.cpp

clean: 
	cd ../capture; make clean
	cd ../gtp; make clean
	rm -f $(EXEC) *.o  
