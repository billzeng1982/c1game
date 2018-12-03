#pragma once

#include "define.h"
#include "singleton.h"
#include "common_proto.h"
#include "ss_proto.h"
#include "LogMacros.h"
#include "redis/RedisSyncHandler.h"
#include "../../cfg/FriendSvrCfgDesc.h"
#include "RedisWorkThread.h"
using namespace PKGMETA;
/*** 单独定义在Redis中缓存的 结构, 以后添加字段时如果 好友不用可以不修改好友的, 但这时对Redis的更新可能要放在ZoneSvr上
 *  
 **/



class RedisWorkThreadMgr : public TSingleton<RedisWorkThreadMgr>
{
public:
	bool Init(FRIENDSVRCFG*	m_pstConfig);
    void Fini();
	void SendReq(DT_FRIEND_PLAYERINFO_REDIS_REQ& rstReq);	//发送Redis请求
	void HandleRspMsg();									//处理收到的所有线程的回复.
private:
	int m_iWorkThreadNum;
	RedisWorkThread* m_astWorkThreads;
	DT_FRIEND_PLAYERINFO_REDIS_RSP m_stRsp;
};
