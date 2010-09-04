/* connectionhandler.h
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

connectionhandler.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

connectionhandler.h was developed by Carlos Justiniano for use on the
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
 @file connectionhandler.h
 @author Carlos Justiniano
 @brief TCP/IP Connection handler
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef _CONNECTION_HANDLER_H
#define _CONNECTION_HANDLER_H

#define HRC_CONNECTION_HANDLER_OK					0x0000
#define HRC_CONNECTION_HANDLER_CREATEFAILED         0x0001
#define HRC_CONNECTION_HANDLER_MEMALLOC_FAILED		0x0002
#define HRC_CONNECTION_HANDLER_INTERNAL_BUF_FULL	0x0003
#define HRC_CONNECTION_HANDLER_SETUP_FAILED         0x0004
#define HRC_CONNECTION_HANDLER_ENDTASK				0x0005
#define HRC_CONNECTION_HANDLER_SENDERROR			0x0006
#define HRC_CONNECTION_HANDLER_PARSE_NOT_READY		0x0007
#define HRC_CONNECTION_HANDLER_INVALIDATED_TASK     0x0008
#define HRC_COMMAND_HANDLER_UNABLE_TO_SEND_MESSAGE	0x0009
#define HRC_COMMAND_HANDLER_UNABLE_TO_CREATE_SOCKET	0x000A

class cMsg;
class cMsgQueue;
class cAccessLog;
class cSysMetrics;

#include "connectionrecord.h"

/**
 @class cConnectionHandler 
 @brief TCP/IP Connection handler
 @todo Complete cConnectionHandler class.
*/
class cConnectionHandler
{
public:
	enum ProtocolType
	{
		eProtocol_HTTP_GET,
		eProtocol_HTTP_POST,
		eProtocol_HTTP_MCP,
		eProtocol_MCP,
		eProtocol_INSUFFICIENT_DATA,
		eProtocol_UNKNOWN
	};
};

#endif // _CONNECTION_HANDLER_H
