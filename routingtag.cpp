/* routingtag.cpp
   Copyright (C) 2006 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

routingtag.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

routingtag.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with routingtag.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file routingtag.cpp
 @brief  Routing Tag
 @author Carlos Justiniano
 @attention Copyright (C) 2006 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#include "exception.h"
#include "log.h"
#include "routingtag.h"

using namespace std;

cRoutingTag::cRoutingTag()
{
}

cRoutingTag::~cRoutingTag()
{
}

const char* cRoutingTag::GetTag()
{
	return m_sFullTag.c_str();
}

const char* cRoutingTag::GetTagName()
{
	return m_sTagName.c_str();
}

const char* cRoutingTag::GetTagIPAddr()
{
	return m_sTagIPAddr.c_str();
}

const char* cRoutingTag::GetTagPort()
{
	return m_sTagPort.c_str();
}

void cRoutingTag::SetTag(const char *pTag)
{
	string tag = pTag;
	string::size_type at_idx = tag.find("@");
	string::size_type colon_idx = tag.find(":");

	// handle tag name
	if (at_idx != string::npos && at_idx > 0)
		m_sTagName = tag.substr(0, at_idx);
	else
	{
		if (at_idx == string::npos && colon_idx == string::npos)
		{
			m_sTagName = pTag;
			m_sFullTag = m_sTagName;
			return;
		}
		else
		{
			m_sTagName = "unknown";
		}
	}

	// handle port address
	if (colon_idx != string::npos && colon_idx > 0)
		m_sTagPort = tag.substr(colon_idx+1);
	else
		m_sTagPort = "0000";

	// handle IP address
	int iplen = colon_idx - at_idx - 1;
	if (iplen > 0)
	{
		m_sTagIPAddr = tag.substr(at_idx+1, iplen);
	}
	else
	{
		string::size_type dot_idx = tag.find(".");
		if (dot_idx != string::npos)
		{
			m_sTagIPAddr = tag;
		}
		else
		{
			m_sTagIPAddr = "0.0.0.0";
		}
	}

	BuildFullTag();
}

void cRoutingTag::SetTagName(const char *pTagName)
{
	m_sTagName = pTagName;
	BuildFullTag();
}

void cRoutingTag::SetTagIPAddr(const char *pIPAddr)
{
	m_sTagIPAddr = pIPAddr;
	BuildFullTag();
}

void cRoutingTag::SetTagPort(const char *pPort)
{
	m_sTagPort = pPort;
	BuildFullTag();
}

void cRoutingTag::BuildFullTag()
{
	stringstream sstag (stringstream::in | stringstream::out);
	sstag << m_sTagName << "@" << m_sTagIPAddr << ":" << m_sTagPort;
	m_sFullTag = sstag.str();
}

