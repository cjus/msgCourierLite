/* webserver.h
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

webserver.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

webserver.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with webserver.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file webserver.h
 @brief WebServer component
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
 webserver header class.
*/

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <vector>

#include "msgCourierPlugin.h"
#include "webrequesthandler.h"
#include "sysinfo.h"

class cVirtualDirectoryMap;

class cWebServer : cICommandHandler
{
public:
	cWebServer(cVirtualDirectoryMap *pVirtualDirectoryMap, int iMaxHandlers);

	char *GetCommandName();

	void OnStartup(cIMsgQueue *pMsgQueue, cISysInfo *pSysInfo, cILogger *pILogger);
	int  OnProcess(cIMsg *pMessage);
	void OnShutdown();

private:
	cVirtualDirectoryMap *m_pVirtualDirectoryMap;
	int m_iMaxHandlers;
	cIMsgQueue *m_pMsgQueue;
	cISysInfo *m_pSysInfo;

	std::vector<cWebRequestHandler*> m_WebRequestHandlers;
	int m_iTotalThreads;
};

#endif //WEBSERVER_H

