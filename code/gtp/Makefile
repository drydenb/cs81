# A simple makefile to compile the gtp protocol and board 
# representation
CC = g++
CFLAGS = -std=c++11 -Wall 
EXEC = gtp 
all: $(EXEC)

$(EXEC): gtp.o board.o 
	$(CC) $(CFLAGS) -o gtp gtp.o board.o 

gtp.o: gtp.cpp gtp.hpp
	$(CC) $(CFLAGS) -c gtp.cpp 

board.o: board.cpp board.hpp
	$(CC) $(CFLAGS) -c board.cpp  

clean: 
	rm -f $(EXEC) *.o  
