#pragma once

#include "define.h"
#include "common_proto.h"
#include "ss_proto.h"
#include "LogMacros.h"
#include "redis/RedisSyncHandler.h"
#include <time.h>  
using namespace PKGMETA;

#define REDIS_COMMAND_ARGUC (MAX_FRIEND_AGREE_LIST_CNT*2 + 2)
#define REDIS_PLAYER_INFO_TABLE_NAME  "PlayerInfo"
class PlayerInfoTable
{
public:
    static const uint8_t CONST_VERSION_LEN = 4;
public:
	PlayerInfoTable() {};
    ~PlayerInfoTable() {};
	bool Init( RedisSyncHandler* poRedisHandler) ;

public:
	int GetPlayerInfo(INOUT DT_FRIEND_PLAYER_INFO& rstFriendPlayerInfo);
	int GetListPlayerInfo(IN uint64_t ullUin, IN DT_FRIEND_AGREE_INFO& rstArgreeInfo, IN DT_FRIEND_APPLY_INFO& rstApplyInfo, OUT DT_FRIEND_WHOLE_DATA_FRONT& rstWholeDataFront);
	int SavePlayerInfo(IN DT_FRIEND_PLAYER_INFO& rstFriendPlayerInfo);

private:
	RedisSyncHandler* m_poRedisHandler;
	redisReply* m_poReply;
	size_t m_ulUseSize;
	int m_iGetIndex;		//用来表示 取Redis返回的数据的索引 使用时置0
private:
	DT_FRIEND_PLAYER_BLOB	m_stPlayerBlob;
};

