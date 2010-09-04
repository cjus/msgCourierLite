/* connectionhandlerinbound.h
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

connectionhandlerinbound.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

connectionhandlerinbound.h was developed by Carlos Justiniano for use on the
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
 @file connectionhandlerinbound.h
 @author Carlos Justiniano
 @brief TCP/IP Connection handler
 @warning This class is currently only briefly implemented and is thus incomplete!!
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef _CONNECTION_HANDLER_INBOUND_H
#define _CONNECTION_HANDLER_INBOUND_H

#ifdef _PLATFORM_LINUX
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netinet/tcp.h>
	#include <arpa/inet.h>
	#include <netdb.h>

	#include <sys/time.h>
	#include <sys/ioctl.h>
	#include <fcntl.h>
	#include <unistd.h>
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32
	#include "winsockoverride.h"
#endif //_PLATFORM_WIN32

#include <time.h>
#include <string>

#include "connectionhandler.h"
#include "buffer.h"
#include "thread.h"
#include "headerextract.h"
#include "connectionrecord.h"
#include "sysmetrics.h"

#define CLI_MAX_BUF_SIZE 16384

class cMsgQueue;
class cConnectionQueue;

/**
 @class cConnectionHandlerInbound 
 @brief TCP/IP Connection handler
 @todo Complete cConnectionHandlerInbound class.
*/
class cConnectionHandlerInbound : public cConnectionHandler
{
public:
	cConnectionHandlerInbound();
	~cConnectionHandlerInbound();

	int HandleConnection(cConnectionQueue *pConnectionQueue, ConnectionRecord *pConnectionRecord);
	bool DoCloseConnection() { return m_bCloseConnection; }

private:
	bool m_bCloseConnection;
	cConnectionQueue *m_pConnectionQueue;
	cHeaderExtract m_HeaderExtract;

	ConnectionRecord *m_pConnectionRecord;
	std::string m_ipaddr;
	char m_szData[CLI_MAX_BUF_SIZE];
	int m_msgHeaderEnd;
	int m_msgSize;

	int Process();
	cConnectionHandler::ProtocolType AnalyzeProtocol(int &iContentLength, int &iDataOffset);
	int ProcessHTTP(cConnectionHandler::ProtocolType &pt);
	int ProcessMCP(cConnectionHandler::ProtocolType &pt);
};

#endif // _CONNECTION_HANDLER_INBOUND_H
