#include "MCODBC.H"
#include "core.h"
#include <ctype.h>

static void DefaultMsgFunc(long, char* szMsg, char* szTitle)
{
	LOG("%s: %s", szTitle, szMsg);
}
static MSGFUNC pMsgFunc = DefaultMsgFunc;      

MSGFUNC SetMsgFunc(MSGFUNC NewFunc)
{
	MSGFUNC OldFunc = pMsgFunc;	
	pMsgFunc = NewFunc ? NewFunc : DefaultMsgFunc;
	return OldFunc;
}

cMCODBC::cMCODBC()
{
	m_bOpen	 = FALSE;
	m_hEnv	 = SQL_NULL_HENV;
	m_hDBC	 = SQL_NULL_HDBC;
	m_hQuery = SQL_NULL_HSTMT;
	m_lLoginTimeout = 0;
	m_lQueryTimeout = 0;
	m_iCol = 1;
}

cMCODBC::~cMCODBC()
{
	Disconnect();
}            


//-------------------------------------------------------------------------
//  high-level open connection to data source
int cMCODBC::Connect(const char* szConnect)
{	
	HWND hWnd = NULL;
 	char caConnect[255]; 
	caConnect[0] = '\0';  

	Disconnect();

	if (szConnect != NULL) 
		m_Connect.Sprintf(1024, "%s", szConnect); 

	if (!SQLAllocEnv(&m_hEnv) && OK(SQLAllocConnect(m_hEnv, &m_hDBC)))
	{
		if (m_lLoginTimeout > 0)
		{
			::SQLSetConnectOption(m_hDBC, SQL_LOGIN_TIMEOUT, m_lLoginTimeout);
		}

		SWORD cbConnect;
		if (OK(SQLDriverConnect(m_hDBC, hWnd, (UCHAR FAR*)szConnect, SQL_NTS, (UCHAR FAR*)caConnect, sizeof (caConnect), &cbConnect, SQL_DRIVER_NOPROMPT)))
		{ 
			m_bOpen = TRUE;
		}
	}

	if (!m_bOpen)
	{
		cBuffer ErrMsg;
		ErrMsg.Sprintf(1024, "cMCODBC::Open failed [szConnect:%s, caConnect:%s, hEnv:%d, hDbc:%d, Timeout: %d", szConnect, caConnect, m_hEnv, m_hDBC, m_lLoginTimeout);
		(*pMsgFunc)(0L, (LPSTR)(LPCTSTR)ErrMsg, "Error");
	}

	return (m_bOpen == true) ? HRC_DB_SUCCESS : HRC_DB_CONNECTION_FAILURE;
}

//-------------------------------------------------------------------------
// Close connection to data source
int cMCODBC::Disconnect()
{
	if (m_hQuery != SQL_NULL_HSTMT)
	{
		OK(SQLFreeStmt(m_hQuery, SQL_DROP));
		m_hQuery = SQL_NULL_HSTMT;   
	}
	if (m_hDBC != SQL_NULL_HDBC)
	{
		OK(SQLDisconnect(m_hDBC));
		OK(SQLFreeConnect(m_hDBC));
		m_hDBC = SQL_NULL_HDBC;
	}
	if (m_hEnv != SQL_NULL_HENV)
	{
		OK(SQLFreeEnv (m_hEnv));
		m_hEnv = SQL_NULL_HENV;
	}                
	m_bOpen = FALSE;
	m_Cmd.Reset();
	m_Connect.Reset();

	return HRC_DB_SUCCESS;
}

//-------------------------------------------------------------------------
// Send a query to the data source
int cMCODBC::Query(const char* szQuery)
{
	BOOL bRval = FALSE;	
	m_Cmd.ReplaceWith((char*)szQuery);	
	if (IsOpen())
	{   
		if (m_hQuery != SQL_NULL_HSTMT)
		{                          
			OK(SQLFreeStmt(m_hQuery, SQL_DROP));
			m_hQuery = SQL_NULL_HSTMT;
		}
		
		if (OK(SQLAllocStmt(m_hDBC, &m_hQuery)))
		{ 
			if (m_lQueryTimeout > 0)
			{
				SQLSetStmtOption(m_hQuery, SQL_QUERY_TIMEOUT, m_lQueryTimeout);
			}

			bRval = OK(SQLExecDirect(m_hQuery, (UCHAR FAR*)(LPCTSTR)m_Cmd, SQL_NTS));
			m_Cmd.Reset();
		}        
	}

	m_iTotalFields = GetFieldCount();
	return (bRval == TRUE) ? HRC_DB_SUCCESS : HRC_DB_QUERY_FAILURE;
}


int cMCODBC::QueryFree() 
{ 
	return HRC_DB_SUCCESS; 
}

//-------------------------------------------------------------------------
// Fetch a result row from the data source
int cMCODBC::GetRow()
{
	bool bIgnoreICS = false;
	return (OK(SQLFetch(m_hQuery), bIgnoreICS) == TRUE) ? HRC_DB_SUCCESS : HRC_DB_ROWFETCH_FAILURE;
}

//-------------------------------------------------------------------------
// Fetch a result set from the data source
//BOOL cMCODBC::GetNextResultSet(void)
//{
//	return OK(SQLMoreResults(m_hQuery));
//}

//-------------------------------------------------------------------------
// Test return code and print error message (if necessary)
BOOL cMCODBC::OK(RETCODE r, bool bIgnoreICS)
{
	BOOL	  bRval = FALSE, bSkipLog = false;
	cBuffer   Title;
	char	  szSQLState[5 + 1];
	char	  szErrorMsg[SQL_MAX_MESSAGE_LENGTH];
	SWORD	  cbErrorMsg;
	SDWORD    NativeError;
	
	switch (r) 
	{
	case SQL_SUCCESS:
		bRval = TRUE;
		break;
	case SQL_SUCCESS_WITH_INFO:
		bRval = TRUE;
		break;
	default:
		szSQLState[5] = 0x00;
		szErrorMsg[SQL_MAX_MESSAGE_LENGTH-1] = 0x00;
		szSQLState[0] = 0x00;
		szErrorMsg[0] = 0x00;

		//while (SQLError(m_hEnv, m_hDBC, m_hQuery, (UCHAR FAR*)szSQLState, &NativeError, (UCHAR FAR*)szErrorMsg, sizeof (szErrorMsg) - 1, &cbErrorMsg) != SQL_NO_DATA_FOUND) 
		if (SQLError(m_hEnv, m_hDBC, m_hQuery, (UCHAR FAR*)szSQLState, &NativeError, (UCHAR FAR*)szErrorMsg, sizeof (szErrorMsg) - 1, &cbErrorMsg) != SQL_NO_DATA_FOUND) 
		{
			//if (bIgnoreICS && r == SQL_ERROR && memcmp(szSQLState, "24000", 5) == 0) 
			//	continue; 

			Title.Sprintf(1024, "ODBC [%s]:[%s],[%s](%08lx)", (r == SQL_SUCCESS_WITH_INFO) ? "Warning" : "Error", szErrorMsg, szSQLState, NativeError);	
			switch (NativeError) 
			{       
				case 0x00001645:  // changed database connect message
				case 0x00001647:  // changed language message
					break;				
				default:  
					(*pMsgFunc) (0L, (LPSTR)(LPCTSTR)Title.c_str(), ((r == SQL_SUCCESS_WITH_INFO) ? "Warning" : "Error"));
					if (r != SQL_SUCCESS_WITH_INFO && !bSkipLog) 
					{
						cBuffer Err;
						if (!m_Connect.IsEmpty()) 
							Err.ReplaceWith(m_Connect);
						else 
							Err.ReplaceWith("Unknown Connection");
						if (!m_Cmd.IsEmpty()) 
						{
							Err.Append(" ");
							Err.Append(m_Cmd);
						}
						(*pMsgFunc)(0L, (LPSTR)(LPCTSTR)Err.c_str(), "Error"); 
						bSkipLog = true;
					}
					break;
			}  
		}
		break;
	}
	return bRval;
}

const char *cMCODBC::GetFirstField()
{
	m_iCol = 1;
	return GetField((short)m_iCol, m_FieldBuffer);
}

const char *cMCODBC::GetNextField()
{
	char *pRet;
	m_iCol++;
	if (m_iCol > m_iTotalFields)
	{
		pRet = 0;
	}
	else
	{
		pRet = (char*)GetField((short)m_iCol, m_FieldBuffer);
    }
	return pRet;
}

const char *cMCODBC::GetField(short iCol, cBuffer &buffer)
{
	SDWORD resultLen;
	buffer.SetBufferSize(16000);
	buffer.Reset();
	OK(SQLGetData(m_hQuery, iCol, SQL_C_CHAR, buffer.cstr(), 16000, &resultLen));
	buffer.SetBufferLength(resultLen);
	return buffer.c_str();
}

int cMCODBC::GetFieldCount()
{
	short NumColumns = 0;
	OK(SQLNumResultCols(m_hQuery, &NumColumns));
	return (int)NumColumns;
}

/*
int cMCODBC::CreateEscapedString(char *pTo, const char *pFrom, int iFromSize)
{
#ifdef _USEDB
	return mysql_escape_string(pTo, pFrom, iFromSize);
#else
    return 0;
#endif //_USEDB
}
*/
/*
char* cMCODBC::EscapeSingleQuotes(const char* pszText, char* pszEscapedText)
{
	size_t	stRelPosn;
	char *	pcCurPosnA = (char*)pszText;
	char *	pcCurPosnB = pszEscapedText;

	if ( pszText && *pszText )
	{
		do
		{
			stRelPosn = strcspn(pcCurPosnA, "\'" ) + 1;
			memcpy(pcCurPosnB, pcCurPosnA, stRelPosn);
			pcCurPosnA += stRelPosn;
			pcCurPosnB += stRelPosn;
			*pcCurPosnB++ = '\'';

		}  while( *(pcCurPosnA - 1) );
	}
	else
	{
		*pszEscapedText = 0;
	}

	return pszEscapedText;
}
*/

const char *cMCODBC::EscapeSingleQuotes(cBuffer &Text)
{
	Text.Replace("'", "''");
	return Text.c_str();
}

const char *cMCODBC::GetErrorDesc()
{
	return m_LastErrorMessage.c_str();;
}

void cMCODBC::SetLoginTimeout(long lLoginTimeout)
{
	if (lLoginTimeout > 0) 
		m_lLoginTimeout = lLoginTimeout;
}

void cMCODBC::SetQueryTimeout(long lQueryTimeout)
{
	if (lQueryTimeout > 0) 
		m_lQueryTimeout = lQueryTimeout;
}

