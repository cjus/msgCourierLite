/* threadsync.cpp
   Copyright (C) 2004 Carlos Justiniano

threadsync.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

threadsync.cpp was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with threadsync.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file threadsync.cpp 
 @brief Thread Synchronization objects
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
 Thread Synchronization objects
*/

#include <stdio.h>
#include <stdlib.h>
#include "threadsync.h"

#ifdef _PLATFORM_LINUX
	#include <unistd.h>
	#include <signal.h>
	#define MUTEX_ERROR_CHECK 1
#endif //_PLATFORM_LINUX

/**
 * cThreadSync Constructor
 */
cThreadSync::cThreadSync()
{
#ifdef _PLATFORM_LINUX
	#ifdef MUTEX_ERROR_CHECK	
		pthread_mutexattr_init(&m_attr);
		//pthread_mutexattr_setkind_np(&m_attr, PTHREAD_MUTEX_ERRORCHECK_NP);
		pthread_mutex_init(&m_lock, &m_attr);
	#else
		pthread_mutex_init(&m_lock, NULL);
	#endif
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32
	InitializeCriticalSection(&m_lock);
#endif //_PLATFORM_WIN32
}

/**
 * cThreadSync Destructor 
 */
cThreadSync::~cThreadSync()
{
#ifdef _PLATFORM_LINUX
	pthread_mutex_destroy(&m_lock);
#endif //_PLATFORM_LINUX
#ifdef _PLATFORM_WIN32
	DeleteCriticalSection(&m_lock);
#endif //_PLATFORM_WIN32
}

/**
 * Acquire exclusive access to code block 
 * @return HRC_THREAD_SYNC_OK if lock is acquired  
 * @note Thread blocks if unable to acquire lock
 */
int cThreadSync::Lock()
{
	int rc = HRC_THREAD_SYNC_OK;
#ifdef _PLATFORM_LINUX
	#ifdef MUTEX_ERROR_CHECK	
		rc = pthread_mutex_lock(&m_lock);
		//MC_ASSERT(rc != EDEADLK);
	#else
		pthread_mutex_lock(&m_lock);
	#endif
#endif //_PLATFORM_LINUX
#ifdef _PLATFORM_WIN32
	EnterCriticalSection(&m_lock);
#endif //_PLATFORM_WIN32
	return rc;
}

/**
 * Attempts to acquire exclusive access to code block 
 * @return HRC_THREAD_SYNC_OK if lock is acquired or HRC_THREAD_SYNC_BUSY
 * if another thread already has exclusive access to the code block. 
 */
int cThreadSync::TryLock()
{
	int rc = HRC_THREAD_SYNC_OK;
#ifdef _PLATFORM_LINUX
	#ifdef MUTEX_ERROR_CHECK	
		rc = pthread_mutex_trylock(&m_lock);
		//MC_ASSERT(rc != EDEADLK);
		if (rc == EBUSY)
			return HRC_THREAD_SYNC_BUSY;
	#else
		rc = pthread_mutex_trylock(&m_lock);
		if (rc == EBUSY)
			return HRC_THREAD_SYNC_BUSY;
	#endif
#endif //_PLATFORM_LINUX
#ifdef _PLATFORM_WIN32
		rc = (TryEnterCriticalSection(&m_lock)) ? HRC_THREAD_SYNC_OK : HRC_THREAD_SYNC_BUSY;
#endif //_PLATFORM_WIN32
	return rc;
}

/**
 * Release exclusive access to code block 
 * @return HRC_THREAD_SYNC_OK 
 */
int cThreadSync::Unlock()
{
	int rc = HRC_THREAD_SYNC_OK;
#ifdef _PLATFORM_LINUX
	pthread_mutex_unlock(&m_lock);
#endif //_PLATFORM_LINUX
#ifdef _PLATFORM_WIN32
	LeaveCriticalSection(&m_lock);
#endif //_PLATFORM_WIN32
	return rc;
}

/**
 * Automatic lock and unlock on stack frame 
 * @param pThreadSync pointer to a cThreadSync object
 * @note block if unable to acquire exclusive lock 
 */
cAutoThreadSync::cAutoThreadSync(cThreadSync *pThreadSync)
: m_pThreadSync(pThreadSync) 
{
	m_pThreadSync->Lock();
}

cAutoThreadSync::~cAutoThreadSync()
{
	m_pThreadSync->Unlock();
}

/**
 * Experimental Automatic timed lock and unlock on stack frame 
 * @param pThreadSync pointer to a cThreadSync object
 * @note make sure to use the GetRetCode() function to determine
 * whether the lock was successful. 
 */
cAutoTimedThreadSync::cAutoTimedThreadSync(cThreadSync *pThreadSync)
: m_pThreadSync(pThreadSync)
, m_rc(HRC_THREAD_SYNC_BUSY)
{
	int spin = 10;
	while (spin--)
	{
		if (m_pThreadSync->TryLock() != HRC_THREAD_SYNC_BUSY)
		{
			m_rc = HRC_THREAD_SYNC_OK;
			break;
		}
#ifdef _PLATFORM_LINUX
		usleep(1000);
#endif //_PLATFORM_LINUX
#ifdef _PLATFORM_WIN32
		Sleep(10);
#endif //_PLATFORM_WIN32
	}
}

/**
 * cAutoTimedThreadSync Destructor 
 */
cAutoTimedThreadSync::~cAutoTimedThreadSync()
{
	m_pThreadSync->Unlock();
}

