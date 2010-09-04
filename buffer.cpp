/* buffer.cpp
   Copyright (C) 2002 Carlos Justiniano

buffer.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

buffer.cpp was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with buffer.cpp; if not, write to the Free Software Foundation, Inc.,
675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 The cBuffer class provides memory buffer and string handling
 functionality
 @file buffer.cpp
 @brief Memory buffer class
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <memory.h>

#define MEMALLOC "Unable to allocate memory"
#define THROW(reason)\
{ char _bufferException[1024];\
	sprintf(_bufferException, "%s in %s:%d", reason, (char*)__FILE__, __LINE__);\
throw _bufferException; } 

#include "buffer.h"

cBuffer::cBuffer()
: m_pBuffer(0)
, m_iBufferSize(0)
, m_iTail(0)
{
	m_minAlloc = 32;
	m_midAlloc = 256;
}

cBuffer::~cBuffer()
{
	Flush();
}

void cBuffer::SetMemAlgo(int minAlloc, int midAlloc)
{
	m_minAlloc = minAlloc;
	m_midAlloc = midAlloc;
}


/**
 * Sets the size of the internal buffer
 * @param iSize size of new buffer in bytes
 * @return BUFFER_NO_ERROR
 * @throw MEMALLOC memory allocation error
 */
int cBuffer::SetBufferSize(int iSize)
{
	int iRet = BUFFER_NO_ERROR;

	// if buffer already exist
	if (m_pBuffer != NULL)
	{
		// and if buffer is smaller then size of new buffer
		if (iSize >= m_iBufferSize) 
		{
			// create a new buffer and copy the contents of the old
			// buffer into it.
			iSize = AdjustSize(iSize);

			char *pBuffer = (char*)malloc(iSize);
			if (pBuffer != NULL)
			{
				//printf("malloc called for size of %d in cBuffer.cpp\n",
				//	   iSize);
				memcpy(pBuffer, m_pBuffer, m_iBufferSize);
				free(m_pBuffer);
				
				// When done , remap internal buffer pointer (m_pBuffer)
				// to newly created buffer.
				m_pBuffer = pBuffer;
				m_iBufferSize = iSize;
			}
			else
			{
				THROW(MEMALLOC);
			}
		}
	}
	else
	{
		// buffer doesn't already exist so let's create it
		iSize = AdjustSize(iSize);
		m_pBuffer = (char*)malloc(iSize);
		if (m_pBuffer == NULL)
		{
			THROW(MEMALLOC);
		}
		else
		{
			//printf("malloc called for size of %d in cBuffer.cpp\n",
			//	   iSize);
			m_iBufferSize = iSize;
			m_iTail = 0;
		}
	}
	return iRet;
}

/**
 * Limits (truncates) the size of the internal buffer
 * @param iSize new size of buffer
 * @return BUFFER_NO_ERROR
 * @throw MEMALLOC memory allocation error
 * @note calls SetBufferSize()
 */
int cBuffer::SetBufferLength(int iSize)
{
	int iRet;
	iRet = SetBufferSize(iSize);
	if (iRet == BUFFER_NO_ERROR)
		m_iTail = iSize;
	return iRet;
}

/**
 * Retrieve the buffer size
 * @return buffer size in bytes
 */
int cBuffer::GetBufferSize()
{
	return m_iBufferSize;
}

/**
 * Retrieve the true internal buffer size
 * @return buffer size in bytes
 */
int cBuffer::GetBufferCount()
{
	return m_iTail;
}

/**
 * Deletes the internal buffer.
 * @note This function is automatically called
 * on object destruction.
 */
void cBuffer::Flush()
{
	if (m_pBuffer != NULL)
	{
		free(m_pBuffer);
		m_pBuffer = NULL;
	}
	m_iBufferSize = 0;
	m_iTail = 0;
}

/**
 * Resets the length of the buffer to zero without deleting the memory
 * block.
 */
void cBuffer::Reset()
{
	m_iTail = 0;
}

/**
 * Returns a null terminated string pointer based on the length of the
 * buffer.
 * @return a null terminated point to the internal buffer  
 * @warning The buffer may already have nulls inside of it
 * because this class represents memory buffers.
 */
cBuffer::operator char*()
{
	if (m_pBuffer == 0)
		return 0;
	m_pBuffer[m_iTail] = 0;
	return m_pBuffer;
}

/**
 * Returns a null terminated string const pointer based on the length
 * of the buffer.
 * @return a null terminated const string
 * @warning The buffer may already have nulls inside of it
 * because this class represents memory buffers.
 */
cBuffer::operator const char*()
{
	if (m_pBuffer == 0)
		return 0;
	m_pBuffer[m_iTail] = 0;
	return m_pBuffer;
}

/**
 * Returns a null terminated string pointer based on the length
 * of the buffer.
 * @return a null terminated point to the internal buffer  
 * @warning The buffer may already have nulls inside of it
 * because this class represents memory buffers.
 */
char *cBuffer::cstr()
{
	if (m_pBuffer == 0)
		return 0;
	m_pBuffer[m_iTail] = 0;
	return m_pBuffer;
}

const char *cBuffer::c_str()
{
	if (m_pBuffer == 0)
		return 0;
	m_pBuffer[m_iTail] = 0;
	return m_pBuffer;
}

/**
 * Returns a null terminated string pointer based on the length
 * of the buffer.
 * @return a null terminated point to the internal buffer  
 * @warning The buffer may already have nulls inside of it
 * because this class represents memory buffers.
 * See ReleaseBuffer()
 */
void *cBuffer::GetBuffer()
{
	if (m_pBuffer == 0)
		return 0;
	m_pBuffer[m_iTail] = 0;
	return m_pBuffer;
}

/**
 * The complement of the GetBuffer() function.
 * @warning The this call resets the internal length of the buffer
 * based on the first occurence of a null in the buffer.
 */
void cBuffer::ReleaseBuffer()
{
	m_iTail = strlen(m_pBuffer);
}

/**
 * Get a raw (not null-terminated) buffer
 * @return a pointer to the internal buffer
 * @note this function does not try to return a null terminated string
 */
void *cBuffer::GetRawBuffer()
{
	if (m_pBuffer == 0)
		return 0;
	return m_pBuffer;
}

/**
 * Removes data starting at iIndex for the length of iLength
 * @param iIndex starting index in buffer starting at zero
 * @param iLength size in bytes of region to remove
 * @return returns BUFFER_NO_ERROR on success or 
 * BUFFER_REGION_OUTOFBOUNDS if an invalid region is 
 * specified.
 * @note The actual internal buffer is not reallocated 
 */
int cBuffer::Remove(int iIndex, int iLength)
{
	if (m_iTail == 0)
		return BUFFER_IS_EMPTY;
	int iRet = BUFFER_NO_ERROR;
	int iLen = (iLength > m_iBufferSize) ? m_iBufferSize : iLength;
	int iOffset = iIndex;
	int iCalcOffset = iLen + iOffset;

	// validate region
	if (iLen == 0 || iCalcOffset > m_iBufferSize)
		return BUFFER_REGION_OUTOFBOUNDS;

	if (m_iTail != iCalcOffset)
		memmove(&m_pBuffer[iOffset], &m_pBuffer[iCalcOffset], m_iTail - iCalcOffset);
	m_iTail -= iLen;
	return iRet;
}

/**
 * Insert data into a buffer 
 * @param iIndex Starting location inside of buffer
 * @param pData Pointer to data that will be copied
 * @param iLength Length of data to be inserted
 * @return BUFFER_NO_ERROR
 * @throw MEMALLOC memory allocation error
 */
int cBuffer::Insert(int iIndex, const char *pData, int iLength)
{
	int iRet = BUFFER_NO_ERROR;

	if (iLength == -1)
		iLength = strlen(pData);

	int iNewSize = iLength+m_iTail+1;
	if (iNewSize > m_iBufferSize)
	{
		// we don't have enough room in the buffer
		// so we must create it
		iNewSize = AdjustSize(iNewSize);
		char *pBuffer = (char*)malloc(iNewSize+1);
		if (pBuffer == NULL)
			THROW(MEMALLOC);
		
		memcpy(&pBuffer[iIndex+iLength], &m_pBuffer[iIndex], m_iTail-iIndex);
		memcpy(&pBuffer[iIndex], (void*)pData, iLength);
		if (iIndex != 0)
		{
			memcpy(&pBuffer[0], &m_pBuffer[0], iIndex);
		}

		free(m_pBuffer);
		m_pBuffer = pBuffer;
		m_iBufferSize = iNewSize;
	}
	else
	{
		// we have enough room in the buffer
		memmove(&m_pBuffer[iIndex+iLength], &m_pBuffer[iIndex], m_iTail-iIndex);
		memcpy(&m_pBuffer[iIndex], pData, iLength);
	}
	m_iTail += iLength;
	MakeString();
	return iRet;
}

/**
 * Append data to buffer 
 * @param pData Pointer to data that will be copied
 * @param iLen Length of data to append
 * @return BUFFER_PARAM_INVALID if pData is NULL or BUFFER_NO_ERROR
 * @throw MEMALLOC memory allocation error
 */
int cBuffer::Append(char *pData, int iLen)
{
	if (iLen == 0)
		return 0;
	if (pData == 0)
		return BUFFER_PARAM_INVALID;
	if (iLen == -1)
		iLen = strlen(pData);
				
	int iRet = BUFFER_NO_ERROR;
	if (iLen >= (m_iBufferSize-m_iTail))
	{
		// ok, it is.. so compute the new buffer size as being
		// the size of the SrcBuffer minus the size of the space
		// we have available, plus our buffer's size.
		int iNewSize = iLen -
            (m_iBufferSize - m_iTail) +
            m_iBufferSize;
		SetBufferSize(iNewSize);
	}
	memcpy(&m_pBuffer[m_iTail], pData, iLen);
	m_iTail += iLen;
	
	MakeString();

	return iRet;
}

/**
 * Prepend data into buffer
 * @param pData Pointer to data that will be copied
 * @param iSize Size of data to prepend
 * @return BUFFER_PARAM_INVALID if pData is NULL or BUFFER_NO_ERROR
 * @throw MEMALLOC memory allocation error
 */
int cBuffer::Prepend(char *pData, int iSize)
{
	if (pData == 0)
		return BUFFER_PARAM_INVALID;
	if (iSize == -1)
		iSize = strlen(pData);
	return Insert(0, pData, iSize);
}

int cBuffer::ReplaceWith(char *pData, int iSize)
{
	int iRet;

	if (pData == 0)
	{
		Reset();
		return BUFFER_NO_ERROR;
	}
	
	if (iSize == -1)
		iSize = strlen(pData);
	
	Reset();
	iRet = Append(pData, iSize);
	return iRet;
}

int cBuffer::Truncate(int iCount)
{
	m_iTail -= iCount;
	return BUFFER_NO_ERROR; 
}

int cBuffer::Find(char *pszText, int iOffset)
{
	if (m_iTail == 0)
		return -1;
	
	MakeString();
	int iPos;
	char *ploc = strstr(&m_pBuffer[iOffset], pszText);
	if (ploc != 0)
		iPos = (ploc - m_pBuffer); 
	else
		iPos = -1;
	return iPos;
}

int cBuffer::BinaryFind(int index, unsigned char *pPattern, int patternlen)
{
	if (m_iTail == 0 || index > m_iTail)
		return -1;
	
	unsigned char *pData = (unsigned char*)m_pBuffer+index;
	int datalen = m_iTail;
	int i,j,k;

	i = 0;
	while (i < datalen)
	{
		j = 0;
		k = i;
		while (j < patternlen)
		{
			if (pPattern[j] == pData[i])
			{	  
				j++;
				i++;
				continue;
			}
			break;
		}
		if (j == patternlen)
			return k+index;
		i = k+1;
	}
	return -1;
}

int cBuffer::CopyBetween(char *pPattern1, char *pPattern2, cBuffer &intoBuffer)
{
	int iStart, iEnd;
	intoBuffer.Reset();
	
	iStart = Find(pPattern1,0);
	if (iStart == -1)
		return -1;
	iEnd = Find(pPattern2, iStart+1);
	if (iEnd == 1)
		return -1;
	iStart += strlen(pPattern1);
	intoBuffer.ReplaceWith(m_pBuffer+iStart, iEnd-iStart);
	intoBuffer.MakeString();
	
	return 0;	
}


int cBuffer::MakeString()
{
	if (m_pBuffer)
		m_pBuffer[m_iTail] = 0;
	return 0;
}

int cBuffer::StringLen()
{
	return m_iTail;
}

int cBuffer::IsEmpty()
{
	return (m_iTail == 0) ? 1 : 0;
}

unsigned int cBuffer::HashKey()
{
	char *pKey = m_pBuffer;
	unsigned int iHash = 0;
	int i;
	for (i=0; i < m_iTail; i++)
	{
		iHash = (iHash<<5) + iHash + *pKey++;
	}
	return iHash;
}

void cBuffer::Crypto(char *pszKey)
{
	char val;
	char *pKey = pszKey;
	char *pBuffer = m_pBuffer;
	for (int i = 0; i < m_iTail; i++, pBuffer++, pKey++)
	{
		// rotate key if we reach its end
		// warning: assumes key is null terminated string
		if (!*(pKey))		
			pKey = pszKey;

		val = (*pBuffer ^ *pKey ^ (char)i);
		*pBuffer = val;
	}
}

int cBuffer::FindChar(char ch, int iStart)
{
	bool f = false;
	int i;
	for (i = iStart; i < m_iTail; i++)
	{
		if (m_pBuffer[i] == ch)
		{
			f = true;
			break;
		}
	}
	return (f==true) ? i : -1;
}

int cBuffer::ReverseFindChar(char ch, int iStart)
{
	int end = (iStart == -1) ? m_iTail : iStart;
	int i;
	for (i = end; i > -1; i--)
	{
		if (m_pBuffer[i] == ch)
			break;
	}
	return i;
}

char cBuffer::operator[](int idx)
{
	if (idx >= 0 && idx < m_iTail)
		return m_pBuffer[idx];
	return 0;
}

char* cBuffer::operator[](char *pPattern)
{
	return strstr(m_pBuffer, pPattern);
}

int cBuffer::Replace(char *pPattern, char *with)
{
	int iStart = Find(pPattern,0);
	if (iStart == -1)
		return BUFFER_REPLACE_FAILED;
	int ilen = strlen(pPattern);
	Remove(iStart, ilen);
	Insert(iStart, with, strlen(with));
	return  BUFFER_NO_ERROR;
}

int cBuffer::ReplaceAll(char *pPattern, char *with)
{
	int idx = 0;
	int iPatternLen = strlen(pPattern);
	int iWithLen = strlen(with);
	while ((idx = Find(pPattern, idx)) != -1)
	{		
		Remove(idx, iPatternLen);
		Insert(idx, with, iWithLen);
		idx += iWithLen;
	}
	return 0;
}


int cBuffer::Sprintf(int iBufferSize, char *pformat, ...)
{
	va_list argptr;
	int n;
	SetBufferSize(iBufferSize);
	char  *p = (char*)GetBuffer();
	if (p == 0)
		return 0;
	iBufferSize = GetBufferSize();
	va_start(argptr, pformat);
	n = vsprintf(p, (const char*)pformat, argptr);
	p[n] = 0;

	va_end(argptr);
	ReleaseBuffer();
	MakeString();
	return StringLen();
}

void cBuffer::ToUpper()
{
	int ichar;
	for (int i=0; i < m_iTail; i++)
	{
		ichar = m_pBuffer[i];
		if (ichar > 96 && ichar < 123)
		{
			m_pBuffer[i] = _toupper(ichar);
		}
	}
}

void cBuffer::ToLower()
{
	int ichar;
	for (int i=0; i < m_iTail; i++)
	{
		ichar = m_pBuffer[i];
		if (ichar > 64 && ichar < 91)
		{
			m_pBuffer[i] = _tolower(ichar);
		}
	}
}

int cBuffer::Cleanup()
{
	ReplaceAll("\t","");
	ReplaceAll("\r","");
	ReplaceAll("\n","");

	if (m_pBuffer == 0)
		return 0;

	int i;
	int j = 0;
	for (i=0; i< (int)strlen(m_pBuffer); i++)
	{
		if (m_pBuffer[i]==' ')
		{
			j++;
		}
		else
		{
			break;
		}
	}
	if (j != 0)
		Remove(0,j);

	j = 0;
	for (i=strlen(m_pBuffer)-1; i>-1; i--)
	{
		if (m_pBuffer[i]==' ')
		{
			j++;
		}
		else
		{
			break;
		}
	}
	if (j != 0)
		Truncate(j);

	MakeString();
	return 0;
}

int cBuffer::AdjustSize(int iSize)
{
	iSize += sizeof(long);
	if (iSize <= m_minAlloc)
		iSize = m_minAlloc;
	else if (iSize > m_minAlloc && iSize < m_midAlloc)
		iSize = (iSize * 3) / 2;
	return iSize;
}
