/************************************************************************/
/* 文件名称：MTDBConnectionPool.cpp
/* 功能介绍：数据库连接池
/* 当前版本：1.0
/* 修改记录：无
/************************************************************************/
#include "StdAfx.h"
#include "MTDBDef.h"

extern volatile char		m_szSQLServer[MAX_PATH];
extern volatile char		m_szSQLUser[MAX_PATH];
extern volatile char		m_szSQLPass[MAX_PATH];
extern volatile char		m_szSQLDB[MAX_PATH];
extern volatile DWORD		m_nSQLPort;

extern BOOL __stdcall db_InitUTF8(CDBConnection *pConnection);

CDBConnectionPool::CDBConnectionPool()
{
	m_pDBConnection = NULL;
	m_wMaxPoolNum   = 0;
}

CDBConnectionPool::CDBConnectionPool(WORD wMaxPoolNum)
{
	m_pDBConnection = NULL;
	m_wMaxPoolNum   = wMaxPoolNum;
}

CDBConnectionPool::~CDBConnectionPool()
{
	if(m_pDBConnection != NULL)
	{
		delete [] m_pDBConnection;
		m_pDBConnection = NULL;
	}
}
	
bool CDBConnectionPool::SetMaxPoolNum(WORD wMaxPoolNum)
{
	//动态设置只能比现有连接池数量大
	if(wMaxPoolNum < m_wMaxPoolNum)
		return false;

	m_wMaxPoolNum = wMaxPoolNum;
	return true;
}

WORD CDBConnectionPool::CreateConnectionPool()
{
	m_pDBConnection = new CDBConnection[m_wMaxPoolNum];
	if(m_pDBConnection == NULL)
	{
		return 0;
	}

	DWORD	nStart = GetTickCount();
	int		nRet = 0;
	char	*pszCharset = "utf-8";
	for(WORD wi=0; wi<m_wMaxPoolNum; wi++)
	{
		CDBConnection * pDBConnection = (CDBConnection *)&m_pDBConnection[wi];
		pDBConnection->SetParams((char *)m_szSQLServer, m_nSQLPort,		\
								 (char *)m_szSQLDB, (char *)m_szSQLUser, (char *)m_szSQLPass, (char *)pszCharset);
		nRet = pDBConnection->Connect();
		if(nRet == 0)
		{
			printf(_T("%s连接MySQL失败，错误原因: %s\n"),EXE_TITLE,pDBConnection->GetErrorMsg().c_str());
			return 0;
		}
	}

	printf(_T("%s连接MySQL成功，(耗时:%d ms)\n"),EXE_TITLE,GetTickCount() - nStart);
	db_InitUserAdmin();
	return m_wMaxPoolNum;
}

WORD CDBConnectionPool::CloseConnectionPool()
{
	for(WORD wi=0; wi<m_wMaxPoolNum; wi++)
	{
		CDBConnection * pDBConnection = (CDBConnection *)&m_pDBConnection[wi];
		if(pDBConnection != NULL)
		{
			char	szMsg[MAX_PATH] = {0x00};
			wsprintf(szMsg,"[%u] CloseConnectionPool free mysql pointer: %08x",wi,pDBConnection);
#ifdef DEBUG_MODE
			printf(szMsg);
			printf("\n");
#endif
			AddSQLLog(szMsg);
			AddSEHLog(szMsg);

			pDBConnection->CloseConnection();
			pDBConnection = NULL;
		}
	}

	m_wMaxPoolNum = 0;
	if(m_pDBConnection != NULL)
	{
		delete [] m_pDBConnection;
		m_pDBConnection = NULL;
	}

	return 0;
}

CDBConnection * CDBConnectionPool::GetConnection()
{
	for(WORD wi=0; wi<m_wMaxPoolNum; wi++)
	{
		CDBConnection * pDBConnection = (CDBConnection *)&m_pDBConnection[wi];
		if(pDBConnection && pDBConnection->IsIdle()) 
			return pDBConnection;
	}

	return NULL;
}

BOOL CDBConnectionPool::ExecUTF8()
{
	for(WORD wi=0; wi<m_wMaxPoolNum; wi++)
	{
		CDBConnection * pDBConnection = (CDBConnection *)&m_pDBConnection[wi];
		if(pDBConnection && pDBConnection->IsIdle()) 
		{
			db_InitUTF8(pDBConnection);
		}
	}
	
	return TRUE;
}

BOOL CDBConnectionPool::Ping()
{
	for(WORD wi=0; wi<m_wMaxPoolNum; wi++)
	{
		CDBConnection * pDBConnection = (CDBConnection *)&m_pDBConnection[wi];
		if(pDBConnection && pDBConnection->IsIdle()) 
		{
			if(!pDBConnection->Ping())
			{
				pDBConnection->CloseConnection();
				pDBConnection->StartReConnect();
			}
		}
	}
	
	return TRUE;
}