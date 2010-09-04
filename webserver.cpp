/* webserver.cpp
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

webserver.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

webserver.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with webserver.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file webserver.cpp
 @brief WebServer component
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
 webserver implementation class.
*/

#include "log.h"
#include "exception.h"
#include "webserver.h"
#include "msg.h"

using namespace std;
#define COMMAND_NAME "MCWS"

cWebServer::cWebServer(cVirtualDirectoryMap *pVirtualDirectoryMap, int iMaxHandlers)
: m_pVirtualDirectoryMap(pVirtualDirectoryMap)
, m_iMaxHandlers(iMaxHandlers)
, m_iTotalThreads(0)
{
}

char *cWebServer::GetCommandName()
{
	return COMMAND_NAME;
}

void cWebServer::OnStartup(cIMsgQueue *pMsgQueue, cISysInfo *pSysInfo, cILogger *pLogger)
{
	m_pMsgQueue = pMsgQueue;
	m_pSysInfo = pSysInfo;
}

void cWebServer::OnShutdown()
{
	try
	{			
		// cleanup handlers
		vector<cWebRequestHandler*>::iterator it;
		for (it = m_WebRequestHandlers.begin(); it != m_WebRequestHandlers.end(); it++)
		{
			if (*it)
				delete (*it);
		}	
		m_WebRequestHandlers.erase(m_WebRequestHandlers.begin(),m_WebRequestHandlers.end());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

int cWebServer::OnProcess(cIMsg *pMessage)
{
	bool bAssigned = false;

	try
	{
		vector<cWebRequestHandler*>::iterator it;
		for (it = m_WebRequestHandlers.begin(); it != m_WebRequestHandlers.end(); it++)
		{
			if ((*it)->IsConnected()==false)
			{
				//LOG("WebServer is reusing thread");
				(*it)->Setup(m_pVirtualDirectoryMap, m_pMsgQueue, pMessage);
				int rc = (*it)->Start();
				if (rc == HRC_THREAD_OK)
				{
					bAssigned = true;
					break;
				}
				else
				{
					LOG("cWebServer::Unable to start handler | Unable to create thread");
					MC_ASSERT(0);						
				}
			}
		}

		if (bAssigned == false)
		{
			if (m_WebRequestHandlers.size() < m_iMaxHandlers)
			{
				m_iTotalThreads++;
				LOG("%d of %d WebServer threads created", m_iTotalThreads, m_iMaxHandlers);

				cWebRequestHandler *pHandler;
				MC_NEW(pHandler, cWebRequestHandler());
				m_WebRequestHandlers.push_back(pHandler);
				pHandler->Setup(m_pVirtualDirectoryMap, m_pMsgQueue, pMessage);
				int rc = pHandler->Start();
				if (rc == HRC_THREAD_OK)
				{
					bAssigned = true;
				}
				else
				{
					LOG("cWebServer::Unable to start handler | Unable to create thread");
					MC_ASSERT(0);						
				}
			}
			else
			{
				// can't process message at this time.  set message state back to pending
				cMsg *pMsg = (cMsg*)pMessage;
				MARK_TRAVEL(pMsg);
				pMsg->SetMsgState(cIMsg::MSG_STATE_PENDING);
			}
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}

	return 0;
}

