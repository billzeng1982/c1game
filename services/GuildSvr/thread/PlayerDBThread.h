#pragma once

#include "define.h"
#include "MyTdrBuf.h"
#include "ThreadQueue.h"
#include "ThreadFrame.h"
#include "common_proto.h"
#include "../cfg/GuildSvrCfgDesc.h"
#include "PlayerTable.h"

using namespace PKGMETA;

class PlayerDBThread : public CThreadFrame
{
public:
	PlayerDBThread();
	~PlayerDBThread(){}

	void SendReqPkg(DT_GUILD_PLAYER_DB_REQ& rstDBReq);

private:
    void SendRspPkg(DT_GUILD_PLAYER_DB_RSP& rstDBRsp);

protected:
	virtual bool _ThreadAppInit(void* pvAppData);
	virtual void _ThreadAppFini();
	virtual int _ThreadAppProc();

	int _HandleMsg();
	int _CheckMysqlConn();

protected:
	CMysqlHandler 	        m_oMysqlHandler;
	PlayerTable		        m_oPlayerTable;
    int                     m_iPingFreq;

	MyTdrBuf		           m_stSendBuf;
	DT_GUILD_PLAYER_DB_REQ     m_stReqPkg;
    DT_GUILD_PLAYER_DB_RSP     m_stRspPkg;
};

