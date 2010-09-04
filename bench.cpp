/* bench.cpp
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

bench.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

bench.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with bench.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file bench.cpp
 @brief Benchmark component
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
 bench header class.
*/

#include "bench.h"
#include "log.h"

using namespace std;

cBench::cBench()
{
#ifdef _PLATFORM_WIN32
	m_once = 1;
#endif	
}

cBench::cBench(char *pFile, char *pLabel, int iLine)
{
	try
	{		
		stringstream ss (stringstream::in | stringstream::out);
		ss << pFile << "(" 
		   << pLabel << ")["
		   << iLine
		   << "]";
		m_sLabel = "BENCH: " + ss.str();
	}
	catch (exception const &e)
	{
		LOGALL(e.what());
	}
#ifdef _PLATFORM_WIN32
	m_once = 1;
#endif
	Start();
}

cBench::~cBench()
{
	try
	{	
		if (m_sLabel.length() != 0)
		{
			Stop();
			LOG("%s %.4f", m_sLabel.c_str(), Elapsed());
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

cBench &cBench::operator =(const cBench &that)
{
	try
	{	
    if (this != &that) 
	{
	#ifdef _PLATFORM_WIN32
			m_freq.QuadPart = m_freq.QuadPart;
			m_once = that.m_once;
	#endif //_PLATFORM_WIN32
			m_sLabel = that.m_sLabel;
	#ifdef _PLATFORM_WIN32
			m_start.QuadPart = that.m_start.QuadPart;
			m_stop.QuadPart = that.m_stop.QuadPart;
	#endif //_PLATFORM_WIN32
	#ifdef _PLATFORM_LINUX
			m_tvalStart = that.m_tvalStart;
			m_tvalStop = that.m_tvalStop;
	#endif //_PLATFORM_LINUX
	    }
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
    return *this;
}

void cBench::Start()
{
#ifdef _PLATFORM_WIN32
	if (m_once)
	{
		QueryPerformanceFrequency(&m_freq);
		m_once = 0;
	}
	QueryPerformanceCounter(&m_start);
#else	
	gettimeofday(&m_tvalStart, 0);
#endif	
}

void cBench::Stop()
{
#ifdef _PLATFORM_WIN32
	QueryPerformanceCounter(&m_stop);
#else	
	gettimeofday(&m_tvalStop, 0);
#endif
}

double cBench::Elapsed()
{
#ifdef _PLATFORM_WIN32
	return (double)((double)m_stop.QuadPart - (double)m_start.QuadPart) / ((double)m_freq.QuadPart);
#else
	double t1,t2;
	t1 = (double)m_tvalStart.tv_sec + (double)m_tvalStart.tv_usec/(1000.0*1000.0);
	t2 = (double)m_tvalStop.tv_sec + (double)m_tvalStop.tv_usec/(1000.0*1000.0);
	return t2-t1;
#endif
}

double cBench::ElapsedMicroSeconds()
{
#ifdef _PLATFORM_WIN32
	return ((double)m_stop.QuadPart - (double)m_start.QuadPart) / ((double)m_freq.QuadPart);
#else
	double t1,t2;
	t1 = (double)m_tvalStart.tv_sec + (double)m_tvalStart.tv_usec;
	t2 = (double)m_tvalStop.tv_sec + (double)m_tvalStop.tv_usec;
	return t2-t1;
#endif
}

#ifdef _PLATFORM_WIN32
void cBench::SetStart(LARGE_INTEGER &start)
{
	m_start.QuadPart = start.QuadPart;
}
#endif

#ifndef _PLATFORM_WIN32
void cBench::SetStart(timeval &tvalStart)
{
	m_tvalStart.tv_sec = tvalStart.tv_sec;
	m_tvalStart.tv_usec = tvalStart.tv_usec;
}
#endif


double cBench::GetTimeStamp()
{
#ifdef _PLATFORM_WIN32
	LARGE_INTEGER now;
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&now);
	return ((double)now.QuadPart) / ((double)freq.QuadPart);
#else
	timeval tvalNow;
	double t1;
	gettimeofday(&tvalNow, 0);
	t1 = (double)tvalNow.tv_sec + (double)tvalNow.tv_usec/(1000*1000);
	return t1;
#endif
}

