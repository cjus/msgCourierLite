/* headerextract.h
   Copyright (C) 2002 Carlos Justiniano

headerextract.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

headerextract.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with headerextract.h; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file headerextract.h
 @brief Parse HTTP style headers and create value/data pairs
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence (see source file header)
 Parse HTTP style headers and create value/data pairs
*/

#ifndef _HEADEREXTRACT_H
#define _HEADEREXTRACT_H

#include <string>
#include <map>
#include "buffer.h"

/**
 @class cHeaderExtract
 @brief Parse HTTP style headers and create value/data pairs
*/
class cHeaderExtract
{
public:
	void Parse(cBuffer *pBuf);
	void GetValue(char *pName, std::string &sValue);
	const char *GetValue(const char *pName);
    const char *GetRawHeader();
private:
	std::map<std::string,std::string> m_Params;
	std::string m_sHeader;
};

#endif // _HEADEREXTRACT_H
