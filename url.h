/* url.h
   Copyright (C) 2004 Carlos Justiniano

url.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

url.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with url.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file url.h
 @brief URL encode/decoder
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)

 URL encode/decoder
*/

#ifndef _URL_H
#define _URL_H

#include <string>

//#define CURL_GENERATE_LOOKUP_TABLE 1

/**
 @class cURL
 @brief URL encoder/decoder
*/
class cURL
{
public:
	cURL();
	~cURL();

	void Encode(std::string &sInBuffer, std::string &sOutBuffer);
	void Decode(std::string &sInBuffer, std::string &sOutBuffer);
#ifdef CURL_GENERATE_LOOKUP_TABLE
	void GenerateLookUPTable();
#endif //CURL_GENERATE_LOOKUP_TABLE
private:
	static char *m_pURLEncodeLookupTable[255]; 
	int inline HexToInt(char *pHexBytePair)
	{
		int iHiByte, iLoByte, iValue;

		// get value for first byte
		iHiByte = pHexBytePair[0];
		if ('0' <= iHiByte && iHiByte <= '9')
			iHiByte -= '0';
		else if ('a' <= iHiByte && iHiByte <= 'f')
			iHiByte -= ('a'-10);
		else if ('A' <= iHiByte && iHiByte <= 'F')
			iHiByte -= ('A'-10);

		// get value for second byte
		iLoByte = pHexBytePair[1];
		if ('0' <= iLoByte && iLoByte <= '9')
			iLoByte -= '0';
		else if ('a' <= iLoByte && iLoByte <= 'f')
			iLoByte -= ('a'-10);
		else if ('A' <= iLoByte && iLoByte <= 'F')
			iLoByte -= ('A'-10);
		iValue = iLoByte + (16*iHiByte);
		return iValue;
	}
};

#endif // _URL_H

/* Tests
	#include "url.h"

	cURL url;
	string src;
	string dst;

	//src = "%22Aardvarks+lurk%2C+OK%3F%22";
	//src = "%22What+the+-_.!~*'()%22";
	//src = "C%3A%5C%5Cmsgsrv%5C%5Cmsgsrv-src";

	src = "C:\\msgsrv\\msgsrv source code\\";
	url.Encode(src, dst);
	src = dst;
	url.Decode(src, dst);
*/
