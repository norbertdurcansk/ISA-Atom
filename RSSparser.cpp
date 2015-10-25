
#include "RSSparser.hpp"

using namespace std;

RSS::RSS(string RSSfeed)
{
	this->RSSfeed=RSSfeed;
	return;	
}

bool RSS::RSSparse(xmlNodePtr root,Connection* Atom)
{



int pocet=0;
Dochandling(root,&pocet,true,NULL);
Atom->entryarr=new entry[pocet+5];
int index=0;
Dochandling(root,&index,false,Atom);
output(Atom);


return true;

}
bool RSS::output(Connection* Atom)
{
int x=0;
int br=false;
while(Atom->entryarr[x].type!="")
{	
	if(x>0 && (Atom->MyCommand.aflag==true || Atom->MyCommand.Tflag==true || Atom->MyCommand.Iflag==true || Atom->MyCommand.uflag==true))
		printf("\n");

	if(Atom->entryarr[x].type=="feed")
	{
		printf("*** %s ***",Atom->entryarr[x].title.c_str() );
		x++;
		continue;
	}
	if(Atom->MyCommand.Iflag==true)
	{
		br=true;
		x=News(Atom->entryarr);
	}

	printf("\n%s",Atom->entryarr[x].title.c_str() );

	if(Atom->MyCommand.aflag==true && Atom->entryarr[x].author!="" )
		printf("\nAutor: %s",Atom->entryarr[x].author.c_str() );

	if(Atom->MyCommand.Tflag==true && Atom->entryarr[x].update!="" )
		printf("\nAktualizace: %s",Atom->entryarr[x].update.c_str() );

	if(Atom->MyCommand.uflag==true && Atom->entryarr[x].url!="" )
		printf("\nURL: %s",Atom->entryarr[x].url.c_str() );

	if(br)
		break;
	x++;
}
return true;

}

bool RSS::Dochandling(xmlNodePtr root, int *index,bool search,Connection* Atom)
{

xmlNodePtr current;
xmlChar* contt;
int i;

if(search)
{
	for(current=root->children;current!=NULL;current=current->next)
	{	
		if ( !(xmlStrcmp ( current->name, ( const xmlChar * ) "item" ) ) )  
			(*index)++;

		if(current->type==XML_ELEMENT_NODE)
		{
			Dochandling(current,index,search,Atom );
		}
	}
}else{
for(current=root->children;current!=NULL;current=current->next)
	{
		
		if(current->type==XML_ELEMENT_NODE)
		{


		if ( !(xmlStrcmp ( current->name, ( const xmlChar * ) "item" ) ) ) 
			(*index)++;


		i=*index;


		if ( !(xmlStrcmp ( current->name, ( const xmlChar * ) "title" ) ) ) 	
		{	

			if(!(xmlStrcmp ( root->name, ( const xmlChar * ) "item" ) ))
			{
				contt=xmlNodeListGetString(Atom->doc,current->xmlChildrenNode,1);
				Atom->entryarr[i].title=(char*)contt;			
				xmlFree(contt);
			}
			else if((!(xmlStrcmp ( root->name, ( const xmlChar * ) "channel" ) )))
			{
				contt=xmlNodeListGetString(Atom->doc,current->xmlChildrenNode,1);
				Atom->entryarr[i].title=(char*)contt;
				xmlFree(contt);
			}
		}

		if ( !(xmlStrcmp ( current->name, ( const xmlChar * ) "pubDate" ) ) ) 	
		{	

			if(!(xmlStrcmp ( root->name, ( const xmlChar * ) "item" ) ))
			{
				contt=xmlNodeListGetString(Atom->doc,current->xmlChildrenNode,1);
				Atom->entryarr[i].update=(char*)contt;			
				xmlFree(contt);
			}
		}

		if ( !(xmlStrcmp ( current->name, ( const xmlChar * ) "link" ) ) ) 	
		{	

			if(!(xmlStrcmp ( root->name, ( const xmlChar * ) "item" ) ))
			{
				contt=xmlNodeListGetString(Atom->doc,current->xmlChildrenNode,1);
				Atom->entryarr[i].url=(char*)contt;			
				xmlFree(contt);
			}
		}
		if ( !(xmlStrcmp ( current->name, ( const xmlChar * ) "name" ) ) ) 
		{	
			if( !(xmlStrcmp ( root->name, ( const xmlChar * ) "author" ) ) )
			{
				contt=xmlNodeListGetString(Atom->doc,current->xmlChildrenNode,1);
				Atom->entryarr[i].author=(char*)contt;
			
				xmlFree(contt);
			}
		}

			
		if((*index)==0)
			Atom->entryarr[i].type="feed";
		else
			Atom->entryarr[i].type="entry";

			Dochandling(current,index,search,Atom);
		}
	}
}
	return true;

}

int News(entry *arr)
{
	int x =1;
	int year=0;
	int month=0;
	int day=0;
	int hour=0;
	int min=0;
	int sec=0;
	int max=1;
	int maxyear=0;
	int maxmonth=0;
	int maxday=0;
	int maxhour=0;
	int maxmin=0;
	int maxsec=0;

		string pom1=arr[x].update;
		maxyear=atoi(pom1.substr(0,pom1.find("-")).c_str());
		pom1=pom1.substr(pom1.find("-")+1);
		maxmonth=atoi(pom1.substr(0,pom1.find("-")).c_str());
		pom1=pom1.substr(pom1.find("-")+1);
		maxday=atoi(pom1.substr(0,pom1.find("-")).c_str());
		pom1=pom1.substr(pom1.find("T")+1);
		maxhour=atoi(pom1.substr(0,pom1.find(":")).c_str());
	    pom1=pom1.substr(pom1.find(":")+1);
		maxmin=atoi(pom1.substr(0,pom1.find(":")).c_str());
	    pom1=pom1.substr(pom1.find(":")+1);
		maxsec=atoi(pom1.substr(0,2).c_str());
		//porovnanie

	while(arr[x+1].type!="")
	{
		x++;
		bool changed=false;
		pom1=arr[x].update;
		year=atoi(pom1.substr(0,pom1.find("-")).c_str());
		pom1=pom1.substr(pom1.find("-")+1);
		month=atoi(pom1.substr(0,pom1.find("-")).c_str());
		pom1=pom1.substr(pom1.find("-")+1);
		day=atoi(pom1.substr(0,pom1.find("-")).c_str());
		pom1=pom1.substr(pom1.find("T")+1);
		hour=atoi(pom1.substr(0,pom1.find(":")).c_str());
	    pom1=pom1.substr(pom1.find(":")+1);
		min=atoi(pom1.substr(0,pom1.find(":")).c_str());
	    pom1=pom1.substr(pom1.find(":")+1);
		sec=atoi(pom1.substr(0,2).c_str());

		//porovnanie
		if(year>maxyear)
		{
			max=x;
			changed=true;

		}else if(maxyear > year)
			continue;
		else if(month>maxmonth)
		{
			max=x;
			changed=true;

		}else if (maxmonth>month)
			continue;
		else if(day>maxday)
		{
			max=x;
			changed=true;

		}else if(maxday>day)
			continue;
		else if(maxhour>hour)
			continue;
		else if(hour>maxhour)
		{
			max=x;
			changed=true;
		}
		else if(maxmin<min)
		{
			max=x;
			changed=true;

		}else if(maxmin>min)
			continue;
		else if(maxsec<sec)
		{
			max=x;
			changed=true;
		}
		else if(maxsec>sec)
			continue;


		if(changed)
		{
			maxyear=year;
			
			maxmonth=month;
			
			maxday=day;
		
			maxhour=hour;
		  
			maxmin=min;

			maxsec=sec;
		}

	}

	return max;
}