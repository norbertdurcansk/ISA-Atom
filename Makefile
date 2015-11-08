CC = g++ 
CFLAGS  = -std=c++11 -finput-charset=UTF-8 -Wall -Wextra -pedantic 

default: arfeed

arfeed:  arfeed.o RSSparser.o
	@$(CC) $(CFLAGS) -L/usr/include/libxml2  -I/usr/include/libxml2  arfeed.o RSSparser.o -o arfeed -lssl -lcrypto  -lxml2 -lm 

arfeed.o:   arfeed.cpp 
	@$(CC) $(CFLAGS) -L/usr/include/libxml2 -I/usr/include/libxml2   -c arfeed.cpp   -lssl -lcrypto  -lxml2 -lm 

RSSparser.o: RSSparser.cpp 
	@$(CC) $(CFLAGS) -L/usr/include/libxml2 -I/usr/include/libxml2   -c RSSparser.cpp  -lssl -lcrypto  -lxml2 -lm 

clean: 
	@$(RM) arfeed *.o *~

