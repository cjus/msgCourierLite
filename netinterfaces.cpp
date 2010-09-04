/* netinterfaces.cpp
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

netinterfaces.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

netinterfaces.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with service.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file netinterfaces.cpp
 @brief Track Network Interfaces
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef _PLATFORM_WIN32
	#include <unistd.h>
	#include <netdb.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif //!_PLATFORM_WIN32

#ifdef _PLATFORM_WIN32
	#include <windows.h>
	#include <Iphlpapi.h>
#endif //_PLATFORM_WIN32

#include <ctype.h>

#include "exception.h"
#include "log.h"
#include "netinterfaces.h"
#include "buffer.h"

using namespace std;

cNetInterfaces::cNetInterfaces()
{
	GetHostName();

#ifdef _PLATFORM_LINUX
	GetLinuxNetInterfaces();
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32
	GetWindowsNetInterfaces();
#endif // _PLATFORM_WIN32
}

cNetInterfaces::~cNetInterfaces()
{
	try
	{
		cNetInterfaceEntry* pEntry;
		for (m_it = m_Interfaces.begin(); m_it != m_Interfaces.end(); m_it++)
		{
			pEntry = (*m_it);
			delete pEntry;
		}
		m_Interfaces.erase(m_Interfaces.begin(), m_Interfaces.end());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

cNetInterfaceEntry *cNetInterfaces::QueryFirstInterface()
{
	cNetInterfaceEntry *pEntry = NULL;
	try
	{
		m_it = m_Interfaces.begin();
		pEntry = (*m_it);
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return pEntry;
}

cNetInterfaceEntry *cNetInterfaces::QueryNextInterface()
{
	cNetInterfaceEntry *pEntry = NULL;
	try
	{
		m_it++;
		pEntry = (m_it == m_Interfaces.end()) ? 0 : (*m_it);
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return pEntry;
}

void cNetInterfaces::ExtractValue(const char *pLine, const char *pField, string &value)
{
	try
	{
		string::size_type pos1, pos2;
		string sLine = pLine;
		pos1 = sLine.find(pField);
		pos1 += strlen(pField);
		pos2 = sLine.find(" ", pos1);
		value = sLine.substr(pos1, pos2-pos1);
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

void cNetInterfaces::AddInterface(std::string &sIPAddress, std::string &sBroadcastAddress, std::string &sMacAddress)
{
	if (ExistBroadCastAddress(sBroadcastAddress))
		return;

	cNetInterfaceEntry *pEntry;
	try
	{
		MC_NEW(pEntry, cNetInterfaceEntry());
		pEntry->m_IPAddress = sIPAddress;
		pEntry->m_BroadcastAddress = sBroadcastAddress;
		pEntry->m_MacAddress = sMacAddress;
		m_Interfaces.push_back(pEntry);

		if (m_PrimaryIP == sIPAddress)
			m_PrimaryMac = sMacAddress;

		// Change primary IP address if it is localhost
		if (m_PrimaryIP == "127.0.0.1" && sIPAddress != "127.0.0.1")
		{
			m_PrimaryIP = sIPAddress;
			m_PrimaryMac = sMacAddress;
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	catch (...)
	{
		LOGALL("Exception in AddInterface");
	}
}

#ifdef _PLATFORM_LINUX
void cNetInterfaces::GetLinuxNetInterfaces()
{
	FILE *pf;
	char oneline[255];

	string sIP;
	string sBroadcastIP;
	string sMacAddress;

	try
	{
		m_PrimaryIP = "127.0.0.1";
		m_PrimaryMac = "00:00:00:00:00:00";
		m_BroadcastIP = "128.0.0.1";

		bool bFirstInterface = true;
		bool bInterfaceStatus = false;
		bool bHasEntry = false;
		pf = fopen("server.netinfo","r");
		if (pf != 0)
		{
			while (fgets(oneline,80,pf) != 0)
			{
				if (isalpha(oneline[0]))
				{
					ExtractValue((const char*)oneline, "HWaddr ", sMacAddress);
					if (bHasEntry == true)
					{
						// add entry
						//
						AddInterface(sIP, sBroadcastIP, sMacAddress);
						if (bFirstInterface == true)
						{
							m_PrimaryIP = sIP;
							m_PrimaryMac = sMacAddress;
							m_BroadcastIP = sBroadcastIP;
							bFirstInterface = false;
						}
					}
				}
				else
				{
					if (strstr(oneline, "inet addr:")!=0)
					{
						ExtractValue((const char*)oneline, "inet addr:", sIP);
						ExtractValue((const char*)oneline, "Bcast:", sBroadcastIP);
						bInterfaceStatus = false;
						bHasEntry = true;
					}
					else if (strstr(oneline, "UP BROADCAST RUNNING")!=0)
					{
						bInterfaceStatus = true;
						bHasEntry = true;
					}
				}
			}
			fclose(pf);
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}
#endif // _PLATFORM_LINUX


#ifdef _PLATFORM_WIN32
void cNetInterfaces::GetWindowsNetInterfaces()
{
	string sIP;
	string sBroadcastIP;
	string sMacAddress;

	cBuffer MACAddress;

	bool bFirstInterface = true;

	try
	{
		m_PrimaryIP = "127.0.0.1";
		m_PrimaryMac = "00:00:00:00:00:00";
		m_BroadcastIP = "128.0.0.1";

		IP_ADAPTER_INFO *pAdapterInfo;
		PIP_ADAPTER_INFO pAdapter = NULL;

		pAdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
		ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

		// Make an initial call to GetAdaptersInfo to get the
		// necessary size into the ulOutBufLen variable
		if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
		{
			free(pAdapterInfo);
			pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
		}

		DWORD dwRetVal;
		if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
		{
			pAdapter = pAdapterInfo;
			while (pAdapter)
			{
				if (strcmp(pAdapter->IpAddressList.IpAddress.String,"0.0.0.0") != 0)
				{
					sIP = pAdapter->IpAddressList.IpAddress.String;
					sBroadcastIP = GetBroadcastAddress(sIP.c_str(), pAdapter->IpAddressList.IpMask.String);
					MACAddress.Sprintf(20,"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
										pAdapter->Address[0],
										pAdapter->Address[1],
										pAdapter->Address[2],
										pAdapter->Address[3],
										pAdapter->Address[4],
										pAdapter->Address[5]);
					sMacAddress = MACAddress.cstr();

					// add entry
					//
					AddInterface(sIP, sBroadcastIP, sMacAddress);

					if (bFirstInterface == true)
					{
						m_PrimaryIP = sIP;
						m_PrimaryMac = sMacAddress;
						m_BroadcastIP = sBroadcastIP;
						bFirstInterface = false;
					}
				}
				pAdapter = pAdapter->Next;
			}
		}
		else
		{
			printf("Call to GetAdaptersInfo failed.\n");
		}
		free(pAdapterInfo);
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	catch (...)
	{
		LOGALL("exception GetWindowNetInterfaces()");
	}
}
#endif //_PLATFORM_WIN32


const char *cNetInterfaces::GetHostName()
{
	char ac[260];
	char *pHostName = NULL;

	try
	{
	#ifdef _PLATFORM_WIN32
    	if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR)
		{
			LOG("Error %d when getting local host name.", WSAGetLastError());
		}
	#endif //_PLATFORM_WIN32
	#ifndef _PLATFORM_WIN32
		if (gethostname(ac, sizeof(ac)) != 0)
		{
			LOG("Error %d when getting local host name.", errno);
		}
	#endif //!_PLATFORM_WIN32
		m_ServerName = ac;

		pHostName = (char*)m_ServerName.c_str();
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return pHostName;
}

bool cNetInterfaces::ExistBroadCastAddress(string &address)
{
	try
	{
		cNetInterfaceEntry* pEntry;
		for (m_it = m_Interfaces.begin(); m_it != m_Interfaces.end(); m_it++)
		{
			pEntry = (*m_it);
			if (pEntry->m_BroadcastAddress == address)
				return true;
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return false;
}


const char *cNetInterfaces::GetBroadcastAddress(const char *pIPAddress, const char *pNetworkMask)
{
	char *pMask = NULL;
	try
	{
		// thanks to the WinSock programming FAQ
		// http://tangentsoft.net/wskfaq/intermediate.html
		unsigned long host_addr = inet_addr(pIPAddress);   // local IP addr
		unsigned long net_mask = inet_addr(pNetworkMask);  // LAN netmask
		unsigned long net_addr = host_addr & net_mask;
		unsigned long dir_bcast_addr = net_addr | (~net_mask);
		struct in_addr in;
		memcpy(&in, &dir_bcast_addr,sizeof(in_addr));
		m_NetMaskTemp = inet_ntoa(in);
		pMask = (char*)m_NetMaskTemp.c_str();
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return pMask;
}

