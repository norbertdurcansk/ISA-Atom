#include <cstring>
#include <string>
#include <getopt.h>
#include <unistd.h>
#include "arfeed.hpp"
#include <fstream>

#include "RSSparser.hpp"
#include "arfeed.hpp"

const char* Error[]={"OK","Option Error","Wrong Input","Not supported protocol","No file to download","Wrong port","File is empty","Cannot load trust store location","Error attempting to connect","Certificate verification error"\
,"Cannot create bio structure","GET request error","Could not parse document (XML:error)"};

// inicialize object  variables 
Connection::Connection(Command MyCommand)
{
this->MyCommand=MyCommand;
this->line_counter=0;
}
/*=============================*/
//Function parsing xml
/*=============================*/
bool Connection::Parse(xmlNodePtr root,int *index, bool count)
{
xmlNodePtr current; // current node
xmlChar* contt; // string value
int i=*index;

if(count) // we are counting no extracting 
{
	//rss feed find, change function
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

		//load structure entries 
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
		//first is feed , next are entries 
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

/*=============================*/
//Function for parsing Feed file 
/*=============================*/
bool Connection::Feedparser()
{

size_t px; //find the part of the header 
string text=""; // sup string for chunked content 

string header=Feed.substr(0,Feed.find("\r\n\r\n")); // save header 

px=(header.find("HTTP/1.1 200 OK")); // response 

// different response 
if(header.find("HTTP/1.1 200 OK")==string::npos)
	{	
			// moved so  connect to moved link 
			if(header.find("HTTP/1.1 301 Moved Permanently")==string::npos)
			{

				Error_number=10;
				return false;
			}
		// if moved parse location
		string Location=header.substr(header.find("Location: ")+strlen("Location: "));
		Location=Location.substr(0,Location.find('\x0d')).c_str();
		// new location saved 
		MyCommand.Url=Location;
	    //same thing 
		if(!URLparser())
			return false;
					
		int ret;
		if(MyCommand.protocol==HTTP)
			ret=TCPdownload();
		else
			ret=SSLdownload();

		if(!ret)
			return false;

		return true;
	}

if(px!=0)
{
	Error_number=10;
	return false;
}

// we got chunkie zombie mode  lets  do magic stuff 

if(header.find("Transfer-Encoding: chunked")!=string::npos)
{
Feed=Feed.substr(Feed.find("\r\n\r\n")+4); // erase header 
string number; // hex number of the first chunkie 

//for eternity do  magic 
while(true){

number=Feed.substr(0,Feed.find("\x0d\x0a")); // get hex number 
unsigned int x = strtoul(number.c_str(), NULL, 16); // convert to int 
if(x==0) // if last chunk break; 
	break;
Feed=Feed.substr(Feed.find("\x0d\x0a")+2); // remove dirt from our xml string 
text+=Feed.substr(0,x); // add  to  string 
Feed=Feed.substr(x+2); // remove and continue 
}
Feed=text; // return it back 
}

//not chunked  good :) 
else
{
Feed=Feed.substr(Feed.find("\r\n\r\n")+4); //just remove header part 
}

// read mem xml 
doc = xmlReadMemory(Feed.c_str(),Feed.length(),NULL,"UTF-8",1);

if (doc == NULL) 
{
	Error_number=10;
	return false;
}

  xmlNodePtr root;

  root = xmlDocGetRootElement(doc);
  if(root==NULL)
  	return false;

int pocet=0;
//give me number of entries to create 
Parse(root,&pocet,true); // return number

if(Feedtype=="rss") // if document is rss change  function 
{
	RSS rss(Feed);
	return (rss.RSSparse(root,this));
}
else
{
	entryarr=new entry[pocet+5]; //create array of structures 
	int index=0;
	Parse(root,&index,false);
}

int x=0;
int br=false;
// output
HtmlTagremover(&entryarr);
while(entryarr[x].type!="")
{	
	// if flags enabled new line each time
	if(x>1 && (MyCommand.aflag==true || MyCommand.Tflag==true || MyCommand.Iflag==true || MyCommand.uflag==true))
		printf("\n");

	// name of the source 
	if(entryarr[x].type=="feed")
	{
		printf("*** %s ***",entryarr[x].title.c_str() );
		x++;
		continue;
	}
	//give me first update  value 
	if(MyCommand.Iflag==true)
	{
		br=true;
		x=News(entryarr);
	}
	//print title of the first unit 
	printf("\n%s",entryarr[x].title.c_str() );

	//if something then write 
	if(MyCommand.aflag==true && entryarr[x].author!="" )
		printf("\nAutor: %s",entryarr[x].author.c_str() );

	if(MyCommand.Tflag==true && entryarr[x].update!="" )
		printf("\nAktualizace: %s",entryarr[x].update.c_str() );

	if(MyCommand.uflag==true && entryarr[x].url!="" )
		printf("\nURL: %s",entryarr[x].url.c_str() );
	//br true , only one we need 
	if(br)
		break;
	x++;
}
Feed="";
//clear feed and continue
    xmlFreeDoc(doc);       // free document
    xmlCleanupParser();    // Free globals

	return true;
}
//removing html tags 
void HtmlTagremover(entry **entryarr)
{
int x=1;
	while((*entryarr)[x].type!="")
{
	while(true)
	{
		int pos=(*entryarr)[x].title.find('<');
		if((*entryarr)[x].title.find('<')!=std::string::npos)
		{

			int pos1=(*entryarr)[x].title.find('>',pos);
			if((*entryarr)[x].title.find('>')!=std::string::npos)
				(*entryarr)[x].title.erase((*entryarr)[x].title.begin()+pos,(*entryarr)[x].title.begin()+pos1+1);
			continue;
		}
	    pos=(*entryarr)[x].author.find('<');
		if((*entryarr)[x].author.find('<')!=std::string::npos)
		{
			int pos1=(*entryarr)[x].author.find('>',pos);
			if((*entryarr)[x].author.find('>')!=std::string::npos)
				(*entryarr)[x].author.erase((*entryarr)[x].author.begin()+pos,(*entryarr)[x].author.begin()+pos1+1);
			continue;
		}

		else break;
	}
x++;	
}
return;
}
/*=============================*/
//Function for http download 
/*=============================*/
bool Connection::TCPdownload()
{
	BIO * bio;
    int p;
    //get request 
	string link="GET /"+MyCommand.file+" HTTP/1.1\r\nHost: "+MyCommand.server+":"+MyCommand.port+"\r\nUser-Agent: ARFEED-xdurca01\r\nAccept: application/xml;charset=UTF-8\r\nAccept-Charset: UTF-8\r\nAccept-Language: en-US,en;q=0.5\r\nConnection: Close\r\n\r\n";
   
    char r[1025];

    /* Set up the library */
    ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();


    /* Create and setup the connection */
    string hey=MyCommand.server+":"+MyCommand.port;
    char* con = new char[hey.length()];
    unsigned int cc=0;
    for(;cc<hey.length();cc++)
    {
    	con[cc]=hey.at(cc);
    }
    con[cc]='\0';
 
    bio = BIO_new_connect(con);
    if(bio == NULL) 
    {
    	 Error_number=9;
    	 
    	 return false; 
    }

    if(BIO_do_connect(bio) <= 0)
    {
        BIO_free_all(bio);
        Error_number=8;
        
        return false;
    }
    //send request 
    BIO_write(bio, link.c_str(), strlen(link.c_str()));

    Feed="";
    for(;;)
    {
        p = BIO_read(bio, r, 1024);
        if(p <= 0) break;
        r[p] = 0;
        Feed+=string(r);
       
    }
    
 
    BIO_free_all(bio);
    // parse feed file
return Feedparser();
}
/*=============================*/
//Function for https download 
/*=============================*/
bool Connection::SSLdownload()

{

	// Using IBM  code for connection 

    BIO * bio;  // create bio struct 
    SSL * ssl; //create ssl struct 
    SSL_CTX * ctx; //create context 
 
    int p; // variable

    //get request 
	string link="GET /"+MyCommand.file+" HTTP/1.1\r\nHost: "+MyCommand.server+"\r\nUser-Agent: ARFEED-xdurca01\r\nAccept: application/xml;charset=UTF-8\r\nAccept-Charset: UTF-8\r\nAccept-Language: en-US,en;q=0.5\r\nConnection: Close\r\n\r\n";
    //sup buffer
    char r[1025];


    /* Set up the library */
    SSL_library_init();
    ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();


   /* Create and setup the connection */
    string hey=MyCommand.server+":"+MyCommand.port;  // usign structure loaded below
     char* con = new char[hey.length()];
    unsigned int cc=0;
    for(;cc<hey.length();cc++)
    {
    	con[cc]=hey.at(cc);
    }
    con[cc]='\0';

  
  /* Set up the SSL context */
  ctx = SSL_CTX_new(SSLv23_client_method());

  char *file=NULL; //file not used yet 
  string folder=DEFAULTDIR;  // set up our default dir  /ect/ssl/certs

 if(MyCommand.cargv!="")
 {
 	//set up the file for search 
  	file=(char *)malloc(sizeof(char)*MyCommand.cargv.length());
  	strcpy(file,MyCommand.cargv.c_str());
 }
 // change the folder for search 
 if(MyCommand.Cargv!="")
 	folder=MyCommand.Cargv;

  if(! SSL_CTX_load_verify_locations(ctx,file,folder.c_str()))
    {
        SSL_CTX_free(ctx);      
        Error_number=7;
        return false; // cannot verify  location , we failed 
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
        Error_number=8;
        
        BIO_free_all(bio); // clear structures 
        SSL_CTX_free(ctx);
        return false;
    }

    //SSL_set_connect_state(ssl); 
	SSL_do_handshake(ssl); 

    /* Check the certificate */
    if(SSL_get_verify_result(ssl) != X509_V_OK)
    {
        
        Error_number=9;
        
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        return false;
    }

    /* Send the request */
    BIO_write(bio, link.c_str(), strlen(link.c_str()));

    /* Read in the response */
    //our feed response 
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

  return Feedparser();
}


bool Connection::ConnectionCreate(char *argv[],int optind)
{ 
	// use url or feed file 
	if(MyCommand.fargv.empty())
	{
	Error_number=0;
	// Saving URL address 
	MyCommand.Url=argv[optind];
	if(!URLparser())  // something wrong happend return false , code already saved 
		return false;

		int ret;

	if(MyCommand.protocol==HTTP)
		ret=TCPdownload();
	else
		ret=SSLdownload();
		
	if(!ret)
			return false;
	}
	else
	{
		//get the first address from feedfile
		int i=0;
		while((MyCommand.Url=FeedFileParser())!="EOL")	
		{	

			Error_number=0;

			if(i>0)
				/** add \n for each source */
				printf("\n\n");

			if(!URLparser())  // error value saved 
				return false;
						
			int ret;
			if(MyCommand.protocol==HTTP)
				ret=TCPdownload();
			else
				ret=SSLdownload();

			if(!ret)
				fprintf(stderr, "Arfeed failure, error: %s\n",Error[Error_number ]);

			i=1;
		}
	}
//everything  was ok  
return true;		
}

/*======================================*/
/** parsing feed file */
/* each time function returns new  url address */
/*======================================*/
string Connection::FeedFileParser()
{

ifstream file(MyCommand.fargv.c_str());
string line;
int counter=-1;
/* check if exists */
if(!file.good())
{
	fprintf(stderr, "Arfeed failure, error: Cannot open feedfile\n" );
  	exit(6); //returning eror code 
}
// check if not empty 
else if (file.peek()==std::ifstream::traits_type::eof())
{
	Error_number=6;
    fprintf(stderr, "Arfeed failure, error: %s\n",Error[Error_number] );
  	exit(Error_number); //returning eror code 

}
/** get line from the file */
while( getline(file, line)){
counter++;	

/* line empty skip */

if(line.empty()){
	line_counter++;
	continue;
}
/* erase all whitespaces , for sure :) */
for(unsigned int i=0; i<line.length(); i++)
     if(line.at(i) ==' ') line.erase(i,1);

/** everytime return new url address */
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
//nothing so return
return "EOL";
}

/*======================================*/
/**parsing URL address loading structure*/
/*======================================*/
bool Connection::URLparser()
{
unsigned int i=0; //support variable 
string protocol=MyCommand.Url; // sup string 

// change protocol to lowercase 
for(;i<MyCommand.Url.length();i++)
{
protocol.at(i)=tolower(MyCommand.Url.at(i));
}

string url=MyCommand.Url; // save url string 

if(protocol.find("http://")==0) //checking for http protocol
{
	MyCommand.protocol=HTTP; //save to struct 
	MyCommand.port="80";  //set up default port 
	url.erase(url.begin(),url.begin()+7); // erase from string 
}
else
{
	if(protocol.find("https://")==0) //checking for https protocol 
	{
		MyCommand.protocol=HTTPS; //save to struct 
		url.erase(url.begin(),url.begin()+8);
		MyCommand.port="443";

	}else if(protocol.find("://")!=std::string::npos)
		{
			Error_number=3;
			return false;
		}
	else
	{

		 MyCommand.protocol=HTTP; //nothing found default is HTTP 
	 	 MyCommand.port="80"; 
	}
}

//get file from url
unsigned int pos; 
pos=url.find("/");
if(url.find("/")!=std::string::npos)
{
	if(pos+1!=url.length())
		MyCommand.file=url.substr(pos+1);
	url.erase(url.begin()+pos,url.end());
}
else 
{
	Error_number=4; //no file to download 
	return false;
}
pos=url.find(":");
if(url.find(":")!=std::string::npos)
{
	if(pos+1!=url.length())
		MyCommand.port=url.substr(pos+1);
	else
	{
		Error_number=5;
		return false;
	}
	//check if number valid 
	for(unsigned i=0;i<MyCommand.port.length();i++)
	{
		if(MyCommand.port.at(i)<'0' || MyCommand.port.at(i)>'9' )
		{
			Error_number=5;
			return false;
		}	
	}
	url.erase(url.begin()+pos,url.end()); //erase another part 
}
MyCommand.server=url; // last part should be the server 
return true;
}


/*======================================*/
/*Function for parsing arguments */
/*======================================*/
bool Connection::ArgumentParser(int argc , char *argv[])
{

int c ; // support variable 

/* set flags and strings if pressed  */
while ((c = getopt (argc, argv, "f:c:C:lTau")) != -1)
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
		  case 'l':
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
	      	Error_number=1;
	        return false;

	      default:
	      		Error_number=2;
	      		return false;
    }
if((optind+1!=argc and MyCommand.fargv.empty()))
{
	Error_number=2;
	return false;
}
	return true;
}


/*======================================*/
/*Main function */
/*======================================*/
int main(int argc , char *argv[])
{
	Command MyCommand; /* structure where we hold parsed url*/

	Connection Atom(MyCommand); /* create object Connection*/
	
  	if(!Atom.ArgumentParser(argc,argv))
  		{
  			fprintf(stderr, "Arfeed failure, error: %s\n",Error[Atom.Error_number ]);
  			return Atom.Error_number; //returning eror code 
  		} 

	if(!Atom.ConnectionCreate(argv,optind))  // creating Connection 
  		{
  			fprintf(stderr, "Arfeed failure, error: %s\n",Error[Atom.Error_number ] );
  			return Atom.Error_number; //returning eror code 
  		}

  	printf("\n"); //everything ok 
  	return 0;
}
