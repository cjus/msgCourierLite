/* sysinfo.cpp
   Copyright (C) 2006 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

sysinfo.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

sysinfo.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with sysinfo.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file sysinfo.cpp
 @brief  System Information
 @author Carlos Justiniano
 @attention Copyright (C) 2006 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#include "sysinfo.h"
#include "log.h"
#include "exception.h"

using namespace std;

cSysInfo::cSysInfo()
{
}

cSysInfo::~cSysInfo()
{
	try
	{	
		m_SysInfoMap.erase(m_SysInfoMap.begin(), m_SysInfoMap.end());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}


void cSysInfo::SetSysInfo(const char *pKey, const char *pValue)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	
	try
	{
		string sKey = pKey;
		string sValue = pValue;
		map<std::string,std::string>::iterator it;
		it = m_SysInfoMap.find(pKey);
		if (it == m_SysInfoMap.end())
			m_SysInfoMap.insert(pair<string,string>(sKey, sValue));
		else
			(*it).second = sValue;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

const char *cSysInfo::Query(const char *pKey)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	const char *pRet = 0;
	
	try
	{
		map<std::string,std::string>::iterator it;
		it = m_SysInfoMap.find(pKey);
		if (it == m_SysInfoMap.end())
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


