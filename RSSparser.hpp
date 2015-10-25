#include "arfeed.hpp"

int News(entry *);
class RSS
{
	public:
		std::string RSSfeed;
		RSS(std::string);
		bool RSSparse(xmlNodePtr,Connection*);
		bool Dochandling(xmlNodePtr , int*,bool,Connection*);
		bool output(Connection*);

};
