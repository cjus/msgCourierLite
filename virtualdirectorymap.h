/* virtualdirectorymap.h
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

virtualdirectorymap.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

virtualdirectorymap.h was developed by Carlos Justiniano for use on the
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
 @file virtualdirectorymap.h
 @brief Virtual directory map of port names to file system directory paths
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef VIRTUAL_DIRECTORY_MAP_H
#define VIRTUAL_DIRECTORY_MAP_H

#include <string>
#include <map>
#include "threadsync.h"

/**
 @class cVirtualDirectoryMap
 @brief Maps port names to directories
*/
class cVirtualDirectoryMap
{
public:
	cVirtualDirectoryMap();
	virtual ~cVirtualDirectoryMap();

	std::string GetVirtualDirectory(std::string &portName);
	void MapVirtualDirectory(std::string &portName, std::string &virtualDirectory);
	bool HasVirtualDirectory(std::string &portName);
private:
	cThreadSync m_ThreadSync;
	std::map<std::string,std::string> m_Map;
	std::map<std::string,std::string>::iterator m_MapIterator;
};

#endif // VIRTUAL_DIRECTORY_MAP_H

