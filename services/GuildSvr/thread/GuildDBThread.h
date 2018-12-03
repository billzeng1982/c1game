#pragma once

#include "define.h"
#include "ThreadQueue.h"
#include "ThreadFrame.h"
#include "mysql/MysqlHandler.h"
#include "MyTdrBuf.h"
#include "common_proto.h"
#include "../cfg/GuildSvrCfgDesc.h"
#include "GuildTable.h"

using namespace PKGMETA;

class GuildDBThread : public CThreadFrame
{
public:
	GuildDBThread();
	~GuildDBThread(){}

	void SendReqPkg(DT_GUILD_DB_REQ& rstDBReq);

private:
    void SendRspPkg(DT_GUILD_DB_RSP& rstDBRsp);

protected:
	virtual bool _ThreadAppInit(void* pvAppData);
	virtual void _ThreadAppFini();
	virtual int _ThreadAppProc();

	int _HandleMsg();
	int _CheckMysqlConn();

protected:
	CMysqlHandler 	m_oMysqlHandler;
	GuildTable		m_oGuildTable;
    int             m_iPingFreq;

	MyTdrBuf		    m_stSendBuf;
	DT_GUILD_DB_REQ     m_stReqPkg;
    DT_GUILD_DB_RSP     m_stRspPkg;
};

