/* date.cpp
   Copyright (C) 2005 Carlos Justiniano

date.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

date.cpp was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with date.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file date.cpp 
 @brief Retrives the current Date/Time Stamp in GMT
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 Retrives the current Date/Time Stamp in GMT
*/

#include <time.h>
#include "date.h"
#include "exception.h"
#include "log.h"

using namespace std;

void cDate::GetDate(string &datestring)
{
	try
	{	
		time_t rawtime;
		struct tm *t;
		char buf[60];
	
		time(&rawtime);
		t = gmtime(&rawtime);

		datestring = "";
		switch (t->tm_wday)
		{
			case 0:
				datestring += "Sun, ";
				break;
			case 1:
				datestring += "Mon, ";
				break;
			case 2:
				datestring += "Tue, ";
				break;
			case 3:
				datestring += "Wed, ";
				break;
			case 4:
				datestring += "Thu, ";
				break;
			case 5:
				datestring += "Fri, ";
				break;
			case 6:
				datestring += "Sat, ";
				break;
		}
		sprintf(buf,"%2.2d ", t->tm_mday);
		datestring += buf;

		switch (t->tm_mon)
		{
			case 0:
				datestring += "Jan ";
				break;
			case 1:
				datestring += "Feb ";
				break;
			case 2:
				datestring += "Mar ";
				break;
			case 3:
				datestring += "Apr ";
				break;
			case 4:
				datestring += "May ";
				break;
			case 5:
				datestring += "Jun ";
				break;
			case 6:
				datestring += "Jul ";
				break;
			case 7:
				datestring += "Aug ";
				break;
			case 8:
				datestring += "Sep ";
				break;
			case 9:
				datestring += "Oct ";
				break;
			case 10:
				datestring += "Nov ";
				break;
			case 11:
				datestring += "Dec ";
				break;
		}
		int year = t->tm_year - 100 + 2000;
		sprintf(buf,"%d %2.2d:%2.2d:%2.2d GMT", year, t->tm_hour, t->tm_min, t->tm_sec);
		datestring += buf;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

