/* threadsync.h
   Copyright (C) 2004 Carlos Justiniano

threadsync.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

threadsync.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with threadsync.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file threadsync.h
 @brief Thread Synchronization objects 
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
 Thread Synchronization objects
*/

#ifndef THREADSYNC_H
#define THREADSYNC_H

//#include "master.h"

#define HRC_THREAD_SYNC_OK				0x0000
#define HRC_THREAD_SYNC_CREATEFAILED	0x0001
#define HRC_THREAD_SYNC_BUSY			0x0002

#ifdef _PLATFORM_LINUX
	#include <pthread.h>
	#include <errno.h>
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32	
	#include <windows.h>
	#include <Winbase.h>
#endif // _PLATFORM_WIN32

/**
 @class cThreadSync
 @brief Thread Synchronization object 
*/
class cThreadSync
{
public:
	cThreadSync();
	~cThreadSync();

	int Lock();
	int TryLock();
	int Unlock();
private:
#ifdef _PLATFORM_LINUX
	pthread_mutex_t	m_lock;
	pthread_mutexattr_t m_attr;
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32	
	CRITICAL_SECTION m_lock;
#endif // _PLATFORM_WIN32
};

/**
 @class cAutoThreadSync 
 @brief Automatic Thread Synchronization object 
*/
class cAutoThreadSync
{
public:
	cAutoThreadSync(cThreadSync *pThreadSync);
	~cAutoThreadSync();
private:
	cThreadSync *m_pThreadSync;
};

/**
 @class cAutoTimedThreadSync
 @brief Automatic Timed Thread Synchronization object 
*/
class cAutoTimedThreadSync
{
public:
	cAutoTimedThreadSync(cThreadSync *pThreadSync);
	~cAutoTimedThreadSync();
	int GetRetCode() { return m_rc; }
private:
	cThreadSync *m_pThreadSync;
	int m_rc;
};

#endif //THREADSYNC_H

