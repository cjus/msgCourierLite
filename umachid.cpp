/* umachid.cpp
   Copyright (C) 2002 Carlos Justiniano

umachid.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

umachid.cpp was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with umachid.h; if not, write to the Free Software Foundation, Inc.,
675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 The cUMachID class provides a unique machine id
 @file umachid.cpp
 @brief Unique machine ID class
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence
*/

#ifdef _PLATFORM_WIN32
#include <windows.h>
#endif //_PLATFORM_WIN32

#include "master.h"
#include "umachid.h"
using namespace std;

cUMachID::cUMachID()
{
	char macaddr[80];
	char temp[80];
	string temp2;
	bool isMAC = false;

	m_UMachID = APP_OS;
	strcpy(macaddr, m_MAC.GetMACAddr());
	if (strcmp(macaddr,"00:00:00:00:00:00") != 0)
	{
		isMAC = true;
		m_UMachID.append(":");
	}

#ifdef _PLATFORM_LINUX
	FILE *pf;
	char oneline[255];
	if (!isMAC)
	{
		pf = popen("/bin/cat /proc/ioports","r");
		if (pf != 0)
		{
			while (fgets(oneline,80,pf) != 0)
			{
				temp2.append(oneline);
			}
			sprintf(temp, ":%x", m_CRC.crc((unsigned char*)temp2.c_str(), temp2.length()));
			m_UMachID.append(temp);
			pclose(pf);
		}
		pf = popen("/bin/uname -a","r");
		if (pf != 0)
		{
			fgets(oneline,80,pf);
			sprintf(temp, ":%x", m_CRC.crc((unsigned char*)oneline, strlen(oneline)));
			m_UMachID.append(temp);
			pclose(pf);
		}
	}
	else
	{
		m_UMachID.append(macaddr);
	}
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32
	if (!isMAC)
	{
		LPTSTR lpVolumeNameBuffer;
		DWORD nVolumeNameSize;
		DWORD VolumeSerialNumber;
		DWORD MaximumComponentLength;
		DWORD FileSystemFlags;
		LPTSTR lpFileSystemNameBuffer;
		DWORD nFileSystemNameSize;

		BOOL bGVI = GetVolumeInformation("c:\\", 
										 NULL,
										 0,
										 &VolumeSerialNumber,
										 &MaximumComponentLength,
										 &FileSystemFlags,
										 NULL,
										 0);

		SYSTEM_INFO SystemInfo;
		GetSystemInfo(&SystemInfo);
		sprintf(temp, ":%x:%x:%x:%x", VolumeSerialNumber, SystemInfo.dwProcessorType, SystemInfo.wProcessorLevel, SystemInfo.wProcessorRevision);
		m_UMachID.append(temp);
	}
	else
	{
		m_UMachID.append(macaddr);
	}
#endif //_PLATFORM_WIN32
}

const char *cUMachID::GetID()
{
	return m_UMachID.c_str();
}

