#ifndef ARFEED_HPP
#define ARFEED_HPP

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <ctype.h>
#include <iostream>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#define HTTP ":http"
#define HTTPS ":https"
#define DEFAULTDIR "/etc/ssl/certs"

using namespace std;

typedef struct entry
{
std::string type="";
std::string title="";
std::string url="";
std::string	update="";
std::string author="";
}entry;

typedef struct Command
{
std::string Url="";	
bool Iflag=false;
bool Tflag=false;
bool aflag=false;
bool uflag=false;
std::string server="";
std::string port="";
std::string file="";
std::string protocol;
std::string fargv="";
std::string Cargv="";
std::string cargv="";
}Command;

int News(entry *);


class Connection
{
	public:
		BIO * bio;
    	SSL * ssl;
    	SSL_CTX * ctx;
		int socket_id;
		int line_counter;
		Command MyCommand;
		string Feed;
		xmlDocPtr doc;
		entry *entryarr;
		string Feedtype;

		Connection(Command);
		bool ConnectionCreate(char **,int);
		bool URLparser();
		bool ArgumentParser(int  , char **);
		string  FeedFileParser();
		bool SSLdownload();
		bool TCPdownload();
		bool Feedparser();
		bool Parse(xmlNodePtr,int*,bool);
};

/**
HEADER FILE
*/

/**
Program commandline structure for ATOM-file requests
*/
#endif
