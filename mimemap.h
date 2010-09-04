/* mimemap.h
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

mimemap.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

mimemap.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with mimemap.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file mimemap.h
 @brief MIME MAP
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef MIME_MAP_H
#define MIME_MAP_H

#include <string>
#include <map>

/**
 @class cMIMEMap
 @brief File extention to Mime Content Type
 */
class cMIMEMap
{
public:
	cMIMEMap();
	~cMIMEMap();
	void GetContentType(std::string &fileExtention, std::string &sContentType);
	char *GetContentType(char *pFileExtention);
private:
	std::map<std::string,std::string> m_MimeMap;
	std::map<std::string,std::string>::iterator m_MimeMapIterator;
};

#endif // MIME_MAP_H


