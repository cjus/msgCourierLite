/* bench.h
   Copyright (C) 2004 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

bench.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

bench.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with bench.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file bench.h
 @brief Benchmark component
 @author Carlos Justiniano
 @attention Copyright (C) 2004 Carlos Justiniano, GNU GPL Licence (see source file header)
 bench header class.
*/

#ifndef BENCH_H
#define BENCH_H

#ifdef _PLATFORM_WIN32
	#include <windows.h>
#endif //_PLATFORM_WIN32

#ifndef _PLATFORM_WIN32
	#include <unistd.h>
	#include <sys/time.h>
#endif //not _PLATFORM_WIN32

#include <time.h>
#include "log.h"
#include <string>
#include <sstream>

class cBench
{
public:
	cBench();
	~cBench();
	cBench(char *pFile, char *pLabel, int iLine);

	cBench &operator =(const cBench&);

	double GetTimeStamp();
	void Start();
	void Stop();
	double Elapsed();
	double ElapsedMicroSeconds();
	void LogElapsedTime();
	
#ifdef _PLATFORM_WIN32
	void SetStart(LARGE_INTEGER &start);
#endif
	
#ifndef _PLATFORM_WIN32
	void SetStart(timeval &m_tvalStart);
#endif
	
private:
	std::string m_sLabel;
#ifdef _PLATFORM_WIN32
	int m_once;
	LARGE_INTEGER m_start;
	LARGE_INTEGER m_stop;
	LARGE_INTEGER m_freq;
#endif //_PLATFORM_WIN32

#ifndef _PLATFORM_WIN32
	timeval m_tvalStart;
	timeval m_tvalStop;
#endif //not _PLATFORM_WIN32
};

#ifdef NDEBUG
	#define ST(LABEL) ((void)0)
	#define ST_BEGIN(LABEL) ((void)0)
	#define ST_END() ((void)0)
#else
	#define ST(LABEL) cBench _scopebench(__FILE__,LABEL,__LINE__);
	#define ST_BEGIN(LABEL) { cBench _scopebench(__FILE__,LABEL,__LINE__);
	#define ST_END() }
#endif//NDEBUG

#endif // BENCH_H
