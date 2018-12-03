#pragma once

#include "define.h"
#include "ThreadQueue.h"
#include "mysql/MysqlHandler.h"
#include "ss_proto.h"
#include "MyTdrBuf.h"
#include "FriendTable.h"
#include "../../cfg/FriendSvrCfgDesc.h"
#include "ThreadFrame.h"

class CDBWorkThread : public CThreadFrame
{
public:
	CDBWorkThread();
	~CDBWorkThread(){}

	FriendTable& 	GetFriendTable() { return m_oFriendTable; }
	//void SendPkg( PKGMETA::SSPKG& rstSsPkg );

	void SendReq(DT_FRIEND_DB_REQ& rstReq);
	void SendRsp(DT_FRIEND_DB_RSP& rstRsp);
protected:
	virtual bool _ThreadAppInit(void* pvAppData);
	virtual void _ThreadAppFini();
	virtual int _ThreadAppProc();
	
	int _HandleFriendMgrMsg();
	int _CheckMysqlConn();
	
protected:
	CMysqlHandler 	m_oMysqlHandler;
	FRIENDSVRCFG*	m_pstConfig;
	FriendTable		m_oFriendTable;

	MyTdrBuf		m_stSendBuf;
	PKGMETA::DT_FRIEND_DB_REQ 	m_stReq;
	PKGMETA::DT_FRIEND_DB_RSP 	m_stRsp;
};

