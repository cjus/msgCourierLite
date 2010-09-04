/* keyvalue.h
   Copyright (C) 2006 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

keyvalue.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

keyvalue.h was developed by Carlos Justiniano for use on the
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
 @file keyvalue.h
 @brief Key Value Pair Map
 @author Carlos Justiniano
 @attention Copyright (C) 2006 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef KEY_VALUE_H
#define KEY_VALUE_H

#include <string>
#include <map>
#include "buffer.h"

#define HRC_OK	0x0000
#define HRC_UNBALANCED_PAIRS	0x0001


/**
 @class cKeyValue
 @brief Key Value Pair Map
 */
class cKeyValue
{
public:
	cKeyValue();
	~cKeyValue();

	void Reset();
	int Parse(const char *pKeyValuePairStream, int iLength, bool bEncoded=true);
	void GetStream(std::string &sBuffer, bool bEncode=true);

	void Set(const char *pKey, const char *pValue, int iLength);
	void Get(const char *pKey, cBuffer &buffer);

    int Count() { return m_Map.size(); }
private:
	std::map<std::string,cBuffer*> m_Map;
};

#endif // KEY_VALUE_H
