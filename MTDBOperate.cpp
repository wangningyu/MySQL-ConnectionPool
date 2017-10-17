/************************************************************************/
/* �ļ����ƣ�MTDBOperate.cpp
/* ���ܽ��ܣ�����˲���MySQL�ӿں��� ���� ʵ��
/* ��ǰ�汾��1.0
/* �޸ļ�¼��
/* 2017-06-02	ȡʵʱ�������
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

CDBConnectionPool 	*g_pDBConnectionPool = NULL;			// SQL�̳߳�
volatile char		m_szSQLServer[MAX_PATH] = "127.0.0.1";	// SQL������
volatile char		m_szSQLUser[MAX_PATH] = "root";			// SQL�û���
volatile char		m_szSQLPass[MAX_PATH] = "123456";		// SQL����
volatile char		m_szSQLDB[MAX_PATH] = "test";			// SQL���ݿ�
volatile DWORD		m_nSQLPort = 3306;						// SQL�˿�

// ��ʼ��MySQL�̳߳�
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

// �ͷ�MySQL�̳߳�
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

// ���MySQL����
BOOL MTInitDBPing()
{
	if(g_pDBConnectionPool)
		return g_pDBConnectionPool->ExecUTF8();

	return FALSE;
}

// ʹ�÷���
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
	// ���绯MySQL�̳߳�����
	bRet = MTInitDBConnect(4);
	if(!bRet)
	{
		printf("MTInitDBConnect failed.\n");
		goto STEP_END;
	}
	
	// ��ȡһ����������
	pConnect = pDBConnectionPool->GetConnection();
	if(pConnect == NULL)
	{
		printf("GetConnection failed, all thread is busy.\n");
		goto STEP_END;
	}
	
	//////////////////////////////////////////////////////////////////////////
	// ִ��һ����ѯ���Select
	memset(szSQL,0x00,MYSQL_STRING_LEN);
	wsprintf(szSQL,_T("SELECT * FROM `%s` where `%s` = \"%s\" AND `%s` = \"%s\""),DB_TABLE_USER,DB_USER_NAME,szUser,,DB_USER_PASS,szPass);

	pRecordset = pConnection->Select(szSQL); 
	if( pRecordset == NULL ) 
	{
		printf("Select failed: %s\n",pConnection->GetErrorMsg().c_str());
		goto STEP_END;
	}
	
	// ���û���κμ�¼
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