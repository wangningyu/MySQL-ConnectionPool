/************************************************************************/
/* 文件名称：MTDBConnection.cpp
/* 功能介绍：封装MySQL C API的操作类 —— 实现
/* 当前版本：1.0
/* 修改记录：无
/************************************************************************/
#include "stdafx.h" 
#include "MTDBDef.h"

extern volatile char		m_szSQLServer[MAX_PATH];	// SQL服务器
extern volatile char		m_szSQLUser[MAX_PATH];		// SQL用户名
extern volatile char		m_szSQLPass[MAX_PATH];		// SQL密码
extern volatile char		m_szSQLDB[MAX_PATH];		// SQL数据库
extern volatile DWORD		m_nSQLPort;					// SQL端口

void AddSQLLog(const char *pszTemp)
{
	// add you log file
}

CDBConnection::CDBConnection() 
{
	m_dwTickCount1 = 0;
	m_dwTickCount2 = 0;
	m_dwExecUTF8   = 0;
	m_bBusy = false;
	m_dwTickCount1 = 0;
	m_nPort = 0;
	m_dwConnectTime = 0;
	memset(m_szErrorMsg,0x00,MAX_PATH);
}
 
CDBConnection::~CDBConnection()
{
}
 
void CDBConnection::SetParams(char * host,int port ,char * db,char * user,char* passwd,char * charset)
{
	m_sHost = host;
	m_sUser = user;;
	m_nPort = port;; 
	m_sPasswd = passwd;; 
	m_sDbname = db;;  
	m_sCharset = charset; 

}
// 初始化数据库连接 
int CDBConnection::Connect()
{
	if( mysql_init(&m_mysqlInstance) == NULL )
	{ 
		m_sErrorMsg = "inital mysql handle error"; 
		AddSQLLog((char *)m_sErrorMsg.c_str());
		const char *pMsg = mysql_error(&m_mysqlInstance);
		AddSQLLog(pMsg);
		AddSQLLog("");
		return FALSE; 
	} 

	//m_mysqlInstance.reconnect = true;
	if (!mysql_real_connect(&m_mysqlInstance,
			m_sHost.c_str(),m_sUser.c_str(),m_sPasswd.c_str(),m_sDbname.c_str(),m_nPort,NULL,0)) 
	{ 
		m_sErrorMsg = "failed to connect to database";
		AddSQLLog((char *)m_sErrorMsg.c_str());
		const char *pMsg = mysql_error(&m_mysqlInstance);
		memset(m_szErrorMsg,0x00,MAX_PATH);
		memcpy(m_szErrorMsg,pMsg,strlen(pMsg));
		AddSQLLog(pMsg);
		AddSQLLog("");
		return FALSE; 
	}     
 
	char value = 1;
	mysql_options(&m_mysqlInstance, MYSQL_OPT_RECONNECT, (char *)&value);

	if(mysql_set_character_set(&m_mysqlInstance,"utf8") != 0)
	{
		m_sErrorMsg = "mysql_set_character_set error";
		AddSQLLog((char *)m_sErrorMsg.c_str());
		const char *pMsg = mysql_error(&m_mysqlInstance);
		memset(m_szErrorMsg,0x00,MAX_PATH);
		memcpy(m_szErrorMsg,pMsg,strlen(pMsg));
		AddSQLLog(pMsg);
		AddSQLLog("");
		return FALSE;
	}

	m_dwConnectTime = GetTickCount();
	m_bBusy = false;
	return TRUE; 
}
 
// 查询数据 
CDBRecordset * CDBConnection::Select(char * sql)
{
	CDBRecordset * pRecordset = NULL;
	try
	{
		SetBusy();
		Ping();
		if(mysql_query(&m_mysqlInstance,sql) != 0) 
		{ 
			m_sErrorMsg = "Select execute mysql_query() error."; 
			AddSQLLog((char *)m_sErrorMsg.c_str());
			AddSQLLog(sql);
			const char *pMsg = mysql_error(&m_mysqlInstance);
			AddSQLLog(pMsg);
			AddSQLLog("");
			SetIdle();
			//StartReConnect();
			return NULL; 
		} 

		MYSQL_RES * pMYSQLRES = mysql_store_result(&m_mysqlInstance); 
		if(pMYSQLRES == NULL) 
		{ 
			m_sErrorMsg = "execute mysql_store_result() error."; 
			AddSQLLog((char *)m_sErrorMsg.c_str());
			AddSQLLog(sql);
			const char *pMsg = mysql_error(&m_mysqlInstance);
			AddSQLLog(pMsg);
			AddSQLLog("");
			SetIdle();
			//StartReConnect();
			return NULL; 
		} 
		
		DWORD		wFieldNum = 0;
		DWORD		wRowNum = 0;	
		MYSQL_FIELD *pMYSQLFIELD = NULL;
		DWORD		wFieldIndex = 0;

		wFieldNum	= mysql_num_fields(pMYSQLRES);
		wRowNum		= (DWORD)mysql_num_rows(pMYSQLRES);
		pRecordset	= new CDBRecordset(wRowNum, wFieldNum);
		while(pMYSQLFIELD = mysql_fetch_field(pMYSQLRES))
		{
			TRACE("Index:%d  type:%d  name:%s  flags:%d  length:%d\n",wFieldIndex+1,	\
				pMYSQLFIELD->type, pMYSQLFIELD->name,pMYSQLFIELD->flags,pMYSQLFIELD->length);
			pRecordset->SetFieldName(wFieldIndex++, pMYSQLFIELD->type, pMYSQLFIELD->name,pMYSQLFIELD->flags,pMYSQLFIELD->length);
		}

		MYSQL_ROW	rowCurrent;
		DWORD		wRowIndex = 0;
		while(rowCurrent = mysql_fetch_row(pMYSQLRES)) 
		{ 
			for(DWORD i = 0;i < wFieldNum; i++)
			{
				int		fieldtype = pRecordset->GetFieldName(i)->FieldType;
				CString	rowCur    = rowCurrent[i];
				if(!rowCur.IsEmpty())
					pRecordset->SetFieldValue(wRowIndex, i, fieldtype, rowCurrent[i]);
			}

			wRowIndex++;
		} 
		 
		mysql_free_result(pMYSQLRES);
		SetIdle();
	}
	catch(...)
	{
		const char *pMsg = mysql_error(&m_mysqlInstance);
		AddSQLLog(pMsg);
		AddSQLLog("");
		//StartReConnect();
		SetIdle();
	}

	return pRecordset;
}
 
// 插入数据
int CDBConnection::Execute(char * sql, ULONGLONG& wAffectedRows)
{
	DWORD id = 0;
	SetBusy();

	Ping();
	if(mysql_query(&m_mysqlInstance,sql) != 0) 
	{ 
		const char *pMsg = mysql_error(&m_mysqlInstance);
		AddSQLLog(pMsg);

		m_sErrorMsg = "Execute execute mysql_query() error"; 
		AddSQLLog((char *)m_sErrorMsg.c_str());
		AddSQLLog(sql);
		AddSQLLog("");
		//StartReConnect();
		SetIdle();
		return FALSE; 
	}

	wAffectedRows = mysql_affected_rows(&m_mysqlInstance);
	id = mysql_insert_id(&m_mysqlInstance);
	SetIdle();
	return TRUE; 
}
 
int CDBConnection::Insert(char * sql, ULONGLONG& dwAutoid)
{
	SetBusy();
	Ping();
	if(mysql_query(&m_mysqlInstance,sql) != 0) 
	{ 
		m_sErrorMsg = "Insert execute mysql_query() error"; 
		AddSQLLog((char *)m_sErrorMsg.c_str());
		AddSQLLog(sql);

		const char *pMsg = mysql_error(&m_mysqlInstance);
		AddSQLLog(pMsg);
		AddSQLLog("");
		//StartReConnect();
		SetIdle();
		return FALSE; 
	} 

	dwAutoid = mysql_insert_id(&m_mysqlInstance);
	SetIdle();

	return TRUE; 
}

//关闭数据库连接 
void CDBConnection::CloseConnection()
{
	mysql_close(&m_mysqlInstance);
}
 
void CDBConnection::SetIdle()
{
	m_dwTickCount2 = ::GetTickCount();
	m_bBusy = false;
}

void CDBConnection::SetBusy()
{
	m_dwTickCount1 = ::GetTickCount();
	m_bBusy = true;
}
bool CDBConnection::IsIdle()
{
	return !m_bBusy;
}

DWORD CDBConnection::GetComsumedTime()
{
	return m_dwTickCount2-m_dwTickCount1;
}

MYSQL& CDBConnection::GetInstance()
{	
	return m_mysqlInstance;
}

string CDBConnection::GetErrorMsg()
{
	return m_sErrorMsg;
}


BOOL CDBConnection::ExecUTF8()
{
	ULONGLONG	dwId = 0;
	BOOL		bRet = FALSE;
	
	Ping();
	bRet = Execute(_T("set names 'utf8'"), dwId);
	if(bRet)
	{
		CloseConnection();
		bRet = StartReConnect();
	}

	return bRet;
}

BOOL CDBConnection::Ping()
{
	if(IsIdle())
		return mysql_ping(&m_mysqlInstance);

	return TRUE;
}

BOOL CDBConnection::IsConnected()
{
	return m_dwConnectTime != 0;
}

BOOL CDBConnection::IsLoop()
{
	if(m_dwExecUTF8 == 0)
	{
		m_dwExecUTF8 = GetTickCount();
		return TRUE;
	}

	DWORD	nTemp = GetTickCount();
	if( (nTemp - m_dwExecUTF8) <= 10000)
		return FALSE;

	m_dwExecUTF8 = nTemp;
	return TRUE;
}

BOOL CDBConnection::StartReConnect()
{
	if((GetTickCount() - m_dwConnectTime) >= 3 * 1000)
	{
		m_dwConnectTime = GetTickCount();
		CloseConnection();
		if(!Connect())
		{
			m_dwConnectTime = 0;
			return FALSE;
		}

		return TRUE;
	}

	return TRUE;
}