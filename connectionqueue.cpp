/* connectionqueue.cpp
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

connectionqueue.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

connectionqueue.cpp was developed by Carlos Justiniano for use on the
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
 @file connectionqueue.cpp
 @brief Connection Queue is used by the cConnectionHandler to queue TCP/IP
 connections in an effort to handle high traffic connection
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#include <time.h>
#include <string.h>
#include "connectionqueue.h"
#include "exception.h"
#include "log.h"

using namespace std;

cConnectionQueue::cConnectionQueue(cMsgQueue *pMsgQueue, int iConnectionTimeout)
: m_pMsgQueue(pMsgQueue)
, m_iConnectionTimeout(iConnectionTimeout)
, m_pSysMetrics(0)
, m_PreQueueIndex(0)
{
	SetThreadName("cConnectionQueue");
}

cConnectionQueue::~cConnectionQueue()
{
}

int cConnectionQueue::Start()
{
	int iRet = HRC_CONNECTION_QUEUE_OK;
	cThread::Create();
	cThread::Start();
	return iRet;
}

int cConnectionQueue::Stop()
{
	int iRet = HRC_CONNECTION_QUEUE_OK;
	cThread::Destroy();
	return iRet;
}

int cConnectionQueue::Run()
{
	int maxsocketfd = 0;
	int socketcount;

	m_timer.SetInterval(1);
	m_timer.Start();

	m_outboundQueueTimer.SetInterval(10);
	m_outboundQueueTimer.Start();

	m_ConMsgTimer.SetInterval(60);
	m_ConMsgTimer.Start();

    while (ThreadRunning())
	{
		m_ThreadSync.Lock();

		socketcount = 0;
		if (m_timer.IsReady())
		{
			m_ConnectionCache.TimeoutConnections(m_iConnectionTimeout);
			m_timer.Reset();
		}

		// Add prequeued connections
		if (m_PreQueueIndex != 0)
		{
			cAutoThreadSync ThreadSync(&m_AddConnectionThreadSync);
			int ret;
			for (int i = 0; i < m_PreQueueIndex; i++)
			{
				ret = m_ConnectionCache.AddConnection(m_PreQueue[i].Socketfd, (sockaddr*)&m_PreQueue[i].SocketAddr, m_PreQueue[i].arrivalPort);
				//if (ret == HRC_CONNECTION_CACHE_OK)
				//{
				//}
			}
			m_PreQueueIndex = 0;
		}

		// this is for debug purposes only.
		// make sure to disable later
		/*
		if (m_ConMsgTimer.IsReady())
		{
			LOG("Active TCP connections: %d", m_ConnectionCache.GetActiveConnectionCount());
			m_ConMsgTimer.Reset();
		}
		*/

		maxsocketfd = PrepareConnections();
		if (maxsocketfd != 0)
		{
			socketcount = CheckConnections(maxsocketfd);
		}
		//if (socketcount)
			ProcessInboundTasks();
		if (m_outbound_priority_queue.empty() == false)
			ProcessOutboundTasks();

		m_ThreadSync.Unlock();
		YieldSleep();
    }
    return HRC_CONNECTION_QUEUE_OK;
}

int cConnectionQueue::PrepareConnections()
{
	int maxsocketfd = 0;
	FD_ZERO(&m_stReadfds);
	m_ConnectionCache.SetActiveConnections(&m_stReadfds, FD_SETSIZE, &maxsocketfd);
	return maxsocketfd;
}

int cConnectionQueue::CheckConnections(int maxsocketfd)
{
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	return select(maxsocketfd+1, &m_stReadfds, 0, 0, (struct timeval*)&tv);
}

int cConnectionQueue::ProcessInboundTasks()
{
	ConnectionRecord *pConnectionRecord = NULL;
	m_ConnectionCache.ResetEnumerator();
	while ((pConnectionRecord = m_ConnectionCache.GetNextSetRecord(&m_stReadfds)) != NULL)
	{
		m_InHandler.HandleConnection(this, pConnectionRecord);
		if (m_InHandler.DoCloseConnection())
			m_ConnectionCache.FreeConnection(pConnectionRecord->Socketfd);
	}
    return HRC_CONNECTION_QUEUE_OK;
}

int cConnectionQueue::ProcessOutboundTasks()
{
	int count = 0;
	cMsg *pMsg = 0;
	bool bDispatched = false;
	ConnectionRecord *pConnectionRecord;

	if (m_outboundQueueTimer.IsReady())
	{
		int size = m_outbound_priority_queue.size();
		if (size > 1)
			LOG("%d messages in outbound connection queue.", size);
		m_outboundQueueTimer.Reset();
	}

	try
	{
		while (m_outbound_priority_queue.empty() == false)
		{
			pMsg = m_outbound_priority_queue.top();
			MC_ASSERT(pMsg);

			if (pMsg->GetConnectionType() == cIMsg::MSG_CT_OB_TCP)
				pConnectionRecord = m_ConnectionCache.GetConnectionRecord(pMsg->GetTCPSocketHandle());
			else
				pConnectionRecord = NULL;

			m_OutHandler.HandleConnection(this, pMsg, pConnectionRecord);
			if (pMsg->GetConnectionType() == cIMsg::MSG_CT_OB_TCP)
			{
				if (m_OutHandler.DoCloseConnection())
				{
					m_ConnectionCache.FreeConnection(pMsg->GetTCPSocketHandle());
				}
			}
			m_outbound_priority_queue.pop();
			count++;
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	catch (cException &ex)
	{
		LOGALL("Exception in cConnectionQueue::ProcessOutboundTasks()");
	}
	catch (const char *pszException)
	{
		LOGALL(pszException);
	}
	catch (...)
	{
		LOGALL("Exception in cConnectionQueue::ProcessOutboundTasks()");
	}

	return count;
}

int cConnectionQueue::AddConnection(int Socketfd, struct sockaddr* pSocketAddr, int iArrivalPort)
{
	cAutoThreadSync ThreadSync(&m_AddConnectionThreadSync);
	ConnectionRecord rec;

	if (m_PreQueueIndex >= m_PreQueue.size())
	{
		m_PreQueue.push_back(rec);
	}
	m_PreQueue[m_PreQueueIndex].Socketfd = Socketfd;
	memcpy(&m_PreQueue[m_PreQueueIndex].SocketAddr, pSocketAddr, sizeof(struct sockaddr_in));
	m_PreQueue[m_PreQueueIndex].arrivalPort = iArrivalPort;
	m_PreQueueIndex++;
	return HRC_CONNECTION_QUEUE_OK;
}

int cConnectionQueue::FreeConnection(int Socketfd)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	m_ConnectionCache.FreeConnection(Socketfd);
	return HRC_CONNECTION_QUEUE_OK;
}

void cConnectionQueue::SendMsg(cMsg *pMsg)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	try
	{
		m_outbound_priority_queue.push(pMsg);
		if (pMsg->GetConnectionType() == cIMsg::MSG_CT_OB_TCP)
			m_ConnectionCache.MarkPendingWrite(pMsg->GetTCPSocketHandle());
		else if (pMsg->GetConnectionType() == cIMsg::MSG_CT_OB_UDP)
			m_ConnectionCache.MarkPendingWrite(pMsg->GetUDPSocketHandle());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	catch (...)
	{
		LOGALL("Exception in cConnectionQueue::SendMsg()");
	}
}
