CC = g++ 
CFLAGS  = -std=c++11 -Wall -Wextra -pedantic 

default: arfeed

arfeed:  arfeed.o 
	$(CC) $(CFLAGS) -L/usr/include/libxml2  -I/usr/include/libxml2  arfeed.o -o arfeed -lssl -lcrypto  -lxml2 -lm

arfeed.o:   arfeed.cpp arfeed.hpp
	$(CC) $(CFLAGS) -L/usr/include/libxml2 -I/usr/include/libxml2   -c arfeed.cpp arfeed.hpp  -lssl -lcrypto  -lxml2 -lm

clean: 
	$(RM) arfeed *.o *~

