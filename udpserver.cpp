/* udpserver.cpp
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

udpserver.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

udpserver.cpp was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with udpserver.cpp; if not, write to the Free Software Foundation, Inc.,
675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file udpserver.cpp
 @brief UDP Server
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence
*/

#include "master.h"

#ifdef _PLATFORM_LINUX	
	#include <sys/time.h>
	#include <sys/resource.h>
	#include <unistd.h>

	#include <netdb.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/ioctl.h>
	#include <sys/types.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <sys/sysinfo.h>
	#include <netinet/in.h>
#endif //PLATFORM_LINUX

#include <string.h>

#include "udpserver.h"
#include "log.h"
#include "exception.h"
#include "msg.h"

cUDPServer::cUDPServer(cMsgQueue *pMsgQueue, const char *pBindAddr, int iPort, cIPAccess* pIPAccess)
: m_pMsgQueue(pMsgQueue)
, m_iPort(iPort)
, m_ServerSocketfd(0)
, m_pSysMetrics(0)
, m_pIPAccess(pIPAccess)
{
	SetThreadName("cUDPServer");
    strncpy(m_BindAddr, pBindAddr, IP_ADDR_SIZE);
    m_BindAddr[IP_ADDR_SIZE] = 0;
}

cUDPServer::~cUDPServer()
{
}

int cUDPServer::Run()
{
	struct sockaddr_in client_addr;
    int rc;
	struct timeval tv;
	
    while (ThreadRunning())
    {
        int client_len = sizeof(client_addr);

		FD_ZERO(&m_stReadfds);
		FD_SET(m_ServerSocketfd, &m_stReadfds);

		tv.tv_sec = 10;
		tv.tv_usec = 0;
		rc = select(m_ServerSocketfd + 1, &m_stReadfds, (fd_set*)0, (fd_set*)0, (struct timeval*)&tv);
		if (rc == 0)
			continue; 
        if (rc == -1)
		{
	        LOG("@@@ UDP Server select() return a socket error");
			break;
		}
		if (rc != 0)
		{
			if (!(FD_ISSET(m_ServerSocketfd, &m_stReadfds)))
			{
		        LOG("@@@ UDP Server select() this should never happen!");
				break;
			}
		}
		
		cMsg *pNewMessage = (cMsg*)m_pMsgQueue->CreateMessage();
		MARK_TRAVEL(pNewMessage);
		cBuffer *pBuffer = pNewMessage->GetBuffer();
		pBuffer->SetBufferSize(MAX_DGRAM_SIZE);
		char *pData = (char*)pBuffer->GetRawBuffer();

#ifdef _PLATFORM_LINUX
        rc = recvfrom(m_ServerSocketfd, pData, MAX_DGRAM_SIZE, 0, 
                      (struct sockaddr*)&client_addr, (size_t*)&client_len);
#endif //_PLATFORM_LINUX
#ifdef _PLATFORM_WIN32
        rc = recvfrom(m_ServerSocketfd, pData, MAX_DGRAM_SIZE, 0, 
                      (struct sockaddr*)&client_addr, &client_len);
#endif //_PLATFORM_WIN32

        if (rc < 0)
		{
            LOG("@@@ UDP Server unable to recvfrom");
			MARK_TRAVEL(pNewMessage);
			pNewMessage->SetMsgState(cMsg::MSG_STATE_MARKFORREUSE);
            continue;
		}
        else if (rc == 0)
        {
            // time to shutdown
            break;
        }

		if (m_pIPAccess->Validate(inet_ntoa(client_addr.sin_addr)) == HRC_IPACCESS_DENY)
		{
			if (m_pSysMetrics)
				m_pSysMetrics->IncConRejected();
			MARK_TRAVEL(pNewMessage);
			pNewMessage->SetMsgState(cMsg::MSG_STATE_MARKFORREUSE);
			continue;
		}

		pBuffer->SetBufferLength(rc);
		pNewMessage->SetSourceIP(inet_ntoa(client_addr.sin_addr));

		rc = pNewMessage->Parse(cIMsg::MSG_CT_IB_UDP, cIMsg::MSG_FORMAT_REQ);
		if (rc == HRC_MSG_OK)
		{
			// new message is now ready to be processed by a message handler
			pNewMessage->SetConnectionType(cIMsg::MSG_CT_IB_UDP);
			pNewMessage->SetProtocolType(cIMsg::MSG_PROTOCOL_MCP);
			pNewMessage->SetArrivalPort(m_iPort);

			MARK_TRAVEL(pNewMessage);
			pNewMessage->DispatchMsg();
		}
		else
		{
			MARK_TRAVEL(pNewMessage);
			pNewMessage->SetMsgState(cMsg::MSG_STATE_MARKFORREUSE);
		}

		if (pNewMessage->GetReplyAction() == cIMsg::MSG_REPLY_ACTION_WAIT)
		{
			pNewMessage->SetSourcePort(client_addr.sin_port);
			pNewMessage->SetUDPSocketHandle(m_ServerSocketfd);
		}
    }

	CloseSocket();
    return HRC_UDPSERVER_OK;
}

void cUDPServer::CloseSocket()
{
	if (m_ServerSocketfd != -1)
	{
#ifdef _PLATFORM_LINUX
		close(m_ServerSocketfd);
#endif //_PLATFORM_LINUX
#ifdef _PLATFORM_WIN32
		closesocket(m_ServerSocketfd);
#endif //_PLATFORM_WIN32
	}
}

int cUDPServer::Start()
{
    //
    // Init server socket
    //
    m_ServerSocketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_ServerSocketfd == -1)
	{
        LOG("@@@ UDP Server unable to create server socket");
		return HRC_UDPSERVER_SOCKET_ALLOC_FAILED;
    }

	 // convert socket to non-blocking
#ifdef _PLATFORM_WIN32
	 unsigned long value = 1;
	 ioctlsocket(m_ServerSocketfd, FIONBIO, &value);
#else
	 fcntl(m_ServerSocketfd, F_SETFL, O_NONBLOCK);
#endif

	struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(m_BindAddr);
	server_addr.sin_port = htons(m_iPort);
	int server_len = sizeof(server_addr);

    int yes = 1;
	setsockopt(m_ServerSocketfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));
    if (bind(m_ServerSocketfd, (struct sockaddr*)&server_addr, server_len) != 0)
	{
        LOG("@@@ UDP Server socket bind error");
        LOGERROR("UDP Server socket bind error. Check msgCourier.xml configuration file");
		return HRC_UDPSERVER_SOCKET_BIND_FAILED;
    }

    int rc = cThread::Create();
	if (rc == HRC_THREAD_OK)
		cThread::Start();
	return rc;
}

int cUDPServer::Stop()
{
	CloseSocket();
    cThread::Destroy();
    return HRC_UDPSERVER_OK;
}

