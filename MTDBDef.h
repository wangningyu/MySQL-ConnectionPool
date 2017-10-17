/************************************************************************/
/* 文件名称：MTDBDef.h
/* 功能介绍：封装MySQL C API的操作类，结构定义，常量定义
/* 当前版本：1.0
/* 修改记录：无
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

//连接类，在连接上执行查询插入更新操作
//连接池，一个，每次从连接池申请连接
//结果集类，保持SELECT的查询结果

const WORD SOC_FIELD_NAME_LEN = 32;			//数据库表字段名称最大长度

const BYTE SOC_FIELD_TYPE_INT		= 1;	//字段类型，整形
const BYTE SOC_FIELD_TYPE_LONGLONG	= 2;	//字段类型，LONGLONG
const BYTE SOC_FIELD_TYPE_UINT		= 3;	//字段类型，整形
const BYTE SOC_FIELD_TYPE_ULONGLONG	= 4;	//字段类型，LONGLONG
const BYTE SOC_FIELD_TYPE_FLOAT		= 5;	//字段类型，浮点
const BYTE SOC_FIELD_TYPE_STRING	= 6;	//字段类型，字符串

typedef struct tagSQLFIELDNAME
{
	int	 FieldLength;
	WORD FieldType;
	char FieldName[SOC_FIELD_NAME_LEN];		//字段名称

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
	SQLFIELDNAME	*m_pFieldName;	//字段名称
	FieldValue		*m_pFieldValue;	//字段值

	DWORD			m_wFieldCount;	//结果集字段数
	DWORD			m_wRowCount;	//结果集记录数	

public:	
	//构造函数和析构函数        
	CDBRecordset(DWORD wRowCount, DWORD wFieldCount);	
	~CDBRecordset();
	void	FreeRecord();

	DWORD GetFieldCount();		//获得记录集的字段数
	DWORD GetRowCount();			//获得记录及的记录数/行数
	SQLFIELDNAME * GetFieldName(DWORD wFieldIndex);	//根据字段名索引获得字段名称和类型
	SQLFIELDNAME * SetFieldName(DWORD wFieldIndex, int nMySQLType,
								string sFieldName,int nFlags,
								int	   nLength);//设置字段类型和值

	FieldValue * GetFieldValue(DWORD wRowIndex, DWORD wFieldIndex);	//根据记录/行索引和字段索引获得字段值
	FieldValue * GetFieldValue(DWORD wRowIndex, string sFieldName);	//根据记录/行索引和字段名称获得字段值
	FieldValue * SetFieldValue(DWORD wRowIndex, DWORD wFieldIndex, DWORD wFieldType, string sFieldValue);	//字段记录/行数据
};


class CDBConnection
{
private:	
	volatile bool	m_bBusy;		//连接是否在使用的标记;

	string	m_sHost;
	string	m_sUser;
	string	m_sPasswd;
	string	m_sDbname;
	string	m_sCharset;
	int		m_nPort;

	char	m_szErrorMsg[MAX_PATH];
	DWORD	m_dwTickCount1;	//执行命令开始前计数
	DWORD	m_dwTickCount2; //执行命令结束时计数
	
	MYSQL	m_mysqlInstance;	//MYSQL实例变量 
	string	m_sErrorMsg;		//上条命令执行错误消息描述
	DWORD	m_dwExecUTF8;		//上次执行UTF8命令时间
	DWORD	m_dwConnectTime;	// 上次连接时间

public: 
	//构造函数和稀构函数        
	CDBConnection(); 
	~CDBConnection();
 
	BOOL IsConnected();	//是否连接成功
	BOOL Ping();
	BOOL ExecUTF8();
	BOOL IsLoop();
	BOOL StartReConnect();

	MYSQL& GetInstance();
	string GetErrorMsg();

	void SetIdle();		//设置连接为空闲
	void SetBusy();		//设置连接为忙
	bool IsIdle();		//查询连接是否空闲
	DWORD GetComsumedTime();	//上个命令执行的耗时时间
       
	void SetParams(char * host,int port ,char * db,char * user,char* passwd,char * charset);//设置连接参数
	int Connect();	//初始化数据库,连接数据库,设置字符集
	void CloseConnection(); //关闭数据库连接,和Connect匹配使用

	CDBRecordset * Select(char * sql);	//执行SELECT查询，CDBRecordset指针需要调用代码主动销毁
	int Execute(char * sql, ULONGLONG& wAffectedRows);	//执行UPDATE，INSERT，DELTE命令，0表示成功；1表示失败
	int Insert(char * sql, ULONGLONG& dwAutoid);	//执行UPDATE，并返回自动生成的ID
};

//数据库连接池
class CDBConnectionPool
{
private:
	WORD			m_wMaxPoolNum;		//最大连接池数量，建议不超过256
	CDBConnection	*m_pDBConnection;	//数据库连接对象数组

public:
	CDBConnectionPool();
	CDBConnectionPool(WORD wMaxPoolNum);
	~CDBConnectionPool();
	
	bool SetMaxPoolNum(WORD wMaxPoolNum);	//设置连接池最大连接数
	WORD CreateConnectionPool();			//创建连接池，同时创建连接
	WORD CloseConnectionPool();				//销毁连接池，同时断开和销毁所有连接
	BOOL Ping();							//判断是否所有线程池都空闲
	BOOL ExecUTF8();

	CDBConnection * GetConnection();	//获取连接，每一次执行Select和Execute之前都需要获得连接
};

#endif	// end of define __MT__DB__DEF__H__
