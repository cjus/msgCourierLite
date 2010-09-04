/* connectionhandleroutbound.cpp
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

   connectionhandleroutbound.cpp is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   connectionhandleroutbound.cpp was developed by Carlos Justiniano for use on
   the msgCourier project and the ChessBrain Project and is now distributed
   in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
   even the implied warranty of MERCHANTABILITY or FITNESS FOR A
   PARTICULAR PURPOSE.  See the GNU General Public License for more
   details.

   You should have received a copy of the GNU General Public License
   along with main.cpp; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file connectionhandleroutbound.cpp
 @author Carlos Justiniano
 @brief TCP/IP Connection handler
 @warning This class is currently only briefly implemented and is thus incomplete!!
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
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
#endif //PLATFORM_LINUX

#ifdef _PLATFORM_WIN32
	#include "winsockoverride.h"
#endif //_PLATFORM_WIN32

#include "core.h"
#include "log.h"
#include "bench.h"
#include "connectionqueue.h"
#include "connectionhandleroutbound.h"

cConnectionHandlerOutbound::cConnectionHandlerOutbound()
: m_pMsg(0)
, m_bSocketCreated(false)
, m_pConnectionQueue(0)
, m_pConnectionRecord(0)
{
}

cConnectionHandlerOutbound::~cConnectionHandlerOutbound()
{
}

int cConnectionHandlerOutbound::HandleConnection(cConnectionQueue *pConnectionQueue, cMsg *pMsg, ConnectionRecord *pConnectionRecord)
{
	m_bCloseConnection = false;
	m_pConnectionQueue = pConnectionQueue;
	m_pConnectionRecord = pConnectionRecord;

	m_pMsg = pMsg;
	int ret = TransmitMsg();
	if (m_pConnectionRecord)
        m_pConnectionRecord->pendingwrite = false;
	return HRC_CONNECTION_HANDLER_OK;
}

int cConnectionHandlerOutbound::TransmitMsg()
{
	int rc = HRC_COMMAND_HANDLER_OK;

	cCore *pCore = cCore::GetInstance();
	cBench *pBench = ((cMsg*)m_pMsg)->GetBench();

	int iPort  = m_pMsg->GetArrivalPort();
	const char *pMessageType = "Reply";

	if (m_pMsg->GetFormat() == cIMsg::MSG_FORMAT_NOTIFY)
		pMessageType = "Notify";

	if (m_pMsg->GetProtocolType() == cIMsg::MSG_PROTOCOL_HTTP_GET ||
		m_pMsg->GetProtocolType() == cIMsg::MSG_PROTOCOL_HTTP_POST)
	{
		pBench->Stop();
		m_pConnectionQueue->GetSysMetrics()->UpdateMsgResponseTime(pBench->Elapsed());

		const char *p = m_pMsg->GetContentPayload();
		if (p == 0)
		{
			// message empty, most likely due to incorrect handling in an add-on or external module.
			// the best we can do here is simpy return.
			MARK_TRAVEL(m_pMsg);
			m_pMsg->SetMsgState(cMsg::MSG_STATE_UNDELIVERED);

			m_pConnectionQueue->GetSysMetrics()->IncMsgUndelivered();
			return HRC_COMMAND_HANDLER_UNABLE_TO_SEND_MESSAGE;
		}

		if (strstr(p, "HTTP/1.") == 0)
		{
			// HTTP header missing
			m_response.Sprintf(5000,
								"HTTP/1.1 200 OK\r\n"
								"Server: %s\r\n"
								"Content-Type: %s\r\n"
								"Content-Length: %d\r\n"
								"Connection: %s\r\n"
								"\r\n",
								pCore->GetServerName(),
								m_pMsg->GetContentType(), m_pMsg->GetContentLength(),
								(m_pMsg->IsKeepAlive()) ? "keep-alive" : "close");
			TCPSend(m_pMsg->GetTCPSocketHandle(), (char*)m_response.GetRawBuffer(), m_response.GetBufferCount());
		}

		bool bRet = TCPSend(m_pMsg->GetTCPSocketHandle(),
							m_pMsg->GetContentPayload(),
							m_pMsg->GetContentLength());
		if (bRet == true)
		{
			MARK_TRAVEL(m_pMsg);
			m_pMsg->SetMsgState(cMsg::MSG_STATE_DELIVERED);
			m_pConnectionQueue->GetSysMetrics()->IncMsgDelivered();
			return HRC_COMMAND_HANDLER_OK;
		}
		else
		{
			MARK_TRAVEL(m_pMsg);
			m_pMsg->SetMsgState(cMsg::MSG_STATE_UNDELIVERED);

			m_pConnectionQueue->GetSysMetrics()->IncMsgUndelivered();
			return HRC_COMMAND_HANDLER_UNABLE_TO_SEND_MESSAGE;
		}
	}
	else if (m_pMsg->GetFormat() == cIMsg::MSG_FORMAT_RES || m_pMsg->GetFormat() == cIMsg::MSG_FORMAT_NOTIFY)
	{
		if (iPort == 0)
		{
			if (m_pMsg->GetConnectionType() == cIMsg::MSG_CT_OB_TCP)
				iPort = atoi(pCore->GetTCPPort());
			else
				iPort = atoi(pCore->GetUDPPort());
		}

		cIMsg::eMsgProtocolType pt = m_pMsg->GetProtocolType();
		if (pt == cIMsg::MSG_PROTOCOL_MCP)
		{
			if (m_pMsg->GetContentLength() == 0)
			{
				pBench->Stop();
				m_pConnectionQueue->GetSysMetrics()->UpdateMsgResponseTime(pBench->Elapsed());

				sprintf(m_TestBuffer, "MCP/1.0 %3.3d %s\r\nMsgID: %s\r\nMsgPT: %.4f\r\nConnection: %s\r\n",
					m_pMsg->GetResponseCode(), pMessageType, m_pMsg->GetMsgID(), pBench->Elapsed(),
					(m_pMsg->IsKeepAlive()) ? "keep-alive" : "close");
				m_response.ReplaceWith(m_TestBuffer);

				if (strlen(m_pMsg->GetTo()) > 0)
				{
					m_response.Append("To: ");
					m_response.Append(const_cast<char *>(m_pMsg->GetTo()));
					m_response.Append("\r\n");
				}

				sprintf(m_TestBuffer, "From: %s\r\n",
						m_pMsg->GetFrom());
						//pCore->GetServerName(), pCore->GetServerPrimaryAddr(),
						//iPort);
				m_response.Append(m_TestBuffer);

				const char *pCommand = m_pMsg->GetCommand();
				if (pCommand && strlen(pCommand) > 0)
				{
					sprintf(m_TestBuffer, "Command: %s\r\n", m_pMsg->GetCommand());
					m_response.Append(m_TestBuffer);
					m_response.Append("\r\n");
				}

				m_response.Append("\r\n");
			}
			else
			{
				pBench->Stop();
				m_pConnectionQueue->GetSysMetrics()->UpdateMsgResponseTime(pBench->Elapsed());

				m_response.Sprintf(5000,"MCP/1.0 %3.3d %s\r\n"
										"MsgID: %s\r\n"
										"To: %s\r\n"
										"From: %s\r\n"
										"Content-Type: %s\r\n"
										"Content-Length: %d\r\n"
										"Connection: %s\r\n"
										"\r\n",
										m_pMsg->GetResponseCode(), pMessageType,
										m_pMsg->GetMsgID(), m_pMsg->GetTo(), m_pMsg->GetFrom(),
										//pCore->GetServerName(), pCore->GetServerPrimaryAddr(), iPort,
										m_pMsg->GetContentType(), m_pMsg->GetContentLength(),
										(m_pMsg->IsKeepAlive()) ? "keep-alive" : "close");
				m_response.Append((char*)m_pMsg->GetContentPayload(), m_pMsg->GetContentLength());
			}
		}
		else if (pt == cIMsg::MSG_PROTOCOL_HTTP_MCP)
		{
			if (m_pMsg->GetContentLength() == 0)
			{
				pBench->Stop();
				m_pConnectionQueue->GetSysMetrics()->UpdateMsgResponseTime(pBench->Elapsed());
				m_response.Sprintf(5000,
									"HTTP/1.1 200 OK\r\n"
									"Server: %s\r\n"
									"ETag: %s\r\n"
									"Pragma: RC=%d %s\r\n"
									"Connection: %s\r\n"
									"\r\n",
									pCore->GetServerName(), pMessageType, m_pMsg->GetMsgID(),
									m_pMsg->GetResponseCode(),
									(m_pMsg->IsKeepAlive()) ? "keep-alive" : "close");
			}
			else
			{
				pBench->Stop();
				m_pConnectionQueue->GetSysMetrics()->UpdateMsgResponseTime(pBench->Elapsed());
				m_response.Sprintf(5000,
									"HTTP/1.1 200 OK\r\n"
									"Server: %s\r\n"
									"ETag: %s\r\n"
									"Pragma: RC=%dr\n"
									"Content-Type: %s\r\n"
									"Content-Length: %d\r\n"
									"Connection: %s\r\n"
									"\r\n",
									pCore->GetServerName(), pMessageType, m_pMsg->GetMsgID(),
									m_pMsg->GetResponseCode(),
									m_pMsg->GetContentType(), m_pMsg->GetContentLength(),
									(m_pMsg->IsKeepAlive()) ? "keep-alive" : "close");
				m_response.Append((char*)m_pMsg->GetContentPayload(), m_pMsg->GetContentLength());
			}
		}
	}
	else
	{
		if (m_pMsg->GetContentLength() > 0 )
		{
			pBench->Stop();
			m_pConnectionQueue->GetSysMetrics()->UpdateMsgResponseTime(pBench->Elapsed());
			m_response.Sprintf(5000,"%s MCP/1.0\r\nMsgID: %s\r\nTo: %s\r\nFrom: %s\r\nContent-Type: %s\r\nContent-Length: %d\r\nConnection: %s\r\n\r\n",
								m_pMsg->GetCommand(),
								m_pMsg->GetMsgID(), m_pMsg->GetTo(), m_pMsg->GetFrom(),
								//pCore->GetServerName(), pCore->GetServerPrimaryAddr(), iPort,
								m_pMsg->GetContentType(),
								m_pMsg->GetContentLength(),
								(m_pMsg->IsKeepAlive()) ? "keep-alive" : "close");
			m_response.Append((char*)m_pMsg->GetContentPayload(), m_pMsg->GetContentLength());
		}
		else
		{
			pBench->Stop();
			m_pConnectionQueue->GetSysMetrics()->UpdateMsgResponseTime(pBench->Elapsed());
			m_response.Sprintf(5000,"%s MCP/1.0\r\nMsgID: %s\r\nTo: %s\r\nFrom: %s\r\nConnection: %s\r\n\r\n",
								m_pMsg->GetCommand(),
								m_pMsg->GetMsgID(), m_pMsg->GetTo(), m_pMsg->GetFrom(),
								(m_pMsg->IsKeepAlive()) ? "keep-alive" : "close");
								//pCore->GetServerName(), pCore->GetServerPrimaryAddr(), iPort);
		}
	}

	if (strlen(m_pMsg->GetTo()) > 0)
	{
		m_temp.ReplaceWith(const_cast<char*>(m_pMsg->GetTo()));
		int iPos1 = m_temp.FindChar('@');
		int iPos2 = m_temp.FindChar(':');
		m_ServerAddr.ReplaceWith(m_temp.cstr() + iPos1+1, iPos2-iPos1-1);
		m_ServerPort = atoi(m_temp.cstr()+iPos2+1);
	}

	bool bRet = false;

	if (m_pMsg->GetConnectionType() != cIMsg::MSG_CT_OB_UDP &&
		m_pMsg->GetConnectionType() != cIMsg::MSG_CT_OB_TCP)
	{
		THROW("Warning connection type is not OB_UDP or OB_TCP");
	}

	if (m_pMsg->GetConnectionType() == cIMsg::MSG_CT_OB_UDP)
		bRet = UDPSend();
	if (m_pMsg->GetConnectionType() == cIMsg::MSG_CT_OB_TCP)
	{
		/*
		if (m_pMsg->GetNotifyAction() != cIMsg::MSG_NOTIFY_ACTION_YES &&
			m_pMsg->GetReplyAction() == cIMsg::MSG_REPLY_ACTION_NOWAIT)
		{
			// if message reply action was set to nowait then the socket
			// handle is no longer valid and a new connection must be
			// established to the sender.
			int socketfd = OpenTCP(m_ServerAddr.cstr(), m_ServerPort);
			if (socketfd)
				m_pMsg->SetTCPSocketHandle(socketfd);
		}
		*/

        int socketfd = m_pMsg->GetTCPSocketHandle();
		if (socketfd == -1)
		{
			socketfd = OpenTCP(m_ServerAddr.cstr(), m_ServerPort);
			if (socketfd)
				m_pMsg->SetTCPSocketHandle(socketfd);
		}

		if (socketfd != -1)
		{
            bRet = TCPSend(m_pMsg->GetTCPSocketHandle(),
                            (char*)m_response.GetRawBuffer(),
                            m_response.GetBufferCount());
            bRet = true;
		}
        else
        {
            bRet = false;
        }
	}

	if (bRet)
	{
		MARK_TRAVEL(m_pMsg);
		m_pMsg->SetMsgState(cMsg::MSG_STATE_DELIVERED);
		m_pConnectionQueue->GetSysMetrics()->IncMsgDelivered();
		rc = HRC_COMMAND_HANDLER_OK;
	}
	else
	{
		MARK_TRAVEL(m_pMsg);
		m_pMsg->SetMsgState(cMsg::MSG_STATE_UNDELIVERED);
		m_pConnectionQueue->GetSysMetrics()->IncMsgUndelivered();
		//LOG("TCP:OB:Unable to send message");
		rc = HRC_COMMAND_HANDLER_UNABLE_TO_SEND_MESSAGE;
	}
	return rc;
}

bool cConnectionHandlerOutbound::UDPSend()
{
	bool bSent = false;
    int Socketfd;

	if (m_pMsg->GetUDPSocketHandle() == -1)
	{
		Socketfd = socket(AF_INET, SOCK_DGRAM, 0);
		if (Socketfd == -1)
		{
			LOG2("@@@ Unable to create UDP socket");
			return false; //HRC_COMMAND_HANDLER_UNABLE_TO_CREATE_SOCKET;
		}
	}
	else
	{
		Socketfd = m_pMsg->GetUDPSocketHandle();
	}

	struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

	server_addr.sin_family = AF_INET;

	if (m_pMsg->GetUDPSocketHandle() != -1)
	{
		server_addr.sin_addr.s_addr = inet_addr(m_pMsg->GetSourceIP());
		server_addr.sin_port = m_pMsg->GetSourcePort();
	}
	else
	{
		server_addr.sin_addr.s_addr = inet_addr(m_ServerAddr.cstr());
		server_addr.sin_port = htons(m_ServerPort);
	}
	int server_len = sizeof(server_addr);
    int rc = sendto(Socketfd, (const char*)m_response.GetRawBuffer(), m_response.GetBufferCount(), 0,
                (struct sockaddr*)&server_addr, server_len);
    if (rc < 0)
    {
        LOG("@@@ UDP Server unable to sendto");
		bSent = false;
    }
    else
    {
		bSent = true;
    }

	if (m_pMsg->GetUDPSocketHandle() == -1)
	{
		//shutdown(Socketfd, 2);
	#ifdef _PLATFORM_LINUX
LOG2("close(%d) called", Socketfd);
		close(Socketfd);
	#endif //_PLATFORM_LINUX
	#ifdef _PLATFORM_WIN32
		closesocket(Socketfd);
	#endif //_PLATFORM_WIN32
	}
	return bSent;
}

int cConnectionHandlerOutbound::OpenTCP(char *pServer, int port)
{
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
			//Print("Error, Unable to connect to %s on port %d\n", pServer, port);
            return -1;
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
#ifdef _PLATFORM_LINUX
		if (!inet_aton(pServer, &socketaddr.sin_addr))
		{
			HostEntry = gethostbyname(pServer);
			if (HostEntry == NULL)
				return -1;
			socketaddr.sin_addr = *(struct in_addr*)HostEntry->h_addr;
		}
#endif //!_PLATFORM_WIN32

#ifdef _PLATFORM_WIN32
		HostEntry = gethostbyname(pServer);
		if (HostEntry == NULL)
			return -1;
		socketaddr.sin_addr = *(struct in_addr*)HostEntry->h_addr;
#endif //_PLATFORM_WIN32
	}

	// create socket
	int Socketfd = socket(PF_INET, SOCK_STREAM, 0);
	if (Socketfd < 0)
	{
		//Print("Error, Unable to create socket: %d\n", errno);
		return -1;
	}

	// connect to host
	int rc = connect(Socketfd, (struct sockaddr*)&socketaddr, sizeof(socketaddr));
	if (rc != 0)
	{
        //Print("Error, Unable to connect to %s on port %d\n", pServer, port);
		return -1;
	}

	 // convert socket to non-blocking
#ifdef _PLATFORM_WIN32
	unsigned long value = 1;
	ioctlsocket(Socketfd, FIONBIO, &value);
#else
	fcntl(Socketfd, F_SETFL, O_NONBLOCK);
#endif

	m_bSocketCreated = true;
	return Socketfd;
}

bool cConnectionHandlerOutbound::TCPSend(int iSocketfd, const char *pBuf, int size)
{
	bool bRet = true;
	int bytesSent;
	int totalSent = 0;
	int iteration = 0;
	do
	{
		iteration++;
		bytesSent = send(iSocketfd, pBuf + totalSent, size - totalSent, 0);

		//if (bytesSent == -1 && iteration > 1)
		//	LOG("Warning, TCPSend[%x] iteration is %d and %d bytes out of %d bytes were sent.", this, iteration, totalSent, size);
#ifdef _PLATFORM_LINUX
		if (bytesSent == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			{
				continue;
			}
			//LOG("TCP send error (OB): %s(%d) on socket %d", strerror(errno), errno, iSocketfd);
			bRet = false;
			break;
		}
#endif //_PLATFORM_LINUX
#ifdef _PLATFORM_WIN32
		if (bytesSent == SOCKET_ERROR)
		{
			int err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK)
			{
				continue;
			}
			//LOG("TCP send error (OB): (%d) on socket %d", WSAGetLastError(), iSocketfd);
			bRet = false;
			break;
		}
#endif //_PLATFORM_WIN32
		else
		{
			totalSent += bytesSent;
		}
	} while (totalSent < size);

	if (bRet == false)
	{
		if (m_bSocketCreated)
		{
			//shutdown(iSocketfd, 2);
		#ifdef _PLATFORM_LINUX
LOG2("close(%d) called", iSocketfd);
			close(iSocketfd);
		#endif //_PLATFORM_LINUX
		#ifdef _PLATFORM_WIN32
			closesocket(iSocketfd);
		#endif //_PLATFORM_WIN32
		}
		else
		{
			m_bCloseConnection = true;
		}
	}
	else
	{
		if (m_pConnectionRecord)
			m_pConnectionRecord->timestamp = time(0);
	}

	if (m_pMsg->IsKeepAlive() == false)
	{
		m_bCloseConnection = true;
	}
	return bRet;
}
