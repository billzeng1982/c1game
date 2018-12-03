#pragma once

#include "redis/RedisSyncHandler.h"
#include "common_proto.h"
#include <string.h>
#include <stdio.h>
#include "strutil.h"
#include "../../cfg/FriendSvrCfgDesc.h"
#include "ThreadFrame.h"
#include "PlayerInfoTable.h"
using namespace PKGMETA;

class RedisWorkThread : public CThreadFrame
{
public:
	RedisWorkThread() : m_stSendReqBuf(sizeof(DT_FRIEND_PLAYERINFO_REDIS_REQ)*2+1), m_stSendRspBuf(sizeof(DT_FRIEND_PLAYERINFO_REDIS_RSP)*2+1) {m_pstConfig = NULL;}
	~RedisWorkThread(){}



	void SendReq(DT_FRIEND_PLAYERINFO_REDIS_REQ& rstReq);
	void SendRsp(DT_FRIEND_PLAYERINFO_REDIS_RSP& rstRsp);

	void GetPlayerInfo(DT_FRIEND_PLAYERINFO_REDIS_REQ& rstReq);
	void SavePlayerInfo(DT_FRIEND_PLAYERINFO_REDIS_REQ& rstReq);
	void GetListPlayerInfo(DT_FRIEND_PLAYERINFO_REDIS_REQ& rstReq);
    
protected:
	virtual bool _ThreadAppInit(void* pvAppData);
	virtual void _ThreadAppFini() {};
	virtual int _ThreadAppProc();
	int _HandleReqMsg();		//处理外部发来的 数据库请求
    int _CheckRedisConn();

protected:
	RedisSyncHandler 				m_oRedisHandler;
	FRIENDSVRCFG*					m_pstConfig;
	PlayerInfoTable					m_stPlayerInfoTable;
	MyTdrBuf						m_stSendReqBuf;
	MyTdrBuf						m_stSendRspBuf;
	DT_FRIEND_PLAYERINFO_REDIS_REQ 	m_stReq;
	DT_FRIEND_PLAYERINFO_REDIS_RSP 	m_stRsp;
    time_t m_tLastPingTime;
};
