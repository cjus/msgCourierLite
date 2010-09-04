/* buffer.h
   Copyright (C) 2002 Carlos Justiniano

buffer.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

buffer.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with buffer.h; if not, write to the Free Software Foundation, Inc.,
675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 The cBuffer class provides memory buffer and string handling
 functionality
 @file buffer.h
 @brief Memory buffer class
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence
*/

#ifndef _CBUFFER_H
#define _CBUFFER_H

#define BUFFER_NO_ERROR						0x0000
#define BUFFER_MEMALLOC_FAILED				0x0001
#define BUFFER_WRITEMARKER_OUT_OF_BOUNDS	0x0002
#define BUFFER_READMARKER_OUT_OF_BOUNDS		0x0003
#define BUFFER_REGION_OUTOFBOUNDS			0x0004
#define BUFFER_PARAM_INVALID				0x0005
#define BUFFER_REPLACE_FAILED				0x0006
#define BUFFER_IS_EMPTY						0x0007

#define MAXCHARS 255

#include <ctype.h>
#include <string.h>

/**
 @class cBuffer
 @brief The cBuffer class provides memory buffer and string handling
 functionality.
*/
class cBuffer
{
public:
	cBuffer();
	~cBuffer();

	void SetMemAlgo(int minAlloc, int midAlloc);
	void Reset();
	void Flush();

	int SetBufferSize(int iSize);
	int SetBufferLength(int iSize);

	int GetBufferSize();
	int GetBufferCount();

	int IsEmpty();
	int MakeString();

	operator char*();
	operator const char*();
	char *cstr();
	const char* c_str();
	char operator [](int idx);
	char* operator [](char *pPattern);

	void *GetBuffer();
	void ReleaseBuffer();
	void *GetRawBuffer();

	int Remove(int iIndex, int iLength);

	int Replace(char *pPattern, char *with);
	int ReplaceAll(char *pPattern, char *with);

	int Append(char *pData, int iSize=-1);
	int Prepend(char *pData, int iSize=-1);

	int ReplaceWith(char *pData, int iSize=-1);
	int Insert(int iIndex, const char *pData, int iLength=-1);
	int Truncate(int iCount);

	int Find(char *pszText, int offset=0);
	int BinaryFind(int index, unsigned char *pPattern, int patternlen);

	int CopyBetween(char *pPattern1, char *pPattern2, cBuffer &intoBuffer);
	
	int StringLen();

	unsigned int HashKey();
	void Crypto(char *pszKey);

	int FindChar(char ch, int iStart=0);
	int ReverseFindChar(char ch, int iStart=-1);

	int Sprintf(int iBufferSize, char *pformat, ...);

	void ToUpper();
	void ToLower();

	int Cleanup();

protected:
	char *m_pBuffer;
	int m_iBufferSize;
	int m_iTail;

	int m_minAlloc;
	int m_midAlloc;

	int AdjustSize(int iSize);
};

#endif //_CBUFFER_H

