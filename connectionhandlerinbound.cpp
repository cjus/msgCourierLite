/* connectionhandlerinbound.cpp
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

   connectionhandlerinbound.cpp is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   connectionhandlerinbound.cpp was developed by Carlos Justiniano for use on
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
 @file connectionhandlerinbound.cpp
 @author Carlos Justiniano
 @brief TCP/IP Connection handler
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _PLATFORM_LINUX
	#include <fcntl.h>
	#include <sys/ioctl.h>
	#include <errno.h>
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32
	#include "winsockoverride.h"
#endif //_PLATFORM_WIN32

#include <time.h>

#include "log.h"
#include "connectionhandlerinbound.h"
#include "commandhandlerfactory.h"
#include "msg.h"
#include "msgqueue.h"
#include "connectionqueue.h"

#define MAX_ALLOWED_SIZE 4096
#define TRAILINGCRLFS 4

using namespace std;

cConnectionHandlerInbound::cConnectionHandlerInbound()
: m_pConnectionQueue(NULL)
, m_pConnectionRecord(NULL)
{
}

cConnectionHandlerInbound::~cConnectionHandlerInbound()
{
}

int cConnectionHandlerInbound::HandleConnection(cConnectionQueue *pConnectionQueue, ConnectionRecord *pConnectionRecord)
{
	int nread;
	int rc;
	int iSocketErrorCode = 0;
	int bpipe = 0;

	m_bCloseConnection = false;
	m_pConnectionQueue = pConnectionQueue;
MC_ASSERT(pConnectionRecord != NULL);
	m_pConnectionRecord = pConnectionRecord;
	m_pConnectionRecord->pActiveHandler = this;


	bool bSocketError = false;
	MC_ASSERT(m_pConnectionRecord != NULL);
	nread = recv(m_pConnectionRecord->Socketfd, m_szData, CLI_MAX_BUF_SIZE, 0);

#ifdef _PLATFORM_LINUX
    if (nread == -1)
		bpipe = (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) ? 0 : 1;
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32
	if (nread == SOCKET_ERROR)
		bpipe = (WSAGetLastError() == WSAEWOULDBLOCK) ? 0 : 1;
#endif //_PLATFORM_WIN32

	if (bpipe == 1)
	{
		bSocketError = true;
	}

	if (nread > 0)
	{
		m_pConnectionRecord->buffer.Append(m_szData, nread);
		m_pConnectionRecord->timestamp = time(0);
	}
	if (m_pConnectionRecord->buffer.Find("\r\n\r\n") != -1)
	{
		rc = Process();
		if (rc == HRC_CONNECTION_HANDLER_ENDTASK)
		{
       		bpipe = 0;
		}
    	else if (rc == HRC_CONNECTION_HANDLER_SENDERROR)
		{
          		bpipe = 2;
		}
    	else if (rc == HRC_CONNECTION_HANDLER_INVALIDATED_TASK)
       	{
       		LOG("==>%s caused an invalidated task!", m_ipaddr.c_str());
			cBuffer b;
			b.ReplaceWith((char*)m_pConnectionRecord->buffer.GetRawBuffer(),
							   	m_pConnectionRecord->buffer.GetBufferSize());
			b.MakeString();
			cLog::DebugDump("recv buffer", b.cstr(), strlen(b.cstr()));
			bpipe = 3;
		}
	}

	if (bSocketError || bpipe != 0)
	{
		m_bCloseConnection = true;
	}

	m_pConnectionRecord->pActiveHandler = NULL;

	return HRC_CONNECTION_HANDLER_OK;
}

int cConnectionHandlerInbound::Process()
{
	int rc = HRC_CONNECTION_HANDLER_ENDTASK;
	int iContentLength = -1;
	int iDataOffset = -1;

	cConnectionHandlerInbound::ProtocolType pt = AnalyzeProtocol(iContentLength, iDataOffset);

	if (pt == cConnectionHandler::eProtocol_INSUFFICIENT_DATA)
		return HRC_CONNECTION_HANDLER_PARSE_NOT_READY;

	if (pt == cConnectionHandler::eProtocol_HTTP_GET ||
		pt == cConnectionHandler::eProtocol_HTTP_POST)
	{
		rc = ProcessHTTP(pt);
	}
	else if (pt == cConnectionHandler::eProtocol_UNKNOWN)
	{
		rc = HRC_CONNECTION_HANDLER_INVALIDATED_TASK;
	}
	else
	{
		rc = ProcessMCP(pt);
	}
    return rc;
}

int cConnectionHandlerInbound::ProcessHTTP(cConnectionHandler::ProtocolType &pt)
{
	int rc = HRC_CONNECTION_HANDLER_ENDTASK;

	cMsg *pMessage = (cMsg*)m_pConnectionQueue->GetMsgQueue()->CreateMessage();
	MC_ASSERT(pMessage != NULL);

	if (pMessage)
	{
		MARK_TRAVEL(pMessage);
		pMessage->SetTCPSocketHandle(m_pConnectionRecord->Socketfd);
		pMessage->SetConnectionType(cIMsg::MSG_CT_IB_TCP);

		m_ipaddr = inet_ntoa(m_pConnectionRecord->SocketAddr.sin_addr);
		pMessage->SetSourceIP((char*)m_ipaddr.c_str());

		if (pt == cConnectionHandler::eProtocol_HTTP_GET)
			pMessage->SetProtocolType(cIMsg::MSG_PROTOCOL_HTTP_GET);
		else if (pt == cConnectionHandler::eProtocol_HTTP_POST)
			pMessage->SetProtocolType(cIMsg::MSG_PROTOCOL_HTTP_POST);

		// store message to pass to web server component
		//int isize = m_pConnectionRecord->buffer.GetBufferCount();
		//MC_ASSERT(isize);
		pMessage->SetContentPayload((const char*)m_pConnectionRecord->buffer.GetRawBuffer(), m_msgSize);
		m_pConnectionRecord->buffer.Remove(0, m_msgSize);
		pMessage->Parse(cIMsg::MSG_CT_IB_TCP, cIMsg::MSG_FORMAT_REQ);

		// Update MsgID because HTTP message wont be carrying one
		pMessage->UpdateMsgID();

		// set message priority to high because under msgCourier HTTP request
		// are intended as console requests
		pMessage->SetMsgPriority(cIMsg::MSG_PRIORITY_HIGH);

        // by default HTTP messages do not require notification
        // and connections wait for a reply
		pMessage->SetNotifyAction(cIMsg::MSG_NOTIFY_ACTION_NO);
		pMessage->SetReplyAction(cIMsg::MSG_REPLY_ACTION_WAIT);
		pMessage->SetArrivalPort(m_pConnectionRecord->arrivalPort);

		MARK_TRAVEL(pMessage);
		pMessage->DispatchMsg();

		rc = HRC_CONNECTION_HANDLER_ENDTASK;
	}
	else
	{
		L
		MC_ASSERT(0);
	}

    return rc;
}

int cConnectionHandlerInbound::ProcessMCP(cConnectionHandler::ProtocolType &pt)
{
	int rc = HRC_CONNECTION_HANDLER_ENDTASK;

	cMsg *pMessage = (cMsg*)m_pConnectionQueue->GetMsgQueue()->CreateMessage();
	MC_ASSERT(pMessage != NULL);

	if (pMessage)
	{
		MARK_TRAVEL(pMessage);
		pMessage->SetTCPSocketHandle(m_pConnectionRecord->Socketfd);
		pMessage->SetConnectionType(cIMsg::MSG_CT_IB_TCP);

		m_ipaddr = inet_ntoa(m_pConnectionRecord->SocketAddr.sin_addr);
		pMessage->SetSourceIP((char*)m_ipaddr.c_str());
		pMessage->SetArrivalPort(m_pConnectionRecord->arrivalPort);

		if (pt == cConnectionHandler::eProtocol_HTTP_MCP)
			pMessage->SetProtocolType(cIMsg::MSG_PROTOCOL_HTTP_MCP);
		else if (pt == cConnectionHandler::eProtocol_MCP)
			pMessage->SetProtocolType(cIMsg::MSG_PROTOCOL_MCP);

		pMessage->SetContentPayload((const char*)m_pConnectionRecord->buffer.GetRawBuffer(), m_msgSize);
		m_pConnectionRecord->buffer.Remove(0, m_msgSize);
		rc = pMessage->Parse(cIMsg::MSG_CT_IB_TCP, cIMsg::MSG_FORMAT_REQ);
		if (rc == HRC_MSG_OK)
		{
			// new message is now ready to be processed by a message handler
			MARK_TRAVEL(pMessage);
			pMessage->DispatchMsg();

			rc = HRC_CONNECTION_HANDLER_ENDTASK;
		}
		else
		{
			MARK_TRAVEL(pMessage);
			rc = HRC_CONNECTION_HANDLER_INVALIDATED_TASK;
			L
			MC_ASSERT(0);
		}
	}
	else
	{
		L
		MC_ASSERT(0);
	}

    return rc;
}

cConnectionHandlerInbound::ProtocolType cConnectionHandlerInbound::AnalyzeProtocol(int &iContentLength, int &iDataOffset)
{
	iContentLength = -1;
	iDataOffset = - 1;

	int cj = 0;

	char *pData = (char*)m_pConnectionRecord->buffer.GetBuffer();

MC_ASSERT(pData != NULL);
	if (strstr(pData, "\r\n\r\n")==0)
		return cConnectionHandler::eProtocol_INSUFFICIENT_DATA;

	iDataOffset = m_pConnectionRecord->buffer.BinaryFind(0, (unsigned char *)"\r\n\r\n", TRAILINGCRLFS);
MC_ASSERT(iDataOffset != -1);
	m_msgHeaderEnd = iDataOffset + TRAILINGCRLFS;
	m_msgSize = m_msgHeaderEnd;

	int iBufCount = m_pConnectionRecord->buffer.GetBufferCount();
MC_ASSERT(iBufCount != 0);

	if (memcmp(pData, "GET ", 4)==0 &&
		strstr(pData, "HTTP/")!=0 &&
		strstr(pData, "\r\n\r\n")!=0)
	{
		return cConnectionHandler::eProtocol_HTTP_GET;
	}

	m_HeaderExtract.Parse(&m_pConnectionRecord->buffer);
	string sContentLength;
	m_HeaderExtract.GetValue("Content-Length", sContentLength);
	if (sContentLength.length() == 0)
		m_HeaderExtract.GetValue("Content-length", sContentLength);
	if (sContentLength.length())
	{
		if (iDataOffset == -1)
			return cConnectionHandler::eProtocol_INSUFFICIENT_DATA;
		iDataOffset += TRAILINGCRLFS;
		iContentLength = atoi(sContentLength.c_str());
		if (iBufCount < (iDataOffset + iContentLength))
		{
			iDataOffset = -1;
			iContentLength = -1;
			return cConnectionHandler::eProtocol_INSUFFICIENT_DATA;
		}
		m_msgSize = iDataOffset + iContentLength;
	}

	if (memcmp(pData, "POST ", 5)==0 &&
		strstr(pData, "HTTP/")!=0 &&
		strstr(pData, "\r\n\r\n")!=0)
	{
		return cConnectionHandler::eProtocol_HTTP_POST;
	}

	if (strstr(pData, "MCP/") != 0)
	{
		return cConnectionHandler::eProtocol_MCP;
	}

	return cConnectionHandler::eProtocol_UNKNOWN;
}
