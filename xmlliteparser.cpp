/* xmlliteparser.cpp
   Copyright (C) 2002 Carlos Justiniano

xmlliteparser.cpp is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

xmlliteparser.cpp was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with xmlliteparser.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file xmlliteparser.cpp 
 @brief The XML Lite Parser is a fast and simple XML parser.
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence (see source file header)

 The XML Lite Parser is a fast and simple XML parser.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xmlliteparser.h"

#define XMLLITEPARSER_NEW(var, object)\
try { var = new object; }\
catch (...){\
var = NULL; }

#define INITAL_WORKING_BUFFER_SIZE 1024

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//                                                        //
//  X M L  L I T E  S T R I N G                           //
//                                                        //
//  This class is the basis for all string storage within //
//  the XMLLiteParser. Every tokenized tag, attribute     //
//  and value is stored as an XMLLiteString               //
//                                                        //
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

cXMLLiteString::cXMLLiteString()
: m_pData(NULL)
, m_iLength(0)
{   
}

cXMLLiteString::~cXMLLiteString()
{
	if (m_pData)
		free(m_pData);
    m_pData = NULL;
    m_iLength = 0;
}

/* cFTVector::Clear
 * Free the memory associated with this class.  Reset's all the list
 * pointers to NULL.  The caller is responsible for deleting all valid
 * memory pointers prior to calling this reset function!!!*/
int cXMLLiteString::ReplaceWith(const char *pDataPtr, int iLen)
{
	int iSize, iAllocSize;
	if (iLen != -1)
		iSize = iLen;
	else 
		iSize = (int)strlen(pDataPtr);

	iAllocSize = iSize;
	if (iAllocSize < 16)
		iAllocSize = 16;

	if (iAllocSize > m_iLength)
	{
		if (m_pData)
			free(m_pData);
		m_pData = (char*)malloc((iAllocSize * sizeof(char)) + 1);
		if (m_pData == NULL)
			return HRC_XMLLITEPARSER_ALLOC;
		memcpy(m_pData, pDataPtr, iSize);
		m_pData[iSize] = 0;
		m_iLength = iSize;
	}
	else
	{
		if (!m_pData)
		{
			m_pData = (char*)malloc((iAllocSize * sizeof(char)) + 1);
			if (m_pData == NULL)
				return HRC_XMLLITEPARSER_ALLOC;
		}
		memcpy(m_pData, pDataPtr, iSize);
		m_pData[iSize] = 0;
		m_iLength = iSize;
	}
	return HRC_XMLLITEPARSER_OK;
}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//                                                        //
//  X M L  L I T E  A T T R I B U T E  P A I R            //
//                                                        //
//  Class to store a name/value attribute pair            //
//                                                        //
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

cXMLLiteAttributePair::cXMLLiteAttributePair()
{
}

cXMLLiteAttributePair::~cXMLLiteAttributePair()
{
}

void cXMLLiteAttributePair::SetPair(const char *pName, const char *pValue)
{
    m_AttributeName.ReplaceWith(pName);
    m_AttributeValue.ReplaceWith(pValue);
}

int cXMLLiteAttributePair::GetAttributeValueAsInt()
{
	const char *p = m_AttributeValue.cstr();
	return (p!=NULL) ? atoi(p) : -1;
}

long cXMLLiteAttributePair::GetAttributeValueAsLong()
{
	const char *p = m_AttributeValue.cstr();
	return (p!=NULL) ? atol(p) : -1L;
}

float cXMLLiteAttributePair::GetAttributeValueAsFloat()
{
	const char *p = m_AttributeValue.cstr();
	return (p!=NULL) ? (float)atof(p) : -1L;
}

double cXMLLiteAttributePair::GetAttributeValueAsDouble()
{
	const char *p = m_AttributeValue.cstr();
	return (p!=NULL) ? (double)atof(p) : -1L;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//                                                        //
//  X M L  L I T E  E L E M E N T                         //
//                                                        //
//  Class to hold an XML Element and any list of          //
//  attributes it may have.                               //
//                                                        //
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

cXMLLiteElement::cXMLLiteElement()
: m_hasCDATA(false)
{
}

cXMLLiteElement::~cXMLLiteElement()
{
	Flush();
}

void cXMLLiteElement::Flush()
{
	cXMLLiteAttributePair *pAttribPair;
	if (m_xmlAttributes.MoveToTail() == HRC_XMLLITELIST_OK)
	{
		do
		{
			pAttribPair = (cXMLLiteAttributePair*)m_xmlAttributes.GetData();
			if (pAttribPair)
            {
				delete pAttribPair;
                pAttribPair = 0;
            }
		} while (m_xmlAttributes.MovePrev() == HRC_XMLLITELIST_OK);
	}
    m_xmlAttributes.Clear();
}

cXMLLiteAttributePair* cXMLLiteElement::GetNewAttribPair()
{
	cXMLLiteAttributePair *pAttribPair;
	XMLLITEPARSER_NEW(pAttribPair, cXMLLiteAttributePair());
	if (pAttribPair == NULL)
		return NULL;
	m_xmlAttributes.Add(pAttribPair);
	return pAttribPair;
}

void cXMLLiteElement::SetElementName(const char *pName)
{
    m_xmlTagName.ReplaceWith(pName);
}

void cXMLLiteElement::SetElementValue(const char *pValue)
{
    m_xmlValue.ReplaceWith(pValue);
}

int cXMLLiteElement::AddAttributePair(const char *pName, const char *pValue)
{
	cXMLLiteAttributePair* pAttrib = GetNewAttribPair();
	if (pAttrib == NULL)
		return HRC_XMLLITEPARSER_ALLOC;
	pAttrib->SetPair(pName, pValue);
	return HRC_XMLLITEPARSER_OK;
}

int cXMLLiteElement::GetElementValueAsInt()
{
	const char *p = m_xmlValue.cstr();
	return (p!=NULL) ? atoi(p) : -1;
}

long cXMLLiteElement::GetElementValueAsLong()
{
	const char *p = m_xmlValue.cstr();
	return (p!=NULL) ? atol(p) : -1L;
}

float cXMLLiteElement::GetElementValueAsFloat()
{
	const char *p = m_xmlValue.cstr();
	return (p!=NULL) ? (float)atof(p) : -1;
}

double cXMLLiteElement::GetElementValueAsDouble()
{
	const char *p = m_xmlValue.cstr();
	return (p!=NULL) ? (double)atof(p) : -1;
}

int cXMLLiteElement::MoveHead()
{
	int rc;
	if (m_xmlAttributes.MoveToHead() == HRC_XMLLITELIST_OK)
		rc = HRC_XMLLITEPARSER_OK;
	else
		rc = HRC_XMLLITEPARSER_EMPTY;
	return rc;
}

int cXMLLiteElement::MoveNext()
{
	int rc;
	rc = m_xmlAttributes.MoveNext();
	switch (rc)
	{
    case HRC_XMLLITELIST_OK:
        rc = HRC_XMLLITEPARSER_OK;
        break;
    case HRC_XMLLITELIST_EMPTY:
        rc = HRC_XMLLITEPARSER_EMPTY;
        break;
    case HRC_XMLLITELIST_END:
        rc = HRC_XMLLITEPARSER_END;
        break;
	};
	return rc;
}

cXMLLiteAttributePair *cXMLLiteElement::GetAttributePair()
{
	return (cXMLLiteAttributePair*)m_xmlAttributes.GetData();
}

bool cXMLLiteElement::hasAttributes()
{
	MoveHead();
	return (m_xmlAttributes.GetData()!=NULL) ? true : false;
}

const char *cXMLLiteElement::GetAttributeValue(const char *pAttributeName)
{
	MoveHead();
	const char *p;
	cXMLLiteAttributePair *pAttr = (cXMLLiteAttributePair*)m_xmlAttributes.GetData();
	while (pAttr)
	{
		p = pAttr->GetAttribName();
		if (strcmp(pAttributeName, p)==0)
			return pAttr->GetAttribValue();
		MoveNext();
		pAttr = (cXMLLiteAttributePair*)m_xmlAttributes.GetData();
	}
	return XMLPARSEREMPTYVALUE;
}

int cXMLLiteElement::GetAttributeValueAsInt(const char *pAttributeName)
{
	const char *p = GetAttributeValue(pAttributeName);
	return (p!=XMLPARSEREMPTYVALUE) ? atoi(p) : -1;
}

long cXMLLiteElement::GetAttributeValueAsLong(const char *pAttributeName)
{
	const char *p = GetAttributeValue(pAttributeName);
	return (p!=XMLPARSEREMPTYVALUE) ? atol(p) : -1L;
}

float cXMLLiteElement::GetAttributeValueAsFloat(const char *pAttributeName)
{
	const char *p = GetAttributeValue(pAttributeName);
	return (p!=XMLPARSEREMPTYVALUE) ? (float)atof(p) : -1L;
}

double cXMLLiteElement::GetAttributeValueAsDouble(const char *pAttributeName)
{
	const char *p = GetAttributeValue(pAttributeName);
	return (p!=XMLPARSEREMPTYVALUE) ? (double)atof(p) : -1L;
}

const char *cXMLLiteElement::GetElementCDATA()
{
	return m_xmlCDATA.cstr();
}

void cXMLLiteElement::SetElementCDATA(const char *pBlock, int iLen)
{
	m_hasCDATA = true;
	m_xmlCDATA.ReplaceWith(pBlock, iLen);
}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//                                                        //
//  X M L  L I T E  L I S T                               //
//                                                        //
//  This class represents the link list that the          //
//  XMLLiteParser classes use to hold list of elements    //
//  and attributes.                                       //
//                                                        //
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

cXMLLiteList::cXMLLiteList()
: m_pHead(0)
, m_pTail(0)
, m_pCurrent(0)
, m_iTotalEntries(0)
{
}

cXMLLiteList::~cXMLLiteList()
{
	Clear();
}

void cXMLLiteList::Clear()
{
	if (m_pHead == NULL)
		return;

	List *p = m_pTail;
	List *pTemp;
	while (p)
	{
		pTemp = p->pPrev;
		free(p);
		p = NULL;
		p = pTemp;
	}
	free(p);
	p = NULL;
	m_pHead = m_pTail = m_pCurrent = NULL;
}

int cXMLLiteList::Add(void* pData)
{
	if (m_pHead == NULL)
		return PlaceAtHead(pData);

	List *p = m_pTail;
	p->pNext = (List*)malloc(sizeof(List));
	if (p->pNext==NULL)
		return HRC_XMLLITEPARSER_ALLOC;
	
	p = p->pNext;
	p->pPrev = m_pTail;
	p->pData = pData;
	p->pNext = NULL;
	m_pTail = p;
	m_iTotalEntries++;
	return HRC_XMLLITELIST_OK;
}

int cXMLLiteList::Insert(void* pData)
{
	if (m_pHead == NULL)
		return PlaceAtHead(pData);
	
	List *p = (List*)malloc(sizeof(List));
	if (p==NULL)
		return HRC_XMLLITEPARSER_ALLOC;

	if (m_pCurrent->pNext == NULL)
	{
		// append
		m_pCurrent->pNext = p;
		p->pPrev = m_pCurrent;
		p->pNext = NULL;
	}
	else
	{
		// insert
		p->pNext = m_pCurrent->pNext;
		p->pPrev = m_pCurrent;
		m_pCurrent->pNext->pPrev = p;
		m_pCurrent->pNext = p;
	}
	return HRC_XMLLITELIST_OK;
}

int cXMLLiteList::PlaceAtHead(void* pData)
{
	m_pHead = (List*)malloc(sizeof(List));
	if (m_pHead==NULL)
		return HRC_XMLLITEPARSER_ALLOC;
	m_pCurrent = m_pHead;
	m_pCurrent->pData = pData;
	m_pCurrent->pNext = NULL;
	m_pCurrent->pPrev = NULL;
	m_pTail = m_pCurrent;
	m_iTotalEntries++;
	return HRC_XMLLITELIST_OK;
}

int cXMLLiteList::GetTotalEntries()
{
	return m_iTotalEntries;
}

void* cXMLLiteList::GetData()
{
	if (m_pCurrent==NULL)
		return NULL;
	return m_pCurrent->pData;
}

int cXMLLiteList::MoveToHead()
{
	if (m_pHead == NULL)
	{
		m_pCurrent = NULL;
		return HRC_XMLLITELIST_EMPTY;
	}
	m_pCurrent = m_pHead;
	return HRC_XMLLITELIST_OK;
}

int cXMLLiteList::MoveToTail()
{
	if (m_pTail == NULL)
	{
		m_pCurrent = NULL;
		return HRC_XMLLITELIST_EMPTY;
	}
	m_pCurrent = m_pTail;
	return HRC_XMLLITELIST_OK;
}

int cXMLLiteList::MoveNext()
{
	if (m_pHead == NULL)
	{
		m_pCurrent = NULL;
		return HRC_XMLLITELIST_EMPTY;
	}
	if (m_pCurrent && m_pCurrent->pNext != NULL)
	{
		m_pCurrent = m_pCurrent->pNext;
		return HRC_XMLLITELIST_OK;
	}
	m_pCurrent = NULL;
	return HRC_XMLLITELIST_END;
}

int cXMLLiteList::MovePrev()
{
	if (m_pHead == NULL)
	{
		m_pCurrent = NULL;
		return HRC_XMLLITELIST_EMPTY;
	}
	if (m_pCurrent && m_pCurrent->pPrev != NULL)
	{
		m_pCurrent = m_pCurrent->pPrev;
		return HRC_XMLLITELIST_OK;
	}
	m_pCurrent = NULL;
	return HRC_XMLLITELIST_BEGIN;
}

const XML_NODE_HANDLE cXMLLiteList::GetCurrentNode() const
{ 
	return m_pCurrent; 
}

int cXMLLiteList::MoveToNode(const XML_NODE_HANDLE handle)
{
	m_pCurrent = reinterpret_cast<List*>(handle);
	return HRC_XMLLITELIST_OK;
}

int cXMLLiteList::IsEmpty()
{
	return (m_pHead==NULL);
}

void cXMLLiteList::SavePos()
{
	m_pMarker = m_pCurrent;
}

void cXMLLiteList::RestorePos()
{
	m_pCurrent = m_pMarker;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
//                                                        //
//  X M L  L I T E  P A R S E R                           //
//                                                        //
//  Main XMLLiteParser class.                             //
//                                                        //
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

cXMLLiteParser::cXMLLiteParser()
: m_pWorkingBuffer(NULL)
, m_iWorkingBufferSize(0)
, m_pSplitBufferName(NULL)
, m_iSplitBufferNameSize(0)
, m_pSplitBufferValue(NULL)
, m_iSplitBufferValueSize(0)
{
}

cXMLLiteParser::~cXMLLiteParser()
{
	if (m_pWorkingBuffer)
		free(m_pWorkingBuffer);
	m_pWorkingBuffer = NULL;
	if (m_pSplitBufferName)
		free(m_pSplitBufferName);
	m_pSplitBufferName = NULL;
	if (m_pSplitBufferValue)
		free(m_pSplitBufferValue);
	m_pSplitBufferValue = NULL;
	Flush();
}

void cXMLLiteParser::Flush()
{
	cXMLLiteElement *pElement;
	if (m_xmlElements.MoveToTail() == HRC_XMLLITELIST_OK)
	{
		do
		{
			pElement = (cXMLLiteElement*)m_xmlElements.GetData();
			if (pElement)
            {
				delete pElement;
                pElement = 0;
            }
		} while (m_xmlElements.MovePrev() == HRC_XMLLITELIST_OK);
	}
    m_xmlElements.Clear();
}

int cXMLLiteParser::Parse(const char *pszData)
{
	char ch = 0, lastch = 0;
	int idx = 0;
	int inQoute = 0;
	int iTagMarkStart = 0;
	int iAttribMarkStart = 0;
	int iScanMarkStart  = 0;
	int iState = stTagScan;
	int	iLastState = stTagScan;
	cXMLLiteElement *pElement = 0;
	bool hasShortTerminator;

	Flush();

	while (1)
	{
		lastch = ch;
		ch = pszData[idx];
		if (ch == 0)
			break;
		if (ch == '<')
		{
			// test for XML comments
			if (strncmp(&pszData[idx],"<!--", 4) == 0)
			{
				const char *pCommentEnd = strstr(&pszData[idx], "-->");
				if (pCommentEnd)
				{
					idx += (int)(pCommentEnd - &pszData[idx]) + 3;
					iState = stTagScan;
					if (m_pWorkingBuffer)
						m_pWorkingBuffer[0] = 0;
					continue;
				}
			}

			iLastState = iState;

			if (strncmp(&pszData[idx],"<![CDATA[", 8) == 0)
				iState = stTagEnd;
			else
				iState = stTagBegin;
		}
		if (ch == '>')
		{
			iLastState = iState;
			iState = stTagEnd;
		}

		// XML DOC state machine
		switch (iState)
		{
        case stTagBegin:
            if (iScanMarkStart > 0 && iLastState == stTagScan && (idx-iScanMarkStart) > 0)
            {
                if (CheckWorkingBuffer(idx-iScanMarkStart+1) == HRC_XMLLITEPARSER_ALLOC)
                    return HRC_XMLLITEPARSER_ALLOC;

                m_pWorkingBuffer[idx-iScanMarkStart] = 0;
                StripLeading(m_pWorkingBuffer);
                StripTrailing(m_pWorkingBuffer);

                if (m_pWorkingBuffer[0] != 0)
                {
                    if (pElement)
					{
						TransformXMLEntities(m_pWorkingBuffer);
                        pElement->SetElementValue(m_pWorkingBuffer);
					}
                }
            }
				
            pElement = GetNewEntry();
            if (pElement == NULL)
                return HRC_XMLLITEPARSER_ALLOC;

            iScanMarkStart = 0;
            iTagMarkStart = idx;
            iLastState = iState;
            iState = stTagProcess;
            break;
        case stTagProcess:
            if (ch == '"')
            {
                if (inQoute == 0)
                    inQoute = 1;
                else if (inQoute == 1)
                    inQoute = 0;
            }

            if (ch == 0x20 && iAttribMarkStart==0)
            {
                iAttribMarkStart = idx;
            }
            else if ((ch == 0x20 || ch == 0x09) && inQoute == 0)
            {
                if (CheckWorkingBuffer(idx-iAttribMarkStart+1) == HRC_XMLLITEPARSER_ALLOC)
                    return HRC_XMLLITEPARSER_ALLOC;
                strncpy(m_pWorkingBuffer, &pszData[iAttribMarkStart], idx-iAttribMarkStart+1);
                m_pWorkingBuffer[idx-iAttribMarkStart+1] = 0;
                CleanInput(m_pWorkingBuffer);

                if (m_pWorkingBuffer[0] != 0)
                {
                    SplitParam(m_pWorkingBuffer, pElement);
                }
                iAttribMarkStart += idx-iAttribMarkStart;
            }
            idx++;
            continue;
            break;
        case stTagEnd:
			// test for CDATA block
			if (pElement && strncmp(&pszData[idx],"<![CDATA[", 8) == 0)
			{
				const char *pMarker = strstr(&pszData[idx], "]]>");
				if (pMarker)
				{
					iState = stTagBegin;
					int iSize = int(pMarker - &pszData[idx] - 1) - 8;
					pElement->SetElementCDATA(&pszData[idx+9], iSize);
					idx += iSize + 9 + 3 + 2;
					char *p = (char*)strstr(&pszData[idx], "<");
					if (p)
						idx += (p - &pszData[idx]);
					continue;
				}
			}

            if (CheckWorkingBuffer(idx-iTagMarkStart+1) == HRC_XMLLITEPARSER_ALLOC)
                return HRC_XMLLITEPARSER_ALLOC;
            strncpy(m_pWorkingBuffer, &pszData[iTagMarkStart], idx-iTagMarkStart+1);
            m_pWorkingBuffer[idx-iTagMarkStart+1] = 0;

            if (m_pWorkingBuffer[strlen(m_pWorkingBuffer)-2] == '/')
				hasShortTerminator = true;
			else
				hasShortTerminator = false;
            CleanInput(m_pWorkingBuffer);

            if (pElement)
			{
				TerminateAtChar(m_pWorkingBuffer,' ');
				if (hasShortTerminator)
					TerminateAtChar(m_pWorkingBuffer,'/');
                pElement->SetElementName(m_pWorkingBuffer);
			}

            if (iAttribMarkStart != 0)
            {
                strncpy(m_pWorkingBuffer, &pszData[iAttribMarkStart], idx-iAttribMarkStart+1);
                m_pWorkingBuffer[idx-iAttribMarkStart+1] = 0;
                CleanInput(m_pWorkingBuffer);
                if (m_pWorkingBuffer[0] != 0)
                {
                    SplitParam(m_pWorkingBuffer, pElement);
                }
                iAttribMarkStart = 0;
            }

			if (pElement && hasShortTerminator)
			{
				pElement = GetNewEntry();
				pElement->SetElementName("/>");	
			}

            iTagMarkStart = 0;
            iLastState = iState;
            iState = stTagScan;
            iScanMarkStart = idx+1;
            break;
        case stTagScan:
            if (idx-iScanMarkStart+1 > m_iWorkingBufferSize)
            {
                if (CheckWorkingBuffer(idx-iScanMarkStart+1) == HRC_XMLLITEPARSER_ALLOC)
                    return HRC_XMLLITEPARSER_ALLOC;
            }
            m_pWorkingBuffer[idx-iScanMarkStart] = ch;
            break;
		};

		idx++;
	}

	return HRC_XMLLITEPARSER_OK;
}

int cXMLLiteParser::CheckWorkingBuffer(int iAgainstNewSize)
{
	int iNewAlloc;
	if (m_iWorkingBufferSize == 0)
	{
		m_iWorkingBufferSize = INITAL_WORKING_BUFFER_SIZE;
		m_pWorkingBuffer = (char*)malloc(sizeof(char)*m_iWorkingBufferSize);
		if (m_pWorkingBuffer == NULL)
			return HRC_XMLLITEPARSER_ALLOC;
	}
	else if (m_iWorkingBufferSize < iAgainstNewSize)
	{
		iNewAlloc = iAgainstNewSize * 2;
		char *p = (char*)malloc(sizeof(char)*iNewAlloc);
		if (p == NULL)
			return HRC_XMLLITEPARSER_ALLOC;
		memcpy(p, m_pWorkingBuffer, m_iWorkingBufferSize);
		if (m_pWorkingBuffer)
			free(m_pWorkingBuffer);
		m_pWorkingBuffer = p;
		m_iWorkingBufferSize = iNewAlloc;
	}
	return HRC_XMLLITEPARSER_OK;
}

const XML_NODE_HANDLE cXMLLiteParser::GetCurrentNodeHandle() const
{
	return m_xmlElements.GetCurrentNode();
}

int cXMLLiteParser::MoveHead()
{
	int rc;
	if (m_xmlElements.MoveToHead() == HRC_XMLLITELIST_OK)
		rc = HRC_XMLLITEPARSER_OK;
	else
		rc = HRC_XMLLITEPARSER_EMPTY;
	return rc;
}

int cXMLLiteParser::MoveNext()
{
	int rc;
	rc = m_xmlElements.MoveNext();
	switch (rc)
	{
		case HRC_XMLLITELIST_OK:
			rc = HRC_XMLLITEPARSER_OK;
			break;
		case HRC_XMLLITELIST_EMPTY:
			rc = HRC_XMLLITEPARSER_EMPTY;
			break;
		case HRC_XMLLITELIST_END:
			rc = HRC_XMLLITEPARSER_END;
			break;
	};
	return rc;
}

int cXMLLiteParser::MoveToNode(const XML_NODE_HANDLE handle)
{
	return m_xmlElements.MoveToNode(handle);
}

cXMLLiteElement *cXMLLiteParser::GetElement()
{
	return (cXMLLiteElement*)m_xmlElements.GetData();
}

// Finds the first tag (pTag) in the entire XML document
// Warning: resets the internal iterator to the beginning 
// of the xml document - if successful, otherwise remains 
// at current node.
cXMLLiteElement *cXMLLiteParser::FindTag(const char *pTag)
{
	cXMLLiteElement *pElement = 0;
	const char *p;

	m_xmlElements.SavePos();
	if (m_xmlElements.MoveToHead() == HRC_XMLLITELIST_OK)
	{
		do
		{
			pElement = (cXMLLiteElement*)m_xmlElements.GetData();
			if (pElement)
			{
				p = pElement->GetElementName();
				if (p && strcmp(p, pTag)==0)
					return pElement;
			}
		} while (m_xmlElements.MoveNext() == HRC_XMLLITELIST_OK);
	}
	m_xmlElements.RestorePos();
	return 0;
}

// Finds the next tag (pTag) in the XML document.  If another FindTag 
// is called the internal interator position is reset to the beginning 
// of the xml document.
cXMLLiteElement *cXMLLiteParser::FindNextTag(const char *pTag, const char *pNot)
{
	const char *p;
	cXMLLiteElement *pElement;
	m_xmlElements.SavePos();
	m_xmlElements.MoveNext();
	do
	{
		pElement = (cXMLLiteElement*)m_xmlElements.GetData();
		if (pElement)
		{
			p = pElement->GetElementName();
			if (p && pNot != 0)
			{
				if (strcmp(p, pNot)==0)
				{
					return 0;
				}
			}
			if (p && strcmp(p, pTag)==0)
			{
				return pElement;
			}
		}
	} while (m_xmlElements.MoveNext() == HRC_XMLLITELIST_OK);
	m_xmlElements.RestorePos();
	return 0;
}

cXMLLiteElement* cXMLLiteParser::GetNewEntry()
{
	cXMLLiteElement *pElement;

	// if we get here we need to create an element 
	// because a reusable one doesn't exist
	XMLLITEPARSER_NEW(pElement, cXMLLiteElement());
	if (pElement == NULL)
		return NULL;
	m_xmlElements.Add(pElement);
	return pElement;
}

void cXMLLiteParser::CleanInput(char *pBuffer)
{
	if (pBuffer==NULL)
		return;
	StripChars(pBuffer);
	if (pBuffer[0] != 0)
		StripLeadingSpaces(pBuffer);
	else
		return;
	if (pBuffer[0] != 0)
		StripTrailingSpaces(pBuffer);
	else 
		return;
	if (pBuffer[0] != 0)
		StripBeginningAndEndingQoutes(pBuffer);
}

void cXMLLiteParser::StripChars(char *pBuffer)
{
	int i, k=0;
	bool qoute = false;
	for (i=0; i < (int)strlen(pBuffer)+1; i++)
	{
		if (pBuffer[i] == '"' || pBuffer[i] == '\'')
			qoute = !qoute;
		if (pBuffer[i] == '<' || pBuffer[i] == '>' || pBuffer[i] == 0x0D || pBuffer[i] == 0x0A || pBuffer[i] == 0x09)
			continue;
		if (!qoute && pBuffer[i] == '?')
			continue;
		pBuffer[k] = pBuffer[i];
		k++;
	}
	pBuffer[k] = 0;
}

void cXMLLiteParser::StripTrailingSpaces(char *pBuffer)
{
 	for (int i=(int)strlen(pBuffer)-1; i > 0; i--)
	{
		if (pBuffer[i] == 0x20)
			pBuffer[i] = 0;
		else
			break;
	}
}

void cXMLLiteParser::StripLeadingSpaces(char *pBuffer)
{
	int k=0;
	int iLength = (int)strlen(pBuffer);
	for (int i=0; i < iLength; i++)
	{
		if (pBuffer[i] == 0x20)
			k = i+1;
		else
			break;
	}
	memmove(pBuffer, &pBuffer[k], iLength - k);
	pBuffer[iLength - k] = 0;
}

void cXMLLiteParser::StripLeading(char *pBuffer)
{
	int k=0;
	int iLength = (int)strlen(pBuffer);
	for (int i=0; i < iLength; i++)
	{
		if (pBuffer[i] == 0x20 || pBuffer[i] == 0x0A || pBuffer[i] == 0x0D || pBuffer[i] == 0x09)
			k = i+1;
		else
			break;
	}
	memmove(pBuffer, &pBuffer[k], iLength - k);
	pBuffer[iLength - k] = 0;
}

void cXMLLiteParser::StripTrailing(char *pBuffer)
{
 	for (int i=(int)strlen(pBuffer)-1; i > 0; i--)
	{
		if (pBuffer[i] == 0x20 || pBuffer[i] == 0x0A || pBuffer[i] == 0x0D || pBuffer[i] == 0x09)
			pBuffer[i] = 0;
		else
			break;
	}
}

void cXMLLiteParser::StripBeginningAndEndingQoutes(char *pBuffer)
{
	int i;
	int iLength = (int)strlen(pBuffer);

	for (i=0; i < iLength; i++)
	{
		if (pBuffer[i] == '"' || pBuffer[i] == '\'')
			break;
	}
	if (i==iLength)
		return;

	memmove(&pBuffer[i], &pBuffer[i+1], iLength-i);

	iLength = (int)strlen(pBuffer);
	for (i=iLength; i > 0; i--)
	{
		if (pBuffer[i] == '"' || pBuffer[i] == '\'')
			break;
	}
	memmove(&pBuffer[i], &pBuffer[i+1], iLength-i);
	pBuffer[i] = 0;
}

void cXMLLiteParser::TerminateAtChar(char *pBuffer, char ch)
{
	int iLength = (int)strlen(pBuffer);
	for (int i=0; i < iLength; i++)
	{
		if (pBuffer[i] == ch)
		{
			pBuffer[i] = 0;
			break;
		}
	}
}

int cXMLLiteParser::SplitParam(const char *pBuf, cXMLLiteElement *pElement)
{
	int i = 0;
	int iFound = 0;
	for (i = 0; i<(int)strlen(pBuf); i++)
	{
		if (pBuf[i] == '=')
		{
			iFound = i;
			break;
		}
	}

	if (iFound)
	{
		int iBufSize = strlen(pBuf);
		if (m_iSplitBufferNameSize < iBufSize)
		{
			m_iSplitBufferNameSize = iBufSize;
			m_iSplitBufferValueSize = iBufSize;
			if (m_pSplitBufferName)
				free(m_pSplitBufferName);
			if (m_pSplitBufferValue)
				free(m_pSplitBufferValue);
			m_pSplitBufferName = (char*)malloc((iBufSize * sizeof(char)) + 1);
			m_pSplitBufferValue = (char*)malloc((iBufSize * sizeof(char)) + 1);
		}
		strncpy(m_pSplitBufferName, pBuf, iFound);
		m_pSplitBufferName[iFound] = 0;
		if (pBuf[iFound+1] == ' ')
			iFound++;
		strcpy(m_pSplitBufferValue, pBuf+iFound+1);
		i = (int)strlen(m_pSplitBufferValue);
		if (m_pSplitBufferValue[i-1] == '\r')
			m_pSplitBufferValue[i-1] = 0;
		TransformXMLEntities(m_pSplitBufferValue);
		pElement->AddAttributePair(m_pSplitBufferName, m_pSplitBufferValue);
	}
	return HRC_XMLLITEPARSER_OK;
}

void cXMLLiteParser::TransformXMLEntities(char *pBuffer)
{
	char *pIdx, *pLoc;
	int len = strlen(pBuffer);
	pLoc = pBuffer;
	while (len  > 0)
	{
		pIdx = pLoc;
		if (pIdx==0)
			break;
		pLoc = strstr(pIdx, "&");
		if (pLoc)
		{
			if (strncmp(pLoc, "&lt;", 4)==0)
			{
				*pLoc = '<';
				len = strlen(pLoc)-3;
				memmove(pLoc+1, pLoc+4, len);
			}
			else if (strncmp(pLoc, "&gt;", 4)==0)
			{
				*pLoc = '>';
				len = strlen(pLoc)-3;
				memmove(pLoc+1, pLoc+4, len);
			}
			else if (strncmp(pLoc, "&apos;", 6)==0)
			{
				*pLoc = '\'';
				len = strlen(pLoc)-5;
				memmove(pLoc+1, pLoc+6, len);
			}
			else if (strncmp(pLoc, "&amp;", 5)==0)
			{
				*pLoc = '&';
				len = strlen(pLoc)-4;
				memmove(pLoc+1, pLoc+5, len);
			}
			else if (strncmp(pLoc, "&quot;", 6)==0)
			{
				*pLoc = '"';
				len = strlen(pLoc)-5;
				memmove(pLoc+1, pLoc+6, len);
			}
			else
			{
				len--;
			}
			pLoc++;
		}
		else
		{
			len--;
		}
	}
}
