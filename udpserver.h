/* udpserver.h
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

udpserver.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

udpserver.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with udpserver.h; if not, write to the Free Software Foundation, Inc.,
675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file udpserver.h
 @brief UDP Server
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence
*/
#ifndef _UDPSERVER_H
#define _UDPSERVER_H

#include "thread.h"
#include "buffer.h"
#include "sysmetrics.h"
#include "msgqueue.h"
#include "ipaccess.h"

#define HRC_UDPSERVER_OK					0x0000
#define HRC_UDPSERVER_SOCKET_ALLOC_FAILED	0x0001
#define HRC_UDPSERVER_SOCKET_BIND_FAILED	0x0002
#define HRC_UDPSERVER_INVALID_MESSAGE		0x0003

#define IP_ADDR_SIZE 16
#define MAX_DGRAM_SIZE 512

/**
 @class cUDPServer 
 @brief UDP Server
*/
class cUDPServer : public cThread
{
public:
    cUDPServer(cMsgQueue *pMsgQueue, const char *pBindAddr, int iPort, cIPAccess* pIPAccess);
    ~cUDPServer();

	void SetSysMetrics(cSysMetrics *pSysMetrics) {m_pSysMetrics = pSysMetrics; }

    int Start();
    int Stop();
protected:
    int Run();
private:
	cMsgQueue *m_pMsgQueue;
    int m_iPort;
    char m_BindAddr[IP_ADDR_SIZE];

	int m_ServerSocketfd;
	fd_set m_stReadfds;
	cSysMetrics *m_pSysMetrics;
    cIPAccess *m_pIPAccess;

	void CloseSocket();
};

#endif //_UDPSERVER_H
