CC = g++ 
CFLAGS  = -std=c++11 -Wall -Wextra -pedantic 

default: arfeed

arfeed:  arfeed.o Timefun.o
	@$(CC) $(CFLAGS) -L/usr/include/libxml2  -I/usr/include/libxml2  arfeed.o Timefun.o -o arfeed -lssl -lcrypto  -lxml2 -lm 

arfeed.o:   arfeed.cpp 
	@$(CC) $(CFLAGS) -L/usr/include/libxml2 -I/usr/include/libxml2   -c arfeed.cpp   -lssl -lcrypto  -lxml2 -lm 

Timefun.o: Timefun.cpp 
	@$(CC) $(CFLAGS) -L/usr/include/libxml2 -I/usr/include/libxml2   -c Timefun.cpp  -lssl -lcrypto  -lxml2 -lm 

clean: 
	@$(RM) arfeed *.o *~

