/* uniqueidprovider.cpp
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

   uniqueidprovider.cpp is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   uniqueidprovider.cpp was developed by Carlos Justiniano for use on
   the msgCourier project and the ChessBrain Project and is now distributed
   in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
   even the implied warranty of MERCHANTABILITY or FITNESS FOR A
   PARTICULAR PURPOSE.  See the GNU General Public License for more
   details.

   You should have received a copy of the GNU General Public License
   along with main.cpp; if not, write to the Free Software 
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file uniqueidprovider.cpp
 @brief Unique ID Provider
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
 Unique ID Provider
*/

#include "exception.h"
#include "uniqueidprovider.h"

#ifdef _PLATFORM_LINUX
	#include "uuid.h"
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32
	#include <windows.h>
	#include <stdlib.h>
#endif //_PLATFORM_WIN32

#include "log.h"

using namespace std;

cUniqueIDProvider *cUniqueIDProvider::m_pInstance = 0;

cUniqueIDProvider::cUniqueIDProvider()
: m_iIndex(0)
{
	SetThreadName("cUniqueIDProvider");
	try
	{
		for (int i=0; i < MAX_UNIQUE_IDS; i++)
		{	
			CreateNewID(m_sGUID);
			m_vector.push_back(m_sGUID);
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

cUniqueIDProvider::~cUniqueIDProvider()
{
}

void cUniqueIDProvider::Create()
{
    if (m_pInstance == 0)
    {
        MC_NEW(m_pInstance, cUniqueIDProvider());
        m_pInstance->cThread::Create();
		m_pInstance->cThread::Start();
    }
}

void cUniqueIDProvider::Destroy()
{
    if (m_pInstance != 0)
    {
		m_pInstance->cThread::Destroy();
        delete m_pInstance;
        m_pInstance = 0;
    }
}

int cUniqueIDProvider::Run()
{
    while (ThreadRunning())
    {
		m_ThreadSync.Lock();
		try
		{
			string newGUID;
			for (int i=0; i < MAX_UNIQUE_IDS; i++)
			{
				newGUID = m_vector[i];
				if (newGUID.length() == 0)
				{
					CreateNewID(newGUID);
					m_vector[i] = newGUID;
				}
			}
		}
		catch (exception const &e)
		{
			LOGALL(e.what());	
		}
		m_ThreadSync.Unlock();
		this->Sleep(250);
    }
	return 0;
}

void cUniqueIDProvider::GetID(string &sGUID)
{
	cAutoThreadSync ThreadSync(&m_ThreadSync);
	try
	{
		for (int i=0; i < MAX_UNIQUE_IDS; i++)
		{
			if (m_vector[i].length() != 0)
			{
				sGUID = m_vector[i];
				m_vector[i].assign("");
				return;
			}
		}
		CreateNewID(sGUID);
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

void cUniqueIDProvider::CreateNewID(string &sGUID)
{
	try
	{	
#ifdef _PLATFORM_LINUX
	#ifdef _DONT_USE_LINUX_UUID	
		char guid[40];
		sprintf(guid, "%X-0000-0000-0000-000000000000", time(0));
		sGUID.assign(guid);
	#else
		uuid_t *out;
		char *pszUUID = NULL;
		uuid_create(&out);
		uuid_make(out, UUID_MAKE_V1);
		uuid_export(out, UUID_FMT_STR, (void**)&pszUUID, NULL);
		sGUID.assign(pszUUID);		
		uuid_destroy(out);
		free(pszUUID);

MC_ASSERT(sGUID.c_str());
		
	#endif //_DONT_USE_LINUX_UUID	
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32
	unsigned char *stringuuid;
	UUID Uuid;
	RPC_STATUS rt;
	rt = UuidCreate(&Uuid);
	rt = UuidToString(&Uuid, &stringuuid);
	sGUID = (const char*)stringuuid;
	RpcStringFree(&stringuuid);
#endif //_PLATFORM_WIN32
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}


