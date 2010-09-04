/* msgqueue.h
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

msgqueue.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

msgqueue.h was developed by Carlos Justiniano for use on the
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
 @file msgqueue.h
 @brief Message Queue 
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)

 cMsgQueue is a threaded singleton class which queues messages for
 internal dispatching to message handlers.  cMsgQueue offers cMsg's
 using its CreateMessage() member function. Message handlers receive
 cMsg's for processing and flag the cMsg object as it's processed. 
 cMsgQueue handles moving the cMsg objects throughout the
 msgCourier application and transmits messages to other clients and
 servers.
*/

#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include <queue>
#include <vector>

#include "thread.h"
#include "threadsync.h"
#include "msg.h"
#include "imsg.h"
#include "imsgqueue.h"
#include "msgpool.h"
#include "msgprioritycomp.h"
#include "msgrouterrulesengine.h"
#include "commandhandlerfactory.h"
#include "timer.h"
#include "sysmetrics.h"

#define HRC_MSG_QUEUE_OK     0x0000
#define HRC_MSG_QUEUE_END    0x0001
#define HRC_MSG_QUEUE_FULL   0x0002
#define HRC_MSG_QUEUE_STARTFAILED	0x0003
#define HRC_MSG_QUEUE_DB_TABLE_CREATE_FAILED 0x0004
#define HRC_MSG_QUEUE_DB_FAILED 0x0005

class cConnectionQueue;

/**
 @class cMsgQueue 
 @brief Message Queue 
*/
class cMsgQueue : public cIMsgQueue, cThread
{
public:
    cMsgQueue(cCommandHandlerFactory *pFactory, cMsgRouterRulesEngine *pRouterRulesEngine);
    ~cMsgQueue();

	void SetConnectionQueue(cConnectionQueue *pConnectionQueue) { m_pConnectionQueue = pConnectionQueue; }
	void SetSysMetrics(cSysMetrics *pSysMetrics) { m_pSysMetrics = pSysMetrics; }

	int Start();
	int Stop();

	cIMsg* CreateMessage();
	cIMsg* CreateMessageReply(cIMsg* pToThisMsg);
	cIMsg* CreateInternalMessage();

	void SendToLogger(const char *pCategory, const char *pLogMessage);
	//void SendToConsole(const char *pText);

	int UseDatabase(char *pName, int iFlushInterval);
    int Shutdown();
protected:
    int Run();
private:
	cThreadSync m_ThreadSync;
	cThreadSync m_CMRThreadSync;

	cCommandHandlerFactory *m_pCommandHandlerFactory;
	cSysMetrics *m_pSysMetrics;
	cConnectionQueue *m_pConnectionQueue;

	cTimer m_timer;

	cMsgPool m_msgPool;
	std::priority_queue<cIMsg*, std::vector<cIMsg*>, cMsgPriorityComp > m_priority_queue;
	cMsgRouterRulesEngine *m_pRouterRulesEngine;

	int FlushMessages();
	int PostNotifyMessage(cMsg *pBasedOnThisMsg);

	void ProcessPendingMessages();
	void ProcessPriorityQueue();
	void ProcessInternalMessage(cIMsg *pIMsg);
	void ProcessInboundMessage(cIMsg *pIMsg);
	void ProcessOutboundMessage(cIMsg *pIMsg);
#ifdef _PLATFORM_LINUX	
	void FileDescriptorWatch();
#endif //_PLATFORM_LINUX	
};

#endif // MSG_QUEUE_H
