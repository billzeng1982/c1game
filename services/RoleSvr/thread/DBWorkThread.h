#pragma once

#include "define.h"
#include "ThreadQueue.h"
#include "mysql/MysqlHandler.h"
#include "ss_proto.h"
#include "MyTdrBuf.h"
#include "../logic/RoleTable.h"
#include "../cfg/RoleSvrCfgDesc.h"
#include "ThreadFrame.h"
#include "../framework/RoleSvrMsgLayer_r.h"

class CDBWorkThread : public CThreadFrame
{
public:
	CDBWorkThread();
	~CDBWorkThread(){}

	RoleTable& 	GetRoleTable() { return m_oRoleTable; }
	void SendPkg( PKGMETA::SSPKG& rstSsPkg );

protected:
	virtual bool _ThreadAppInit(void* pvAppData);
	virtual void _ThreadAppFini();
	virtual int _ThreadAppProc();
	
	int _HandleRoleSvrMsg();
	int _CheckMysqlConn();
	
protected:
	CMysqlHandler 	m_oMysqlHandler;
	ROLESVRCFG*		m_pstConfig;
	RoleTable		m_oRoleTable;

	MyTdrBuf		m_stSendBuf;
	PKGMETA::SSPKG 	m_stSsRecvPkg;

	RoleSvrMsgLayer_r m_oMsgLayer;
};

