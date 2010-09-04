/* netinterfaces.h
   Copyright (C) 2005 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

netinterfaces.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

netinterfaces.h was developed by Carlos Justiniano for use on the
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
 @file netinterfaces.h
 @brief Track Network Interfaces
 @author Carlos Justiniano
 @attention Copyright (C) 2005 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef NETINTERFACES_H
#define NETINTERFACES_H

#include <string>
#include <vector>

class cNetInterfaceEntry
{
public:
	std::string m_IPAddress;
	std::string m_BroadcastAddress;
	std::string m_MacAddress;
};

class cNetInterfaces
{
public:
	cNetInterfaces();
	~cNetInterfaces();

	cNetInterfaceEntry *QueryFirstInterface();
	cNetInterfaceEntry *QueryNextInterface();

	const char *GetHostName();
	const char *GetPrimaryIP() { return m_PrimaryIP.c_str(); }
	const char *GetBroadcastIP() { return m_BroadcastIP.c_str(); }
	const char *GetPrimaryMac() { return m_PrimaryMac.c_str(); }
	
	const char *GetBroadcastAddress(const char *pIPAddress, const char *pNetworkMask);

private:
	std::vector<cNetInterfaceEntry*> m_Interfaces;
	std::vector<cNetInterfaceEntry*>::iterator m_it;
	std::string m_ServerName;
	std::string m_PrimaryIP;
	std::string m_PrimaryMac;
	std::string m_BroadcastIP;
	std::string m_NetMaskTemp;

	void AddInterface(std::string &sIPAddress, std::string &sBroadcastAddress, std::string &sMacAddress);
	void ExtractValue(const char *pLine, const char *pField, std::string &value);
	bool ExistBroadCastAddress(std::string &address);

#ifdef _PLATFORM_LINUX
	void GetLinuxNetInterfaces();
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32
	void GetWindowsNetInterfaces();
#endif //_PLATFORM_WIN32
};

#endif // NETINTERFACES_H

