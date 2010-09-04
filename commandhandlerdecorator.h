/* commandhandlerdecorator.h
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

commandhandlerdecorator.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

commandhandlerdecorator.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with commandhandlerdecorator.h; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file commandhandlerdecorator.h
 @brief Decorates command handler objects with support for multithreading and queing
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
 Decorates command handler objects with support for multithreading and queing
*/

#ifndef COMMANDHANDLERDECORATOR_H
#define COMMANDHANDLERDECORATOR_H

#include "commandhandlerdecorator.h"

#include "icommandhandler.h"
#include "imsg.h"
#include "imsgqueue.h"
#include "msgprioritycomp.h"

#include "thread.h"
#include "threadsync.h"
#include <queue>

#include "timer.h"

class cCommandHandlerDecorator : cICommandHandler, cThread
{
public:
	cCommandHandlerDecorator(cICommandHandler *pICommandHandler);

	char *GetCommandName();

	void OnStartup(cIMsgQueue *pMsgQueue, cISysInfo *pSysInfo, cILogger *pLogger);
	int  OnProcess(cIMsg *pMessage);
	const char *OnProcessInternal(const char *pMessage);
	void OnShutdown();
protected:
    int Run();
private:
	cICommandHandler *m_pICommandHandler;
	cIMsgQueue *m_pMsgQueue;
	cThreadSync m_ThreadSync;
	std::priority_queue<cIMsg*, std::vector<cIMsg*>, cMsgPriorityComp > m_jobQueue;

	cTimer m_timer;
	cTimer m_ChronTimer;

};

#endif //COMMANDHANDLERDECORATOR_H

