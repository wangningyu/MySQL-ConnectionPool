/************************************************************************/
/* 文件名称：MTDBRecordset.cpp
/* 功能介绍：服务端操作MySQLCDBRecordset对象实现
/* 当前版本：1.0
/* 修改记录：无
/************************************************************************/
#include "MTDBDef.h"

#ifndef MYSQL_TYPE_BIT
#define MYSQL_TYPE_BIT	16
#endif

CDBRecordset::CDBRecordset(DWORD wRowCount, DWORD wFieldCount)
{
	m_wFieldCount = wFieldCount;
	m_wRowCount   = wRowCount;

	m_pFieldName = new SQLFIELDNAME[m_wFieldCount];
	m_pFieldValue = new FieldValue[m_wRowCount*m_wFieldCount];

	memset(m_pFieldName, 0, sizeof(SQLFIELDNAME)*m_wFieldCount);
	memset(m_pFieldValue, 0, sizeof(FieldValue)*m_wRowCount*m_wFieldCount);
}
	
CDBRecordset::~CDBRecordset()
{
	FreeRecord();
}

void CDBRecordset::FreeRecord()
{
	if(m_pFieldName != NULL)
	{
		delete[] m_pFieldName;
		m_pFieldName = NULL;
	}
	
	for(int i=0; i<m_wFieldCount*m_wRowCount; i++)
	{
		if(m_pFieldValue[i].FieldValueString != NULL)
			delete [] m_pFieldValue[i].FieldValueString;
	}
	
	if(m_pFieldValue != NULL)
	{
		delete [] m_pFieldValue;
		m_pFieldValue = NULL;
	}
}

DWORD CDBRecordset::GetRowCount()
{
	return m_wRowCount;
}

DWORD CDBRecordset::GetFieldCount()
{
	return m_wFieldCount;
}

SQLFIELDNAME * CDBRecordset::GetFieldName(DWORD wFieldIndex)
{
	return (SQLFIELDNAME *)&m_pFieldName[wFieldIndex];
}

FieldValue * CDBRecordset::GetFieldValue(DWORD wRowIndex, DWORD wFieldIndex)
{
	return (FieldValue *)&m_pFieldValue[wRowIndex*m_wFieldCount+wFieldIndex];
}

FieldValue * CDBRecordset::GetFieldValue(DWORD wRowIndex, string sFieldName)
{
	int nFieldIndex = -1;
	for(DWORD i=0; i<m_wFieldCount; i++)
	{
		SQLFIELDNAME * pName = (SQLFIELDNAME *)(&m_pFieldName[i]);
		if(_stricmp(pName->FieldName, sFieldName.c_str()) == 0)
		{
			nFieldIndex = i;
			break;
		}
	}
	
	if(nFieldIndex == -1) return NULL;

	return GetFieldValue(wRowIndex, nFieldIndex);
}

SQLFIELDNAME * CDBRecordset::SetFieldName(DWORD wFieldIndex, int nMySQLType, string sFieldName, int nFlags,int nFieldLength)
{
	//安全性越界检查
	if(wFieldIndex >= m_wFieldCount) return NULL;
	if(sFieldName.length() > SOC_FIELD_NAME_LEN-1) return NULL;

	SQLFIELDNAME * pFieldName = GetFieldName(wFieldIndex);
	strcpy(pFieldName->FieldName, sFieldName.c_str());
		
	//if (IS_NUM(field->type))    printf("Field is numeric\n");
	if( nMySQLType == MYSQL_TYPE_TINY	||	//TINYINT字段
		nMySQLType == MYSQL_TYPE_SHORT	||	// SMALLINT字段
		nMySQLType == MYSQL_TYPE_LONG	||	// INTEGER字段
		nMySQLType == MYSQL_TYPE_INT24	||  // MEDIUMINT字段
		nMySQLType == MYSQL_TYPE_BIT)		// BIT字段
	{
		if(nFlags & UNSIGNED_FLAG)
		{
			if(nFieldLength <= 4)
				pFieldName->FieldType = SOC_FIELD_TYPE_UINT;
			else
				pFieldName->FieldType = SOC_FIELD_TYPE_ULONGLONG;
		}
		else
		{
			if(nFieldLength <= 4)
				pFieldName->FieldType = SOC_FIELD_TYPE_INT;
			else
				pFieldName->FieldType = SOC_FIELD_TYPE_ULONGLONG;
		}
	}
	else if (nMySQLType == MYSQL_TYPE_LONGLONG)	// BIGINT字段
	{
		if(nFlags & UNSIGNED_FLAG)
			pFieldName->FieldType = SOC_FIELD_TYPE_ULONGLONG;
		else
			pFieldName->FieldType = SOC_FIELD_TYPE_LONGLONG;
	}
	else if (
		nMySQLType == MYSQL_TYPE_DECIMAL ||	// DECIMAL或NUMERIC字段
		//nMySQLType == MYSQL_TYPE_NEWDECIMA||// 精度数学DECIMAL或NUMERIC
		nMySQLType == MYSQL_TYPE_FLOAT ||	// FLOAT字段
		nMySQLType == MYSQL_TYPE_DOUBLE)	// DOUBLE或REAL字段
		pFieldName->FieldType = SOC_FIELD_TYPE_FLOAT;
	else if (
		nMySQLType == MYSQL_TYPE_STRING ||	// CHAR字段
		nMySQLType == MYSQL_TYPE_VAR_STRING)// VARCHAR字段
		pFieldName->FieldType = SOC_FIELD_TYPE_STRING;

//MYSQL_TYPE_TIMESTAMP// TIMESTAMP字段 
//MYSQL_TYPE_DATE// DATE字段 
//MYSQL_TYPE_TIME// TIME字段 
//MYSQL_TYPE_DATETIME// DATETIME字段 
//MYSQL_TYPE_YEAR// YEAR字段 
//MYSQL_TYPE_BLOB// BLOB或TEXT字段（使用max_length来确定最大长度） 
//MYSQL_TYPE_SET// SET字段 
//MYSQL_TYPE_ENUM// ENUM字段 
//MYSQL_TYPE_GEOMETRY// Spatial字段 
//MYSQL_TYPE_NULL// NULL-type字段
 
	return pFieldName;
}

FieldValue * CDBRecordset::SetFieldValue(DWORD wRowIndex, DWORD wFieldIndex, DWORD wFieldType, string sFieldValue)
{
	//安全性越界检查
	if(wFieldIndex >= m_wFieldCount) return NULL;
	if(wRowIndex >= m_wRowCount) return NULL;

	char		*pTemp = NULL;
	FieldValue	*pFieldValue = GetFieldValue(wRowIndex, wFieldIndex);
	pFieldValue->FieldType = wFieldType;
	pFieldValue->FieldValueString = new char[sFieldValue.length()+1];
	strcpy(pFieldValue->FieldValueString, sFieldValue.c_str());

	if(wFieldType == SOC_FIELD_TYPE_INT)
	{
		pFieldValue->FieldValueInt = atoi(sFieldValue.c_str());
		//sscanf(sFieldValue.c_str(),"%d",&pFieldValue->FieldValueInt);
	}
	else if (wFieldType == SOC_FIELD_TYPE_FLOAT)
	{
		pFieldValue->FieldValueFloat = (float)atof(sFieldValue.c_str());
	}
	else if(wFieldType == SOC_FIELD_TYPE_UINT)
	{
		//pFieldValue->FieldValueUInt = (UINT)strtoul(sFieldValue.c_str(), NULL, 0);;
		sscanf(sFieldValue.c_str(),"%u",&pFieldValue->FieldValueUInt);
	}
	else if(wFieldType == SOC_FIELD_TYPE_ULONGLONG)
	{
		sscanf(sFieldValue.c_str(),"%I64u",&pFieldValue->FieldValueUInt64);
	}
	else if(wFieldType == SOC_FIELD_TYPE_LONGLONG)
	{
		sscanf(sFieldValue.c_str(),"%I64d",&pFieldValue->FieldValueInt64);
	}

	return pFieldValue;
}

