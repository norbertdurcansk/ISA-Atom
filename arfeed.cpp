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
		void OpenSSL();
		void TCPcreate();
		bool URLparser();
		bool ArgumentParser(int  , char **);
		string  FeedFileParser();
};
Connection::Connection(Command MyCommand)
{
this->MyCommand=MyCommand;
this->line_counter=0;
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
MyCommand.port=MyCommand.protocol;
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