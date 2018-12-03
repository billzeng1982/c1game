#pragma once

#include "AppFrame.h"
#include "singleton.h"
#include "cfg/FriendSvrCfgDesc.h"
#include "./module/FriendInfo/FriendTable.h"
#include "redis/RedisSyncHandler.h"
class FriendSvr : public CAppFrame, public TSingleton<FriendSvr>
{
public:
	FriendSvr();
	virtual ~FriendSvr() {}

	virtual bool AppInit();
	virtual void AppUpdate();
	virtual void AppFini();
	virtual int GetProcID(){ return m_stConfig.m_iProcID; }
    FRIENDSVRCFG& GetConfig() {return m_stConfig; }

public:


private:
	virtual void _SetAppCfg();
	virtual bool _ReadCfg();

private:
    FRIENDSVRCFG m_stConfig;
    CMysqlHandler m_oMysqlHandler;
    FriendTable m_oFriendTable;
    //RedisSyncHandler m_oRedisHandler;
};

