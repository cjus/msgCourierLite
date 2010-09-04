/* isysinfo.h
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

isysinfo.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

isysinfo.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with main.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef ISYS_INFO_H
#define ISYS_INFO_H

/**
 @class cISysInfo
 @brief cISysInfo is an interface to the cSysInfo object
*/
class cISysInfo
{
public:
	virtual const char* Query(const char *pKey) = 0;
};

#endif // ISYS_INFO_H
