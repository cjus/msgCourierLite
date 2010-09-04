/* thread.h
   Copyright (C) 2002 Carlos Justiniano

thread.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

thread.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with thread.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file thread.h
 @brief C++ class that encapsulates an operating system thread 
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence (see source file header)

 C++ class that encapsulates an operating system thread 
*/

#ifndef THREAD_H
#define THREAD_H

//#include "master.h"

#ifdef _PLATFORM_LINUX
	#include <pthread.h>
	#include <errno.h>	
    #include <sys/time.h>
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32
	#include <windows.h>	
	#include <process.h>

	#ifdef Yield
	#undef Yield // don't allow the use of Yield - because it may be used when YieldSleep is required!
	#endif
#endif //_PLATFORM_WIN32

#define HRC_THREAD_OK				0x0000
#define HRC_THREAD_CREATEFAILED		0x0001
#define HRC_THREAD_NOTSUPPORTED_UNDER_LINUX	0x0002


/**
 @class cThread 
 @brief C++ class that encapsulates an operating system thread 
*/
class cThread  
{
public:
	cThread();
	virtual ~cThread();

	int Create();
	int Destroy();

	void Start();
	void Stop();
	
	void Suspend();
	void Resume();
	bool IsSuspended();

	void Sleep(int iMilliseconds);
	void YieldSleep();

	void SetThreadName(char *pName);
	const char* GetThreadName();

#ifdef _PLATFORM_LINUX
	static void* _LinuxThreadEntryPoint(void *pThread);
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32	
#ifdef _SUB_PLATFORM_WINCE
	static DWORD WINAPI _WindowsThreadEntryPoint(LPVOID pThread);
#else
	static unsigned int __stdcall _WindowsThreadEntryPoint(void *pThread);
#endif //_SUB_PLATFORM_WINCE
#endif // _PLATFORM_WIN32	

	bool IsActive();

#ifdef _PLATFORM_LINUX
	pthread_t GetThreadHandle() { return m_ThreadHandle; }
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32	
 	HANDLE GetThreadHandle() { return m_ThreadHandle; }
	unsigned int GetThreadID() { return m_ThreadID; }
#endif //_PLATFORM_WIN32

protected:
	enum threadstates
	{
		THREAD_ACTIVE = 1,
		THREAD_INACTIVE = 0
	};

	int ThreadRunning();
	virtual int Run() = 0;

#ifdef _PLATFORM_WIN32	
	HANDLE m_ThreadHandle;
	unsigned int m_ThreadID;
	HANDLE m_EventObject;
#endif //_PLATFORM_WIN32
#ifdef _PLATFORM_LINUX	
	pthread_t m_ThreadHandle;
	pthread_mutex_t m_mutex;
	pthread_cond_t m_cond;
#endif //_PLATFORM_LINUX

private:
	int	m_bActive;	// is thread still active?
	int	m_bDone;	// is thread done?

	char m_threadname[80];
	bool m_bSuspended;
	int m_iSleepInterval;

	int Shutdown();
	void Exit();
};

#endif // THREAD_H

