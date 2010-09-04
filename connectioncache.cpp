/* connectioncache.cpp
   Copyright (C) 2006 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

connectioncache.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

connectioncache.cpp was developed by Carlos Justiniano for use on the
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
 @file connectioncache.cpp
 @author Carlos Justiniano
 @brief TCP/IP Connection Cache
 @attention Copyright (C) 2006 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#include "connectioncache.h"
#include "log.h"
#include "exception.h"

#ifdef _PLATFORM_LINUX
	#include <sys/time.h>
	#include <sys/resource.h>
	#include <unistd.h>

	#include <netdb.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/ioctl.h>
	#include <sys/types.h>
	#include <unistd.h>
	#include <fcntl.h>
	#include <sys/sysinfo.h>
	#include <netinet/in.h>
	#include <errno.h>
#endif //_PLATFORM_LINUX

using namespace std;

cConnectionCache::cConnectionCache()
{
	m_itIncompleteMarker = m_ActiveConnectionList.end();
}

cConnectionCache::~cConnectionCache()
{
}

int cConnectionCache::AddConnection(int Socketfd, struct sockaddr* pSocketAddr, int iArrivalPort)
{
	MC_ASSERT(Socketfd > 0 && Socketfd < 65536);

	try
	{
		//LOG("OOO Adding socket %d", Socketfd);
		ConnectionRecord *pConnectionRecord = NULL;

//		map<int, ConnectionRecord>::iterator it;
//		it = m_ConnectionMap.find(Socketfd);
//		if (it == m_ConnectionMap.end())
//		{
			pConnectionRecord = &m_ConnectionMap[Socketfd];
//			pConnectionRecord->State = ConnectionRecord::CRS_FREE;
//		}
//		else
//		{
//			pConnectionRecord = &((*it).second);
//		}

		pConnectionRecord->arrivalPort = iArrivalPort;
		memcpy(&pConnectionRecord->SocketAddr, pSocketAddr, sizeof(struct sockaddr_in));
		pConnectionRecord->Socketfd = Socketfd;
		pConnectionRecord->timestamp = time(0);
		pConnectionRecord->buffer.SetBufferSize(8096);
		pConnectionRecord->buffer.Reset();
		pConnectionRecord->pActiveHandler = NULL;
		pConnectionRecord->pendingwrite = false;
		pConnectionRecord->State = ConnectionRecord::CRS_WAITING;

		m_ActiveConnectionList.push_back(pConnectionRecord);
		pConnectionRecord->m_it = --(m_ActiveConnectionList.end());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return HRC_CONNECTION_CACHE_OK;
}

int cConnectionCache::FreeConnection(int Socketfd)
{
	if (Socketfd == -1) return HRC_CONNECTION_CACHE_OK;

	if (m_ConnectionMap.find(Socketfd) == m_ConnectionMap.end())
	{
		// Trying to free a socket(%d) which doesn't exist in m_ConnectionMap!
        CloseSocket(Socketfd);
        return HRC_CONNECTION_CACHE_OK;
	}

	try
	{
		//LOG("*** Freeing socket %d", Socketfd);
		map<int, ConnectionRecord>::iterator it = m_ConnectionMap.find(Socketfd);
	  	ConnectionRecord *pConnectionRecord = &((*it).second);

		MC_ASSERT(pConnectionRecord != NULL);

		if (pConnectionRecord->m_it != m_ActiveConnectionList.end())
			m_ActiveConnectionList.erase(pConnectionRecord->m_it);

        CloseSocket(Socketfd);

		pConnectionRecord->State = ConnectionRecord::CRS_FREE;
		pConnectionRecord->pActiveHandler = NULL;
		pConnectionRecord->Socketfd = -1;
		pConnectionRecord->pendingwrite = false;
		pConnectionRecord->m_it = m_ActiveConnectionList.end();
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	catch (...)
	{
		LOGALL("exception in cConnectionCache::FreeConnection()");
	}
	return HRC_CONNECTION_CACHE_OK;
}

void cConnectionCache::CloseSocket(int socketfd)
{
    //shutdown(pConnectionRecord->Socketfd, 2);
    #ifdef _PLATFORM_LINUX
        close(socketfd);
    #endif //_PLATFORM_LINUX
    #ifdef _PLATFORM_WIN32
        closesocket(socketfd);
    #endif //_PLATFORM_WIN32
}

void cConnectionCache::MarkPendingWrite(int Socketfd)
{
	ConnectionRecord *rec = GetConnectionRecord(Socketfd);
	if (rec != NULL)
        rec->pendingwrite = true;
}

int cConnectionCache::SetActiveConnections(fd_set *fds, int size, int *maxsocketfd)
{
	try
	{
		*maxsocketfd = 0;
		if (!m_ActiveConnectionList.empty())
		{
			int count = 0;
			int curSocketfd = 0;
			ConnectionRecord *pConnectionRecord = NULL;

			// check whether the m_itIncompleteMarker is not NULL.
			// What this means is that a prior SetActiveConnections() call was unable
			// to employ all connection records.  When there are more connection records
			// available then there are file descriptor (fd_set) entries we must make
			// sure to address the remaining entries (file descriptors in ConnectRecord
			// structures).
			//
			// So the m_itIncompleteMarker allows the code below to begin where it last
			// left off... or to start at the beginning otherwise.
			if (m_itIncompleteMarker != m_ActiveConnectionList.end())
				m_it = m_itIncompleteMarker;
			else
				m_it = m_ActiveConnectionList.begin();

			// Reset the m_itIncompleteMarker to end indicating that during this pass
			// the code is making an effort to use the remaining entries.
			// NOTE: if there are *still* remaining entries which can't be addressed
			//       during this pass then m_itIncompleteMarker will be reset below
			m_itIncompleteMarker = m_ActiveConnectionList.end();

			while (m_it != m_ActiveConnectionList.end())
			{
				pConnectionRecord = (*m_it);
				if (pConnectionRecord->State == ConnectionRecord::CRS_WAITING)
				{
					curSocketfd = pConnectionRecord->Socketfd;
					MC_ASSERT(curSocketfd > 0 && curSocketfd < 65536);

					FD_SET(curSocketfd, fds);
					if (*maxsocketfd < curSocketfd)
						*maxsocketfd = curSocketfd;
					if (++count == size)
					{
						//LOG("OOO max file descriptors loaded");

						// Max file descriptors loaded.  Save a marker to the next
						// items in the list.
						m_itIncompleteMarker = m_it;
						m_itIncompleteMarker++;
						break;
					}
				}
				++m_it;
			}
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return HRC_CONNECTION_CACHE_OK;
}

ConnectionRecord *cConnectionCache::GetConnectionRecord(int Socketfd)
{
    if (Socketfd == -1 || Socketfd == 0)
        return 0;

	MC_ASSERT(Socketfd > 0 && Socketfd < 65536);
	ConnectionRecord *rec = NULL;
	try
	{
		rec = &m_ConnectionMap[Socketfd];
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return rec;
}

int cConnectionCache::TimeoutConnections(int iTimeout)
{
	try
	{
		int iCount = 0;
		if (m_ActiveConnectionList.empty() == false)
		{
			int curTime = time(0);
			ConnectionRecord *pConnectionRecord = NULL;

			m_it = m_ActiveConnectionList.begin();
			while (m_it != m_ActiveConnectionList.end())
			{
				pConnectionRecord = *m_it;
				if (pConnectionRecord->pendingwrite == false &&
					pConnectionRecord->State == ConnectionRecord::CRS_WAITING &&
					(pConnectionRecord->timestamp + iTimeout) < curTime)
				{
					FreeConnection(pConnectionRecord->Socketfd);
					iCount++;
					m_it = m_ActiveConnectionList.begin();
					continue;
				}
				else
				{
					++m_it;
				}
			}
		}
		//if (iCount != 0)
		//	LOG("OOO %d connections timed out", iCount);
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	catch (...)
	{
		LOGALL("Exception in cConnectionCache::TimeoutConnections()");
	}
	return HRC_CONNECTION_CACHE_OK;
}

void cConnectionCache::ResetEnumerator()
{
	try
	{
		m_it = m_ActiveConnectionList.begin();
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
}

ConnectionRecord *cConnectionCache::GetNextSetRecord(fd_set *fds)
{
	ConnectionRecord *pConnectionRecord = NULL;
	try
	{
		while (m_it != m_ActiveConnectionList.end())
		{
			pConnectionRecord = *m_it;
			if (pConnectionRecord->State == ConnectionRecord::CRS_WAITING &&
				pConnectionRecord->pActiveHandler == NULL &&
				(FD_ISSET(pConnectionRecord->Socketfd, fds) || pConnectionRecord->buffer.GetBufferCount() > 0) )
			{
				m_it++;
				return pConnectionRecord;
			}
			m_it++;
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return NULL;
}

int cConnectionCache::GetActiveConnectionCount()
{
	int count = 0;

	try
	{
		m_it = m_ActiveConnectionList.begin();
		while (m_it != m_ActiveConnectionList.end())
		{
			if ((*m_it)->State == ConnectionRecord::CRS_WAITING)
				count++;
			++m_it;
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
	return count;
}

int cConnectionCache::GetTotalConnectionEntries()
{
	return m_ActiveConnectionList.size();
}


