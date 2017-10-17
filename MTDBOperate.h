/************************************************************************/
/* 文件名称：MTDBOperate.h
/* 功能介绍：服务端操作MySQL接口函数 —— 声明
/* 当前版本：1.0
/* 修改记录：无
/************************************************************************/
#ifndef __MT__DB__OPERATE__H__
#define __MT__DB__OPERATE__H__

#include "stdafx.h"
#include "MTDBDef.h"

extern volatile char		m_szSQLServer[MAX_PATH];	// SQL服务器
extern volatile char		m_szSQLUser[MAX_PATH];		// SQL用户名
extern volatile char		m_szSQLPass[MAX_PATH];		// SQL密码
extern volatile char		m_szSQLDB[MAX_PATH];		// SQL数据库
extern volatile DWORD		m_nSQLPort;					// SQL端口

BOOL MTInitDBConnect(WORD nThreadNum);
void MTDBFreeSQLPool();
BOOL MTInitDBPing();

#endif	// end of define __MT__DB__OPERATE__H__