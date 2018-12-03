
#include "GameTime.h"
#include "../framework/GameDataMgr.h"
#include "DataMgr.h"
#include "FakeRandom.h"

using namespace PKGMETA;



bool DataMgr::Init(GUILDEXPEDITIONSVRCFG* pstConfig)
{
    if (!pstConfig)
    {
        LOGERR("pstConfig is null");
        return false;
    }
    m_pstConfig = pstConfig;

    //初始化CoDataFrame类
    this->BaseInit(pstConfig->m_iMaxCoroutine);
    this->SetCoroutineEnv(GuildExpeditionSvrMsgLayer::Instance().GetCoroutineEnv());

    m_oAsyncActionPool.Init(pstConfig->m_iAsyncActionNumInit, pstConfig->m_iAsyncActionNumDeta, pstConfig->m_iAsyncActionNumMax);
    m_oGuildPool.Init(pstConfig->m_iGuildNumInit, pstConfig->m_iGuildNumDeta, pstConfig->m_iGuildNumMax);
    m_oPlayerPool.Init(pstConfig->m_iPlayerNumInit, pstConfig->m_iPlayerNumDeta, pstConfig->m_iPlayerNumMax);
    //初始化时间链表
    TLIST_INIT(&m_stGuildTimeListHead);
    m_dwGuildTimeListSize = 0;
    //初始化脏数据相关
    TLIST_INIT(&m_stGuildDirtyListHead);
    m_dwGuildDirtyListSize = 0;
    m_dwGuildWriteTimeVal = m_pstConfig->m_iUpdateInterval;
    m_ullGuildLastWriteTimestamp = CGameTime::Instance().GetCurrSecond();

	TLIST_INIT(&m_stPlayerTimeListHead);
	TLIST_INIT(&m_stPlayerDirtyListHead);
	m_dwPlayerDirtyListSize = 0;
	m_dwPlayerTimeListSize = 0;
	m_dwPlayerWriteTimeVal = m_pstConfig->m_iUpdateInterval;
	m_ullPlayerLastWriteTimestamp = CGameTime::Instance().GetCurrSecond();



//     RESBASIC* poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_ORE_RATE_PARA);
//     assert(poResBasic);

    return true;
}

void DataMgr::Fini()
{
    Guild* pstGuild = NULL;
    for (m_oGuildMapIter = m_oGuildMap.begin(); m_oGuildMapIter != m_oGuildMap.end(); )
    {
        pstGuild = m_oGuildMapIter->second;
        this->SaveGuild(pstGuild);
        //LOGWARN("Save TeamInfo<%lu> to DB", pstGuild->GetUin());
        m_oGuildMap.erase(m_oGuildMapIter++);
        m_oGuildPool.Release(pstGuild);
    }
    Player* pstPlayer = NULL;
    for (m_oPlayerMapIter = m_oPlayerMap.begin(); m_oPlayerMapIter != m_oPlayerMap.end();)
    {
        pstPlayer = m_oPlayerMapIter->second;
        this->SavePlayer(pstPlayer);
        m_oPlayerMap.erase(m_oPlayerMapIter++);
        m_oPlayerPool.Release(pstPlayer);
    }
    return;
}

void DataMgr::Update(bool bIdle)
{
    CoDataFrame::Update();
    this->_WriteDirtyToDB();
}




Guild* DataMgr::GetGuild(uint64_t ullUid)
{
    DataKey tmpKey;
    tmpKey.m_bType = DATA_TYPE_GUILD;
    tmpKey.m_ullKey = ullUid;
    return (Guild*)CoDataFrame::GetData((void*)&tmpKey);
}






Player* DataMgr::GetPlayer(uint64_t ullUin)
{
    DataKey tmpKey;
    tmpKey.m_bType = DATA_TYPE_PLAYER;
    tmpKey.m_ullKey = ullUin;

    return (Player*)CoDataFrame::GetData((void*)&tmpKey);
}

void DataMgr::SaveGuild(Guild* pstGuild)
{
	PKGMETA::SSPKG* pstSsPkg = GuildExpeditionSvrMsgLayer::Instance().GetSendPkg();
	if (pstSsPkg == NULL)
	{
		return;
	}
	pstSsPkg->m_stHead.m_wMsgId = SS_MSG_GUILD_EXPEDITION_UPDATE_GUILD_DATA_NTF;
	SS_PKG_GUILD_EXPEDITION_UPDATE_GUILD_DATA_NTF& rstSsNtf = pstSsPkg->m_stBody.m_stGuildExpeditionUpdateGuildDataNtf;
    if (!pstGuild->PackToData(rstSsNtf.m_astGuildData[0]))
    {
        LOGERR("Save Guild<Uid=%lu>  pack failed ", pstGuild->GetId());
        return ;
    }
    rstSsNtf.m_chCount = 1;
	GuildExpeditionSvrMsgLayer::Instance().SendToClusterDBSvr(pstSsPkg);
}


void DataMgr::DelGuild(Guild* pstGuild)
{
	PKGMETA::SSPKG* pstSsPkg = GuildExpeditionSvrMsgLayer::Instance().GetSendPkg();
	if (pstSsPkg == NULL)
	{
		return;
	}
	pstSsPkg->m_stHead.m_wMsgId = PKGMETA::SS_MSG_GUILD_EXPEDITION_DEL_GUILD_DATA_NTF;
	PKGMETA::SS_PKG_GUILD_EXPEDITION_DEL_GUILD_DATA_NTF& rstReq = pstSsPkg->m_stBody.m_stGuildExpeditionDelGuildDataNtf;
	rstReq.m_Key[0] = pstGuild->GetId();
	rstReq.m_chCount = 1;
	GuildExpeditionSvrMsgLayer::Instance().SendToClusterDBSvr(pstSsPkg);
	DelFromTimeList(pstGuild);
	DelFromDirtyList(pstGuild);
	DelGuildFromMap(pstGuild);
	m_oGuildPool.Release(pstGuild);
}

void DataMgr::SavePlayer(Player* pstPlayer)
{
	PKGMETA::SSPKG* pstSsPkg = GuildExpeditionSvrMsgLayer::Instance().GetSendPkg();
	if (pstSsPkg == NULL)
	{
		return;
	}
	pstSsPkg->m_stHead.m_wMsgId = SS_MSG_GUILD_EXPEDITION_UPDATE_PLAYER_DATA_NTF;
	SS_PKG_GUILD_EXPEDITION_UPDATE_PLAYER_DATA_NTF& rstSsNtf = pstSsPkg->m_stBody.m_stGuildExpeditionUpdatePlayerDataNtf;
    if (!pstPlayer->PackToData(rstSsNtf.m_astPlayerData[0]))
    {
        LOGERR("Save Player<Uin=%lu>  pack failed ", pstPlayer->GetUin());
        return;
    }
    rstSsNtf.m_chCount = 1;
	GuildExpeditionSvrMsgLayer::Instance().SendToClusterDBSvr(pstSsPkg);
}

void DataMgr::_WriteDirtyToDB()
{
    TLISTNODE* pstPos = NULL;
    TLISTNODE* pstNext = NULL;
    TLISTNODE* pstHead = &(m_stGuildDirtyListHead);

    Guild* pstGuild = NULL;

    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();
    if (m_dwGuildDirtyListSize >= (uint32_t) m_pstConfig->m_iDirtyNumMax ||
        ullCurTime - m_ullGuildLastWriteTimestamp >= m_dwGuildWriteTimeVal)
    {
        m_ullGuildLastWriteTimestamp = ullCurTime;

        TLIST_FOR_EACH_SAFE(pstPos, pstNext, pstHead)
        {
			pstGuild = TLIST_ENTRY(pstPos, Guild, m_stDirtyListNode);
            this->SaveGuild(pstGuild);
            this->DelFromDirtyList(pstGuild);
            this->AddToTimeList(pstGuild);
        }
        TLIST_INIT(pstHead);
    }

	Player* pstPlayer = NULL;
	pstHead = &(m_stPlayerDirtyListHead);
	if (m_dwPlayerDirtyListSize >= (uint32_t)m_pstConfig->m_iDirtyNumMax ||
		ullCurTime - m_ullPlayerLastWriteTimestamp >= m_dwPlayerWriteTimeVal)
	{
		m_ullPlayerLastWriteTimestamp = ullCurTime;

		TLIST_FOR_EACH_SAFE(pstPos, pstNext, pstHead)
		{
			pstPlayer = TLIST_ENTRY(pstPos, Player, m_stDirtyListNode);
			this->SavePlayer(pstPlayer);
			this->DelFromDirtyList(pstPlayer);
			this->AddToTimeList(pstPlayer);
		}
		TLIST_INIT(pstHead);
	}
}

Guild* DataMgr::GetNewGuild()
{
	Guild* pstGuild = NULL;
    //mempool中有空余时
	if (m_oGuildPool.CanGetNewNode()) //这个判断是避免Pool中的报错信息
	{
		pstGuild = m_oGuildPool.Get();
		if (pstGuild)
		{
			return pstGuild;
		}	
	}
    //mempool中没有空余时，需要置换，置换采用LRU算法，将最近最少使用的节点回写到数据库
    
	//当回写策略不当时,会出现m_stTimeListHead为空的情况,可以直接return或者回写DirtyList
	if (TLIST_IS_EMPTY(&m_stGuildTimeListHead))
	{
		pstGuild = container_of(TLIST_PREV(&m_stGuildDirtyListHead), Guild, m_stDirtyListNode);
		LOGERR("the strategy is error!!");
		LOGWARN("exchange Guild node from DirtyListHead");
	}
	else
	{
		pstGuild = container_of(TLIST_PREV(&m_stGuildTimeListHead), Guild, m_stTimeListNode);
		LOGWARN("exchange Guild node from TimeListHead");
	}
	this->SaveGuild(pstGuild);
	this->DelFromTimeList(pstGuild);
	this->DelFromDirtyList(pstGuild);
	this->DelGuildFromMap(pstGuild);
    
    return pstGuild;
}


Player* DataMgr::GetNewPlayer()
{
	Player* pstPlayer = NULL; 
	//mempool中有空余时
	if (m_oPlayerPool.CanGetNewNode())	//这个判断是避免Pool中的报错信息
	{
		pstPlayer = m_oPlayerPool.Get();
		if (pstPlayer)
		{
			return pstPlayer;
		}
	}
	//mempool中没有空余时，需要置换，置换采用LRU算法，将最近最少使用的节点回写到数据库
	
	
	//当回写策略不当时,会出现m_stTimeListHead为空的情况,可以直接return或者回写DirtyList
	if (TLIST_IS_EMPTY(&m_stPlayerTimeListHead))
	{
		pstPlayer = container_of(TLIST_PREV(&m_stPlayerDirtyListHead), Player, m_stDirtyListNode);
		LOGERR("pstPlayer the strategy is error!!");
		LOGWARN("exchange Player node from DirtyListHead");
	}
	else
	{
		pstPlayer = container_of(TLIST_PREV(&m_stPlayerTimeListHead), Player, m_stTimeListNode);
		LOGWARN("exchange Player node from TimeListHead");
	}
	this->SavePlayer(pstPlayer);
	this->DelFromDirtyList(pstPlayer);
	this->DelFromTimeList(pstPlayer);
	this->DelPlayerToMap(pstPlayer);
	
	return pstPlayer;
}

void DataMgr::AddGuildToMap(Guild* pstGuild)
{
    m_oGuildMap.insert(Key2Guild_t::value_type(pstGuild->GetId(), pstGuild));


}

void DataMgr::DelGuildFromMap(Guild* pstGuild)
{

	m_oGuildMapIter = m_oGuildMap.find(pstGuild->GetId());

	if (m_oGuildMapIter != m_oGuildMap.end())
	{
		m_oGuildMap.erase(m_oGuildMapIter);
	}
	else
	{
		LOGERR("Del Uin<%lu> pstGuild from map failed, not found", pstGuild->GetId());
	}
}



void DataMgr::AddPlayerToMap(Player* pstPlayer)
{
    m_oPlayerMap.insert(Key2Player_t::value_type(pstPlayer->GetUin(), pstPlayer));

}

void DataMgr::DelPlayerToMap(Player* pstPlayer)
{
    m_oPlayerMapIter = m_oPlayerMap.find(pstPlayer->GetUin());

    if (m_oPlayerMapIter != m_oPlayerMap.end())
    {
        m_oPlayerMap.erase(m_oPlayerMapIter);
    }
    else
    {
        LOGERR("Del Uin<%lu> pstPlayer from map failed, not found", pstPlayer->GetUin());
    }

}


void DataMgr::AddToDirtyList(Guild* pstGuild)
{
    TLISTNODE* pstNode = &pstGuild->m_stDirtyListNode;
	if (!TLIST_IS_EMPTY(pstNode))
	{
		return;
	}
    TLIST_INSERT_NEXT(&m_stGuildDirtyListHead, pstNode);
    m_dwGuildDirtyListSize++;
	//LOGWARN("GuildId<%lu> AddToDirtyList TimeSize<%u> DirtySize<%u>", pstGuild->GetId(), m_dwGuildTimeListSize, m_dwGuildDirtyListSize);

}

void DataMgr::AddToDirtyList(Player* pstPlayer)
{
	TLISTNODE* pstNode = &pstPlayer->m_stDirtyListNode;
	if (!TLIST_IS_EMPTY(pstNode))
	{
		return;
	}
	TLIST_INSERT_NEXT(&m_stPlayerDirtyListHead, pstNode);
	m_dwPlayerDirtyListSize++;
	//LOGWARN("Player<%lu> AddToDirtyList DirtySize<%u> TimeSize<%u>", pstPlayer->GetUin(), m_dwPlayerDirtyListSize, m_dwPlayerTimeListSize);
}

void DataMgr::MoveToTimeListFirst(Guild * pstGuild)
{
    TLISTNODE* pstNode = &pstGuild->m_stTimeListNode;
    if (TLIST_IS_EMPTY(pstNode))
    {
        return;
    }
    TLIST_DEL(pstNode);
    TLIST_INSERT_NEXT(&m_stGuildTimeListHead, pstNode);
}


void DataMgr::MoveToTimeListFirst(Player* pstPlayer)
{
	TLISTNODE* pstNode = &pstPlayer->m_stTimeListNode;
	if (TLIST_IS_EMPTY(pstNode))
	{
		return;
	}

	TLIST_DEL(pstNode);
	TLIST_INSERT_NEXT(&m_stPlayerTimeListHead, pstNode);
}


int DataMgr::MatchGuild(uint64_t ullGuildID, uint64_t ullLi, OUT PKGMETA::DT_GUILD_EXPEDITION_MATCHED_GUILD_INFO *pMatchedGuildIdList)
{
	if (m_oMatchMap.size() < MAX_GUILD_EXPEDITION_MATCH_NUM + 1)
	{
		LOGERR("GuildId<%lu> guild num<%d> not enough, cant match", ullGuildID, (int)m_oMatchMap.size());
		return ERR_GUILD_EXPEDITION_NO_ENOUGH_GUILD_MATCH;
	}

	m_oMatchMapIter = m_oMatchMap.find(ullLi);
	//这里如果没有找到,说明插入删除MatchList有逻辑不闭合,应该排查,暂时不重新插入
	if (m_oMatchMapIter == m_oMatchMap.end())
	{
		LOGERR("GuildId<%lu> match Guild error: not insert the matchlist,Li<%lu> ", ullGuildID, ullLi);
		return ERR_SYS;
	}
	std::map<uint64_t, Guild*>::reverse_iterator r_Iter(m_oMatchMapIter);
	Guild* pstGuild = NULL;
	int iRet = 0;
	//正向查找
	for (; iRet < MAX_GUILD_EXPEDITION_MATCH_NUM && m_oMatchMapIter != m_oMatchMap.end(); m_oMatchMapIter++)
	{
		pstGuild = m_oMatchMapIter->second;
		if (pstGuild->GetId() != ullGuildID)
		{
			pstGuild->GetMatchedInfo(pMatchedGuildIdList[iRet]);
			iRet++;
		}
	}
	//反向查找
	for (; iRet < MAX_GUILD_EXPEDITION_MATCH_NUM && r_Iter != m_oMatchMap.rend(); r_Iter++)
	{
		pstGuild = r_Iter->second;
		if (pstGuild->GetId() != ullGuildID)
		{
			pstGuild->GetMatchedInfo(pMatchedGuildIdList[iRet]);
			iRet++;
		}
	}
	return iRet;
}

void DataMgr::AddToTimeList(Guild* pstGuild)
{
    TLISTNODE* pstNode = &pstGuild->m_stTimeListNode;
    if (!TLIST_IS_EMPTY(pstNode))
    {
        return;
    }
    TLIST_INSERT_NEXT(&m_stGuildTimeListHead, pstNode);
    m_dwGuildTimeListSize++;
    //LOGWARN("GuildId<%lu> AddTimeList TimeSize<%u> DirtySize<%u>", pstGuild->GetId(), m_dwGuildTimeListSize, m_dwGuildDirtyListSize);
}


void DataMgr::AddToTimeList(Player* pstPlayer)
{
	TLISTNODE* pstNode = &pstPlayer->m_stTimeListNode;
	if (!TLIST_IS_EMPTY(pstNode))
	{
		return;
	}
	TLIST_INSERT_NEXT(&m_stPlayerTimeListHead, pstNode);
	m_dwPlayerTimeListSize++;
	//LOGWARN("Player<%lu> AddToTimeList DirtySize<%u> TimeSize<%u>", pstPlayer->GetUin(), m_dwPlayerDirtyListSize, m_dwPlayerTimeListSize);

}

void DataMgr::DelFromDirtyList(Guild* pstGuild)
{
    TLISTNODE* pstNode = &pstGuild->m_stDirtyListNode;
    if (TLIST_IS_EMPTY(pstNode))
    {
        return;
    }
    TLIST_DEL(pstNode);
    m_dwGuildDirtyListSize--;
	//LOGWARN("GuildId<%lu> DelFromDirtyList TimeSize<%u> DirtySize<%u>", pstGuild->GetId(), m_dwGuildTimeListSize, m_dwGuildDirtyListSize);
}


void DataMgr::DelFromDirtyList(Player* pstPlayer)
{
	TLISTNODE* pstNode = &pstPlayer->m_stDirtyListNode;
	if (TLIST_IS_EMPTY(pstNode))
	{
		return;
	}
	TLIST_DEL(pstNode);
	m_dwPlayerDirtyListSize--;
	//LOGWARN("Player<%lu> DelFromDirtyList DirtySize<%u> TimeSize<%u>", pstPlayer->GetUin(), m_dwPlayerDirtyListSize, m_dwPlayerTimeListSize);
}




void DataMgr::AddToMatchList(Guild* pstGuild)
{
	//如果相同战力的公会,只用一个公会即可
	m_oMatchMap.insert(make_pair(pstGuild->GetLi(), pstGuild));
}


void DataMgr::DelFromMatchList(Guild* pstGuild)
{

	m_oMatchMapIter = m_oMatchMap.find(pstGuild->GetLi());
	if (m_oMatchMapIter != m_oMatchMap.end() && m_oMatchMapIter->second == pstGuild)
	{
		//map的值是pstGuild才删除
		m_oMatchMap.erase(m_oMatchMapIter);
	}
}
	

void DataMgr::DelFromTimeList(Guild* pstGuild)
{
    TLISTNODE* pstNode = &pstGuild->m_stTimeListNode;
    if (TLIST_IS_EMPTY(pstNode))
    {
        return;
    }
    TLIST_DEL(pstNode);
    m_dwGuildTimeListSize--;
	//LOGWARN("GuildId<%lu> DelTimeyList TimeSize<%u> DirtySize<%u>", pstGuild->GetId(), m_dwGuildTimeListSize, m_dwGuildDirtyListSize);

}


void DataMgr::DelFromTimeList(Player* pstPlayer)
{
	TLISTNODE* pstNode = &pstPlayer->m_stTimeListNode;
	if (TLIST_IS_EMPTY(pstNode))
	{
		return;
	}
	TLIST_DEL(pstNode);
	m_dwPlayerTimeListSize--;
	//LOGWARN("Player<%lu> DelFromTimeList DirtySize<%u> TimeSize<%u>", pstPlayer->GetUin(), m_dwPlayerDirtyListSize, m_dwPlayerTimeListSize);

}

bool DataMgr::IsInMem(void* pResult)
{
    if (!pResult)
    {
        return false;
    }
    DataResult* pstResult = (DataResult*)pResult;
    if (pstResult->m_bType == DATA_TYPE_GUILD)    //kuang
    {
       return m_oGuildMap.find( ((DT_GUILD_EXPEDITION_GUILD_DATA*)pstResult->m_pstData)->m_ullGuildId ) != m_oGuildMap.end();
    }
    else if (pstResult->m_bType == DATA_TYPE_PLAYER)
    {
        return m_oPlayerMap.find(((DT_GUILD_EXPEDITION_PLAYER_DATA*)pstResult->m_pstData)->m_ullUin) != m_oPlayerMap.end();
    }
    return false;

}

bool DataMgr::SaveInMem(void* pResult)
{
    if (!pResult)
    {
        return false;
    }
    DataResult* pstResult = (DataResult*)pResult;
    if (pstResult->m_bType == DATA_TYPE_GUILD)
    {
		DT_GUILD_EXPEDITION_GUILD_DATA* poData = (DT_GUILD_EXPEDITION_GUILD_DATA*)pstResult->m_pstData;
        Guild * pstGuild = this->GetNewGuild();
        if (!pstGuild)
        {
            return false;
        }
        if (!pstGuild->InitFromDB(*poData))
        {
            return false;
        }
        this->AddToTimeList(pstGuild);
        this->AddGuildToMap(pstGuild);
        return true;
    }
    else if (pstResult->m_bType == DATA_TYPE_PLAYER)
    {
        DT_GUILD_EXPEDITION_PLAYER_DATA* poData = (DT_GUILD_EXPEDITION_PLAYER_DATA*)pstResult->m_pstData;
        Player * pstPlayer = this->GetNewPlayer();
        if (!pstPlayer)
        {
            return false;
        }
        if (!pstPlayer->InitFromDB(*poData))
        {
            return false;
        }
        this->AddToTimeList(pstPlayer);
        this->AddPlayerToMap(pstPlayer);
        return true;
    }

    return true;
}



void* DataMgr::_GetDataInMem(void* key)
{
    if (!key)
    {
        return NULL;
    }
    DataKey* pstKey = (DataKey*)key;
    if (pstKey->m_bType == DATA_TYPE_GUILD)
    {
        m_oGuildMapIter = m_oGuildMap.find(pstKey->m_ullKey);
        if (m_oGuildMapIter == m_oGuildMap.end())
        {
            return NULL;
        }
        this->MoveToTimeListFirst(m_oGuildMapIter->second);
        return (void*)(m_oGuildMapIter->second);
    }
    else if (pstKey->m_bType == DATA_TYPE_PLAYER)
    {
        m_oPlayerMapIter = m_oPlayerMap.find(pstKey->m_ullKey);
        if (m_oPlayerMapIter == m_oPlayerMap.end())
        {
            return NULL;
        }
        this->MoveToTimeListFirst(m_oPlayerMapIter->second);
        return (void*)(m_oPlayerMapIter->second);
    }
    else
    {
        return NULL;
    }

}

CoGetDataAction* DataMgr::_CreateGetDataAction(void* key)
{
    if (!key)
    {
        return NULL;
    }
    DataKey* pstKey = (DataKey*)key;
    AsyncGetDataAction* poAction = m_oAsyncActionPool.Get();
    if (!poAction)
    {
        LOGERR("m_oAsyncActionPool.Get() failed key<%lu>, type<%hhu>", pstKey->m_ullKey, pstKey->m_bType);
        return NULL;
    }
    poAction->SetKey(*pstKey);
    return (CoGetDataAction*)poAction;
}

void DataMgr::_ReleaseGetDataAction(CoGetDataAction* poAction)
{
    AsyncGetDataAction* poGetDataAction = (AsyncGetDataAction*)poAction;
    m_oAsyncActionPool.Release(poGetDataAction);
}
