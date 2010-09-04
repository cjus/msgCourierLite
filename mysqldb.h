/* mysqldb.h
   Copyright (C) 2002 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

mysqldb.h is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

mysqldb.h was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with main.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _MYSQLDB_H
#define _MYSQLDB_H

#include <stdio.h>

#define _USEDB 1

#ifdef _PLATFORM_WIN32
  #ifdef _USEDB
          #include "mysql.h"
  #endif //_USEDB
#else
	#include </usr/include/mysql/mysql.h>
#endif //_PLATFORM_WIN32

#include "idatabase.h"

class cMySQLdb : public cIDatabase
{
public:
	cMySQLdb();
	~cMySQLdb();

	bool IsOpen() { return m_bOpen; }

	//int Connect(const char *pHostMachine, const char *pDatabase, const char *pUserName, const char *pPassword);
	int Connect(const char* szConnect);
	int Disconnect();

	const char *GetDBStats();
	const char *GetErrorDesc();
	int Query(const char *pQuery);
	int QueryFree();
	int GetRow();
	const char *GetFirstField();
	const char *GetNextField();

	int GetFieldCount();

	const char *EscapeSingleQuotes(cBuffer &Text);
	//int CreateEscapedString(char *pTo, const char *pFrom, int iFromSize);

private:
	int m_iTotalFields;
	int m_iCol;
	bool m_bOpen;

#ifdef _USEDB
	MYSQL m_mysql;
	MYSQL_RES *m_pResult;
	MYSQL_ROW m_row;
#endif //_USEDB
};

#endif //_MYSQLDB_H
