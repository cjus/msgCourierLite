/* tcpserver.cpp
   Copyright (C) 2004 Carlos Justiniano

tcpserver.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

tcpserver.cpp was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with tcpserver.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file tcpserver.cpp
 @brief cTCPServer handles incoming TCP/IP messages
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)

 cTCPServer handles incoming TCP/IP messages by posting the connection on the
 cConnectionQueue.
*/

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
	#include <errno.h>
#endif //_PLATFORM_LINUX

#include "tcpserver.h"
#include "log.h"
#include "exception.h"

using namespace std;

#define TCPSERVER_ERROR_UPDATE_INTERVAL 10 // in seconds

cTCPServer::cTCPServer(cConnectionQueue *pConnectionQueue, const char *pIPAddress, int iPort, cIPAccess* pIPAccess)
: m_pConnectionQueue(pConnectionQueue)
, m_sIPAddress(pIPAddress)
, m_iPort(iPort)
, m_pSysMetrics(0)
, m_pIPAccess(pIPAccess)
, m_ServerSocketfd(-1)
, m_TCPMsgPerMinute(0)
{
	SetThreadName("cTCPServer");
	ResetErrorFlags();
}

cTCPServer::~cTCPServer()
{
}

int cTCPServer::Run()
{
    listen(m_ServerSocketfd, 1024);

	struct sockaddr_in client_addr;
    int client_len = sizeof(client_addr);
    int ClientSocketfd = 0;
    int rc;
	struct timeval tv;

    while (ThreadRunning())
    {
        /*
		if (m_timerMsgAcceptedPerMinute.IsReady())
		{
			LOG("TCP messages per minute on port [%d] = %d", m_iPort, m_TCPMsgPerMinute);
			m_TCPMsgPerMinute = 0;
			m_timerMsgAcceptedPerMinute.Reset();
		}
		*/

		if (m_timerErrorReporter.IsReady())
		{
			DisplayErrors();
			m_timerErrorReporter.Reset();
			ResetErrorFlags();
		}

		FD_ZERO(&m_stReadfds);
		FD_ZERO(&m_stExceptfds);
		FD_SET(m_ServerSocketfd, &m_stReadfds);
		FD_SET(m_ServerSocketfd, &m_stExceptfds);

		tv.tv_sec = TCPSERVER_ERROR_UPDATE_INTERVAL;
		tv.tv_usec = 0;

		rc = select(m_ServerSocketfd + 1, &m_stReadfds, (fd_set*)0, &m_stExceptfds, (struct timeval*)&tv);
		if (rc == -1)
		{
	        LOG2("@@@ TCP Server select() return a socket error");
			break;
		}
		if (rc != 0)
		{
			// check for exception
			if (FD_ISSET(m_ServerSocketfd, &m_stExceptfds))
			{
				// check for OOB data, or getsockopt() SO_ERROR
		        LOG2("@@@ TCP Server select() return a socket error");
				break;
			}
		}
		if (rc == 0)
		{
			// select() timed out
			continue;
		}

		if (!FD_ISSET(m_ServerSocketfd, &m_stReadfds))
			continue;

#ifdef _PLATFORM_LINUX
        ClientSocketfd = accept(m_ServerSocketfd, (struct sockaddr*)&client_addr, (size_t*)&client_len);
#endif //_PLATFORM_LINUX
#ifdef _PLATFORM_WIN32
		ClientSocketfd = accept(m_ServerSocketfd, (struct sockaddr*)&client_addr, &client_len);
#endif // _PLATFORM_WIN32
        if (ClientSocketfd == -1 && !IsActive())
        {
            // error caused by shutdown due to shutting down thread.
            // this is done to abort the select above so this thread
            // can exit!
			LOG2("**** error caused by shutdown");
            break;
        }
        else if (ClientSocketfd == -1)
		{
			//LOG("@@@ TCP Server unable to create client socket, error: %s(%d)", strerror(errno), errno);
			m_bWarningUnableToCreateClientSocket = true;
			if (m_pSysMetrics)
				m_pSysMetrics->IncConDropped();
            continue;
		}

		//LOG("TCP Connection accepted on port %d from: %s on socket: %d",
        //    m_iPort, inet_ntoa(client_addr.sin_addr), ClientSocketfd);

		if (m_pIPAccess->Validate(inet_ntoa(client_addr.sin_addr)) == HRC_IPACCESS_DENY)
		{
			if (m_pSysMetrics)
				m_pSysMetrics->IncConRejected();

			shutdown(ClientSocketfd, 2);
#ifdef _PLATFORM_LINUX
LOG2("close(%d) called", ClientSocketfd);
			close(ClientSocketfd);
#endif //_PLATFORM_LINUX
#ifdef _PLATFORM_WIN32
			closesocket(ClientSocketfd);
#endif //_PLATFORM_WIN32
			continue;
		}

		if (m_pSysMetrics)
			m_pSysMetrics->IncConAccepted();

	 // convert socket to non-blocking
#ifdef _PLATFORM_WIN32
		unsigned long value = 1;
		ioctlsocket(ClientSocketfd, FIONBIO, &value);
#else
		fcntl(ClientSocketfd, F_SETFL, O_NONBLOCK);
#endif

		int bufsize = 32768;
		//int bufsize = 65536;
		setsockopt(ClientSocketfd, SOL_SOCKET, SO_SNDBUF, (const char*)&bufsize, sizeof(bufsize));
		setsockopt(ClientSocketfd, SOL_SOCKET, SO_RCVBUF, (const char*)&bufsize, sizeof(bufsize));

		/*
		struct linger so_linger;
		so_linger.l_onoff = 1;
		so_linger.l_linger = 5;
		setsockopt(ClientSocketfd, SOL_SOCKET, SO_LINGER, (const char*)&so_linger, sizeof(so_linger));
		*/

		// Set NO_DELAY for Nagel algorithm
		//int yes = 1;
		//setsockopt(ClientSocketfd, IPPROTO_TCP, TCP_NODELAY, (const char*)&yes, sizeof(yes));

		m_TCPMsgPerMinute++;
		rc = m_pConnectionQueue->AddConnection(ClientSocketfd, (struct sockaddr*)&client_addr, m_iPort);
		if (rc != HRC_CONNECTION_QUEUE_OK)
        {
			// The implication here is that if the Connection Queue
			// is unable to service a connection the socket connection
			// is simply closed.  It's up to the calling application to
			// reconize that the server has closed the connection and
			// handle the suituation accordingly.
			m_bWarningConnectionQueueFull = true;

			//LOG2("Connection Queue is unable to service a connection");
			if (m_pSysMetrics)
				m_pSysMetrics->IncConDropped();
			shutdown(ClientSocketfd, 2);
#ifdef _PLATFORM_LINUX
LOG2("close(%d) called", ClientSocketfd);
			close(ClientSocketfd);
#endif //_PLATFORM_LINUX
#ifdef _PLATFORM_WIN32
			closesocket(ClientSocketfd);
#endif //_PLATFORM_WIN32
		}
    }

	LOG("*** TCP Server for port [%d] is exiting...", m_iPort);
	LOGALL("exit");
	CloseSocket();
    return HRC_TCPSERVER_OK;
}

void cTCPServer::CloseSocket()
{
	if (m_ServerSocketfd <= 0)
		return;
#ifdef _PLATFORM_LINUX
LOG2("close(%d) called", m_ServerSocketfd);
	close(m_ServerSocketfd);
#endif //_PLATFORM_LINUX
#ifdef _PLATFORM_WIN32
	closesocket(m_ServerSocketfd);
#endif //_PLATFORM_WIN32
	m_ServerSocketfd = -1;
}

int cTCPServer::Start()
{
	m_timerMsgAcceptedPerMinute.SetInterval(60);
	m_timerMsgAcceptedPerMinute.Start();

	m_timerErrorReporter.SetInterval(TCPSERVER_ERROR_UPDATE_INTERVAL);
	m_timerErrorReporter.Start();

    //
    // Init server socket
    //
    m_ServerSocketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_ServerSocketfd == -1)
	{
		THROW("Unable to create server socket");
		return HRC_TCPSERVER_SOCKET_ALLOC_FAILED;
	}

	 // convert socket to non-blocking
	 /*
#ifdef _PLATFORM_WIN32
	 unsigned long value = 1;
	 ioctlsocket(m_ServerSocketfd, FIONBIO, &value);
#else
	 fcntl(m_ServerSocketfd, F_SETFL, O_NONBLOCK);
#endif
	*/

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(m_sIPAddress.c_str());
	server_addr.sin_port = htons(m_iPort);
	int server_len = sizeof(server_addr);

	// Set NO_DELAY for Nagel algorithm
    int yes = 1;
	//setsockopt(m_ServerSocketfd, IPPROTO_TCP, TCP_NODELAY, (const char*)&yes, sizeof(yes));

	setsockopt(m_ServerSocketfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));
    if (bind(m_ServerSocketfd, (struct sockaddr*)&server_addr, server_len) != 0)
	{
		LOG("@@@ TCP Server socket bind error on port %d", m_iPort);
		LOGERROR("TCP Server socket bind error. Check msgCourier.xml configuration file");
		return HRC_TCPSERVER_SOCKET_BIND_FAILED;
    }

    int rc = cThread::Create();
	if (rc == HRC_THREAD_OK)
		cThread::Start();
	return rc;
}

int cTCPServer::Stop()
{
	CloseSocket();
	cThread::Destroy();
    return HRC_TCPSERVER_OK;
}

bool cTCPServer::TransmitResponse(int iSocketfd, char *pBuf, int size)
{
	int bytesSent;
	int totalSent = 0;
	do
	{
		bytesSent = send(iSocketfd, pBuf + totalSent, size - totalSent, 0);

#ifdef _PLATFORM_LINUX
		if (bytesSent == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			{
//              sleep(1);
				continue;
			}
			LOG("TCP send error: %s(%d)", strerror(errno), errno);
#endif //_PLATFORM_LINUX
#ifdef _PLATFORM_WIN32
		if (bytesSent == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
//				YieldSleep();
				continue;
			}
			LOG("TCP send error: (%d)", WSAGetLastError());
#endif //_PLATFORM_WIN32
			return false;
		}
		else
		{
			totalSent += bytesSent;
		}
	} while (totalSent < size);
	return true;
}

void cTCPServer::ResetErrorFlags()
{
	m_bWarningConnectionQueueFull = false;
	m_bWarningUnableToCreateClientSocket = false;
	m_bWarningConnectionWasNotValidated = false;
}

void cTCPServer::DisplayErrors()
{
	if (m_bWarningConnectionQueueFull == true)
	{
		LOG("TCPServer Warning: Connection Queue is full");
	}
	if (m_bWarningUnableToCreateClientSocket == true)
	{
		LOG("TCPServer Warning: Unable to create client socket");
	}
	if (m_bWarningConnectionWasNotValidated == true)
	{
		LOG("TCPServer Warning: Connection was dropped because IP validation failed");
	}
}
