#ifndef ARFEED_HPP
#define ARFEED_HPP

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string>
#include <iostream>
#define HTTP ":http"
#define HTTPS ":https"
#define DEFAULTDIR "/etc/ssl/certs"



/**
HEADER FILE
*/

/**
Program commandline structure for ATOM-file requests
*/
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
int port=0;
std::string file="";
std::string protocol;
std::string fargv="";
std::string Cargv="";
std::string cargv="";
}Command;


#endif
