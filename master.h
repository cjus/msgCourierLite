/* master.h
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

master.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

master.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with master.h; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file master.h
 @brief master configuration items.
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence (see source file header)

 master configuration items
*/

/**
 * @todo Consider moving master.h features to core.h
  */

#ifndef _MASTER_H
#define _MASTER_H

#include <stdio.h>
#include <stdlib.h>

#ifdef _PLATFORM_LINUX
	#include <unistd.h>
	#include <signal.h>
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32
	#include <windows.h>
#endif //_PLATFORM_WIN32

#include <assert.h>

#include "exception.h"

#ifdef _PLATFORM_WIN32
	#define APP_NAME "msgCourier.exe"
#else
	#define APP_NAME "msgCouier"
#endif //_PLATFORM_WIN32

#define APP_VERSION  "v0.3.0207"

#ifdef  _PLATFORM_WIN32
	#define APP_OS "MSWindows"
#endif //_PLATFORM_WIN32

#ifdef  _PLATFORM_LINUX
	#define APP_OS "Linux"
#endif //_PLATFORM_LINUX

#endif // _MASTER_H

