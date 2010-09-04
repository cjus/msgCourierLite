/* connectionrecord.h
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

connectionrecord.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

connectionrecord.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with main.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file connectionrecord.h
 @author Carlos Justiniano
 @brief Structure definition containing information pretaining to a TCP connection
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
*/
#ifndef CONNECTION_RECORD_H
#define CONNECTION_RECORD_H

#ifdef _PLATFORM_LINUX	
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <arpa/inet.h>
	#include <netdb.h>

	#include <sys/time.h>
	#include <sys/ioctl.h>
	#include <fcntl.h>
	#include <unistd.h>
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32
	#include "winsockoverride.h"
#endif //_PLATFORM_WIN32

#include <list>
#include "buffer.h"

class cConnectionHandler;

struct ConnectionRecord
{
    int Socketfd;
    struct sockaddr_in SocketAddr;
	int arrivalPort;
    int timestamp;
    int State;
	bool pendingwrite;
	enum ConnectionRecordState { CRS_FREE = 0, CRS_WAITING = 1  };
	cBuffer buffer;
	cConnectionHandler *pActiveHandler;
	std::list<ConnectionRecord*>::iterator m_it;
};

#endif //CONNECTION_RECORD_H

