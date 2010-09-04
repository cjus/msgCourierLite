/* cbstrtok.h
   Copyright (C) 2002 Carlos Justiniano

cbstrtok.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

cbstrtok.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with cbstrtok.h; if not, write to the Free Software Foundation, Inc.,
675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file cbstrtok.h
 @brief Multithreaded strtok defines for use under Linux
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef CBSTRTOK_H
#define CBSTRTOK_H

#define STRTOKVA char *pMarker
#define STRTOKMK pMarker
#ifdef _PLATFORM_LINUX
	#define STRTOK(ST_P1, ST_P2) strtok_r(ST_P1, ST_P2, &STRTOKMK)
#endif // _PLATFORM_LINUX
#ifndef _PLATFORM_LINUX
	#define STRTOK(ST_P1, ST_P2) strtok(ST_P1, ST_P2)
#endif // !_PLATFORM_LINUX

#endif //CBSTRTOK_H

