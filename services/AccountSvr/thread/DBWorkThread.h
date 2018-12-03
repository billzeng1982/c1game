#pragma once

#include "define.h"
#include "ThreadQueue.h"
#include "mysql/MysqlHandler.h"
#include "ss_proto.h"
#include "MyTdrBuf.h"
#include "../logic/AccountTable.h"
#include "../cfg/AccountSvrCfgDesc.h"
#include "ThreadFrame.h"
#include "../framework/AccountSvrMsgLayer_r.h"

class CDBWorkThread : public CThreadFrame
{
public:
	CDBWorkThread();
	~CDBWorkThread(){}

	AccountTable& 	GetAccountTable() { return m_oAccountTable; }
	void SendPkg( PKGMETA::SSPKG& rstSsPkg );

	PKGMETA::SSPKG& GetRspSsPkg() { return m_stRspPkg; }

protected:
	virtual bool _ThreadAppInit(void* pvAppData);
	virtual void _ThreadAppFini();
	virtual int _ThreadAppProc();
	
	int _HandleAccountSvrMsg();
	int _CheckMysqlConn();
	
protected:
	CMysqlHandler 	m_oMysqlHandler;
	ACCOUNTSVRCFG*	m_pstConfig;
	AccountTable	m_oAccountTable;

	MyTdrBuf		m_stSendBuf;
	PKGMETA::SSPKG 	m_stSsRecvPkg;
	PKGMETA::SSPKG	m_stRspPkg;

	AccountSvrMsgLayer_r m_oMsgLayer;
};

