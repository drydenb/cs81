# A simple makefile to compile the gtp protocol and board 
# representation
CC = g++
CFLAGS = -std=c++11 -Wall -g 
EXEC = izumi 
LOGFILE = logfile.txt 

all: $(EXEC)

$(EXEC): gtp.o board.o capture.o
	$(CC) $(CFLAGS) -o $(EXEC) capture.o gtp.o board.o  

gtp.o: gtp.cpp gtp.hpp
	$(CC) $(CFLAGS) -c gtp.cpp 

board.o: board.cpp board.hpp
	$(CC) $(CFLAGS) -c board.cpp  

capture.o: capture.cpp capture.hpp
	$(CC) $(CFLAGS) -c capture.cpp 

clean: 
	rm -f $(EXEC) *.o  
	rm -f $(LOGFILE)
