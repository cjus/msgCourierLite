/* core.h
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

core.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

core.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with core.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file core.h
 @brief The cCore class is used to group application wide shared data, and to
 provide easy system wide access.
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef CORE_H
#define CORE_H

#include <string>
#include "exception.h"
#include "log.h"
#include "logger.h"
#include "imsgqueue.h"
#include "tcpserver.h"
#include "udpserver.h"
#include "webserver.h"
#include "sysmetrics.h"
#include "commandhandlerfactory.h"
#include "msgqueue.h"
#include "connectionqueue.h"
#include "virtualdirectorymap.h"
#include "ipaccess.h"
#include "msgrouterrulesengine.h"
#include "xmlliteparser.h"
#include "filecache.h"
#include "netinterfaces.h"

#define HRC_CORE_OK 0x0000

/**
 * @class cCore
 * @brief The cCore class is used to group application wide shared data, and to
 * provide easy system wide access.
 */
class cCore
{
public:
	void Init();
	void Deinit();

	static int Create(const char *pszModuleName);
	static int Destroy();
	static cCore* GetInstance() { return m_pInstance; }

	cIMsgQueue *GetMSGQueue();

	const char *GetServerID() { return m_ServerID.c_str(); }
	const char *GetServerName() { return m_ServerName.c_str(); }
	const char *GetServerListenAddr() { return m_server_listen_addr.c_str(); }
	const char *GetServerPrimaryAddr() { return m_server_primary_addr.c_str(); }
	const char *GetServerSecondaryAddr() { return m_server_secondary_addr.c_str(); }

	const char *GetTCPPort() { return m_TCPPort.c_str(); }
	const char *GetUDPPort() { return m_UDPPort.c_str(); }
	const char *GetTCPPorts() { return m_TCPServerList.c_str(); }
	const char *GetUDPPorts() { return m_UDPServerList.c_str(); }

	bool IsZeroConfig() { return m_bZeroConfig; }

	cIPAccess* GetIPAccess() { return &m_IPAccess; }
	static void AppMessage(const char *pFormat, ...);

	cFileCache *GetFileCache() { return &m_FileCache; }
	void SendToLogger(const char *pLogMessage);
public:
	cCore(const char *pszModuleName);
	virtual ~cCore();

private:
	static cCore *m_pInstance;
	cThreadSync m_ThreadSync;

	std::string m_ModuleName;
	std::string m_ServerID;
	std::string m_ServerName;
	std::string m_server_listen_addr;
	std::string m_server_primary_addr;
	std::string m_server_secondary_addr;
	std::string m_server_max_cache_size;
	
	std::string m_ServerInstanceID;

	std::string m_ServerMode;
	std::string m_ServerWorkingDirectory;
	std::string m_UDPPort;
	std::string m_TCPPort;
	std::string m_MaxWebServerThreads;
	std::string m_ConnectionTimeout;
	cIPAccess m_IPAccess;

	cWebServer *m_pWebServer;

	cSysMetrics m_SysMetrics;
	cCommandHandlerFactory m_CommandHandlerFactory;
	cMsgQueue *m_pMsgQueue;
	cSysInfo *m_pSysInfo;
	cLogger *m_pLogger;
	cConnectionQueue *m_pConnectionQueue;
	cVirtualDirectoryMap m_VirtualDirectoryMap;
	cMsgRouterRulesEngine m_MsgRouterRulesEngine;
	cNetInterfaces m_NetInterfaces;
	std::map<int, cTCPServer*> m_TCPServers;
	std::map<int, cUDPServer*> m_UDPServers;

	std::string m_TCPServerList;
	std::string m_UDPServerList;

	cFileCache m_FileCache;
	std::string m_UniqueServerID;
	std::string m_MACAddress;
	std::string m_ServerDetectedIP;

	bool m_bZeroConfig;

	void LoadConfig();
	void SetDefaults();

	void DoPostProcesses();
	void ProcessIPRules(cXMLLiteParser *pXML, cXMLLiteElement *pElm);
	void ProcessHandlers(cXMLLiteParser *pXML, cXMLLiteElement *pElm);
	void BuildUniqueServerName();
};

#endif // CORE_H

