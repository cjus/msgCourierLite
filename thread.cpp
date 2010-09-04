/* thread.cpp
   Copyright (C) 2002 Carlos Justiniano

thread.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

thread.cpp was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with thread.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file thread.cpp
 @brief C++ class that encapsulates an operating system thread 
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence (see source file header)

 C++ class that encapsulates an operating system thread 
*/

#include <stdio.h>
#include <string.h>
#include "thread.h"

#ifdef _PLATFORM_LINUX
	#include <unistd.h>
	#include <sched.h>
#endif //_PLATFORM_LINUX

cThread::cThread()
: m_bActive(0)
, m_bDone(0)
, m_bSuspended(true)
, m_iSleepInterval(0)
{
	m_threadname[0] = 0;
#ifdef _PLATFORM_WIN32
	m_EventObject = 0;
#endif //_PLATFORM_WIN32
}

cThread::~cThread()
{
}

int cThread::Create()
{
	int rc;
	m_bActive = 1;

#ifdef _PLATFORM_LINUX
	pthread_mutex_init(&m_mutex, NULL);
	pthread_cond_init(&m_cond, NULL);
	rc = pthread_create(&m_ThreadHandle, NULL, _LinuxThreadEntryPoint, this);
	if (rc != 0)
	{
		//perror("pthread_create: ");
	}
	pthread_detach(m_ThreadHandle);	
#endif //_PLATFORM_LINUX
	
#ifdef _PLATFORM_WIN32
	// Create event object first
	m_EventObject = CreateEvent(NULL,TRUE,FALSE,NULL);
	m_ThreadHandle = (HANDLE)_beginthreadex(NULL, 0, _WindowsThreadEntryPoint, (void*)this, 0, &m_ThreadID);
	rc = 0;
#endif //_PLATFORM_WIN32

#ifdef _PLATFORM_LINUX
	if (rc != 0)
	{
		/*
		switch (rc)
		{
        case EAGAIN:
            LOG("Create thread failed with code %d(EAGAIN)! for %s", rc, GetThreadName());
            break;
        case EINVAL:
            LOG("Create thread failed with code %d(EINVAL)! for %s", rc, GetThreadName());
            break;
        case EPERM:
            LOG("Create thread failed with code %d(EPERM)! for %s", rc, GetThreadName());
            break;
        default:
            LOG("Create thread failed with code %d(unknown)! for %s", rc, GetThreadName());
            break;
		};
		*/
		m_bActive = 0;
		return HRC_THREAD_CREATEFAILED;
	}
#endif //_PLATFORM_LINUX
	return HRC_THREAD_OK;
}

int cThread::Destroy()
{
	Stop();
	if (m_bActive)
	{
		while (!m_bDone)
			YieldSleep();
	}
	Shutdown();
	return HRC_THREAD_OK;
}

int cThread::Shutdown()
{
	int rc = HRC_THREAD_OK;
#ifdef _PLATFORM_LINUX
	pthread_cond_destroy(&m_cond);
	pthread_mutex_destroy(&m_mutex);
	pthread_cancel(m_ThreadHandle);
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32
	CloseHandle(m_EventObject);
	CloseHandle(m_ThreadHandle);
#endif //_PLATFORM_WIN32
	m_ThreadHandle = 0;
	return rc;
}

void cThread::Exit()
{
	m_bDone = 1;
}

void cThread::SetThreadName(char *pName)
{
	strncpy(m_threadname, pName, 80);
}

const char *cThread::GetThreadName()
{
	return (const char*)m_threadname;
}

void cThread::Start()
{
	Resume();
}

void cThread::Stop() 
{ 
	m_bActive = THREAD_INACTIVE; 
	if (m_bSuspended)
		Resume();
}

void cThread::Suspend()
{
	m_bSuspended = true;
#ifdef _PLATFORM_WIN32
	ResetEvent(m_EventObject);
#endif //_PLATFORM_WIN32
}

void cThread::Resume()
{
	m_bSuspended = false;
#ifdef _PLATFORM_LINUX
	pthread_cond_signal(&m_cond);
#endif //_PLATFORM_LINUX
#ifdef _PLATFORM_WIN32
	SetEvent(m_EventObject);
#endif //_PLATFORM_WIN32
}

bool cThread::IsSuspended() 
{ 
	return m_bSuspended; 
}

int cThread::ThreadRunning()
{
	if (!m_bActive)
		return m_bActive;

#ifdef _PLATFORM_LINUX
	pthread_mutex_lock(&m_mutex);
	if (m_iSleepInterval && !m_bSuspended)
	{
        struct timeval tv;
        gettimeofday(&tv, NULL);
                              
		struct timespec condtime;
        condtime.tv_sec = tv.tv_sec;
        condtime.tv_nsec = tv.tv_usec * 1000;
    
        double nsec = condtime.tv_nsec + ((double)m_iSleepInterval*1000000L);
        if (nsec >= 1000000000L)
        {
			double secs = nsec / 1000000000L;
            condtime.tv_sec += (long)secs;
        }
        else
        {
            condtime.tv_nsec = (long)nsec;
        }
		int rc = pthread_cond_timedwait(&m_cond, &m_mutex, &condtime);
		if (rc == ETIMEDOUT)
		{
			//LOG("thread_cond_timedout");
		}
	}
	else
	{		
		if (m_bSuspended)
			pthread_cond_wait(&m_cond, &m_mutex);
	}
	pthread_mutex_unlock(&m_mutex);
#endif //_PLATFORM_LINUX
#ifdef _PLATFORM_WIN32
	if (m_iSleepInterval && !m_bSuspended)
	{
		WaitForSingleObject(m_EventObject, m_iSleepInterval);
		Resume();
	}
	else
	{
		WaitForSingleObject(m_EventObject, INFINITE);
	}
#endif //_PLATFORM_WIN32

	if (m_iSleepInterval)
		m_iSleepInterval = 0;
	return m_bActive;
}

void cThread::Sleep(int iMilliseconds)
{
	m_iSleepInterval = iMilliseconds;
#ifdef _PLATFORM_WIN32
	ResetEvent(m_EventObject);
#endif //_PLATFORM_WIN32
}

void cThread::YieldSleep()
{
#ifdef _PLATFORM_LINUX
	usleep(1000);
#endif //_PLATFORM_LINUX
#ifdef _PLATFORM_WIN32
	::Sleep(1);
#endif //_PLATFORM_WIN32
}

bool cThread::IsActive() 
{ 
	return (m_bActive == THREAD_ACTIVE) ? true : false; 
}

#ifdef _PLATFORM_LINUX	
void *cThread::_LinuxThreadEntryPoint(void *pThread)
{
	cThread *p = (cThread*)pThread;
	p->Run();
	p->Exit();
	return 0;
}
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32
unsigned int __stdcall cThread::_WindowsThreadEntryPoint(void *pThread)
{
	cThread *p = (cThread*)pThread;
	p->Run();
	p->Exit();
	return 0;
}
#endif //_PLATFORM_WIN32

