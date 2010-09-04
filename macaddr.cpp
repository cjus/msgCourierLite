/* macaddr.cpp
   Copyright (C) 2002 Carlos Justiniano

macaddr.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

macaddr.cpp was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with macaddr.cpp; if not, write to the Free Software Foundation, Inc.,
675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 The cMACAddr class retrieves the machine's network MAC address
 @file macaddr.cpp
 @brief MAC Address ID class
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence
*/

#ifdef _PLATFORM_WIN32
	#include <windows.h>
	#include <Iphlpapi.h>
#endif //_PLATFORM_WIN32

#ifdef _PLATFORM_LINUX
	#include <stdio.h>		/* stdio               */
	#include <stdlib.h>		/* strtoul(), realloc() */
	#include <unistd.h>		/* STDIN_FILENO,exec    */
	#include <string.h>		/* memcpy()             */
	#include <errno.h>		/* errno                */
	#include <ctype.h>		/* toupper */
	#include <signal.h>
	#include <getopt.h>
	#include <stdarg.h>
	#include <syslog.h>
	#include <paths.h>

	#include <sys/types.h>		/* socket types         */
	#include <asm/types.h>
	#include <sys/time.h>
	#include <sys/wait.h>
	#include <sys/fcntl.h>
	#include <sys/ioctl.h>		/* ioctl()              */
	#include <sys/select.h>
	#include <sys/socket.h>		/* socket()             */
	#include <linux/if_arp.h>
	//#include <netinet/in.h>

	#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
		#include <netpacket/packet.h>
		#include <net/ethernet.h>
	#else
		#include <asm/types.h>
		#include <linux/if_packet.h>
		#include <linux/if_ether.h>
	#endif
#endif //_PLATFORM_LINUX

#include "macaddr.h"

using namespace std;

#ifdef _PLATFORM_LINUX
int open_device_sk()
{
	int device_sk = -1;
	device_sk = socket(PF_INET, SOCK_DGRAM, 0);
	return device_sk;
}

int get_hw_addr(char *name, int s, char *smac)
{
	struct ifreq ifr;
	strncpy(ifr.ifr_name, name, sizeof (ifr.ifr_name));
	if (ioctl(s, SIOCGIFHWADDR, &ifr) < 0)
	{
		//fprintf(stderr, "ioctl(SIOCGIFHWADDR): %s", name);
		return -1;
	}
	if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER)
	{
		//fprintf(stderr, "interface %s is not Ethernet!", name);
		return -1;
	}
	memcpy(smac, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
	/*
	printf("get_hw_addr:  %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n", smac[0] & 0xff,
		   smac[1] & 0xff, smac[2] & 0xff, smac[3] & 0xff, smac[4] & 0xff,
		   smac[5] & 0xff);
	*/
	return 0;
}
#endif //_PLATFORM_LINUX


cMACAddr::cMACAddr()
{
	int beth0 = 0;
	FILE *pf;
	char oneline[255];
	char macaddr[20];
	char temp[80];
	
	m_MACAddress = "00:00:00:00:00:00";

#ifdef _PLATFORM_LINUX
/*	
	pf = popen("/sbin/ifconfig","r");
	if (pf != 0)
	{
		while (fgets(oneline,80,pf) != 0)
		{
			if (oneline[0] != ' ')
			{
				beth0 = !strncmp(oneline, "eth0", 4);
				if (beth0)
				{
					m_MACAddress.append(&oneline[strlen(oneline)-20],17);
					//char *p = strstr(oneline,"HWaddr ");
					//if (p != 0)
					//{
					//	sscanf(&oneline[38],"%s", macaddr);
					//	m_MACAddress = macaddr;

					//}
				}
			}
		}
		pclose(pf);
	}
*/
	
	int c, dev_sk;
	char device[10] = "eth0";
	char mac[6];
	dev_sk = open_device_sk();
	if (get_hw_addr(device, dev_sk, mac) < 0)
	{
		//fprintf(stderr, "failed to get_hw_addr .. exiting\n");
		//exit(0);
	}
	else
	{
		sprintf(temp, "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x",
							 mac[0] & 0xff, mac[1] & 0xff, mac[2] & 0xff,
							 mac[3] & 0xff, mac[4] & 0xff, mac[5] & 0xff);
		for (int i=0; i<(int)strlen(temp); i++)
			temp[i] = toupper(temp[i]);
	}
	m_MACAddress = temp;
#endif //_PLATFORM_LINUX

	
#ifdef _PLATFORM_WIN32
	// like many windows functions we first pass zero 
	// to determine the size of the required memory block
	IP_ADAPTER_INFO AdapterInfo;
	ULONG size = 0;
	DWORD dw = GetAdaptersInfo(&AdapterInfo, &size);

	// Now that we know the size allocate memory and 
	// display all of the adapters
	IP_ADAPTER_INFO *pInfo = (IP_ADAPTER_INFO *)malloc(size);
	dw = GetAdaptersInfo(pInfo, &size);
	if (dw == ERROR_SUCCESS)
	{
		while (1)
		{
			if (strstr(pInfo->Description,"PPP")==0 &&
				strstr(pInfo->Description,"PGP")==0)
			{				
				sprintf(temp,"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
									 pInfo->Address[0],
									 pInfo->Address[1],
									 pInfo->Address[2],
									 pInfo->Address[3],
									 pInfo->Address[4],
									 pInfo->Address[5]);
				m_MACAddress = temp;
				break;
			}
			if (pInfo->Next != NULL)
				pInfo = pInfo->Next;
			else
				break;
		}
	}
	free (pInfo);
#endif //_PLATFORM_WIN32
}

const char *cMACAddr::GetMACAddr()
{
	return m_MACAddress.c_str();
}

