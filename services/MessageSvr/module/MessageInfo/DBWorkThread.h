#pragma once

#include "define.h"
#include "ThreadQueue.h"
#include "mysql/MysqlHandler.h"
#include "ss_proto.h"
#include "MyTdrBuf.h"
#include "MessageTable.h"
#include "../../cfg/MessageSvrCfgDesc.h"
#include "ThreadFrame.h"

class CDBWorkThread : public CThreadFrame
{
public:
	CDBWorkThread();
	~CDBWorkThread(){}

	MessageTable& 	GetMessageTable() { return m_oMessageTable; }
	//void SendPkg( PKGMETA::SSPKG& rstSsPkg );

	void SendReq(DT_MESSAGE_DB_REQ& rstReq);
	void SendRsp(DT_MESSAGE_DB_RSP& rstRsp);
protected:
	virtual bool _ThreadAppInit(void* pvAppData);
	virtual void _ThreadAppFini();
	virtual int _ThreadAppProc();
	
	int _HandleMainThreadReqMsg();
	int _CheckMysqlConn();
	
protected:
	CMysqlHandler 				m_oMysqlHandler;
	MESSAGESVRCFG*				m_pstConfig;
	MessageTable				m_oMessageTable;

	MyTdrBuf					m_stSendBuf;
	PKGMETA::DT_MESSAGE_DB_REQ 	m_stReq;
	PKGMETA::DT_MESSAGE_DB_RSP 	m_stRsp;
};

