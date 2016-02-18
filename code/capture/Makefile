# A simple makefile to compile the gtp protocol and board 
# representation
CC = g++
CFLAGS = -std=c++11 -Wall -g 
EXEC = capture
all: $(EXEC)

$(EXEC): capture.o
	$(CC) $(CFLAGS) -o capture capture.o

capture.o: capture.cpp capture.hpp
	$(CC) $(CFLAGS) -c capture.cpp

# board.o: board.cpp board.hpp
# 	$(CC) $(CFLAGS) -c board.cpp  

clean: 
	rm -f $(EXEC) *.o  
