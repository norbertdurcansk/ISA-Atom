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


// structure for output 
typedef struct entry
{
std::string type="";
std::string title="";
std::string url="";
std::string	update="";
std::string author="";
}entry;

//url parsed structure 
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

void HtmlTagremover(entry **);
// main object used in our program 
class Connection
{
	public:
		BIO * bio; //bio struct ssl
    	SSL * ssl; // ssl struct ssl
    	SSL_CTX * ctx; //ctx context ssl
		int line_counter; // su variable for feedfile parsing 
		Command MyCommand; // structure described above 
		string Feed; // Feed in string format 
		xmlDocPtr doc; // pointer on doc 
		entry *entryarr; // array of output entries 
		string Feedtype; // type of the feed 
		int Error_number=0; // errors 

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
