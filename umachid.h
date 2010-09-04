/* umachid.h
   Copyright (C) 2002 Carlos Justiniano

umachid.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

umachid.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with umachid.h; if not, write to the Free Software Foundation, Inc.,
675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 The cUMachID class provides a unique machine id
 @file umachid.h
 @brief Unique machine ID class
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence
*/

#ifndef _UMACHID_H
#define _UMACHID_H

#ifdef _PLATFORM_WIN32
#include <windows.h>
#endif //_PLATFORM_WIN32

#ifndef _PLATFORM_WIN32
#endif //!_PLATFORM_WIN32

#include <string>
#include "crc32.h"
#include "macaddr.h"

class cUMachID
{
public:
	cUMachID();
	const char *GetID();
private:
	std::string m_UMachID;
	cMACAddr m_MAC;
	cCRC32 m_CRC;
};

#endif //_UMACHID_H

