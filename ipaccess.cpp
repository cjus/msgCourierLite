/* ipaccess.cpp
   Copyright (C) 2002 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

ipaccess.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

ipaccess.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with ipaccess.cpp; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file ipaccess.cpp
 @brief The cIPAccess class is used to validate IP addresses allowing the application to allow or block
 specific IP addresses.
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence (see source file header)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exception.h"

#include "ipaccess.h"
#include "log.h"
#include "cbstrtok.h"

using namespace std;

cIPAccess::cIPAccess()
{
}

cIPAccess::~cIPAccess()
{
	Flush();
}

void cIPAccess::Flush()
{	
	cIPAccessEntry *pRuleEntry;
	try
	{
		for (int i = 0; i < m_rulelist.size(); i++)
		{
			pRuleEntry = m_rulelist[i];
			delete pRuleEntry;
		}
		m_rulelist.erase(m_rulelist.begin(), m_rulelist.end());
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
}

int cIPAccess::Add(char *pIPAddr)
{
	int rc = HRC_IPACCESS_OK;
	char oneline[255];
	cIPAccessEntry *pEntry = 0;

	try
	{
		strcpy(oneline, pIPAddr);
		if (oneline[0] != '+' && oneline[0] != '-')
		{
			LOG("Warning, Invalid IPAccess rule %s. Rule must be prefixed with a + or - character", oneline, pIPAddr);
			return HRC_IPACCESS_IPPARSE_FAILED;
		}

		LOG("Adding IPRULE: %s", oneline);
		MC_NEW(pEntry, cIPAccessEntry());
		if (ParseIP(oneline, pEntry) == HRC_IPACCESS_OK)
		{
			m_rulelist.push_back(pEntry);
		}
		else
		{
			LOG("Warning, Invalid IPAccess rule %s. Rule must be prefixed with a + or - character", oneline, pIPAddr);
			return HRC_IPACCESS_IPPARSE_FAILED;
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return rc;
}

int cIPAccess::Validate(char *pIPAddr)
{
	cAutoThreadSync mutex(&m_mutex);

	try
	{
		cIPAccessEntry Entry;
		char buf[100];
		buf[0] = '+';
		strncpy(&buf[1], pIPAddr, 98);

		int rc = ParseIP(buf,&Entry);
		if (rc != HRC_IPACCESS_OK)
			return HRC_IPACCESS_IPPARSE_FAILED;

		cIPAccessEntry *pRuleEntry;
		for (int i = 0; i < m_rulelist.size(); i++)
		{
			pRuleEntry = (cIPAccessEntry*)m_rulelist[i];
			if (pRuleEntry)
			{
				if (pRuleEntry->octant[0] != -1)
					if (pRuleEntry->octant[0] != Entry.octant[0])
						continue;
				if (pRuleEntry->octant[1] != -1)
					if (pRuleEntry->octant[1] != Entry.octant[1])
						continue;
				if (pRuleEntry->octant[2] != -1)
					if (pRuleEntry->octant[2] != Entry.octant[2])
						continue;
				if (pRuleEntry->octant[3] != -1)
					if (pRuleEntry->octant[3] != Entry.octant[3])
						continue;
				if (pRuleEntry->type == cIPAccessEntry::INCLUSION)
					return HRC_IPACCESS_OK;
				else
					return HRC_IPACCESS_DENY;
			}
		}
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return HRC_IPACCESS_DENY;
}

int cIPAccess::ParseIP(char *pLine, cIPAccessEntry *pEntry)
{
	 char IP[18];
STRTOKVA;
	 
	 pEntry->type = (pLine[0] == '+') ? cIPAccessEntry::INCLUSION : cIPAccessEntry::EXCLUSION;

	 strncpy(IP, &pLine[1], 16);
	
	 char *pSeps = ".";
	 char *pToken;
	 int octval;

	 pToken = STRTOK(IP, pSeps);
	 octval = (strcmp(pToken, "*")==0) ? -1 : atoi(pToken);
	 if (octval > 255)
		 return HRC_IPACCESS_IPPARSE_FAILED;
	 pEntry->octant[0] = octval;

	 pToken = STRTOK(NULL, pSeps);
	 octval = (strcmp(pToken, "*")==0) ? -1 : atoi(pToken);
	 if (octval > 255)
		 return HRC_IPACCESS_IPPARSE_FAILED;
	 pEntry->octant[1] = octval;

	 pToken = STRTOK(NULL, pSeps);
	 octval = (strcmp(pToken, "*")==0) ? -1 : atoi(pToken);
	 if (octval > 255)
		 return HRC_IPACCESS_IPPARSE_FAILED;
	 pEntry->octant[2] = octval;

	 pToken = STRTOK(NULL, pSeps);
	 octval = (strcmp(pToken, "*")==0) ? -1 : atoi(pToken);
	 if (octval > 255)
		 return HRC_IPACCESS_IPPARSE_FAILED;
	 pEntry->octant[3] = octval;
	 
	 return HRC_IPACCESS_OK;
}

