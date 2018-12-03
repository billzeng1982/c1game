
#include "PlayerInfoTable.h"
#include "TdrPal.h"
#include "PKGMETA_metalib.h"

bool PlayerInfoTable::Init( RedisSyncHandler* poRedisHandler)
{
    if (!poRedisHandler)
    {
        LOGERR_r("poRedisHandler is NULL");
        return false;
    }
	m_poRedisHandler = poRedisHandler;
	return m_poRedisHandler->ArgvInit(MAX_FRIEND_AGREE_LIST_CNT + MAX_FRIEND_APPLY_LIST_CNT + 2,
        MAX_LEN_FRIEND_PLAYER_BLOB);
}


int PlayerInfoTable::GetPlayerInfo(INOUT DT_FRIEND_PLAYER_INFO& rstFriendPlayerInfo)
{
	m_poReply = m_poRedisHandler->SyncCommand( "HGET %s %lu", REDIS_PLAYER_INFO_TABLE_NAME, rstFriendPlayerInfo.m_ullUin);
	if (NULL == m_poReply || REDIS_REPLY_STRING != m_poReply->type)
	{
        if (NULL == m_poReply)
        {
            LOGERR_r("m_poReply is null ");
        }
        else
        {
            LOGERR_r("Redis Reply type err<%d>", m_poReply->type);
        }
		return ERR_REDIS_ERROR;
	}
    uint32_t dwVersion = 0;
    dwVersion = tdr_cpp_ntoh32(*(uint32_t*)m_poReply->str);
	int iRet = rstFriendPlayerInfo.unpack((char*)(m_poReply->str + CONST_VERSION_LEN),
        m_poReply->len - CONST_VERSION_LEN, NULL, dwVersion);
	if (TdrError::TDR_NO_ERROR != iRet)
	{
		LOGERR_r("unpack rstPlayerBriefInfo failed, Ret=%d Uin<%lu> ", iRet, rstFriendPlayerInfo.m_ullUin);
		return ERR_SYS;
	}
	return ERR_NONE;
}


//获取所有好友的信息和申请者信息
int PlayerInfoTable::GetListPlayerInfo(IN uint64_t ullUin, IN DT_FRIEND_AGREE_INFO& rstArgreeInfo,
    IN DT_FRIEND_APPLY_INFO& rstApplyInfo, OUT DT_FRIEND_WHOLE_DATA_FRONT& rstWholeDataFront)
{
    DT_FRIEND_AGREE_FRONT_INFO& rstAgreeFront = rstWholeDataFront.m_stAgreeInfo;
    DT_FRIEND_APPLY_FRONT_INFO& rstApplyFront = rstWholeDataFront.m_stApplyInfo;
    rstAgreeFront.m_wCount = 0;
    rstApplyFront.m_wCount = 0;
	int iRet = 0;
    m_poRedisHandler->ArgvReset();
    m_poRedisHandler->PushArgv("HMGET");
    m_poRedisHandler->PushArgv(REDIS_PLAYER_INFO_TABLE_NAME);
    if (0 == rstArgreeInfo.m_wCount && 0 == rstApplyInfo.m_wCount)
    {//两个都为零,不执行Redis命令,而且如果执行 ,会报错的添加打印,但不影响结果
        return ERR_NONE;
    }

	int i = 0;
	for (;i < rstArgreeInfo.m_wCount; i++)
	{
        m_poRedisHandler->PushArgv(rstArgreeInfo.m_List[i]);
	}
	for (i = 0; i < rstApplyInfo.m_wCount; i++)
	{
        m_poRedisHandler->PushArgv(rstApplyInfo.m_List[i]);
	}
	redisReply* reply = m_poRedisHandler->SyncCommandArgv();
    if (NULL == reply)
    {
        LOGERR_r("Uin<%lu> GetListPlayerInfo:m_poReply is null", ullUin);
        return ERR_REDIS_EXECUTE_FAILED;
    }
	if (reply->type != REDIS_REPLY_ARRAY)
	{
		LOGERR_r("Uin<%lu> GetListPlayerInfo:Reply type err<%d>!", ullUin, reply->type);
		return ERR_REDIS_EXECUTE_FAILED;
	}
	//	取数据
	if (reply->elements != rstArgreeInfo.m_wCount + rstApplyInfo.m_wCount)
	{//数据有丢失,暂时全部丢弃
		LOGERR_r("Uin<%lu> Redis get data error!", ullUin);
		return ERR_REDIS_EXECUTE_FAILED;
	}

	m_iGetIndex = 0;	//这里用来表示 取Redis返回的数据的索引 置0
    uint32_t dwVersion = 0;
	for (i = 0; i < rstArgreeInfo.m_wCount;  m_iGetIndex++, i++)
	{
		if(reply->element[m_iGetIndex]->type != REDIS_REPLY_STRING || reply->element[m_iGetIndex]->str == NULL)
		{
			LOGERR_r("Uin<%lu> get data from Redis erro, redis type<%d>!",ullUin, reply->element[m_iGetIndex]->type);
			continue;
		}
        dwVersion = tdr_cpp_ntoh32(*(uint32_t*)reply->element[m_iGetIndex]->str);
		iRet = rstAgreeFront.m_astList[rstAgreeFront.m_wCount].unpack(reply->element[m_iGetIndex]->str + CONST_VERSION_LEN,
            reply->element[m_iGetIndex]->len - CONST_VERSION_LEN, NULL, dwVersion);
		if (iRet != TdrError::TDR_NO_ERROR)
		{//出现这个错误,一般是Redis中保存的数据不在了, 做个容错性开发,是其他好友信息能够显示
			LOGERR_r("Uin<%lu> unpack AgreeFrontInfo from Redis erro<%d>!",ullUin, iRet);
			//return ERR_SYS;
            continue;
		}
        rstAgreeFront.m_wCount++;
	}

	for (i = 0; i < rstApplyInfo.m_wCount;  m_iGetIndex++, i++)
	{
		if(reply->element[m_iGetIndex]->type != REDIS_REPLY_STRING || reply->element[m_iGetIndex]->str == NULL)
		{
			LOGERR_r("Uin<%lu> get data from Redis erro, redis type<%d>!",ullUin, reply->element[m_iGetIndex]->type);
			continue;
		}
        dwVersion = tdr_cpp_ntoh32(*(uint32_t*)reply->element[m_iGetIndex]->str);
		iRet = rstApplyFront.m_astList[rstApplyFront.m_wCount].unpack(reply->element[m_iGetIndex]->str + CONST_VERSION_LEN,
            reply->element[m_iGetIndex]->len - CONST_VERSION_LEN, NULL, dwVersion);
		if (iRet != TdrError::TDR_NO_ERROR)
		{
			LOGERR_r("Uin<%lu> unpack ApplyFrontInfo from Redis erro<%d>!", ullUin, iRet);
			//return ERR_SYS;
            continue;
		}
        rstApplyFront.m_wCount++;
	}
	return ERR_NONE;
}


int PlayerInfoTable::SavePlayerInfo(IN DT_FRIEND_PLAYER_INFO& rstFriendPlayerInfo)
{
    size_t iSize = 0;
    uint32_t dwVersion = tdr_cpp_hton32((uint32_t)PKGMETA::MetaLib::getVersion());
    memcpy((char*)m_stPlayerBlob.m_szData, (char*)&dwVersion, CONST_VERSION_LEN);
	int iRet = rstFriendPlayerInfo.pack((char*)(m_stPlayerBlob.m_szData + CONST_VERSION_LEN), MAX_LEN_FRIEND_PLAYER_BLOB, &iSize);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Uin<%lu> pack rstFriendPlayerInfo failed, Ret=%d", rstFriendPlayerInfo.m_ullUin, iRet);
        return ERR_SYS;
    }
    iSize += CONST_VERSION_LEN;
	m_poReply = m_poRedisHandler->SyncCommand( "HSET %s %lu %b", REDIS_PLAYER_INFO_TABLE_NAME, rstFriendPlayerInfo.m_ullUin, m_stPlayerBlob.m_szData, iSize);
	if (NULL == m_poReply)
	{
		LOGERR_r("Redis SavePlayerInfo, m_poReply is null! ");
		return ERR_REDIS_ERROR;
	}
	if (m_poReply->type != REDIS_REPLY_INTEGER)
	{
		LOGERR_r("Redis cmd execute err! <%d> <%s>", m_poReply->type, m_poReply->str);
		return ERR_REDIS_ERROR;
	}
	return ERR_NONE;
}

