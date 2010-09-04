/* url.cpp
   Copyright (C) 2004 Carlos Justiniano

url.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

url.cpp was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with url.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file url.cpp
 @brief URL encode/decoder
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)

 URL encode/decoder
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>

#include "url.h"
#include "exception.h"

using namespace std;

char *cURL::m_pURLEncodeLookupTable[255] = 
{
	"%00","%01","%02","%03","%04","%05","%06","%07","%08",
	"%09","%0A","%0B","%0C","%0D","%0E","%0F","%10","%11",
	"%12","%13","%14","%15","%16","%17","%18","%19","%1A",
	"%1B","%1C","%1D","%1E","%1F","%20","%21","%22","%23",
	"%24","%25","%26","%27","%28","%29","%2A","%2B","%2C",
	"%2D","%2E","%2F","","","","","","","","","","","%3A",
	"%3B","%3C","%3D","%3E","%3F","%40","","","","","","",
	"","","","","","","","","","","","","","","","","","",
	"","","%5B","%5C","%5D","%5E","%5F","%60","","","","",
	"","","","","","","","","","","","","","","","","","",
	"","","","","%7B","%7C","%7D","%7E","%7F","%80","%81",
	"%82","%83","%84","%85","%86","%87","%88","%89","%8A",
	"%8B","%8C","%8D","%8E","%8F","%90","%91","%92","%93",
	"%94","%95","%96","%97","%98","%99","%9A","%9B","%9C",
	"%9D","%9E","%9F","%A0","%A1","%A2","%A3","%A4","%A5",
	"%A6","%A7","%A8","%A9","%AA","%AB","%AC","%AD","%AE",
	"%AF","%B0","%B1","%B2","%B3","%B4","%B5","%B6","%B7",
	"%B8","%B9","%BA","%BB","%BC","%BD","%BE","%BF","%C0",
	"%C1","%C2","%C3","%C4","%C5","%C6","%C7","%C8","%C9",
	"%CA","%CB","%CC","%CD","%CE","%CF","%D0","%D1","%D2",
	"%D3","%D4","%D5","%D6","%D7","%D8","%D9","%DA","%DB",
	"%DC","%DD","%DE","%DF","%E0","%E1","%E2","%E3","%E4",
	"%E5","%E6","%E7","%E8","%E9","%EA","%EB","%EC","%ED",
	"%EE","%EF","%F0","%F1","%F2","%F3","%F4","%F5","%F6",
	"%F7","%F8","%F9","%FA","%FB","%FC","%FD","%FE"
};
/*
char *cURL::m_pURLEncodeLookupTable[255] = 
{
	"%00","","","","","","","","","","","","","","","",
	"","","","","","","","","","","","","","","","",
	"%20","%21","%22","%23","%24","%25","%26","","","",
	"","%2B","%2C","","","%2F","","","","","","","","",
	"","","%3A","%3B","%3C","%3D","%3E","%3F","%40","",
	"","","","","","","","","","","","","","","","","",
	"","","","","","","","","%5B","%5C","%5D","%5E","",
	"%60","","","","","","","","","","","","","","","",
	"","","","","","","","","","","","","","","","",
	"","","","","","","","","","","","","","","","",
	"","","","","","","","","","","","","","","","",
	"","","","","","","","","","","","","","","","",
	"","","","","","","","","","","","","","","","",
	"","","","","","","","","","","","","","","","",
	"","","","","","","","","","","","","","","","",
	"","","","","","","","","","","","","","","","",
	"","","","","","","","","","","","","","",""
};
*/
cURL::cURL()
{
}

cURL::~cURL()
{
}

void cURL::Encode(string &sInBuffer, string &sOutBuffer)
{	
	char* buffer;

	try
	{
		buffer = new char[sInBuffer.size() * 3];
	}
	catch(...)
	{
		THROW(MEMALLOC);
	}

	char *pSrc = (char*)sInBuffer.c_str();
	char *pDst = (char*)buffer;
	char *p;

	int i;
	for (i=0; i < (int)sInBuffer.size(); i++)
	{
		int idx = (int)*pSrc;
		p = m_pURLEncodeLookupTable[idx];
		if (*p == 0)
		{
			*pDst++ = *pSrc++;
		}
		else
		{
			*pDst++ = *p++;
			*pDst++ = *p++;
			*pDst++ = *p++;
			*pSrc++;
		}
	}
	*pDst = 0;
	sOutBuffer = buffer;
	delete []buffer;
}

void cURL::Decode(string &sInBuffer, string &sOutBuffer)
{
	char* buffer;

	try
	{
		buffer = new char[sInBuffer.size() * 3];
	}
	catch(...)
	{
		THROW(MEMALLOC);
	}

	char *pEncoded = (char*)sInBuffer.c_str();
	char *pDecoded = (char*)buffer;

	while (*pEncoded)
	{
		if (*pEncoded == '%')
		{
			pEncoded++;
			if (isxdigit(pEncoded[0]) && isxdigit(pEncoded[1]))
			{
				*pDecoded++ = (char)HexToInt(pEncoded);
				pEncoded += 2;
			}
		}
		else if (*pEncoded == '+')
		{
			*pEncoded++;
			*pDecoded++ = ' ';
		}
		else
		{
			*pDecoded++ = *pEncoded++;
		}
	}
	*pDecoded = 0;
	sOutBuffer = buffer;
	delete []buffer;
}

#ifdef CURL_GENERATE_LOOKUP_TABLE
void cURL::GenerateLookUPTable()
{
	FILE *fp;
	fp = fopen("lookup.tbl", "w+");
	
	fprintf(fp, "char *cURL::m_pURLEncodeLookupTable[255] = {");

	char *pUnsafeCharacters = " \"<>%\\^[]`+$,@:;/!#?=&";

	int i;
	for (i = 0; i<255; i++)
	{
		if (i < 48 || (i > 57 && i < 65) || (i > 90 && i < 97) || (i > 122) || strchr(pUnsafeCharacters, i))
		{
			// convert to hex string
			fprintf(fp, "\"%c", '%');
			fprintf(fp, "%-2.2X\",", i);
		}
		else
		{
			// mark character as safe, by setting to empty
			fprintf(fp, "\"\",");
		}
	}
	fprintf(fp, "};");
	fclose(fp);
}
#endif //CURL_GENERATE_LOOKUP_TABLE
