/************************************************************************/
/* 文件名称：MTDBOperate.cpp
/* 功能介绍：服务端操作MySQL接口函数 —— 实现
/* 当前版本：1.0
/* 修改记录：
/* 2017-06-02	取实时返点比例
/************************************************************************/
#include "MTDBOperate.h"
#include "../Common/MD5.h"

// Max MySQL String Length
#define MYSQL_STRING_LEN	8192

// MySQL Table
#define DB_TABLE_USER	"db_custom_user"
#define DB_USER_UID		"mt_uid"			// uint
#define DB_USER_NAME	"mt_username"		// string
#define DB_USER_PASS	"mt_password"		// string

CDBConnectionPool 	*g_pDBConnectionPool = NULL;			// SQL线程池
volatile char		m_szSQLServer[MAX_PATH] = "127.0.0.1";	// SQL服务器
volatile char		m_szSQLUser[MAX_PATH] = "root";			// SQL用户名
volatile char		m_szSQLPass[MAX_PATH] = "123456";		// SQL密码
volatile char		m_szSQLDB[MAX_PATH] = "test";			// SQL数据库
volatile DWORD		m_nSQLPort = 3306;						// SQL端口

// 初始化MySQL线程池
BOOL MTInitDBConnect(WORD nThreadNum)
{
	if(g_pDBConnectionPool != NULL)
	{
		g_pDBConnectionPool->CloseConnectionPool();
		delete g_pDBConnectionPool;
		g_pDBConnectionPool = NULL;
	}

	g_pDBConnectionPool = new CDBConnectionPool(nThreadNum);
	if(g_pDBConnectionPool == NULL)
		return FALSE;

	return g_pDBConnectionPool->CreateConnectionPool();
}

// 释放MySQL线程池
void MTDBFreeSQLPool()
{
	try
	{
		if(g_pDBConnectionPool != NULL)
		{
			g_pDBConnectionPool->CloseConnectionPool();
			delete g_pDBConnectionPool;
			g_pDBConnectionPool = NULL;
		}
	}
	catch (...){}
}

// 检查MySQL连接
BOOL MTInitDBPing()
{
	if(g_pDBConnectionPool)
		return g_pDBConnectionPool->ExecUTF8();

	return FALSE;
}

// 使用方法
int main()
{
	CDBConnection	*pConnect	= NULL;
	CDBRecordset 	*pRecordset = NULL;
	BOOL			bRet = FALSE;
	char 			szSQL[MYSQL_STRING_LEN] = {0x00};
	DWORD 			i = 0;
	char			szUser[] = "admin";
	char			szPass[] = "admin";
	
	//////////////////////////////////////////////////////////////////////////
	// 初如化MySQL线程池数量
	bRet = MTInitDBConnect(4);
	if(!bRet)
	{
		printf("MTInitDBConnect failed.\n");
		goto STEP_END;
	}
	
	// 获取一个空闲连接
	pConnect = pDBConnectionPool->GetConnection();
	if(pConnect == NULL)
	{
		printf("GetConnection failed, all thread is busy.\n");
		goto STEP_END;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// 执行一个查询语句Select
	memset(szSQL,0x00,MYSQL_STRING_LEN);
	wsprintf(szSQL,_T("SELECT * FROM `%s` where `%s` = \"%s\" AND `%s` = \"%s\""),DB_TABLE_USER,DB_USER_NAME,szUser,,DB_USER_PASS,szPass);

	pRecordset = pConnection->Select(szSQL); 
	if( pRecordset == NULL ) 
	{
		printf("Select failed: %s\n",pConnection->GetErrorMsg().c_str());
		goto STEP_END;
	}
	
	// 如果没有任何记录
	if(pRecordset->GetRowCount() == 0)
	{
		delete pRecordset;
		pRecordset = NULL;
		goto STEP_END;
	}
	
	char		*pszUser = NULL;
	char		*pszPass = NULL;
	
	for(i=0; i<pRecordset->GetRowCount(); i++)
	{
		nTemp = pRecordset->GetFieldValue(i,DB_USER_UID)->FieldValueUInt;
		pszUser = pRecordset->GetFieldValue(i,DB_USER_NAME)->FieldValueString;
		pszPass = pRecordset->GetFieldValue(i,DB_USER_PASS)->FieldValueString;
		printf("user index: %d  name:%s   pass:%s\n", nTemp, pszUser, pszPass);
	}
	//////////////////////////////////////////////////////////////////////////
	
	
STEP_END:
	if(pRecordset != NULL)
	{
		delete pRecordset;
		pRecordset = NULL;
	}
	
	MTDBFreeSQLPool();
	return 1;
}