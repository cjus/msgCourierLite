/* mysqldb.cpp
   Copyright (C) 2002 Carlos Justiniano
   cjus@chessbrain.net, cjus34@yahoo.com, cjus@users.sourceforge.net

mysqldb.cpp is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

mysqldb.cpp was developed by Carlos Justiniano for use on the
msgCourier project and the ChessBrain Project and is now distributed in
the hope that it will be useful, but WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public License
along with main.cpp; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mysqldb.h"

cMySQLdb::cMySQLdb()
: m_iTotalFields(0)
, m_iCol(0)
, m_bOpen(false)
{
}

cMySQLdb::~cMySQLdb()
{
	Disconnect();
}

int Connect(const char* szConnect)
//int cMySQLdb::Connect(const char *pHostMachine, const char *pDatabase, const char *pUserName, const char *pPassword)
{
#ifdef _USEDB
	mysql_init(&m_mysql);
	mysql_options(&m_mysql,MYSQL_READ_DEFAULT_GROUP,pDatabase);
	if (strlen(pUserName)==0)
		pUserName = 0;
	if (strlen(pPassword)==0)
		pPassword = 0;
	if (!mysql_real_connect(&m_mysql,pHostMachine,pUserName,pPassword,pDatabase,0,NULL,0))
	{
		return HRC_MYSQLDB_CONNECTION_FAILURE;
	}
	m_bOpen = true;
#endif //_USEDB
	return HRC_MYSQLDB_SUCCESS;
}

int cMySQLdb::Disconnect()
{
#ifdef _USEDB
	mysql_close(&m_mysql);
	m_bOpen = false;
#endif //_USEDB
	return HRC_MYSQLDB_SUCCESS;
}

const char *cMySQLdb::GetDBStats()
{
#ifdef _USEDB
	return (char*)mysql_stat(&m_mysql);
#else
	return "";
#endif //_USEDB
}

const char *cMySQLdb::GetErrorDesc()
{
#ifdef _USEDB
	return (char*)mysql_error(&m_mysql);
#else
	return "";
#endif //_USEDB
}

int cMySQLdb::Query(const char *pQuery)
{
#ifdef _USEDB
	int iRet = mysql_real_query(&m_mysql, pQuery, strlen(pQuery));
	if (iRet)
	{
		iRet = HRC_MYSQLDB_QUERY_FAILURE;
	}
	else
	{
		m_pResult = mysql_store_result(&m_mysql);
		iRet = HRC_MYSQLDB_SUCCESS;
	}
	return iRet;
#else
	return HRC_MYSQLDB_SUCCESS;
#endif //_USEDB
}

int cMySQLdb::QueryFree()
{
#ifdef _USEDB
	mysql_free_result(m_pResult);
#endif //_USEDB
	return HRC_MYSQLDB_SUCCESS;
}

int cMySQLdb::GetFieldCount()
{
#ifdef _USEDB
	return mysql_field_count(&m_mysql);
#else
    return 0;
#endif //_USEDB
}

int cMySQLdb::GetRow()
{
#ifdef _USEDB
	int iRet;
	if ((m_row = mysql_fetch_row(m_pResult)))
	{
		m_iTotalFields = mysql_num_fields(m_pResult);
		iRet = HRC_MYSQLDB_SUCCESS;
	}
	else
	{
		iRet = HRC_MYSQLDB_ROWFETCH_FAILURE;
	}
	return iRet;
#else
    return 0;
#endif //_USEDB
}

const char *cMySQLdb::GetFirstField()
{
#ifdef _USEDB
	m_iCol = 0;
	return m_row[m_iCol];
#else
    return 0;
#endif //_USEDB
}

const char *cMySQLdb::GetNextField()
{
#ifdef _USEDB
	char *pRet;
	m_iCol++;
	if (m_iCol > m_iTotalFields-1)
	{
		pRet = 0;
	}
	else
	{
		pRet = m_row[m_iCol];
    }
	return pRet;
#else
    return 0;
#endif //_USEDB
}

const char *cMySQLdb::EscapeSingleQuotes(cBuffer &Text)
{
	Text.Replace("'", "''");
	return Text.c_str();
}

/*
int cMySQLdb::CreateEscapedString(char *pTo, const char *pFrom, int iFromSize)
{
#ifdef _USEDB
	return mysql_escape_string(pTo, pFrom, iFromSize);
#else
    return 0;
#endif //_USEDB
}
*/

