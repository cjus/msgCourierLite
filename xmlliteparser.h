/* xmlliteparser.h
   Copyright (C) 2002 Carlos Justiniano

xmlliteparser.h is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

xmlliteparser.h was developed by Carlos Justiniano for use on the
ChessBrain Project (http://www.chessbrain.net) and is now distributed
in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with xmlliteparser.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file xmlliteparser.h 
 @brief The XML Lite Parser is a fast and simple XML parser.
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence (see source file header)

 The XML Lite Parser is a fast and simple XML parser.
*/

#ifndef _XMLLITEPARSER_H
#define _XMLLITEPARSER_H

#define HRC_XMLLITEPARSER_OK			0x0000
#define HRC_XMLLITEPARSER_EMPTY			0x0001
#define HRC_XMLLITEPARSER_END			0x0002
#define HRC_XMLLITEPARSER_ALLOC			0x0003

#define XMLPARSEREMPTYVALUE ""

typedef void* XML_NODE_HANDLE;

/**
 @class cXMLLiteString
 @brief This class is the basis for all string storage within the
 XMLLiteParser. Every tokenized tag, attribute and value is stored as an
 XMLLiteString.
*/
class cXMLLiteString
{
public:
    cXMLLiteString();
    ~cXMLLiteString();
    
    int ReplaceWith(const char *pDataPtr, int iLen=-1);
    const char *cstr() { return const_cast<const char *>(m_pData); }
private:
	char *m_pData;
	int   m_iLength;
};

#define HRC_XMLLITELIST_OK					0x0000
#define HRC_XMLLITELIST_EMPTY				0x0001
#define HRC_XMLLITELIST_BEGIN				0x0002
#define HRC_XMLLITELIST_END					0x0003
#define HRC_XMLLITELIST_ITEM_NOT_FOUND		0x0004

/**
 @class cXMLLiteList
 @brief This class represents the link list that the XMLLiteParser classes use
 to hold list of elements and attributes.
*/
class cXMLLiteList  
{
public:
	cXMLLiteList();
	virtual ~cXMLLiteList();

	void Clear();

	int Add(void* pData);
	int Insert(void* pData);
	void* GetData();
	int MoveToHead();
	int MoveToTail();
	int MoveNext();
	int MovePrev();
	const XML_NODE_HANDLE GetCurrentNode() const;
	int MoveToNode(const XML_NODE_HANDLE handle);

	int IsEmpty();

	int GetTotalEntries();

	void SavePos();
	void RestorePos();

private:
	struct List
	{
		void *pData;
		List *pNext;
		List *pPrev;
	};

	List *m_pHead;
	List *m_pTail;
	List *m_pCurrent;
	List *m_pMarker;

	int m_iTotalEntries;

	int PlaceAtHead(void* pData);
};

/**
 @class cXMLLiteAttributePair
 @brief Class to store a name/value attribute pair
 */
class cXMLLiteAttributePair
{
public:
	cXMLLiteAttributePair();
	~cXMLLiteAttributePair();

	void SetPair(const char *pName, const char *pValue);

	const char *GetAttribName() { return m_AttributeName.cstr(); }
	const char *GetAttribValue() { return m_AttributeValue.cstr(); }
	int GetAttributeValueAsInt();
	long GetAttributeValueAsLong();
	float GetAttributeValueAsFloat();
	double GetAttributeValueAsDouble();

private:
	cXMLLiteString m_AttributeName;
	cXMLLiteString m_AttributeValue;
};

/**
 @class cXMLLiteElement
 @brief Class to hold an XML Element and any list of attributes it may have.
*/
class cXMLLiteElement
{
public:
	cXMLLiteElement();
	~cXMLLiteElement();

	void SetElementName(const char *pName);
	void SetElementValue(const char *pValue);
	const char *GetElementName() { return m_xmlTagName.cstr(); }

	const char *GetElementValue() { return m_xmlValue.cstr(); }
	int GetElementValueAsInt();
	long GetElementValueAsLong();
	float GetElementValueAsFloat();
	double GetElementValueAsDouble();

	int AddAttributePair(const char *pName, const char *pValue);

	// Quick value get if you already know the name of the attribute
	const char *GetAttributeValue(const char *pAttributeName);
	int GetAttributeValueAsInt(const char *pAttributeName);
	long GetAttributeValueAsLong(const char *pAttributeName);
	float GetAttributeValueAsFloat(const char *pAttributeName);
	double GetAttributeValueAsDouble(const char *pAttributeName);

	bool hasCDATA() { return m_hasCDATA; }
	const char *GetElementCDATA();
	void SetElementCDATA(const char *pBlock, int iLen);

	// Attribute list travesal (enumeration)
	bool hasAttributes();
	int MoveHead();
	int MoveNext();
	cXMLLiteAttributePair *GetAttributePair();
private:
	cXMLLiteString m_xmlCDATA;
	cXMLLiteString m_xmlTagName;	// tag name
	cXMLLiteString m_xmlValue;		// data element string
	cXMLLiteList m_xmlAttributes;	// list of cXMLLiteAttributePair
	bool m_hasCDATA;
	void Flush();
	cXMLLiteAttributePair* GetNewAttribPair();
};

/**
 @class cXMLLiteParser
 @brief Main XMLLiteParser class.
*/
class cXMLLiteParser
{
public:
	cXMLLiteParser();
	~cXMLLiteParser();

	int Parse(const char *pszData);

	cXMLLiteElement *FindTag(const char *pTag);
	cXMLLiteElement *FindNextTag(const char *pTag, const char *pNot=0);
	const XML_NODE_HANDLE GetCurrentNodeHandle() const;

	int MoveHead();
	int MoveNext();
	int MoveToNode(const XML_NODE_HANDLE handle);

	cXMLLiteElement *GetElement();

private:
	// Parser state machine 
	enum
	{
		stTagScan,
		stTagBegin,
		stTagProcess,
		stTagEnd
	};

	cXMLLiteList m_xmlElements;

	char *m_pWorkingBuffer;
	int   m_iWorkingBufferSize;

	char *m_pSplitBufferName;
	int m_iSplitBufferNameSize;
	char *m_pSplitBufferValue;
	int m_iSplitBufferValueSize;

	void Flush();
	int CheckWorkingBuffer(int iAgainstNewSize);

	cXMLLiteElement* GetNewEntry();
	
	// cleanup utils
	void CleanInput(char *pBuffer);
	void StripChars(char *pBuffer);
	void StripTrailingSpaces(char *pBuffer);
	void StripLeadingSpaces(char *pBuffer);

	void StripLeading(char *pBuffer);
	void StripTrailing(char *pBuffer);

	void StripBeginningAndEndingQoutes(char *pBuffer);
	void TerminateAtChar(char *pBuffer, char ch);
	void TransformXMLEntities(char *pBuffer);

	int  SplitParam(const char *pBuf, cXMLLiteElement *pElement);

};

#endif //_XMLLITEPARSER_H

/*
	cXMLLiteParser xml;
	cXMLLiteElement *pElm; 
	xml.Parse(sXML.c_str());
	xml.MoveHead();
	do 
	{
		pElm = xml.GetElement();
		pData = const_cast<char*>(pElm->GetElementValue());
		if (pData == NULL)
			LOG("|%s|", pElm->GetElementName());
		else
			LOG("|%s|[%s]", pElm->GetElementName(), pData);
		if (pElm->hasAttributes())
		{
			pElm->MoveHead();
			do 
			{
				cXMLLiteAttributePair *pAttr = pElm->GetAttributePair();
				pData = const_cast<char*>(pAttr->GetAttribValue());
				LOG("   attrib: |%s|[%s]", pAttr->GetAttribName(), pData);
			} while (pElm->MoveNext() == HRC_XMLLITELIST_OK);
		}
		if (pElm->hasCDATA())
		{
			LOG("   CDATA: %s", pElm->GetElementCDATA());
		}
	} while (xml.MoveNext() == HRC_XMLLITELIST_OK);
*/

