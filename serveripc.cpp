/* serveripc.cpp
   Copyright (C) 2002 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

serveripc.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

serveripc.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with serveripc.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#ifdef _PLATFORM_WIN32
	#include <windows.h>
	#include <winsock.h>
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netdb.h>
	#include <netinet/in.h>
    #include <netinet/tcp.h>
	#include <arpa/inet.h>
	#include <sys/time.h>
	#include <sys/ioctl.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <errno.h>
#endif

#include "serveripc.h"
//#include "core.h"
//#include "log.h"

cServerIPC::cServerIPC()
: m_Socketfd(0)
{
	Reset();
}

cServerIPC::~cServerIPC()
{
	Close();
}

int cServerIPC::Open(char *pServer, int port)
{
	Close();

	if (pServer == NULL || (port < 0 || port > 65386))
		return HRC_SERVERIPC_INVALID_PARAMS;

	struct in_addr	iaHost;
	struct hostent  *HostEntry=NULL;
	bool bByName=false;

	iaHost.s_addr = inet_addr(pServer);
	if (iaHost.s_addr == INADDR_NONE)
	{
		// Wasn't an IP address string, assume it is a name
		HostEntry = gethostbyname(pServer);
        if (HostEntry==NULL)
        {
			//LOG("SIPC: Unable to connect to %s on port %d", pServer, port);
            return HRC_SERVERIPC_UNABLE_TO_CONNECT_TO_HOST;
        }
        bByName=true;
	}

	struct sockaddr_in socketaddr;
	memset(&socketaddr, 0, sizeof(socketaddr));
	socketaddr.sin_family = AF_INET;
	socketaddr.sin_port = htons(port);
    if (bByName)
	{
        memcpy(&socketaddr.sin_addr.s_addr, HostEntry->h_addr, sizeof(in_addr));
	}
    else
	{
        //socketaddr.sin_addr.s_addr = inet_addr(pServer);
#ifndef _PLATFORM_WIN32
		if (!inet_aton(pServer, &socketaddr.sin_addr))
		{
			HostEntry = gethostbyname(pServer);
			if (HostEntry == NULL)
				return HRC_SERVERIPC_UNABLE_TO_CONNECT_TO_HOST;
			socketaddr.sin_addr = *(struct in_addr*)HostEntry->h_addr;
		}
#endif //!_PLATFORM_WIN32

#ifdef _PLATFORM_WIN32
		HostEntry = gethostbyname(pServer);
		if (HostEntry == NULL)
			return HRC_SERVERIPC_UNABLE_TO_CONNECT_TO_HOST;
		socketaddr.sin_addr = *(struct in_addr*)HostEntry->h_addr;
#endif //_PLATFORM_WIN32
	}

	// create socket
	m_Socketfd = socket(PF_INET, SOCK_STREAM, 0);
	if (m_Socketfd < 0)
	{
		//LOG("SIPC: Unable to create socket: %d", errno);
		return HRC_SERVERIPC_UNABLE_TO_CREATE_SOCKET;
	}

	// connect to host
	int rc = connect(m_Socketfd, (struct sockaddr*)&socketaddr, sizeof(socketaddr));
	if (rc != 0)
	{
        //LOG("SIPC: Unable to connect to %s on port %d", pServer, port);
		Close();
		return HRC_SERVERIPC_UNABLE_TO_CONNECT_TO_HOST;
	}

	// make socket non-blocking
#ifdef _PLATFORM_WIN32
	unsigned long value = 1;
	if (ioctlsocket(m_Socketfd, FIONBIO, &value))
	{
		//LOG("SIPC: Unable to convert socket to non-blocking");
	}
#else
	if (fcntl(m_Socketfd, F_SETFL, O_NONBLOCK) != 0)
	{
		//LOG("SIPC: Unable to convert socket to non-blocking");
	}
#endif

//	int optval = 1;
//int ret;
//	ret = setsockopt(m_Socketfd, SOL_SOCKET, TCP_NODELAY, (char*)&optval, sizeof(optval));

	return HRC_SERVERIPC_SUCCESS;
}

void cServerIPC::Reset()
{
	m_buffer.Reset();
}

int cServerIPC::Send(char *pContent, int size)
{
	const char *pts = pContent;
	int status = 0;
	int n;

	if (size < 0)
		return HRC_SERVERIPC_SENDDATA_FAILED;

	while (status != size)
	{
		n = send(m_Socketfd, pts+status, size-status, 0);
//		if (n < 0)
//		{
//			LOG("SIPC:  Unable to write: %d", errno);
//			return HRC_SERVERIPC_SENDDATA_FAILED;
//		}
#ifdef _PLATFORM_WIN32
		if (n == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				::Sleep(10);
				continue;
			}
			//LOG("TCP send error: (%d)", WSAGetLastError());
#else
		if (n == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			{
                usleep(10000);
				continue;
			}
			//LOG("SIPC: TCP send error: %s(%d)", strerror(errno), errno);
#endif
            return HRC_SERVERIPC_SENDDATA_FAILED;
        }

		status += n;
	}
	return HRC_SERVERIPC_SUCCESS;
}

int cServerIPC::CheckForResponse(int delay)
{
	time_t start, end;
	time(&start);
	end = start;

	int count;
	char buf[1024];
	int anything = 0;
	while (1)
	{
		if (delay != -1)
		{
			if ((start + delay) <= end)
			{
				if (anything)
					return HRC_SERVERIPC_SUCCESS;
				else
					return HRC_SERVERIPC_TIMEDOUT;
			}
		}
		count = recv(m_Socketfd, buf, 1024, 0);
#ifdef _PLATFORM_WIN32
		if (count == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				if (delay == -1)
					return HRC_SERVERIPC_NOT_READY;
			}
			else
			{
				return HRC_SERVERIPC_CONNECTION_CLOSED;
			}
		}
#else
		if (count == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			{
				// if we're here it's because this is a non blocking
				// socket.

				// if there is no delay involved, then simply return
				// notready
				if (delay == -1)
					return HRC_SERVERIPC_NOT_READY;
			}
			else
			{
				return HRC_SERVERIPC_CONNECTION_CLOSED;
			}
		}
#endif
		if (count == 0)
			return HRC_SERVERIPC_SUCCESS;

		if (count > 0)
		{
			anything = count;
			m_buffer.Append(buf, count);
		}

#ifdef _PLATFORM_WIN32
		::Sleep(10);
#else
		usleep(10000);
#endif

		time(&end);
	}
	if (anything == 0)
		return HRC_SERVERIPC_NO_DATA_RECIEVED;
	return HRC_SERVERIPC_SUCCESS;
}

void cServerIPC::Close()
{
	int rc;
	if (m_Socketfd)
	{
		rc = shutdown(m_Socketfd, 1);
#ifdef _PLATFORM_WIN32
		closesocket(m_Socketfd);
#else
		close(m_Socketfd);
#endif
	}
	m_buffer.Reset();
	m_Socketfd = 0;
}

char* cServerIPC::GetResponse()
{
	return (char*)m_buffer;
}

int cServerIPC::GetResponseSize()
{
	 return m_buffer.GetBufferCount();
}
