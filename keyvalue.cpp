/* keyvalue.cpp
   Copyright (C) 2006 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

keyvalue.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

keyvalue.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with keyvalue.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file keyvalue.cpp
 @brief  Key Value Pair Map
 @author Carlos Justiniano
 @attention Copyright (C) 2006 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#include "keyvalue.h"
#include "log.h"
#include "exception.h"
#include "base64.h"

using namespace std;

cKeyValue::cKeyValue()
{
}

cKeyValue::~cKeyValue()
{
	Reset();
}

void cKeyValue::Reset()
{
	int rc = 0;
	try
	{
		cBuffer *pBuffer;
		map<string,cBuffer*>::iterator it;
		for (it = m_Map.begin(); it != m_Map.end(); it++)
		{
			pBuffer = (*it).second;
			delete pBuffer;
		}
		m_Map.erase(m_Map.begin(), m_Map.end());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

int cKeyValue::Parse(const char *pKeyValuePairStream, int iLength, bool bEncoded)
{
	int rc = 0;
	string sKey = "";

	cBuffer buffer;
	buffer.ReplaceWith((char*)pKeyValuePairStream, iLength);

	bool value = false;
	int mark = 0;
	int idx = 0;

	while ((idx = buffer.FindChar('|', idx)) != -1)
	{
		if (value==false)
		{
			sKey.append(buffer.c_str()+mark, idx-mark);
		}
		else
		{
            cBuffer b;
			b.ReplaceWith(buffer.cstr()+mark, idx-mark);
            if (bEncoded)
            {
                cBase64 b64;
                b64.Decode(&b);
            }
            Set(sKey.c_str(), b.c_str(), b.GetBufferCount());
			sKey = "";
		}
		idx++;
		mark = idx;
		value = (value == true) ? false : true;
	}

	return rc;
}

void cKeyValue::GetStream(string &sBuffer, bool bEncode)
{
	cBuffer temp;
	cBuffer *pBuffer;

	map<string,cBuffer*>::iterator it;
	for (it = m_Map.begin(); it != m_Map.end(); it++)
	{
		pBuffer = (*it).second;
		if (pBuffer != 0)
		{
            temp.ReplaceWith(pBuffer->cstr(), pBuffer->GetBufferCount());

			sBuffer.append((*it).first);
			sBuffer.append("|");
            if (bEncode)
            {
                cBase64 b64;
                b64.Encode(&temp);
            }
            sBuffer.append(temp.c_str(), temp.GetBufferCount());
			sBuffer.append("|");
		}
	}
}

void cKeyValue::Set(const char *pKey, const char *pValue, int iLength)
{
	try
	{
		cBuffer *pBuffer;

		map<std::string,cBuffer*>::iterator it;
		it = m_Map.find(pKey);
		if (it == m_Map.end())
		{
			MC_NEW(pBuffer, cBuffer());
			m_Map.insert(pair<string,cBuffer*>(string(pKey), pBuffer));
		}
		else
		{
			pBuffer = (*it).second;
		}
		pBuffer->ReplaceWith((char*)pValue, iLength);
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

void cKeyValue::Get(const char *pKey, cBuffer &buffer)
{
	const char *pRet = 0;
	try
	{
		map<std::string,cBuffer*>::iterator it;
		const char *pRet = 0;
		it = m_Map.find(pKey);
		if (it != m_Map.end())
		{
			buffer.ReplaceWith( (*it).second->cstr(), (*it).second->GetBufferCount() );
		}
		else
		{
            buffer.Reset();
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}
