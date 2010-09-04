/* routingtag.h
   Copyright (C) 2006 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

routingtag.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

routingtag.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with mimemap.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file routingtag.h
 @brief Routing Tag
 @author Carlos Justiniano
 @attention Copyright (C) 2006 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#ifndef ROUTING_TAG_H
#define ROUTING_TAG_H

#include <string>
#include <sstream>

/**
 @class cRoutingTag
 @brief Peer Routing Tag
 */
class cRoutingTag
{
public:
	cRoutingTag();
	~cRoutingTag();

	const char* GetTag();
	const char* GetTagName();
	const char* GetTagIPAddr();
	const char* GetTagPort();

	void SetTag(const char *pTag);
	void SetTagName(const char *pTagName);
	void SetTagIPAddr(const char *pIPAddr);
	void SetTagPort(const char *pPort);

private:
	std::string m_sFullTag;
	std::string m_sTagName;
	std::string m_sTagIPAddr;
	std::string m_sTagPort;
	
	void BuildFullTag();
};

#endif // ROUTING_TAG_H
