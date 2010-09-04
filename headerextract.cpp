/* headerextract.cpp
   Copyright (C) 2002 Carlos Justiniano

headerextract.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

headerextract.cpp was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with headerextract.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file headerextract.cpp
 @brief Parse HTTP style headers and create value/data pairs
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence (see source file header)
 Parse HTTP style headers and create value/data pairs
*/

#include "log.h"
#include "exception.h"
#include "headerextract.h"

using namespace std;

void cHeaderExtract::Parse(cBuffer *pBuf)
{
	try
	{
		// clear param map
		m_Params.erase(m_Params.begin(),m_Params.end());

		int len = pBuf->Find("\r\n\r\n", 4);
		if (len == -1)
			return;

		m_sHeader.assign((char*)pBuf->GetRawBuffer(), len);
		m_sHeader.append("\r\nend\r\n");

		basic_string <char>::size_type idx, offset=0;
		basic_string <char>::size_type split;
		while ((idx = m_sHeader.find_first_of('\r', offset)) != string::npos)
		{
			string line = m_sHeader.substr(offset, idx-offset);
			offset = idx+2;
			split = line.find_first_of(':');
			if (split == string::npos)
				continue;

			if (line[split+1]==' ')
				line.erase(split,2);
			else
				line.erase(split,1);

			m_Params[line.substr(0, split)] = line.substr(split,line.size()-split);
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

void cHeaderExtract::GetValue(char *pName, string &sValue)
{
	try
	{
		//sValue = m_Params[string(pName)];
		map<string,string>::iterator it;
		it = m_Params.find(string(pName));
		if (it != m_Params.end())
			sValue.assign((*it).second);
		else
			sValue = "";
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

const char *cHeaderExtract::GetValue(const char *pName)
{
	const char *p = NULL;
	try
	{
		map<string,string>::iterator it;
		it = m_Params.find(string(pName));
		if (it != m_Params.end())
			p = (*it).second.c_str();
		else
			p = "";
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return p;
}

const char *cHeaderExtract::GetRawHeader()
{
    return m_sHeader.c_str();
}

