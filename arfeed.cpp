#include <iostream>
#include <cstring>
#include <string>
#include <getopt.h>
#include <unistd.h>
#include "arfeed.hpp"
#include <fstream>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
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
		int line_counter;
		Command MyCommand;
		string Feed;
		xmlDocPtr doc;
		entry *entryarr;

		Connection(Command);
		bool ConnectionCreate(char **,int);
		bool URLparser();
		bool ArgumentParser(int  , char **);
		string  FeedFileParser();
		bool SSLdownload();
		bool TCPdownload();
		bool Feedparser();
		bool Parse(xmlNodePtr,int*,bool,string);
};
Connection::Connection(Command MyCommand)
{


this->MyCommand=MyCommand;
this->line_counter=0;
}



bool Connection::Parse(xmlNodePtr root,int *index, bool count,string name)
{
xmlNodePtr current;
xmlChar* contt;
int i=*index;


if(count)
{
	for(current=root->children;current!=NULL;current=current->next)
	{	
		if ( !(xmlStrcmp ( current->name, ( const xmlChar * ) "entry" ) ) )  
			(*index)++;

		if(current->type==XML_ELEMENT_NODE)
		{
			Parse(current,index,count,name);
		}
	}

}
else
{
	for(current=root->children;current!=NULL;current=current->next)
	{
		


		if(current->type==XML_ELEMENT_NODE)
		{


		if ( !(xmlStrcmp ( current->name, ( const xmlChar * ) "entry" ) ) ) 
			(*index)++;


		i=*index;
		if ( !(xmlStrcmp ( current->name, ( const xmlChar * ) "id" ) ) ) 
		{	contt=xmlNodeListGetString(doc,current->xmlChildrenNode,1);
			entryarr[i].url=(char*)contt;
			xmlFree(contt);
		}
				if ( !(xmlStrcmp ( current->name, ( const xmlChar * ) "title" ) ) ) 
		{	contt=xmlNodeListGetString(doc,current->xmlChildrenNode,1);
			entryarr[i].title=(char*)contt;
			xmlFree(contt);
		}
				if ( !(xmlStrcmp ( current->name, ( const xmlChar * ) "name" ) ) ) 
		{	contt=xmlNodeListGetString(doc,current->xmlChildrenNode,1);
			entryarr[i].author=(char*)contt;
			xmlFree(contt);
		}
				if ( !(xmlStrcmp ( current->name, ( const xmlChar * ) "updated" ) ) ) 
		{	contt=xmlNodeListGetString(doc,current->xmlChildrenNode,1);
			entryarr[i].update=(char*)contt;
			xmlFree(contt);
		}

		if((*index)==0)
			entryarr[i].type="feed";
		else
			entryarr[i].type="entry";

		//	printf("%s\n",current->name);
			Parse(current,index,count,name);



		}
	}
}

return true;
}


bool Connection::Feedparser()
{
if(Feed.find("HTTP/1.1 200 OK")!=0)
	return false;

//remove the header
Feed=Feed.substr(Feed.find("\r\n\r\n"));
Feed.erase(Feed.begin(),Feed.begin()+4);


  doc = xmlParseMemory(Feed.c_str(),Feed.length());

  if (doc == NULL) 
        fprintf(stderr,"error: could not parse document\n");
  

  xmlNodePtr root;
 
  root = xmlDocGetRootElement(doc);
  if(root==NULL)
  	return false;

printf("\n\n\n\n\n");

int pocet=0;
string name="";
//give me number of entries to create 
Parse(root,&pocet,true,name);
entryarr=new entry[pocet+5];
int index=0;
Parse(root,&index,false,name);






int x=0;
while(entryarr[x].type!="")
{
printf("%s type\n",entryarr[x].type.c_str() );	
printf("%s title\n",entryarr[x].title.c_str() );
//printf("%s author\n",entryarr[x].author.c_str() );
if(entryarr[x].update!="")
printf("%s update \n",entryarr[x].update.c_str() );
printf("%s url \n\n",entryarr[x].url.c_str() );
	x++;
}


	return true;
}








bool Connection::TCPdownload()
{
	BIO * bio;
    int p;
	string link;
	printf("%s %s\n",MyCommand.file.c_str(),MyCommand.server.c_str() );
	link="GET /"+MyCommand.file+" HTTP/1.1\r\nHost: "+MyCommand.server+"\r\nUser-Agent: OpenSSL\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nAccept-Language: en-US,en;q=0.5\r\nConnection: Close\r\nCache-Control: max-age=0\r\n\r\n";
   
    char r[1024];
    /* Set up the library */
    ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();


        /* Create and setup the connection */
    string hey=MyCommand.server+":80";
    char* con = new char[hey.length() + 1];
    copy(hey.begin(), hey.end(), con);


    bio = BIO_new_connect(con);
    if(bio == NULL) 
    {
    	 fprintf(stderr,"Cannot create bio structure\n"); 
    	 return false; 
    }

    if(BIO_do_connect(bio) <= 0)
    {
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        return false;
    }

    BIO_write(bio, link.c_str(), strlen(link.c_str()));

    for(;;)
    {
        p = BIO_read(bio, r, 1023);
        if(p <= 0) break;
        r[p] = 0;
        Feed+=string(r);
       
    }
 
    BIO_free_all(bio);
   if(Feedparser())
		return true;

return false;

}

bool Connection::SSLdownload()
{

    
    BIO * bio;
    SSL * ssl;
    SSL_CTX * ctx;

    int p;

	string link;
	link="GET /"+MyCommand.file+" HTTP/1.1\r\nHost: "+MyCommand.server+"\r\nUser-Agent: OpenSSL\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nAccept-Language: en-US,en;q=0.5\r\nConnection: Close\r\nCache-Control: max-age=0\r\n\r\n";
    char r[1024];
    /* Set up the library */
    SSL_library_init();
    ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();

   /* Create and setup the connection */
    string hey=MyCommand.server+":443";
    char* con = new char[hey.length() + 1];
    copy(hey.begin(), hey.end(), con);

    /* Set up the SSL context */

  ctx = SSL_CTX_new(SSLv23_client_method());

  if(! SSL_CTX_load_verify_locations(ctx,NULL,"/etc/ssl/certs"))
    {
        fprintf(stderr, "Error loading trust store\n");
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return 0;
    }

    /* Setup the connection */
    bio = BIO_new_ssl_connect(ctx);
    /* Set the SSL_MODE_AUTO_RETRY flag */

    BIO_get_ssl(bio, & ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    /* Create and setup the connection */

    BIO_set_conn_hostname(bio,con);

    if(BIO_do_connect(bio) <= 0)
    {
        fprintf(stderr, "Error attempting to connect\n");
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        return 0;
    }
    //SSL_set_connect_state(ssl); 

	SSL_do_handshake(ssl); 

    /* Check the certificate */

    if(SSL_get_verify_result(ssl) != X509_V_OK)
    {
        fprintf(stderr, "Certificate verification error: %i\n", SSL_get_verify_result(ssl));
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        return 0;
    }

    /* Send the request */
    BIO_write(bio, link.c_str(), strlen(link.c_str()));

    /* Read in the response */

    for(;;)
    {
        p = BIO_read(bio, r, 1023);
        if(p <= 0) break;
        r[p] = 0;
        Feed+=string(r);

    }
    
    /* Close the connection and free the context */

    BIO_free_all(bio);
    SSL_CTX_free(ctx);

   if(Feedparser())
		return true;
	
return false;
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
int ret;
if(MyCommand.protocol==HTTP)
	ret=TCPdownload();
else
	ret=SSLdownload();
	

if(ret)
	return true;
else
	return false;
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
	}else MyCommand.protocol="";
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

	if((MyCommand.port==80 && MyCommand.protocol==HTTPS) || (MyCommand.port==443 && MyCommand.protocol==HTTP) || (MyCommand.port!=443 && MyCommand.port!=80 ))
		return false;
	if(MyCommand.port==80 && MyCommand.protocol=="")
		MyCommand.protocol=HTTP;
	if(MyCommand.port==443 && MyCommand.protocol=="")
		MyCommand.protocol=HTTPS;
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
if((optind+1!=argc and MyCommand.fargv.empty()) || (!MyCommand.cargv.empty() and !MyCommand.Cargv.empty()))
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
