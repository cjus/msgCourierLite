/* exception.h
   Copyright (C) 2002 Carlos Justiniano

exception.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

exception.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with exception.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file exception.h
 @brief Exception based class
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef EXCEPTION_H
#define EXCEPTION_H

#ifdef _PLATFORM_WIN32
	#include <windows.h>
#endif //_PLATFORM_WIN32

#define UNKNOWN_ERROR "Unknown error"
#define MEMALLOC "unable to allocate memory"
#define FILEOPEN "unable to open file"
#define FILESAVE "unable to save file"
#define FILEWRITE "unable to write to file"

#define __STRINGIZE__(x)	#x
#define __STR__(x)			__STRINGIZE__(x)
#define __LOC__				__FILE__"("__STR__(__LINE__)") : "

#ifdef _PLATFORM_WIN32
	#define __PRETTY_FUNCTION__ __FUNCTION__
#endif //_PLATFORM_WIN32

#define THROW(reason)\
{ cException e;\
e.Explaination(reason, (char*)__PRETTY_FUNCTION__, __FILE__, __LINE__);\
e.Log();\
throw e; } 

// This macro attempts to safely allocate memory
// older C++ compilers had new() returning zero
// however newer C++ compilers have new() throw an exception
// The macro below is written to transparently handle both cases.
#define MC_NEW(var, object)\
{ \
	try \
	{ \
		var = new object; \
		if (var == 0) \
			THROW(MEMALLOC); \
	} \
	catch (cException &e) \
	{ \
		throw e; \
	} \
	catch (...) \
	{ \
		THROW(MEMALLOC); \
	} \
}

#define MC_ASSERT(condition) \
{ if (!(condition)) \
	{ \
		char _MC_ASSERT_BUF[256]; \
		sprintf(_MC_ASSERT_BUF, "[%s] failed at %s:%d", __STR__(condition), __FILE__, __LINE__); \
		throw(_MC_ASSERT_BUF); \
	} \
}

/*
#ifdef NDEBUG
#define MC_AFFIRM(condition) ((void)0)
#else
#define MC_AFFIRM(condition) { if (!(condition)) LOG("%s in %s %s:%d", "Affirmation: " __STR__(condition) " failed.", (char*)__PRETTY_FUNCTION__, __FILE__, __LINE__); }
#endif
*/

/**
 @class cException
 @brief Exception based class
*/

// see: http://www.gotw.ca/gotw/059.htm

class cException
{
public:
	cException();
	cException(const cException& other);
	cException& cException::operator=(const cException& other);

	void Explaination(char *pReason, char *pFunction,
					  char *pFilename, int line); 
	void Log();

#ifdef _PLATFORM_LINUX
	void Show_stackframe();
#endif //_PLATFORM_LINUX
private:
	void Swap(cException& other) throw();

	char m_reason[2048];
	char m_function[1024];
	char m_filename[260];
	int m_line;
};

#endif // EXCEPTION_H

