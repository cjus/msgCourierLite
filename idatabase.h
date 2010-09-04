/* idatabase.h
   Copyright (C) 2007 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

idatabase.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

idatabase.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with main.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _IDATABASE_H
#define _IDATABASE_H

#define HRC_DB_SUCCESS				0
#define HRC_DB_CONNECTION_FAILURE	0x0001
#define HRC_DB_QUERY_FAILURE		0x0002
#define HRC_DB_ROWFETCH_FAILURE		0x0003

class cBuffer;

class cIDatabase
{
public:
	virtual bool IsOpen()= 0;

	virtual int Connect(const char* szConnect) = 0;
	virtual int Disconnect() = 0;

	virtual int Query(const char *pQuery) = 0;
	virtual int QueryFree() = 0;

	virtual int GetRow() = 0;

	virtual const char *GetFirstField() = 0;
	virtual const char *GetNextField() = 0;

	virtual const char *GetErrorDesc() = 0;
	virtual const char *GetDBStats() = 0;
	virtual int GetFieldCount() = 0;

	virtual const char *EscapeSingleQuotes(cBuffer &Text) = 0;
};

#endif //_IDATABASE_H
