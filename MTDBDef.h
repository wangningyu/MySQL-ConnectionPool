/************************************************************************/
/* �ļ����ƣ�MTDBDef.h
/* ���ܽ��ܣ���װMySQL C API�Ĳ����࣬�ṹ���壬��������
/* ��ǰ�汾��1.0
/* �޸ļ�¼����
/************************************************************************/
#ifndef __MT__DB__DEF__H__
#define __MT__DB__DEF__H__

#include <stdio.h> 
#include <string> 
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
#include "mswsock.h"

#include "mysql.h" 
#include "mysql_com.h"
using namespace std;

//�����࣬��������ִ�в�ѯ������²���
//���ӳأ�һ����ÿ�δ����ӳ���������
//������࣬����SELECT�Ĳ�ѯ���

const WORD SOC_FIELD_NAME_LEN = 32;			//���ݿ���ֶ�������󳤶�

const BYTE SOC_FIELD_TYPE_INT		= 1;	//�ֶ����ͣ�����
const BYTE SOC_FIELD_TYPE_LONGLONG	= 2;	//�ֶ����ͣ�LONGLONG
const BYTE SOC_FIELD_TYPE_UINT		= 3;	//�ֶ����ͣ�����
const BYTE SOC_FIELD_TYPE_ULONGLONG	= 4;	//�ֶ����ͣ�LONGLONG
const BYTE SOC_FIELD_TYPE_FLOAT		= 5;	//�ֶ����ͣ�����
const BYTE SOC_FIELD_TYPE_STRING	= 6;	//�ֶ����ͣ��ַ���

typedef struct tagSQLFIELDNAME
{
	int	 FieldLength;
	WORD FieldType;
	char FieldName[SOC_FIELD_NAME_LEN];		//�ֶ�����

	tagSQLFIELDNAME::tagSQLFIELDNAME()
	{
		FieldLength = 0;
		FieldType = 0;
		memset(FieldName,0x00,SOC_FIELD_NAME_LEN);
	}
}SQLFIELDNAME;

typedef struct tagFieldValue
{
	WORD	FieldType;
	int		FieldValueInt;
	UINT	FieldValueUInt;
	INT64	FieldValueInt64;
	UINT64	FieldValueUInt64;
	float	FieldValueFloat;
	char	*FieldValueString;

	tagFieldValue::tagFieldValue()
	{
		FieldType = 0;
		FieldValueInt = 0;
		FieldValueUInt = 0;
		FieldValueInt64 = 0;
		FieldValueUInt64 = 0;
		FieldValueFloat = 0.0f;
		FieldValueString = NULL;
	}
}FieldValue;

class CDBRecordset
{
private:
	SQLFIELDNAME	*m_pFieldName;	//�ֶ�����
	FieldValue		*m_pFieldValue;	//�ֶ�ֵ

	DWORD			m_wFieldCount;	//������ֶ���
	DWORD			m_wRowCount;	//�������¼��	

public:	
	//���캯������������        
	CDBRecordset(DWORD wRowCount, DWORD wFieldCount);	
	~CDBRecordset();
	void	FreeRecord();

	DWORD GetFieldCount();		//��ü�¼�����ֶ���
	DWORD GetRowCount();			//��ü�¼���ļ�¼��/����
	SQLFIELDNAME * GetFieldName(DWORD wFieldIndex);	//�����ֶ�����������ֶ����ƺ�����
	SQLFIELDNAME * SetFieldName(DWORD wFieldIndex, int nMySQLType,
								string sFieldName,int nFlags,
								int	   nLength);//�����ֶ����ͺ�ֵ

	FieldValue * GetFieldValue(DWORD wRowIndex, DWORD wFieldIndex);	//���ݼ�¼/���������ֶ���������ֶ�ֵ
	FieldValue * GetFieldValue(DWORD wRowIndex, string sFieldName);	//���ݼ�¼/���������ֶ����ƻ���ֶ�ֵ
	FieldValue * SetFieldValue(DWORD wRowIndex, DWORD wFieldIndex, DWORD wFieldType, string sFieldValue);	//�ֶμ�¼/������
};


class CDBConnection
{
private:	
	volatile bool	m_bBusy;		//�����Ƿ���ʹ�õı��;

	string	m_sHost;
	string	m_sUser;
	string	m_sPasswd;
	string	m_sDbname;
	string	m_sCharset;
	int		m_nPort;

	char	m_szErrorMsg[MAX_PATH];
	DWORD	m_dwTickCount1;	//ִ�����ʼǰ����
	DWORD	m_dwTickCount2; //ִ���������ʱ����
	
	MYSQL	m_mysqlInstance;	//MYSQLʵ������ 
	string	m_sErrorMsg;		//��������ִ�д�����Ϣ����
	DWORD	m_dwExecUTF8;		//�ϴ�ִ��UTF8����ʱ��
	DWORD	m_dwConnectTime;	// �ϴ�����ʱ��

public: 
	//���캯����ϡ������        
	CDBConnection(); 
	~CDBConnection();
 
	BOOL IsConnected();	//�Ƿ����ӳɹ�
	BOOL Ping();
	BOOL ExecUTF8();
	BOOL IsLoop();
	BOOL StartReConnect();

	MYSQL& GetInstance();
	string GetErrorMsg();

	void SetIdle();		//��������Ϊ����
	void SetBusy();		//��������Ϊæ
	bool IsIdle();		//��ѯ�����Ƿ����
	DWORD GetComsumedTime();	//�ϸ�����ִ�еĺ�ʱʱ��
       
	void SetParams(char * host,int port ,char * db,char * user,char* passwd,char * charset);//�������Ӳ���
	int Connect();	//��ʼ�����ݿ�,�������ݿ�,�����ַ���
	void CloseConnection(); //�ر����ݿ�����,��Connectƥ��ʹ��

	CDBRecordset * Select(char * sql);	//ִ��SELECT��ѯ��CDBRecordsetָ����Ҫ���ô�����������
	int Execute(char * sql, ULONGLONG& wAffectedRows);	//ִ��UPDATE��INSERT��DELTE���0��ʾ�ɹ���1��ʾʧ��
	int Insert(char * sql, ULONGLONG& dwAutoid);	//ִ��UPDATE���������Զ����ɵ�ID
};

//���ݿ����ӳ�
class CDBConnectionPool
{
private:
	WORD			m_wMaxPoolNum;		//������ӳ����������鲻����256
	CDBConnection	*m_pDBConnection;	//���ݿ����Ӷ�������

public:
	CDBConnectionPool();
	CDBConnectionPool(WORD wMaxPoolNum);
	~CDBConnectionPool();
	
	bool SetMaxPoolNum(WORD wMaxPoolNum);	//�������ӳ����������
	WORD CreateConnectionPool();			//�������ӳأ�ͬʱ��������
	WORD CloseConnectionPool();				//�������ӳأ�ͬʱ�Ͽ���������������
	BOOL Ping();							//�ж��Ƿ������̳߳ض�����
	BOOL ExecUTF8();

	CDBConnection * GetConnection();	//��ȡ���ӣ�ÿһ��ִ��Select��Execute֮ǰ����Ҫ�������
};

#endif	// end of define __MT__DB__DEF__H__
