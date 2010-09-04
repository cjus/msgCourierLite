/* exception.cpp
   Copyright (C) 2002 Carlos Justiniano

exception.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

exception.cpp was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with exception.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file exception.cpp
 @brief Exception based class
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifdef MSGCOURIER
	#include "log.h"
#endif //MSGCOURIER

#ifdef MCSV
        #include "log.h"
#endif //MCSV

#ifdef MCC
	#include "./mcc/display.h"
	#include <stdlib.h>
#endif // MCC

#include "exception.h"

#include <string>

#ifdef _PLATFORM_WIN32
	#include "win32stackwalker.h"
#endif //_PLATFORM_WIN32

#ifdef _PLATFORM_LINUX
	#include <stdio.h>
	#include <signal.h>
	#include <execinfo.h>
#endif //_PLATFORM_LINUX

using namespace std;

cException::cException()
{
	m_line = 0;
}

cException::cException(const cException& other)
{
	strncpy(this->m_filename, other.m_filename, 259);
	this->m_filename[259] = 0;
	strncpy(this->m_function, other.m_function, 1023);
	this->m_function[1023] = 0;
	strncpy(this->m_reason, other.m_reason, 2047);
	this->m_reason[2047] = 0;
	this->m_line = other.m_line;
}

void cException::Swap(cException& other) throw()
{
	cException temp(other);

	strncpy(other.m_filename, this->m_filename,259);
	other.m_filename[259] = 0;
	strncpy(other.m_function, this->m_function,1023);
	other.m_function[1023] = 0;
	strncpy(other.m_reason, this->m_reason,2047);
	other.m_reason[2047] = 0;
	other.m_line, this->m_line;

	strncpy(this->m_filename, temp.m_filename,259);
	this->m_filename[259] = 0;
	strncpy(this->m_function, temp.m_function,1023);
	this->m_function[1023] = 0;
	strncpy(this->m_reason, temp.m_reason,2047);
	this->m_reason[2047] = 0;
	this->m_line = temp.m_line;
}


cException& cException::operator=(const cException& other)
{
	cException temp(other);
	Swap(temp);
	return *this;
}

void cException::Log()
{
	std::string sInfo;
	char buffer[8096];
	sprintf(buffer, "Exception error: %s in %s %s:%d",
		m_reason, m_function, m_filename, m_line);
	LOGERROR(buffer);
#ifndef _PLATFORM_WIN32
	void *trace[16];
	char **messages = (char **)NULL;
	int i, trace_size = 0;

	trace_size = backtrace(trace, 16);
	messages = backtrace_symbols(trace, trace_size);
	printf("[bt] Execution path:\n");
	for (i=0; i<trace_size; ++i)
	{
		sInfo +=messages[i];	
		sInfo +="\n";
	}
	LOGERROR(sInfo.c_str());
	exit(99);
#else
	cWin32StackWalker::WalkStack(false);
	cWin32StackWalker::CopyString(sInfo);
	LOGERROR(sInfo.c_str());
#endif //_PLATFORM_WIN32
}

void cException::Explaination(char *pReason, char *pFunction,
							  char *pFilename, int line)
{
	strncpy(m_reason, pReason, 2047);
	m_reason[2047] = 0;
	strncpy(m_function, pFunction, 1023);
	m_function[1023] = 0;
	strncpy(m_filename, pFilename, 259);
	m_filename[259] = 0;
	m_line = line;
}

#ifdef _PLATFORM_LINUX
void cException::Show_stackframe() 
{
	void *array[16];	
	int size = backtrace(array, 16);
	char **messages = backtrace_symbols(array, size);
	printf("Exception execution path:\n");
	for (int i=0; i<size; ++i)
		printf("\t%s\n", messages[i]);
	printf("If you did not stop this program, please email\n"); 
    printf("the list shown above to: cjus@chessbrain.net\n");
	free(messages);
}
#endif // _PLATFORM_LINUX

