#include <iostream>
#include <cstring>
#include <string>
#include <getopt.h>
#include <unistd.h>
#include "arfeed.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

using namespace std;



class Connection
{
	public:
		int socket_id;
		Command MyCommand;
		Connection(Command);
		void ConnectionCreate();
		void OpenSSL();
		void TCPcreate();
};

Connection::Connection(Command MyCommand)
{
this->MyCommand=MyCommand;
}
void Connection::ConnectionCreate()
{

/// Firstly establish TCP/IP connetion 

/*create socket*/
struct hostent *server;
struct sockaddr_in server_struct;


server = gethostbyname (SERVER);




if((socket_id=socket (AF_INET, SOCK_STREAM, 0))==-1)
{
      fprintf(stderr, "Cannot create socket");
      socket_id = 0;
}

else
{
	  server_struct.sin_family = AF_INET;
      server_struct.sin_port = htons (PORT);

      server_struct.sin_addr = *((struct in_addr *) server->h_addr);
      bzero (&(server_struct.sin_zero), 8);

      if((connect (socket_id, (struct sockaddr *) &server_struct,sizeof (struct sockaddr)))==-1 )
        {
          fprintf(stderr, "Cannot connect");
          socket_id = 0;
        }
}

}


int main(int argc , char *argv[])
{
  int c;
  Command MyCommand;
  Connection Atom(MyCommand);

  while ((c = getopt (argc, argv, "f:c:C:ITau")) != -1)
    switch (c)
    {
	      case 'f':
	      	Atom.MyCommand.fargv=new  char[strlen(optarg)];
	        break;
	      case 'c':
	      	Atom.MyCommand.cargv=new  char[strlen(optarg)];
	        break;
	      case 'C':
	        Atom.MyCommand.Cargv=new  char[strlen(optarg)];
	        break;
		  case 'I':
		  	Atom.MyCommand.Iflag=true;
		    break;
		  case 'T':
		  	Atom.MyCommand.Tflag=true;
		    break;
		  case 'a':
		  	Atom.MyCommand.aflag=true;
		    break;
		  case 'u':
		 	Atom.MyCommand.uflag=true;
		    break;
	      case '?':
	        if ((optopt == 'f')  || (optopt == 'c')|| (optopt == 'C'))
	          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
	        else if (isprint (optopt))
	          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
	        else
	          fprintf (stderr,
	                   "Unknown option character `\\x%x'.\n",optopt);
	        return 1;
	      default:
	      		fprintf (stderr, "Wrong Input");
	      		return 0;
    }
if(optind+1!=argc)
{
	fprintf(stderr,"Wrong input \n");
	return 0;
}
// Saving URL address 
Atom.MyCommand.Url=new char[strlen(argv[optind])];
Atom.MyCommand.Url=argv[optind];
Atom.ConnectionCreate();





  return 0;
}