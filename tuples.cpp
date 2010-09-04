/* tuples.cpp
   Copyright (C) 2006 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

tuples.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

tuples.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with tuples.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file tuples.cpp
 @brief  Tuple handling
 @author Carlos Justiniano
 @attention Copyright (C) 2006 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#include "tuples.h"
#include "log.h"
#include "exception.h"
#include "cbstrtok.h"

using namespace std;

cTuples::cTuples()
{
}

cTuples::~cTuples()
{
	try
	{	
		m_tuplesMap.erase(m_tuplesMap.begin(), m_tuplesMap.end());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

const char *cTuples::Query(const char *pKey)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	const char *pRet = 0;
	
	try
	{
		map<std::string,std::string>::iterator it;
		it = m_tuplesMap.find(pKey);
		if (it == m_tuplesMap.end())
			pRet = 0;	
		else
			pRet = ((*it).second).c_str();
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}	
	return pRet;
}

void cTuples::AddTupleStream(const char *pstring)
{
STRTOKVA;
	char *pSep = "|";
	char *pToken1, *pToken2;
	int octval;
	
	char *pBuf;
	MC_NEW(pBuf, char[strlen(pstring)+1]);
	strcpy(pBuf, pstring);

	pToken1 = STRTOK(pBuf, pSep);
	if (pToken1 == NULL)
		return;
	while (1)
	{
		pToken2 = STRTOK(NULL, pSep);
		if (pToken2 == NULL)
			break;

		AddTuple(pToken1, pToken2);

		pToken1 = STRTOK(NULL, pSep);
		if (pToken1 == NULL)
			break;
	}

	delete []pBuf;
}

void cTuples::AddTuple(const char *pKey, const char *pValue)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	
	try
	{
		string sKey = pKey;
		string sValue = pValue;
		map<std::string,std::string>::iterator it;
		it = m_tuplesMap.find(pKey);
		if (it == m_tuplesMap.end())
			m_tuplesMap.insert(pair<string,string>(sKey, sValue));
		else
			(*it).second = sValue;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

std::string	cTuples::GetTupleStream()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);

	string stream = "";

	map<std::string,string>::iterator it;
	for (it = m_tuplesMap.begin(); it != m_tuplesMap.end(); it++)
	{
		stream += it->first;
		stream += "|";
		stream += it->second;
		stream += "|";
	}

	return stream;
}

