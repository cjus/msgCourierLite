/* date.h
   Copyright (C) 2005 Carlos Justiniano

date.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

date.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with date.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file date.h 
 @brief Retrives the current Date/Time Stamp in GMT
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)

 Retrives the current Date/Time Stamp in GMT
*/

#ifndef _DATE_H
#define _DATE_H

#include <string>

/**
 @class cDate 
 @brief Retrives the current Date/Time Stamp in GMT
*/
class cDate
{
public:
	static void GetDate(std::string &datestring);
};

#endif //_DATE_H

