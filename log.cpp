/* log.cpp
   Copyright (C) 2002 Carlos Justiniano

log.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

log.cpp was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with log.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file log.cpp 
 @brief Log file and display handler 
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence (see source file header)

 The cLog class is used to display logging information on the application
 console and or to a disk file.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifdef _PLATFORM_LINUX
	#include <unistd.h>
#endif //_PLATFORM_LINUX

#ifdef _PLATFORM_WIN32
	#include <windows.h>
#endif //_PLATFORM_WIN32

#include "log.h"
#include "exception.h"

#define LOGFILE "server.log"
#define MAX_LOG_BUFFER	256

using namespace std;

cLog* cLog::m_pInstance = 0;
char cLog::m_buf[LOG_BUF_SZ];
int cLog::m_FileLogging = 0;
int cLog::m_DisplayLogging = 0;

int cLog::m_timebase = 0;

cThreadSync cLog::m_LOGThreadSync;
cThreadSync cLog::m_ddThreadSync;

vector<string> cLog::m_MessageLog;

#ifdef _PLATFORM_WIN32
	HWND cLog::m_hEdit  = NULL;
	void cLog::Logprintf(const char *pFormat, ...)
	{
		if (pFormat == 0)
			return;
		char buf[LOG_BUF_SZ];
		va_list ap;
		va_start(ap, pFormat);
		vsprintf(buf, pFormat, ap);
		va_end(ap);

		if (m_hEdit == NULL)
			return;

		SetFocus(cLog::m_hEdit);

		int iLen = GetWindowTextLength(cLog::m_hEdit);
		if (iLen > 28000)
		{
			char szBuffer[32000];
			GetWindowText(cLog::m_hEdit , (LPTSTR)szBuffer, 32000);
			SetWindowText(cLog::m_hEdit , (LPTSTR)&szBuffer[2000]);
		}

		SendMessage(cLog::m_hEdit , EM_SETSEL, (WPARAM)iLen, (LPARAM)iLen);
		SendMessage(cLog::m_hEdit , EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)buf);
		SendMessage(cLog::m_hEdit , EM_SCROLLCARET, 0, 0);

		m_MessageLog.push_back(string(buf));
		int size = m_MessageLog.size();
		int start = size - MAX_LOG_BUFFER;
		if (start > 0)
			m_MessageLog.erase(m_MessageLog.begin(), m_MessageLog.begin() + start);
	}
#endif //_PLATFORM_WIN32

#ifndef _PLATFORM_WIN32
void cLog::Logprintf(const char *pFormat, ...)
{
	if (pFormat == 0)
		return;
    char buf[LOG_BUF_SZ];
	va_list ap;
    va_start(ap, pFormat);
    int iCnt = vsprintf(buf, pFormat, ap);
    va_end(ap);

	m_MessageLog.push_back(string(buf));
	int size = m_MessageLog.size();
	int start = size - MAX_LOG_BUFFER;
	if (start > 0)
		m_MessageLog.erase(m_MessageLog.begin(), m_MessageLog.begin() + start);

	if (iCnt > 256)
	{
		buf[iCnt] = 0;
		strcat(buf, "...");
	}
    printf("%s", buf);
}
#endif //_PLATFORM_WIN32

cLog::cLog()
{
}

cLog::~cLog()
{
#ifdef _PLATFORM_WIN32
	m_hEdit = NULL;
#endif //_PLATFORM_WIN32
}

int cLog::Create(int argc, char **argv)
{
	int iRet = HRC_LOG_SUCCESS;
	if (!m_pInstance)
	{
		m_pInstance = new cLog();
		if (!m_pInstance)
			return HRC_LOG_CREATE_FAILURE;

		m_pInstance->ResetTimeBase();

		if (argc > 1)
		{
			if (strstr(argv[1], "d") != 0)
			{
				m_pInstance->SetOptDisplayLogging();
			}
			if (strstr(argv[1], "f") != 0)
			{
				m_pInstance->SetOptFileLogging();
			}
		}
	}
	return iRet;
}

int cLog::Destroy()
{
	delete m_pInstance;
	m_pInstance = 0;
	return HRC_LOG_SUCCESS;
}

cLog* cLog::GetInstance()
{
	return m_pInstance;
}

void cLog::Log(const char *pFormat, ...)
{
	cAutoThreadSync ThreadSync(&m_LOGThreadSync);

	va_list ap;
	va_start(ap, pFormat);
	vsprintf(m_buf, pFormat, ap);
	va_end(ap);

	time_t t = time(0);
	tm *t2 = localtime(&t);

	if (m_DisplayLogging)
	{
#ifndef _PLATFORM_WIN32
		Logprintf("%.2d%.2d-%.2d%.2d%.2d: %s\n", t2->tm_mon+1, t2->tm_mday, t2->tm_hour, t2->tm_min, t2->tm_sec , m_buf);
#endif //!_PLATFORM_WIN32
#ifdef _PLATFORM_WIN32
		Logprintf("%.2d%.2d-%.2d%.2d%.2d: %s\r\n", t2->tm_mon+1, t2->tm_mday, t2->tm_hour, t2->tm_min, t2->tm_sec , m_buf);
#endif //_PLATFORM_WIN32

	}
	if (m_FileLogging)
	{
		FILE *fp = 0;
		fp = fopen(LOGFILE,"a");
		if (fp)
		{
			fprintf(fp, "%.2d%.2d-%.2d%.2d%.2d: %s\n", t2->tm_mon+1, t2->tm_mday, t2->tm_hour, t2->tm_min, t2->tm_sec , m_buf);
		}
		fclose(fp);
	}
}

void cLog::LogEvent(eEventType Type, const char *pFormat, ...)
{
	//cAutoThreadSync ThreadSync(&m_LOGThreadSync);

	va_list ap;
	va_start(ap, pFormat);
	vsprintf(m_buf, pFormat, ap);
	va_end(ap);

#ifdef _PLATFORM_WIN32
    LPTSTR  lpszStrings[1];
    lpszStrings[0] = m_buf;
    HANDLE  hEventSource = RegisterEventSource(NULL, "msgCourier");
    if (hEventSource != NULL)
    {
        ReportEvent(hEventSource, (WORD)Type, 0, 0, NULL, 1, 0, (LPCTSTR*)&lpszStrings[0], NULL);
        DeregisterEventSource(hEventSource);
    }
#endif //_PLATFORM_WIN32

#ifdef _PLATFORM_LINUX
	printf("%s\n", m_buf);
	syslog(Type, m_buf);
#endif //_PLATFORM_LINUX
}

void cLog::LogToString(std::string &s, const char *pFormat, ...)
{
	cAutoThreadSync ThreadSync(&m_LOGThreadSync);

	va_list ap;
	va_start(ap, pFormat);
	vsprintf(m_buf, pFormat, ap);
	va_end(ap);

	s += m_buf;
}

void cLog::DebugDump(char *szText, char *pPtr, int nBytes)
{
	if (pPtr==0 || nBytes < 1)
		return;

	cAutoThreadSync ThreadSync(&m_ddThreadSync);
	
	int offset=0, i=0, oldi, j;
	int iloop;
	FILE *fp = 0;
	FILE *fpcon = 0;

	
	if (m_DisplayLogging)
	{
		printf("[%s]\n", szText);
		printf("[%d bytes dumped]\n", nBytes);
	}
	if (m_FileLogging)
	{
		fp = fopen(LOGFILE,"a");
		fprintf(fp,"[%s]\n", szText);
		fprintf(fp,"[%d bytes dumped]\n", nBytes);
	}

	while (i<=nBytes)
	{
		// print hex values
		oldi=i;
		iloop=0;

		if (m_DisplayLogging)
			printf("%.6x  ",offset);
		if (m_FileLogging)
			fprintf(fp,"%.6x  ",offset);
		while ((i<=nBytes) && (i-oldi<16))
		{
			if (m_DisplayLogging)
				printf("%.2x ",(unsigned char)*(pPtr));
			if (m_FileLogging)
				fprintf(fp, "%.2x ",(unsigned char)*(pPtr));
			++pPtr;
			i++;
			iloop++;
		}

		if (iloop < 16)
		{
			for (j=0; j<16-iloop; j++)
			{
				if (m_DisplayLogging)
					printf("   ");
				if (m_FileLogging)
					fprintf(fp,"   ");
			}
		}

		// print char values
		pPtr-=(i-oldi);

		if (m_DisplayLogging)
			printf(" [");
		if (m_FileLogging)
			fprintf(fp," [");

		for (j=1;j<=(i-oldi);j++)
		{
			if (!isprint(*pPtr))
			{
				if (m_DisplayLogging)
					printf("%c",(char)'.');
				if (m_FileLogging)
					fprintf(fp,"%c",(char)'.');
			}
			else
			{
				if (*pPtr < 0x09 || *pPtr == 0x09 || *pPtr == 0x0a || *pPtr == 0x0d)
				{
					if (m_DisplayLogging)
						printf("%c",(char)'.');
					if (m_FileLogging)
						fprintf(fp,"%c",(char)'.');
				}
				else
				{
					if (m_DisplayLogging)
						printf("%c",(char)*pPtr);
					if (m_FileLogging)
						fprintf(fp,"%c",(char)*pPtr);
				}
			}
			pPtr++;
		}
		if (m_DisplayLogging)
			printf("]\n");
		if (m_FileLogging)
			fprintf(fp,"]\n");
		offset+=16;
	}
	
	if (m_FileLogging)
	{
		fclose(fp);
	}
}

void cLog::GetLogBuffer(string &s)
{
	cAutoThreadSync ThreadSync(&m_ddThreadSync);
	vector<string>::iterator MessagesIterator;
	vector<string>::iterator Start;
	int count = 0;
	/*
	int size = m_MessageLog.size();
	int start = size - MAX_LOG_BUFFER;
	if (start > 0)
		m_MessageLog.erase(m_MessageLog.begin(), m_MessageLog.begin() + start);
	*/
	for (MessagesIterator = m_MessageLog.begin(); MessagesIterator != m_MessageLog.end();  MessagesIterator++)
	{
		s += *MessagesIterator;
	}
}
