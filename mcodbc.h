#ifndef _MCODBC_H
#define _MCODBC_H

#include <windows.h>
#include <SQL.H>
#include <SQLEXT.H>
#include "buffer.h"

#include "idatabase.h"

typedef void (*MSGFUNC)(long, char*, char*);
MSGFUNC SetMsgFunc (MSGFUNC NewFunc);

class cMCODBC : public cIDatabase
{
public:
	cMCODBC();  
	virtual ~cMCODBC();  

	bool IsOpen() { return m_bOpen; }

	int Connect(const char* szConnect);
	int Disconnect(void);

	int Query(const char* szQuery = NULL);
	int QueryFree();

	int GetRow();
	//int GetNextResultSet(void);
	const char *GetFirstField();
	const char *GetNextField();

	const char *GetErrorDesc();
	const char *GetDBStats() { return 0; }

	int GetFieldCount();

	void SetLoginTimeout(long lLoginTimeout);
	void SetQueryTimeout(long lQueryTimeout);

	//char* EscapeSingleQuotes(const char* pszText, char* pszEscapedText);
	const char *EscapeSingleQuotes(cBuffer &Text);

protected:
	HENV  m_hEnv;
	HDBC  m_hDBC;
	HSTMT m_hQuery;
	cBuffer m_Cmd;
	cBuffer m_LastErrorMessage;
	cBuffer m_Connect;
	cBuffer m_FieldBuffer;
	bool  m_bOpen;
	long  m_lLoginTimeout;
	long  m_lQueryTimeout;
	int m_iTotalFields;
	int m_iCol;

	BOOL OK(RETCODE r, bool bIgnoreICS = false);
	const char *GetField(short iCol, cBuffer &buffer);
};

#endif  //_MCODBC_H
