/* tcpserver.h
   Copyright (C) 2004 Carlos Justiniano

tcpserver.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

tcpserver.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with tcpserver.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file tcpserver.h 
 @brief cTCPServer handles incoming TCP/IP messages 
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)

 cTCPServer handles incoming TCP/IP messages 
*/

#ifndef _TCPSERVER_H
#define _TCPSERVER_H

#include <string>
#include "thread.h"
#include "sysmetrics.h"
#include "connectionqueue.h"
#include "ipaccess.h"
#include "timer.h"

#define HRC_TCPSERVER_OK                   0x0000
#define HRC_TCPSERVER_SOCKET_ALLOC_FAILED  0x0001
#define HRC_TCPSERVER_SOCKET_BIND_FAILED   0x0002

/**
 @class cTCPServer 
 @brief cTCPServer handles incoming TCP/IP messages 
*/
class cTCPServer : public cThread
{
public:
    cTCPServer(cConnectionQueue *pConnectionQueue, const char *pIPAddress, int iPort, cIPAccess* pIPAccess);
    ~cTCPServer();

	void SetSysMetrics(cSysMetrics *pSysMetrics) {m_pSysMetrics = pSysMetrics; }

    int Start();
    int Stop();
protected:
    int Run();
private:
	cConnectionQueue *m_pConnectionQueue;
	std::string m_sIPAddress;	
    int m_iPort;
	int m_ServerSocketfd;
	fd_set m_stReadfds;
	fd_set m_stExceptfds;
	cSysMetrics *m_pSysMetrics;
	cIPAccess* m_pIPAccess;

	cTimer m_timerErrorReporter;
	cTimer m_timerMsgAcceptedPerMinute;
	int m_TCPMsgPerMinute;

	bool m_bWarningConnectionQueueFull;
	bool m_bWarningUnableToCreateClientSocket;
	bool m_bWarningConnectionWasNotValidated;

    bool TransmitResponse(int iSocketfd, char *pBuf, int size);
    bool SendConnectionQueueFullMessage(int iSocketfd);
	void CloseSocket();
	void ResetErrorFlags();
	void DisplayErrors();

};

#endif //_TCPSERVER_H

