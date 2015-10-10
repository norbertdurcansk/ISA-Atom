#ifndef ARFEED_HPP
#define ARFEED_HPP

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define SERVER  "tools.ietf.org"
#define PORT 443


/**
HEADER FILE
*/

/**
Program commandline structure for ATOM-file requests
*/
typedef struct Command
{
char *Url=0;	
bool Iflag=false;
bool Tflag=false;
bool aflag=false;
bool uflag=false;
char *fargv=0;
char *Cargv=0;
char *cargv=0;
}Command;

typedef struct {
    int socket;
    SSL *sslHandle;
    SSL_CTX *sslContext;
} OpenSSL;


#endif
