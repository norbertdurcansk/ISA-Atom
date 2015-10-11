#ifndef ARFEED_HPP
#define ARFEED_HPP

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string>
#include <iostream>
#define HTTP 80
#define HTTPS 443

/**
HEADER FILE
*/

/**
Program commandline structure for ATOM-file requests
*/
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
int protocol;
std::string fargv="";
std::string Cargv="";
std::string cargv="";
}Command;


#endif
