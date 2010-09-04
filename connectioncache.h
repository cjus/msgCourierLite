/* connectioncache.h
   Copyright (C) 2006 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

connectioncache.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

connectioncache.h was developed by Carlos Justiniano for use on the
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
 @file connectioncache.h
 @author Carlos Justiniano
 @brief TCP/IP Connection Cache
 @attention Copyright (C) 2006 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef _CONNECTION_CACHE_H
#define _CONNECTION_CACHE_H

#include <list>
#include <map>
#include "connectionrecord.h"

#define HRC_CONNECTION_CACHE_OK					0x0000
#define HRC_CONNECTION_CACHE_ENTRY_NOT_FOUND	0x0001

class cConnectionCache
{
public:
	cConnectionCache();
	~cConnectionCache();

	int AddConnection(int Socketfd, struct sockaddr* pSocketAddr, int iArrivalPort);
	int FreeConnection(int Socketfd);
	ConnectionRecord *GetConnectionRecord(int Socketfd);
	int TimeoutConnections(int iTimeout);

	int SetActiveConnections(fd_set *fds, int size, int *maxsocketfd);
	void ResetEnumerator();
	ConnectionRecord *GetNextSetRecord(fd_set *fds);

	int GetActiveConnectionCount();
	int GetTotalConnectionEntries();

	void MarkPendingWrite(int Socketfd);
private:
	std::list<ConnectionRecord *> m_ActiveConnectionList;
	std::map<int, ConnectionRecord> m_ConnectionMap;
	std::list<ConnectionRecord *>::iterator m_it;
	std::list<ConnectionRecord *>::iterator m_itIncompleteMarker;

    void CloseSocket(int socketfd);
};

#endif //_CONNECTION_CACHE_H
