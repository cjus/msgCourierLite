/* log.h
   Copyright (C) 2002 Carlos Justiniano

log.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

log.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with log.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file log.h 
 @brief Log file and display handler 
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence (see source file header)

 The cLog class is used to display logging information on the application
 console and or to a disk file.
*/

#ifndef _LOG_H
#define _LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include <string>
#include <vector>
#include "threadsync.h"
#include "timer.h"
#include "exception.h"

#ifdef _PLATFORM_LINUX
	#include <syslog.h>
#endif //_PLATFORM_LINUX

#define LOG_BUF_SZ	256000
#define _MAX_PATH   260

#define HRC_LOG_SUCCESS             0x0000
#define HRC_LOG_CREATE_FAILURE		0x0001
#define HRC_LOG_MEMALLOC_FAILURE	0x0002

/**
 @class cLog 
 @brief Log file and display handler 
*/
class cLog
{
public:
	typedef enum EVENTS
	{ 
#ifdef _PLATFORM_WIN32
		LOGEVT_INFO = EVENTLOG_INFORMATION_TYPE,
		LOGEVT_WARNING = EVENTLOG_WARNING_TYPE,
		LOGEVT_ERROR = EVENTLOG_ERROR_TYPE
#else
		LOGEVT_INFO = LOG_INFO,
		LOGEVT_WARNING = LOG_WARNING,
		LOGEVT_ERROR = LOG_ERR
#endif
	} eEventType;

	static int Create(int argc, char **argv);

	static int Destroy();
	static cLog* GetInstance();

	void ResetTimeBase() { m_timebase = time(0); }
			
	void SetOptFileLogging() { m_FileLogging = 1; }
	void SetOptDisplayLogging() { m_DisplayLogging = 1; }
	
	static void Log(const char *pFormat, ...);
	static void LogEvent(eEventType Type, const char *pFormat, ...);
	static void LogToString(std::string &s, const char *pFormat, ...);

	static void DebugDump(char *szText, char *pPtr, int nBytes);
	static char* GetVersionInfo();

	static void Logprintf(const char *pFormat, ...);

#ifdef _PLATFORM_WIN32
	static HWND m_hEdit;
	HWND GetEditCtl() { return m_hEdit; }
	void SetEditCtl(HWND hEditWnd) { m_hEdit = hEditWnd; }
#endif //_PLATFORM_WIN32

	static void GetLogBuffer(std::string &s);
protected:
	cLog();
	~cLog();

private:
	int Init();

	static cLog *m_pInstance;
//#ifdef _PLATFORM_WIN32
//	static char m_MSWindowsBuffer[64000];
//#endif //_PLATFORM_WIN32		
	static char m_buf[LOG_BUF_SZ];
	static int m_timebase;
	static int m_FileLogging;
	static int m_DisplayLogging;

	static cThreadSync m_LOGThreadSync;
	static cThreadSync m_ddThreadSync;

	static std::vector<std::string> m_MessageLog;
};

// Line Trace Macro
#define LT() \
{ \
    cLog::Log("{%s [%s:%d]}", __FILE__,(char*)__PRETTY_FUNCTION__,__LINE__);  \
} \

// Line Trace Short Macro
#define LTS()\
{\
    cLog::Log("{[%s:%d]}", (char*)__PRETTY_FUNCTION__,__LINE__);\
}

#define LFL()\
{\
	cLog::Log("{[%s:%d]}", __FILE__,__LINE__);\
}\

#define L LFL();

#define LOG cLog::Log
#define LOG2\
  cLog::Log("{%s [%s:%d]}", __FILE__,(char*)__PRETTY_FUNCTION__,__LINE__);\
  cLog::Log

#define TRACE(MSG) \
{\
    cLog::Log("%s in {%s [%s:%d]}", MSG, __FILE__,(char*)__PRETTY_FUNCTION__,__LINE__);  \
}

#define LOGINFO(LOGMSG) cLog::LogEvent(cLog::LOGEVT_INFO, LOGMSG)
#define LOGWARNING(LOGMSG) cLog::LogEvent(cLog::LOGEVT_WARNING, LOGMSG)
#define LOGERROR(LOGMSG) cLog::LogEvent(cLog::LOGEVT_ERROR, LOGMSG)

#ifdef _PLATFORM_WIN32
#define LOGALL(LOGMSG)\
{\
	LOG2(LOGMSG);\
	LOGERROR(LOGMSG);\
	throw LOGMSG; \
}	
#endif //_PLATFORM_WIN32

#ifdef _PLATFORM_LINUX
#define LOGALL(LOGMSG)\
{\
	LOG2(LOGMSG);\
	LOGERROR(LOGMSG);\
	cException ex; \
	ex.Show_stackframe();\
	throw LOGMSG; \
}	
#endif //_PLATFORM_LINUX

#define LOGMEMDUMP cLog::DebugDump

#ifdef NDEBUG
	#define HOMER(TIMER_OBJ) ((void)0)
#else
	#define HOMER(TIMER_OBJ)\
		if (TIMER_OBJ.IsReady())\
		{\
			cLog::Log("HOMER[%d]: {%s [%s:%d]}", TIMER_OBJ.GetInterval(), __FILE__,(char*)__PRETTY_FUNCTION__, __LINE__);\
			TIMER_OBJ.Reset(); \
		}
#endif//NDEBUG

#endif //_LOG_H
