/* macaddr.h
   Copyright (C) 2002 Carlos Justiniano

macaddr.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

macaddr.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with macaddr.h; if not, write to the Free Software Foundation, Inc.,
675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 The cMACAddr class retrieves the machine's network MAC address
 @file macaddr.h
 @brief MAC Address ID class
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence
*/

#ifndef _MACADDR_H
#define _MACADDR_H

#ifdef _PLATFORM_WIN32
#include <windows.h>
#endif //_PLATFORM_WIN32

#ifndef _PLATFORM_WIN32
#endif //!_PLATFORM_WIN32

#include <string>

class cMACAddr
{
public:
	cMACAddr();
	const char *GetMACAddr();
private:
	std::string m_MACAddress;
};

#endif //_MACADDR_H
