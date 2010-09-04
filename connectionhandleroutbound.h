/* connectionhandleroutbound.h
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

connectionhandleroutbound.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

connectionhandleroutbound.h was developed by Carlos Justiniano for use on the
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
 @file connectionhandleroutbound.h
 @author Carlos Justiniano
 @brief TCP/IP Connection handler
 @warning This class is currently only briefly implemented and is thus incomplete!!
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef _CONNECTION_HANDLER_OUTBOUND_H
#define _CONNECTION_HANDLER_OUTBOUND_H

#include "connectionhandler.h"
#include "buffer.h"
#include "sysmetrics.h"

class cBuffer;
class cMsg;
class cConnectionQueue;

/**
 @class cconnectionhandleroutbound 
 @brief TCP/IP Connection handler
*/
class cConnectionHandlerOutbound : public cConnectionHandler
{
public:
	cConnectionHandlerOutbound();
	~cConnectionHandlerOutbound();

	int HandleConnection(cConnectionQueue *pConnectionQueue, cMsg *pMsg, ConnectionRecord *pConnectionRecord);
	bool DoCloseConnection() { return m_bCloseConnection; }

private:
	bool m_bCloseConnection;
	cConnectionQueue *m_pConnectionQueue;
	ConnectionRecord *m_pConnectionRecord;
	cMsg *m_pMsg;
	cBuffer m_response;
	cBuffer m_ServerAddr;
	cBuffer m_temp;
	char m_TestBuffer[2048];

	int m_ServerPort;
	bool m_bSocketCreated;

	int TransmitMsg();
	bool UDPSend();
	int OpenTCP(char *pServer, int port);
	bool TCPSend(int iSocketfd, const char *pBuf, int size);
};

#endif // _CONNECTION_HANDLER_OUTBOUND_H



