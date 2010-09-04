/* connectionqueue.h
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

connectionqueue.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

connectionqueue.h was developed by Carlos Justiniano for use on the
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
 @file connectionqueue.h
 @brief Connection Queue is used by the cConnectionHandler to queue TCP/IP
 connections in an effort to handle high traffic connection
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef CONNECTION_QUEUE_H
#define CONNECTION_QUEUE_H

#include <queue>
#include <vector>

#include "thread.h"
#include "threadsync.h"
#include "connectioncache.h"

#include "msgprioritycomp.h"
#include "connectionhandlerinbound.h"
#include "connectionhandleroutbound.h"

#include "msgqueue.h"
#include "timer.h"
#include "sysmetrics.h"

#define HRC_CONNECTION_QUEUE_OK     0x0000
#define HRC_CONNECTION_QUEUE_FULL   0x0001

/**
 @class cConnectionQueue
 @brief Connection Queue is used by the cConnectionHandler to queue TCP/IP connections
*/
class cConnectionQueue : cThread
{
public:
    cConnectionQueue(cMsgQueue *pMsgQueue, int iConnectionTimeout);
    ~cConnectionQueue();
    
	int Start();
	int Stop();

	void SetSysMetrics(cSysMetrics *pSysMetrics) { m_pSysMetrics = pSysMetrics; }
	cMsgQueue *GetMsgQueue() { return m_pMsgQueue; }
	cSysMetrics *GetSysMetrics() { return m_pSysMetrics; }

    int AddConnection(int Socketfd, struct sockaddr* pSocketAddr, int iArrivalPort);
	int FreeConnection(int Socketfd);
	void SendMsg(cMsg *pMsg);

protected:
    int Run();
private:
	int m_iConnectionTimeout;

	cThreadSync m_ThreadSync;
	cThreadSync m_AddConnectionThreadSync;

	cMsgQueue *m_pMsgQueue;
	cSysMetrics *m_pSysMetrics;

	cConnectionCache m_ConnectionCache;

	fd_set m_stReadfds;

	cTimer m_timer;
	cTimer m_outboundQueueTimer;
	cTimer m_ConMsgTimer;

	cConnectionHandlerInbound m_InHandler;
	cConnectionHandlerOutbound m_OutHandler;

	std::priority_queue<cMsg*, std::vector<cMsg*>, cMsgPriorityComp > m_outbound_priority_queue;
	std::vector<ConnectionRecord> m_PreQueue;
	int m_PreQueueIndex;

	int PrepareConnections();
	int CheckConnections(int maxsocketfd);
	int ProcessInboundTasks();
	int ProcessOutboundTasks();
};

#endif // CONNECTION_QUEUE_H

