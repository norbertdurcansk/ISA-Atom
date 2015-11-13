#include "Timefun.hpp"

using namespace std;
// find the latest entry 
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
		if(arr[x].update=="")
			continue;
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
	//return max value , if no update values return first :) 
	return max;
}