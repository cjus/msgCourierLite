/* ipaccess.h
   Copyright (C) 2002 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

ipaccess.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

ipaccess.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with ipaccess.h; if not, write to the Free Software 
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/**
 @file ipaccess.h
 @brief The cIPAccess class is used to validate IP addresses allowing the application to allow or block
 specific IP addresses.
 @author Carlos Justiniano
 @attention Copyright (C) 2002 Carlos Justiniano, GNU GPL Licence (see source file header)
*/
#ifndef _IPACCESS_H
#define _IPACCESS_H

#include <string>
#include <vector>

#include "threadsync.h"

#define HRC_IPACCESS_OK						0x0000
#define HRC_IPACCESS_EMPTY					0x0001
#define HRC_IPACCESS_MEMALLOC_FAILURE		0x0002
#define HRC_IPACCESS_IPPARSE_FAILED			0x0003
#define HRC_IPACCESS_DENY                   0x0004

class cIPAccessEntry
{
 public:
	 int type;
	 int octant[4];
	 enum
	 {
		  INCLUSION, EXCLUSION
	 };
};

class cIPAccess
{
public:
	cIPAccess();
	~cIPAccess();

	void Flush();
	int Add(char *pIP);
	int Validate(char *pIP);
private:
	cThreadSync m_mutex;
	std::vector<cIPAccessEntry*> m_rulelist;
	int ParseIP(char *pLine, cIPAccessEntry *pEntry);
};

#endif // _IPACCESS_H
