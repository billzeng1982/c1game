#pragma once

#include "define.h"
#include "ThreadQueue.h"
#include "mysql/MysqlHandler.h"
#include "ss_proto.h"
#include "MyTdrBuf.h"
#include "../logic/ClusterAccTable.h"
#include "../cfg/ClusterAccountSvrCfgDesc.h"
#include "ThreadFrame.h"
#include "../framework/ClusterAccSvrMsgLayer_r.h"

class CDBWorkThread : public CThreadFrame
{
public:
	CDBWorkThread();
	~CDBWorkThread(){}

	ClusterAccTable& 	GetClusterAccTable() { return m_oClusterAccTable; }
	void SendPkg( PKGMETA::SSPKG& rstSsPkg );

	PKGMETA::SSPKG& GetRspSsPkg() { return m_stRspPkg; }

protected:
	virtual bool _ThreadAppInit(void* pvAppData);
	virtual void _ThreadAppFini();
	virtual int _ThreadAppProc();

	// 处理主线程发来的消息
	int _HandleClusterAccSvrMsg();
	int _CheckMysqlConn();
	
protected:
	CMysqlHandler 	m_oMysqlHandler;
	CLUSTERACCOUNTSVRCFG*	m_pstConfig;
	ClusterAccTable	m_oClusterAccTable;

	MyTdrBuf		m_stSendBuf;
	PKGMETA::SSPKG 	m_stSsRecvPkg;
	PKGMETA::SSPKG	m_stRspPkg;
	
	ClusterAccSvrMsgLayer_r m_oMsgLayer;
};

