#include <iostream>
#include <cstring>
#include <string>
#include <getopt.h>
#include <unistd.h>
#include "arfeed.hpp"
#include <fstream>

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>

using namespace std;

class Connection
{
	public:
		BIO * bio;
    	SSL * ssl;
    	SSL_CTX * ctx;
		int socket_id;
		//load new file for each call 
		int line_counter;
		Command MyCommand;




		Connection(Command);
		bool ConnectionCreate(char **,int);
		bool URLparser();
		bool ArgumentParser(int  , char **);
		string  FeedFileParser();
		bool SSLdownload();

};


Connection::Connection(Command MyCommand)
{
this->MyCommand=MyCommand;
this->line_counter=0;
}
bool Connection::SSLdownload()
{
	SSL_library_init();
	ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();


	ctx = SSL_CTX_new(SSLv23_client_method());

	if (!SSL_CTX_load_verify_locations(ctx, NULL, "/home/norbert/Desktop/certs"))
    {
        fprintf(stderr, "Error loading trust store\n");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return false;
    }

    bio = BIO_new_ssl_connect(ctx);
   BIO_get_ssl(bio, & ssl);
   SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

	bio = BIO_new_connect("www.theregister.co.uk:https");

    if(BIO_do_connect(bio) <= 0)
    {
        fprintf(stderr, "Error attempting to connect\n");
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        return false;
    }

if(SSL_get_peer_certificate(ssl) != NULL){
    if(SSL_get_verify_result(ssl) != X509_V_OK){
        std::cout << "error no = "<< std::endl;
    }
}
int p;
//HTTP/1.1\x0D\x0AHost: www.theregister.co.uk\x0D\x0A\x0D\x0A
    char * request = "GET https://www.theregister.co.uk:443/hardware/headlines.atom \x0D\x0AHost: www.theregister.co.uk\x0D\x0AConnection: keep-alive\x0D\x0AAccept-Encoding: gzip, deflate\x0D\x0ACookie: __cfduid=dcc1a6a8acbb4efd12442585d8ba257c81444509763; __gads=ID=ccfae09049777e55:T=1444509769:S=ALNI_MZAE0a2O8wXpyx35UH0_WT03Pb3dQ\x0D\x0A\x0D\x0A";
    char r[1024];


BIO_write(bio, request, strlen(request));
for(;;)
    {
        p = BIO_read(bio, r, 1023);
        if(p <= 0) break;
        r[p] = 0;
        printf("%s", r);

    }
printf("tu sme \n");
    /* Close the connection and free the context */

    BIO_free_all(bio);
    SSL_CTX_free(ctx);

	return true;
}

bool Connection::ConnectionCreate(char *argv[],int optind)
{ 
	if(MyCommand.fargv.empty())
	{
	// Saving URL address 
	MyCommand.Url=argv[optind];
	if(!URLparser())
		return false;
	}
	else
	{
	//get the first address from feedfile
	MyCommand.Url=FeedFileParser();
	if(!URLparser())
		return false;
	}
	SSLdownload();
	return true;
}

string Connection::FeedFileParser()
{
if(MyCommand.fargv.empty() )
		return "";
try{	
ifstream file(MyCommand.fargv.c_str());
string line;
int counter=-1;
while( getline(file, line)){
counter++;	

if(line.empty()){
	line_counter++;
	continue;
}

if (counter==line_counter)
{
	if (line.at(0)=='#')
	{
		line_counter++;
		continue;
	}
	else{
		line_counter++;
		return line;
	}
}
}
}catch(...){fprintf(stderr, "Cannot open feedfile %s \n",MyCommand.fargv.c_str() );return "";}

return "EOL";
}

/**parsing URL address loading structure*/
bool Connection::URLparser()
{
string url=MyCommand.Url;

if(url.find("http://")==0)
{
	MyCommand.protocol=HTTP;
	url.erase(url.begin(),url.begin()+7);
}
else
{
	if(url.find("https://")==0)
	{
		MyCommand.protocol=HTTPS;
		url.erase(url.begin(),url.begin()+8);
	}else MyCommand.protocol=HTTP;
}
//get file from url
int pos; 
if((pos=url.find("/"))!=-1)
{
	MyCommand.file=url.substr(pos+1);
	url.erase(url.begin()+pos,url.end());
}else return false;

if((pos=url.find(":"))!=-1)
{
	MyCommand.port=atoi((url.substr(pos+1)).c_str());
	url.erase(url.begin()+pos,url.end());
	if((MyCommand.port==80 && MyCommand.protocol!=HTTP) || (MyCommand.port==443 && MyCommand.protocol!=HTTPS) || (MyCommand.port!=443 && MyCommand.port!=80 ))
		return false;
}
MyCommand.server=url;
return true;
}





bool Connection::ArgumentParser(int argc , char *argv[])
{
int c ;

while ((c = getopt (argc, argv, "f:c:C:ITau")) != -1)
    switch (c)
    {
	      case 'f':
	      	MyCommand.fargv=optarg;
	        break;
	      case 'c':
	      	MyCommand.cargv=optarg;
	        break;
	      case 'C':
	        MyCommand.Cargv=optarg;
	        break;
		  case 'I':
		  	MyCommand.Iflag=true;
		    break;
		  case 'T':
		  	MyCommand.Tflag=true;
		    break;
		  case 'a':
		  	MyCommand.aflag=true;
		    break;
		  case 'u':
		 	MyCommand.uflag=true;
		    break;
	      case '?':
	        if ((optopt == 'f')  || (optopt == 'c')|| (optopt == 'C'))
	          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
	        else if (isprint (optopt))
	          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
	        else
	          fprintf (stderr,
	                   "Unknown option character `\\x%x'.\n",optopt);
	        return false;
	      default:
	      		fprintf (stderr, "Wrong Input");
	      		return false;
    }
if(optind+1!=argc and MyCommand.fargv.empty())
{
	fprintf(stderr,"Wrong input \n");
	return false;
}

	return true;
}






int main(int argc , char *argv[])
{
	Command MyCommand;
	Connection Atom(MyCommand);
  	if(!Atom.ArgumentParser(argc,argv))
  		return -1; 

  	printf("%i \n",Atom.ConnectionCreate(argv,optind));

  	return 0;
}