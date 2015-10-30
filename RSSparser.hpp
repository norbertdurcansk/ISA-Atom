#ifndef RSSparser_HPP
#define RSSparser_HPP
#include "arfeed.hpp"

//count max time function
int News(entry *); 

//object for rss parsing 
class RSS
{
	public:
		std::string RSSfeed;
		RSS(std::string);
		bool RSSparse(xmlNodePtr,Connection*);
		bool Dochandling(xmlNodePtr , int*,bool,Connection*);
		bool output(Connection*);

};

#endif