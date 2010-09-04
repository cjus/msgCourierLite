/* sysinfo.h
   Copyright (C) 2006 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

sysinfo.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

sysinfo.h was developed by Carlos Justiniano for use on the
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
 @file sysinfo.h
 @brief System Information
 @author Carlos Justiniano
 @attention Copyright (C) 2006 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef SYS_INFO_H
#define SYS_INFO_H

#include <string>
#include <map>
#include "isysinfo.h"
#include "threadsync.h"

/**
 @class cSysInfo
 @brief System Information
 */
class cSysInfo : cISysInfo
{
public:
	cSysInfo();
	~cSysInfo();
	const char* Query(const char *pKey);
	void SetSysInfo(const char *pKey, const char *pValue);
private:
	cThreadSync m_ThreadSync;
	std::map<std::string,std::string> m_SysInfoMap;
};

#endif // SYS_INFO_H
