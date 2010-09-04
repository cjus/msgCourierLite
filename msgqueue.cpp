/* msgqueue.cpp
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

msgqueue.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

msgqueue.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with main.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 0213e9, USA.
*/

/**
 @file msgqueue.cpp
 @brief Message Queue
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source
 file header)
 cMsgQueue is a threaded singleton class which queues messages for
 internal dispatching to message handlers.  cMsgQueue offers cMsg's
 using its CreateMessage() member function. Message handlers receive
 cMsg's for processing and flag the cMsg object as it's
 processed. cMsgQueue handles moving the cMsg objects throughout the
 msgCourier application and transmits messages to other clients and
 servers.
*/

#include <time.h>
#include <string.h>
#include "msgqueue.h"
#include "exception.h"
#include "log.h"
#include "connectionqueue.h"
#include "url.h"

#ifdef _PLATFORM_LINUX
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/dir.h>
#endif //_PLATFORM_LINUX

using namespace std;

#define MAX_MSG_RETRY	10

/**
 * cMsgQueue Queue Constructor
 * @param iMaxThreads Upper limit on the total threads used to handle message
 * processing
 */
cMsgQueue::cMsgQueue(cCommandHandlerFactory *pFactory, cMsgRouterRulesEngine *pRouterRulesEngine)
: m_pCommandHandlerFactory(pFactory)
, m_pRouterRulesEngine(pRouterRulesEngine)
, m_pSysMetrics(0)
{
	SetThreadName("cMsgQueue");
}

/**
 * cMsgQueue Destructor
 */
cMsgQueue::~cMsgQueue()
{
}

/**
 * MsgQueue start
 * @return HRC_MSG_QUEUE_OK or throws MEMALLOC
 * @throw MEMALLOC
 */
int cMsgQueue::Start()
{
	int iRet = HRC_MSG_QUEUE_OK;
	cThread::Create();
	cThread::Start();
	return iRet;
}

/**
 * Compliment to cMsgQueue::Start() shuts down thread and performs cleanup.
 * @return HRC_MSG_QUEUE_OK
 */
int cMsgQueue::Stop()
{
	int iRet = HRC_MSG_QUEUE_OK;
	Shutdown();
	cThread::Destroy();
	return iRet;
}

/**
 * Main processing loop which is executed by a thread
 * @return HRC_MSG_QUEUE_OK
 * @note This is the main work-horse for the cMsgQueue sington object.  In this
 * function pending cMsg objects are dispatched to cMsgHandler() thread-based
 * handlers.
*/
int cMsgQueue::Run()
{
	bool msgProcessed;
	m_timer.SetInterval(1);
	m_timer.Start();

    while (ThreadRunning())
    {
		m_ThreadSync.Lock();

		// Process message which are cMsg::MSG_STATE_PENDING
		ProcessPendingMessages();

		// check if there are any messages in the priority queue
		// and if so, process the highest priority message
		if (!m_priority_queue.empty())
		{
			ProcessPriorityQueue();
			msgProcessed = true;
		}
		else
		{
			msgProcessed = false;
		}

		// check to see if it's time to flush messages
		if (m_timer.IsReady())
		{
			FlushMessages();
			m_timer.Reset();
			/*
			#ifdef _PLATFORM_LINUX
				FileDescriptorWatch();
			#endif //_PLATFORM_LINUX
			*/
		}

		m_ThreadSync.Unlock();
		if (msgProcessed == false)
			YieldSleep();
	}

    return HRC_MSG_QUEUE_OK;
}

cIMsg *cMsgQueue::CreateMessage()
{
	cIMsg *pIMsg = m_msgPool.GetMessage();
	MARK_TRAVEL(pIMsg);
	return pIMsg;
}

cIMsg *cMsgQueue::CreateMessageReply(cIMsg *pToThisMsg)
{
	cAutoThreadSync ThreadSync(&m_CMRThreadSync);
	cIMsg *pNewMsg = m_msgPool.GetMessage();

	MARK_TRAVEL(pNewMsg);
 	MARK_TRAVEL(pToThisMsg);

	if (pToThisMsg != 0)
	{
		((cMsg*)pToThisMsg)->SetCloned();
		pNewMsg->SetTCPSocketHandle(((cMsg*)pToThisMsg)->GetTCPSocketHandle());
		((cMsg*)pNewMsg)->SetKeepAlive(((cMsg*)pToThisMsg)->IsKeepAlive());
		if (pToThisMsg->GetConnectionType() == cIMsg::MSG_CT_INTERNAL)
		{
			pNewMsg->SetConnectionType(cIMsg::MSG_CT_INTERNAL);
		}
		else if (pToThisMsg->GetConnectionType() == cIMsg::MSG_CT_IB_UDP)
		{
			pNewMsg->SetConnectionType(cIMsg::MSG_CT_OB_UDP);
		}
		else if (pToThisMsg->GetConnectionType() == cIMsg::MSG_CT_IB_TCP)
		{
			pNewMsg->SetConnectionType(cIMsg::MSG_CT_OB_TCP);
		}

		if (pToThisMsg->GetConnectionType() == cIMsg::MSG_CT_INTERNAL)
		{
			pNewMsg->SetCommand(pToThisMsg->GetFrom());
		}

		pNewMsg->SetMsgID(pToThisMsg->GetMsgID());
		pNewMsg->SetFormat(cIMsg::MSG_FORMAT_RES);
		pNewMsg->SetTo(pToThisMsg->GetFrom());

		pNewMsg->SetMsgPriority(pToThisMsg->GetMsgPriority());

		if (pToThisMsg->GetConnectionType() == cIMsg::MSG_CT_INTERNAL)
		{
			pNewMsg->SetReplyAction(cIMsg::MSG_REPLY_ACTION_NONE);
			pNewMsg->SetNotifyAction(cIMsg::MSG_NOTIFY_ACTION_NO);
		}
		else
		{
			pNewMsg->SetReplyAction(pToThisMsg->GetReplyAction());
			pNewMsg->SetNotifyAction(pToThisMsg->GetNotifyAction());
			pNewMsg->SetUDPSocketHandle(pToThisMsg->GetUDPSocketHandle());
			pNewMsg->SetSourceIP(pToThisMsg->GetSourceIP());
			pNewMsg->SetSourcePort(pToThisMsg->GetSourcePort());
		}

		//pNewMsg->SetProtocolType(cIMsg::MSG_PROTOCOL_MCP);
		pNewMsg->SetProtocolType(pToThisMsg->GetProtocolType());
		pNewMsg->SetArrivalPort(pToThisMsg->GetArrivalPort());
		*((cMsg*)pNewMsg)->GetBench() = *((cMsg*)pToThisMsg)->GetBench();
	}

MC_ASSERT(pNewMsg->GetMsgID());

	//LOG2("Msg [%s] reply created",pNewMsg->GetMsgID());
	return pNewMsg;
}

cIMsg* cMsgQueue::CreateInternalMessage()
{
	cMsg *pMsg = (cMsg*)m_msgPool.GetMessage();
	MARK_TRAVEL(pMsg);
	pMsg->SetConnectionType(cIMsg::MSG_CT_INTERNAL);
	pMsg->SetReplyAction(cIMsg::MSG_REPLY_ACTION_NONE);
	pMsg->SetNotifyAction(cIMsg::MSG_NOTIFY_ACTION_NO);
	return pMsg;
}


int cMsgQueue::PostNotifyMessage(cMsg *pBasedOnThisMsg)
{
	cMsg *pMsg = (cMsg*)m_msgPool.GetMessage();

	pMsg->SetTCPSocketHandle(pBasedOnThisMsg->GetTCPSocketHandle());
	if (pBasedOnThisMsg->GetConnectionType() == cIMsg::MSG_CT_IB_UDP)
		pMsg->SetConnectionType(cIMsg::MSG_CT_OB_UDP);
	else if (pBasedOnThisMsg->GetConnectionType() == cIMsg::MSG_CT_IB_TCP)
		pMsg->SetConnectionType(cIMsg::MSG_CT_OB_TCP);
	pMsg->SetMsgID(pBasedOnThisMsg->GetMsgID());
	pMsg->SetFormat(cIMsg::MSG_FORMAT_NOTIFY);
	pMsg->SetTo(pBasedOnThisMsg->GetFrom());
	pMsg->SetMsgPriority(cIMsg::MSG_PRIORITY_HIGHEST + 1000);
	pMsg->SetReplyAction(cIMsg::MSG_REPLY_ACTION_NOWAIT);
	pMsg->SetProtocolType(cIMsg::MSG_PROTOCOL_MCP);
	pMsg->SetNotifyAction(cIMsg::MSG_NOTIFY_ACTION_YES);
	*((cMsg*)pMsg)->GetBench() = *((cMsg*)pBasedOnThisMsg)->GetBench();

	MARK_TRAVEL(pMsg);
	pMsg->DispatchMsg();
	return HRC_MSG_QUEUE_OK;
}

int cMsgQueue::Shutdown()
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
    return HRC_MSG_QUEUE_OK;
}

int cMsgQueue::FlushMessages()
{
	int rc = HRC_MSG_QUEUE_OK;
	cIMsg::MSG_STATE msgState;
	cMsg *pMsg = 0;

	int cnt_tot = 0;
	int cnt_inuse = 0;
	int cnt_delivered = 0;
	int cnt_undelivered = 0;
	int cnt_processed = 0;
	int cnt_pending = 0;
	int currentTime = time(0);

	m_msgPool.Lock();

	m_msgPool.FlushFreeStack();
	pMsg = (cMsg*)m_msgPool.GetHead();
	if (pMsg == 0)
	{
		m_msgPool.Unlock();
		return rc;
	}
	do
	{
		cnt_tot++;
		msgState = pMsg->GetMsgState();
		switch (msgState)
		{
			case cIMsg::MSG_STATE_INACTIVE:
				pMsg->SetMsgState(cMsg::MSG_STATE_MARKFORREUSE);
				break;
			case cIMsg::MSG_STATE_UNDELIVERED:
				cnt_undelivered++;
				break;
			case cIMsg::MSG_STATE_DELIVERED:
				cnt_delivered++;
				MARK_TRAVEL(pMsg);
				pMsg->SetMsgState(cMsg::MSG_STATE_MARKFORREUSE);
				break;
			case cIMsg::MSG_STATE_PROCESSED:
				cnt_processed++;
				MARK_TRAVEL(pMsg);
				pMsg->SetMsgState(cMsg::MSG_STATE_MARKFORREUSE);
				break;
			case cIMsg::MSG_STATE_PENDING:
				cnt_pending++;
				break;
			case cIMsg::MSG_STATE_INUSE:
				cnt_inuse++;
				break;
			default:
				// cIMsg::MSG_STATE_INACTIVE:
				break;
		};

		if (msgState == cIMsg::MSG_STATE_INACTIVE && (currentTime - pMsg->GetMsgInUseTS()) > 60)
		{
			m_msgPool.DeleteCurrent();
			delete pMsg;
			pMsg = 0;
		}
		else if (pMsg->GetMsgState() == cMsg::MSG_STATE_MARKFORREUSE)
		{
			m_msgPool.ReturnToPool(pMsg);
		}

	} while ((pMsg = (cMsg*)m_msgPool.GetNext()) != 0);
	//LOG("FreeMsgPool size: %d", m_msgPool.Size());
	m_msgPool.Unlock();

//#ifdef _DEBUG
	if (cnt_inuse || cnt_pending)
	{
		LOG("     Flushing: tot(%d) iu(%d) u(%d) d(%d) p(%d) a(%d)",
			cnt_tot, cnt_inuse, cnt_undelivered, cnt_delivered, cnt_processed, cnt_pending);
	}
//#endif //_DEBUG

	//if (cnt_inuse > 32)
	//	THROW("Forced abort");
	return rc;
}

void cMsgQueue::ProcessPendingMessages()
{
	cMsg *pMsg;
	cIMsg::MSG_STATE msgState;

	m_msgPool.Lock();
	pMsg = (cMsg*)m_msgPool.GetHead();
	if (pMsg == 0)
	{
		m_msgPool.Unlock();
		return;
	}

	do
	{
		msgState = pMsg->GetMsgState();

		// check if any messages have been placed in a cIMsg::MSG_STATE_PENDING
		// state and add them to the priority queue for processing
		if (msgState == cMsg::MSG_STATE_PENDING)
		{
			MARK_TRAVEL(pMsg);
			pMsg->SetMsgState(cMsg::MSG_STATE_INUSE);
			if (pMsg->GetConnectionType() != cIMsg::MSG_CT_NOTSET)
			{
				MARK_TRAVEL(pMsg);
				m_priority_queue.push(pMsg);
			}
			else
			{
				MARK_TRAVEL(pMsg);
				MC_ASSERT(0);
			}
		}

		// if message was undelivered and it had a socket to respond to,
		// then we need to drop the message because it can no longer communicate
		// with the target server
		if (msgState == cMsg::MSG_STATE_UNDELIVERED && pMsg->GetTCPSocketHandle() != -1)
		{
			// remove message from message pool
			MARK_TRAVEL(pMsg);
			pMsg->SetMsgState(cMsg::MSG_STATE_MARKFORREUSE);
		}

		// if message was undelivered and it *doesn't have* a socket to respond to
		// then there isn't an available handler that can process it.  Try to resend
		// message at a later time.
		if (msgState == cMsg::MSG_STATE_UNDELIVERED && pMsg->GetTCPSocketHandle() == -1)
		{
			// check whether this message is set for delayed processing
			if (pMsg->GetSendRetryCount() != 0)
			{
				int curtime = time(0);
				int lastAttempt = pMsg->GetMsgLastRetryTS();
				if ((lastAttempt + pMsg->GetSendRetryDelay()) > curtime)
				{
					continue;
				}
			}

			int lastAttempt = pMsg->GetMsgLastRetryTS();
			if (lastAttempt == 0)
			{
				pMsg->SetMsgLastRetryTS();
				pMsg->SetSendRetryCount(1);

				MARK_TRAVEL(pMsg);
				pMsg->SetMsgState(cMsg::MSG_STATE_PENDING);
			}
			else
			{
				int iRetryCount = pMsg->GetSendRetryCount();
				if (iRetryCount > (MAX_MSG_RETRY-1))
				{
					// too many attempts to resend this message
					// mark message for removal from message pool
					MARK_TRAVEL(pMsg);
					pMsg->SetMsgState(cMsg::MSG_STATE_MARKFORREUSE);
				}
				else
				{
					pMsg->SetMsgLastRetryTS();
					pMsg->SetSendRetryCount(iRetryCount + 1);
					int iRetryDelay = pMsg->GetSendRetryDelay();
					pMsg->SetSendRetryDelay(iRetryDelay + iRetryDelay);
					MARK_TRAVEL(pMsg);
					pMsg->SetMsgState(cMsg::MSG_STATE_PENDING);
				}
			}
		}
	} while ((pMsg = (cMsg*)m_msgPool.GetNext()) != 0);
	m_msgPool.Unlock();
}

void cMsgQueue::ProcessPriorityQueue()
{
	cIMsg *pIMsg = m_priority_queue.top();
	m_priority_queue.pop();
MC_ASSERT(pIMsg);
MC_ASSERT(pIMsg->GetMsgID());
MC_ASSERT(pIMsg->GetCommand());

MARK_TRAVEL(pIMsg);
	//LOG("ProcessPriorityQueue");
	//LOG("     Msg (%d) %p[%s] taken from top of priority queue",
	//			pIMsg->GetConnectionType(), pIMsg, pIMsg->GetMsgID());

	// Check if internal message
	if (pIMsg->GetConnectionType() == cIMsg::MSG_CT_INTERNAL)
	{
		ProcessInternalMessage(pIMsg);
	}
	else if (pIMsg->GetConnectionType() == cIMsg::MSG_CT_OB_UDP ||
			 pIMsg->GetConnectionType() == cIMsg::MSG_CT_OB_TCP)
	{
		ProcessOutboundMessage(pIMsg);
	}
	else if (pIMsg->GetConnectionType() == cIMsg::MSG_CT_IB_UDP ||
			 pIMsg->GetConnectionType() == cIMsg::MSG_CT_IB_TCP)
	{
		ProcessInboundMessage(pIMsg);
	}
	else
	{
		// An unidentified connection type message
		// should not be on the priority queue!
L
		MC_ASSERT(0);
	}
}

void cMsgQueue::ProcessInternalMessage(cIMsg *pIMsg)
{
	MARK_TRAVEL(pIMsg);

	// first search for internal handler
	cICommandHandler *pHandler = 0;
	pHandler = m_pCommandHandlerFactory->GetCommandHandler(pIMsg->GetCommand());

	if (pHandler != 0)
	{
		m_pSysMetrics->IncMsgDelivered();
		pHandler->OnProcess(pIMsg);
		return;
	}
	else if (pHandler == 0)
	{
		// there is no internal handler, so search for a remote handler
		LOG("OOO No internal handler for %s", pIMsg->GetCommand());
	   	/*
		cServiceEntry *pService = m_pServiceEngine->QueryServiceFirst(pIMsg->GetCommand());
		if (pService != 0)
		{
			string sTo;
			cIMsg *pRemoteMsg = CreateMessage();
			MARK_TRAVEL(pRemoteMsg);
			pRemoteMsg->SetMsgID(pIMsg->GetMsgID());
			pRemoteMsg->SetCommand(pIMsg->GetCommand());
			pRemoteMsg->SetConnectionType(cIMsg::MSG_CT_OB_TCP);

			sTo = pService->m_pNode->m_MachineName;
			sTo += "@";
			sTo += pService->m_pNode->m_IPAddress;
			sTo += ":3400";
			pRemoteMsg->SetTo(sTo.c_str());

			pRemoteMsg->SetMsgPriority(cIMsg::MSG_PRIORITY_LOWEST);
			pRemoteMsg->SetArrivalPort(3400);
			pRemoteMsg->SetProtocolType(cIMsg::MSG_PROTOCOL_MCP);
			pRemoteMsg->SetReplyAction(cIMsg::MSG_REPLY_ACTION_NONE);
			pRemoteMsg->SetNotifyAction(cIMsg::MSG_NOTIFY_ACTION_NO);

			if (pIMsg->GetContentLength() > 0 )
			{
				pRemoteMsg->SetContentType("application/octet-stream");
				pRemoteMsg->SetContentPayload(pIMsg->GetContentPayload(), pIMsg->GetContentLength());
			}
			pRemoteMsg->DispatchMsg();

			MARK_TRAVEL(pIMsg);
			pIMsg->MarkProcessed();
			return;
		}
		else
		{
			//LOG("Warning, internal message can't be routed because a handler can't be located for %s",
			//	pIMsg->GetCommand());
			MARK_TRAVEL(pIMsg);
			((cMsg*)pIMsg)->SetMsgState(cMsg::MSG_STATE_UNDELIVERED);
			return;
		}
		*/
		((cMsg*)pIMsg)->SetMsgState(cMsg::MSG_STATE_UNDELIVERED);
	}
}

void cMsgQueue::ProcessInboundMessage(cIMsg *pIMsg)
{
	MARK_TRAVEL(pIMsg);
	if (pIMsg->GetNotifyAction() == cMsg::MSG_NOTIFY_ACTION_YES)
	{
		MARK_TRAVEL(pIMsg);

		LOG("     Msg requires a notify response, posting notify to priority queue");
		PostNotifyMessage((cMsg*)pIMsg);

		// Now remove notify message flag for this message
		pIMsg->SetNotifyAction(cIMsg::MSG_NOTIFY_ACTION_NO);
	}

	cICommandHandler *pHandler = 0;
	string Match = "";

	// check for message reply
	if (strncmp(pIMsg->GetCommand(), "MCP/", 4) == 0)
	{
		pHandler = m_pCommandHandlerFactory->GetCommandHandler("MC");
		m_pSysMetrics->IncMsgDelivered();
		pHandler->OnProcess(pIMsg);
		return;
	}

	m_pRouterRulesEngine->Search((cMsg*)pIMsg, Match);
	if (Match.length() != 0)
	{
		//LOG("     Msg was matched against %s and will be processed by that handler", Match.c_str());
		pHandler = m_pCommandHandlerFactory->GetCommandHandler(Match.c_str());
		MARK_TRAVEL(pIMsg);
	}
	else
	{
		//LOG("     Msg was not matched and will be processed by the [%s] handler", pIMsg->GetCommand());
		pHandler = m_pCommandHandlerFactory->GetCommandHandler(pIMsg->GetCommand());
		MARK_TRAVEL(pIMsg);
	}

	//if (pHandler == 0)
	//{
	//	// No internal handler found.  Search for remote (service) handler
	//	cServiceEntry *pService = m_pServiceEngine->QueryServiceFirst(pIMsg->GetCommand());
	//	if (pService != 0)
	//	{
	//		// route to MC handler
	//		pHandler = m_pCommandHandlerFactory->GetCommandHandler("MC");
	//		((cMsg*)pIMsg)->SetRemoteHandler(pService);
	//		m_pSysMetrics->IncMsgDelivered();
	//		pHandler->OnProcess(pIMsg);
	//		//LOG("Remote handler located, routing msg = %s", pIMsg->GetContentPayload());
	//	}
	//	else
	//	{
	//		// route to MC handler
	//		pHandler = m_pCommandHandlerFactory->GetCommandHandler("MC");

	//		//LOG("Error, msg [%s] arrived from [%s], but there is no command handler registered.",
	//		//	pIMsg->GetCommand(), pIMsg->GetSourceIP());
	//		pIMsg->SetCommand("MC.ERROR");
	//		m_pSysMetrics->IncMsgDelivered();
	//		pHandler->OnProcess(pIMsg);
	//	}
	//}
	//else
	//{
		MARK_TRAVEL(pIMsg);
		m_pSysMetrics->IncMsgDelivered();
		pHandler->OnProcess(pIMsg);
	//}
}

void cMsgQueue::ProcessOutboundMessage(cIMsg *pIMsg)
{
	MARK_TRAVEL(pIMsg);

	// if msg's connection type is an outbound UDP or TCP based message
	if (pIMsg->GetReplyAction() != cIMsg::MSG_REPLY_ACTION_NONE)
	{
		m_pConnectionQueue->SendMsg((cMsg*)pIMsg);
	}
	else
	{
		pIMsg->MarkProcessed();
	}
}

void cMsgQueue::SendToLogger(const char *pCategory, const char *pLogMessage)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);

	try
	{
		if (pCategory == 0 || pLogMessage == 0 ||
			strlen(pCategory) == 0 || strlen(pLogMessage) == 0)
			return;

		string sExpression = "(LOGGER (";
		sExpression += pCategory;
		sExpression += " \"";
		cIMsg *pLogMsg = CreateMessage();
		if (pLogMsg == 0)
		{
			LOG2("Warning, SendToLogger: message is null");
			return;
		}
		MARK_TRAVEL(pLogMsg);
		pLogMsg->SetCommand("LOGGER");
		pLogMsg->SetContentType("text/s-expression");
		pLogMsg->SetConnectionType(cIMsg::MSG_CT_INTERNAL);
		pLogMsg->SetMsgPriority(cIMsg::MSG_PRIORITY_LOWEST);
		pLogMsg->SetProtocolType(cIMsg::MSG_PROTOCOL_MCP);
		pLogMsg->SetReplyAction(cIMsg::MSG_REPLY_ACTION_NONE);
		pLogMsg->SetNotifyAction(cIMsg::MSG_NOTIFY_ACTION_NO);
		pLogMsg->SetMsgID("0");

		string preEncoded, urlEncoded;
		preEncoded = pLogMessage;
		cURL url;
		url.Encode(preEncoded, urlEncoded);
		sExpression += urlEncoded;
		sExpression += "\"))";
		pLogMsg->SetContentPayload(sExpression.c_str(), sExpression.length());

		pLogMsg->DispatchMsg();
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

/*
void cMsgQueue::SendToConsole(const char *pText)
{
	LOG(pText);
}
*/

#ifdef _PLATFORM_LINUX
void cMsgQueue::FileDescriptorWatch()
{
	char path[260];
	sprintf(path, "/proc/%d/fd", getpid());

	int cnt = 0;
	struct direct *DirEntryPtr;
	DIR *DirPtr = opendir(path);
	while (1)
	{
		DirEntryPtr = readdir(DirPtr);
		if (DirEntryPtr == 0)
			break;
		if (strcmp(DirEntryPtr->d_name,".") != 0 && strcmp(DirEntryPtr->d_name,"..") != 0)
		{
			cnt++;
		}
	}
	closedir(DirPtr);
	LOG("Total file descriptors in use: %d", cnt-4);
}
#endif //_PLATFORM_LINUX
