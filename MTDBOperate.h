/************************************************************************/
/* �ļ����ƣ�MTDBOperate.h
/* ���ܽ��ܣ�����˲���MySQL�ӿں��� ���� ����
/* ��ǰ�汾��1.0
/* �޸ļ�¼����
/************************************************************************/
#ifndef __MT__DB__OPERATE__H__
#define __MT__DB__OPERATE__H__

#include "stdafx.h"
#include "MTDBDef.h"

extern volatile char		m_szSQLServer[MAX_PATH];	// SQL������
extern volatile char		m_szSQLUser[MAX_PATH];		// SQL�û���
extern volatile char		m_szSQLPass[MAX_PATH];		// SQL����
extern volatile char		m_szSQLDB[MAX_PATH];		// SQL���ݿ�
extern volatile DWORD		m_nSQLPort;					// SQL�˿�

BOOL MTInitDBConnect(WORD nThreadNum);
void MTDBFreeSQLPool();
BOOL MTInitDBPing();

#endif	// end of define __MT__DB__OPERATE__H__