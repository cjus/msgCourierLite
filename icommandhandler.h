/* icommandhandler.h
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

icommandhandler.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

icommandhandler.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with icommandhandler.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file icommandhandler.h
 @brief Command handler Interface
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef ICOMMAND_HANDLER_H
#define ICOMMAND_HANDLER_H

#define HRC_COMMAND_HANDLER_OK		0x0000
#define HRC_COMMAND_HANDLER_NOWAIT	0x0001 // signal that handler isn't expecting a message response

class cIMsg;
class cIMsgQueue;
class cISysInfo;
class cILogger;

/**
 @class cICommandHandler
 @brief Command handler Interface
*/
class cICommandHandler
{
public:
	virtual char *GetCommandName() = 0;

	virtual void OnStartup(cIMsgQueue *pMsgQueue, cISysInfo *pSysInfo, cILogger *pILogger) = 0;
	virtual int OnProcess(cIMsg *pMessage) = 0;
	virtual const char *OnProcessInternal(const char *pMessage) { return 0; }
	virtual void OnChron() {}
	virtual void OnShutdown() = 0;
};

#endif // ICOMMAND_HANDLER_H

