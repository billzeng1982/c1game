#pragma once

#include "define.h"
#include "MyTdrBuf.h"
#include "ThreadQueue.h"
#include "ThreadFrame.h"
#include "common_proto.h"
#include "AsyncPvpSvrCfgDesc.h"
#include "TeamTable.h"

using namespace PKGMETA;

class TeamDBThread : public CThreadFrame
{
public:
	TeamDBThread();
	~TeamDBThread(){}

	void SendReqPkg(DT_ASYNC_PVP_TEAM_DB_REQ& rstDBReq);

private:
    void SendRspPkg(DT_ASYNC_PVP_TEAM_DB_RSP& rstDBRsp);

protected:
	virtual bool _ThreadAppInit(void* pvAppData);
	virtual void _ThreadAppFini();
	virtual int _ThreadAppProc();

	int _HandleMsg();
	int _CheckMysqlConn();

protected:
	CMysqlHandler 	        m_oMysqlHandler;
	TeamTable		        m_oTeamTable;
    int                     m_iPingFreq;

	MyTdrBuf		        m_stSendBuf;
	DT_ASYNC_PVP_TEAM_DB_REQ     m_stReqPkg;
    DT_ASYNC_PVP_TEAM_DB_RSP     m_stRspPkg;
};

