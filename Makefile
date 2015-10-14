CC = g++ 
CFLAGS  = -std=c++11 -Wall -Wextra -pedantic 

default: arfeed

arfeed:  arfeed.o
	$(CC) $(CFLAGS)  arfeed.o -o arfeed -lssl -lcrypto  

arfeed.o:  arfeed.cpp arfeed.hpp
	$(CC) $(CFLAGS) -c arfeed.cpp arfeed.hpp -lssl -lcrypto 

clean: 
	$(RM) arfeed *.o *~

