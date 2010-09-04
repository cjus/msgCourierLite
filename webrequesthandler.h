/* webrequesthandler.h
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

webrequesthandler.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

webrequesthandler.h was developed by Carlos Justiniano for use on the
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
 @file webrequesthandler.h
 @author Carlos Justiniano
 @brief Processes requests for the webserver component
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef _WEBREQUEST_HANDLER_HANDLER_H
#define _WEBREQUEST_HANDLER_HANDLER_H

#define HRC_WEBREQUEST_HANDLER_HANDLER_OK					0x0000
#define HRC_WEBREQUEST_HANDLER_HANDLER_CREATEFAILED         0x0001

#include "imsg.h"
#include "imsgqueue.h"
#include "virtualdirectorymap.h"
#include "buffer.h"
#include "bench.h"
#include "mimemap.h"

#include <queue>
#include <string>
#include <sstream>

#include "thread.h"

/**
 @class cWebRequestHandler 
 @brief Processes requests for the webserver component
*/
class cWebRequestHandler : public cThread
{
public:
	cWebRequestHandler();
	~cWebRequestHandler();

	int Setup(cVirtualDirectoryMap *pVirtualDirectoryMap, cIMsgQueue *pMsgQueue, cIMsg *pMessage);

	int Start();
	int Stop();
	
	bool IsConnected();
	int Run();
private:
	int m_NewThread;
	bool m_bConnected;

	cVirtualDirectoryMap *m_pVirtualDirectoryMap;
	cIMsgQueue *m_pMsgQueue;
	cIMsg *m_pIMsg;

	std::vector<std::string> m_URI;
	std::vector<std::string> m_CGI;
	std::string m_CGIParams;
	std::string m_DefaultPage;
	bool m_bMessageTypeGet;
	bool m_bCachedNotModified;
	char m_tempBuffer[16384];
	cBuffer m_Response;
	std::string m_sPath;
	std::string m_sContentType;
	std::string m_sEntityTag;
	std::string m_sMessageETag;
	std::string m_sServerLabel;

	cMIMEMap m_MIMEMap;
	bool m_bWebPage;
	cBench m_processBench;

	int Process(cIMsg *pMessage);
	bool ProcessHTTPHeader(cIMsg *pIMsg);
	void GetResource();
	void GetResourceContentType();

	void ProcessEmbeddedScript();
	void ProcessServerSideScripts();
	void GetDate(std::string &datestring);

};

#endif // _WEBREQUEST_HANDLER_HANDLER_H
