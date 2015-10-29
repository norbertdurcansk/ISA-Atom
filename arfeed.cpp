#include <cstring>
#include <string>
#include <getopt.h>
#include <unistd.h>
#include "arfeed.hpp"
#include <fstream>

#include "RSSparser.hpp"
#include "arfeed.hpp"

Connection::Connection(Command MyCommand)
{
this->MyCommand=MyCommand;
this->line_counter=0;
}

bool Connection::Parse(xmlNodePtr root,int *index, bool count)
{
xmlNodePtr current;
xmlChar* contt;
int i=*index;
if(count)
{
	
	if(!(xmlStrcmp ( root->name, ( const xmlChar * ) "rss" ) ) )
		{
			Feedtype="rss";
			return true;
		}
	if(!(xmlStrcmp ( root->name, ( const xmlChar * ) "rdf" ) ))
	{
		Feedtype="rss";
		return true;
	}
	for(current=root->children;current!=NULL;current=current->next)
	{	
		if ( !(xmlStrcmp ( current->name, ( const xmlChar * ) "entry" ) ) )  
			(*index)++;

		if(current->type==XML_ELEMENT_NODE)
		{
			Parse(current,index,count);
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
		{	
			if( !(xmlStrcmp ( root->name, ( const xmlChar * ) "author" ) ) )
			{
				contt=xmlNodeListGetString(doc,current->xmlChildrenNode,1);
				entryarr[i].author=(char*)contt;
			
				xmlFree(contt);
			}
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

			Parse(current,index,count);
		}
	}
}
return true;
}



bool Connection::Feedparser()
{

size_t px;
bool chunked =false;
string text="";

string header=Feed.substr(0,Feed.find("\r\n\r\n"));

px=(header.find("HTTP/1.1 200 OK"));


if(header.find("HTTP/1.1 200 OK")==string::npos)
	{	
			if(header.find("HTTP/1.1 301 Moved Permanently")==string::npos)
				return false;

		string Location=header.substr(header.find("Location: ")+strlen("Location: "));
		Location=Location.substr(0,Location.find('\x0d')).c_str();
		MyCommand.Url=Location;

		if(!URLparser())
			return false;
					
		int ret;
		if(MyCommand.protocol==HTTP)
			ret=TCPdownload();
		else
			ret=SSLdownload();

		if(!ret)
			printf("\nerror to connect \n");

			return true;
	}
if(px!=0)
	return false;

if(header.find("Transfer-Encoding: chunked")!=string::npos)
	chunked=true;

if(chunked==true)
{
Feed=Feed.substr(Feed.find("\r\n\r\n")+4);
string number;

while(true){

number=Feed.substr(0,Feed.find("\x0d\x0a"));
unsigned int x = strtoul(number.c_str(), NULL, 16);
if(x==0)
	break;
Feed=Feed.substr(Feed.find("\x0d\x0a")+2);
text+=Feed.substr(0,x);
Feed=Feed.substr(x+2);
}
Feed=text;
}
else
{

Feed=Feed.substr(Feed.find("\r\n\r\n")+4);
}


doc = xmlReadMemory(Feed.c_str(),Feed.length(),NULL,"UTF-8",1);

if (doc == NULL) 
{ fprintf(stderr,"error: could not parse document\n");

   		return false;
}
  xmlNodePtr root;
 
  root = xmlDocGetRootElement(doc);
  if(root==NULL)
  	return false;

int pocet=0;
//give me number of entries to create 
Parse(root,&pocet,true);

if(Feedtype=="rss")
{
	RSS rss(Feed);
	return (rss.RSSparse(root,this));
}
else
{
entryarr=new entry[pocet+5];
	int index=0;
	Parse(root,&index,false);
}

int x=0;
int br=false;
while(entryarr[x].type!="")
{	
	if(x>0 && (MyCommand.aflag==true || MyCommand.Tflag==true || MyCommand.Iflag==true || MyCommand.uflag==true))
		printf("\n");

	if(entryarr[x].type=="feed")
	{
		printf("*** %s ***",entryarr[x].title.c_str() );
		x++;
		continue;
	}
	if(MyCommand.Iflag==true)
	{
		br=true;
		x=News(entryarr);
	}

	printf("\n%s",entryarr[x].title.c_str() );

	if(MyCommand.aflag==true && entryarr[x].author!="" )
		printf("\nAutor: %s",entryarr[x].author.c_str() );

	if(MyCommand.Tflag==true && entryarr[x].update!="" )
		printf("\nAktualizace: %s",entryarr[x].update.c_str() );

	if(MyCommand.uflag==true && entryarr[x].url!="" )
		printf("\nURL: %s",entryarr[x].url.c_str() );

	if(br)
		break;
	x++;
}
Feed="";
    xmlFreeDoc(doc);       // free document
    xmlCleanupParser();    // Free globals

	return true;
}





bool Connection::TCPdownload()
{
	BIO * bio;
    int p;
	string link;
	link="GET /"+MyCommand.file+" HTTP/1.1\r\nHost: "+MyCommand.server+":"+MyCommand.port+"\r\nUser-Agent: ['ARFEED']\r\nAccept: application/xml;charset=UTF-8\r\nAccept-Charset: UTF-8\r\nAccept-Language: en-US,en;q=0.5\r\nConnection: Close\r\n\r\n";
   
    char r[1025];
    /* Set up the library */
    ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();


        /* Create and setup the connection */
    string hey=MyCommand.server+":"+MyCommand.port;
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

    Feed="";
    for(;;)
    {
        p = BIO_read(bio, r, 1024);
        if(p <= 0) break;
        r[p] = 0;
        Feed+=string(r);
       
    }
    Feed+='\n';
 
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
	link="GET /"+MyCommand.file+" HTTP/1.1\r\nHost: "+MyCommand.server+"\r\nUser-Agent: ['ARFEED']\r\nAccept: application/xml;charset=UTF-8\r\nAccept-Charset: UTF-8\r\nAccept-Language: en-US,en;q=0.5\r\nConnection: Close\r\n\r\n";
    char r[1025];


    /* Set up the library */
    SSL_library_init();
    ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();


   /* Create and setup the connection */
    string hey=MyCommand.server+":"+MyCommand.port;
    char* con = new char[hey.length() + 1];
    copy(hey.begin(), hey.end(), con);

    /* Set up the SSL context */

  ctx = SSL_CTX_new(SSLv23_client_method());

  char *file=NULL;
  string folder=DEFAULTDIR;




 if(MyCommand.cargv!="")
 {
  	file=(char *)malloc(sizeof(char)*MyCommand.cargv.length());
  	strcpy(file,MyCommand.cargv.c_str());
 }
 if(MyCommand.Cargv!="")
 	folder=MyCommand.Cargv;




  if(! SSL_CTX_load_verify_locations(ctx,file,folder.c_str()))
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
        fprintf(stderr, "Certificate verification error");
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        return 0;
    }

    /* Send the request */
    BIO_write(bio, link.c_str(), strlen(link.c_str()));

    /* Read in the response */
    Feed="";
    for(;;)
    {
        p = BIO_read(bio, r, 1024);
        if(p <= 0) break;
        r[p]=0;
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

		int ret;
	if(MyCommand.protocol==HTTP)
		ret=TCPdownload();
	else
		ret=SSLdownload();
		
	if(!ret)
			printf("error to connect \n");
	
	}
	else
	{
	//get the first address from feedfile
	int i=0;
	while((MyCommand.Url=FeedFileParser())!="EOL")	
	{	
		if(i>0)
			printf("\n\n");

		if(!URLparser())
			return false;
					
		int ret;
		if(MyCommand.protocol==HTTP)
			ret=TCPdownload();
		else
			ret=SSLdownload();

		if(!ret)
			printf("\nerror to connect \n");

	i=1;
	}
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

for(unsigned int i=0; i<line.length(); i++)
     if(line.at(i) ==' ') line.erase(i,1);

if (counter==line_counter)
{
	if (line.at(0)=='#')
	{
		line_counter++;
		continue;
	}
	else{
		
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

unsigned int i=0;

for(;i<MyCommand.Url.length();i++)
{
MyCommand.Url.at(i)=tolower(MyCommand.Url.at(i));
}

string url=MyCommand.Url;

if(url.find("http://")==0)
{
	MyCommand.protocol=HTTP;
	MyCommand.port="80";
	url.erase(url.begin(),url.begin()+7);
}
else
{
	if(url.find("https://")==0)
	{
		MyCommand.protocol=HTTPS;
		url.erase(url.begin(),url.begin()+8);
		MyCommand.port="443";

	}else if(url.find("://")!=std::string::npos)
		return false;

	else{
	 MyCommand.protocol=HTTP;
	 MyCommand.port="80";
	}
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

	MyCommand.port=url.substr(pos+1);
	//check if number valid 
	for(unsigned i=0;i<MyCommand.port.length();i++)
	{
		if(MyCommand.port.at(i)<'0' || MyCommand.port.at(i)>'9' )
			return false;
	}

	url.erase(url.begin()+pos,url.end());

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
if((optind+1!=argc and MyCommand.fargv.empty()))
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
	Atom.ConnectionCreate(argv,optind);
  	printf("\n");

  	return 0;
}
