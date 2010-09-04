/* virtualdirectorymap.cpp
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

virtualdirectorymap.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

virtualdirectorymap.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with virtualdirectorymap.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file virtualdirectorymap.cpp
 @brief Virtual directory map of port names to file system directory paths
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#include "exception.h"
#include "log.h"
#include "virtualdirectorymap.h"

using namespace std;

cVirtualDirectoryMap::cVirtualDirectoryMap()
{
}

cVirtualDirectoryMap::~cVirtualDirectoryMap()
{
	try
	{	
		m_Map.erase(m_Map.begin(), m_Map.end());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

string cVirtualDirectoryMap::GetVirtualDirectory(string &portName)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	try
	{
		m_MapIterator = m_Map.find(portName);
		if (m_MapIterator == m_Map.end())
			return string("");	
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return (*m_MapIterator).second;
}

void cVirtualDirectoryMap::MapVirtualDirectory(string &portName, string &virtualDirectory)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	try
	{
		m_Map.insert(pair<string,string>(portName, virtualDirectory));
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

bool cVirtualDirectoryMap::HasVirtualDirectory(string &portName)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	bool bRet = false;
	try
	{
		m_MapIterator = m_Map.find(portName);
		bRet =(m_MapIterator == m_Map.end()) ? false : true;
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return bRet;
}



