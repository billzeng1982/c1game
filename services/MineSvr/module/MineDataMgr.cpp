
#include "GameTime.h"
#include "../gamedata/GameDataMgr.h"
#include "MineDataMgr.h"
#include "FakeRandom.h"

using namespace PKGMETA;



bool MineDataMgr::Init(MINESVRCFG* pstConfig)
{
    if (!pstConfig)
    {
        LOGERR("pstConfig is null");
        return false;
    }
    m_pstConfig = pstConfig;

    //初始化CoDataFrame类
    this->BaseInit(pstConfig->m_dwMaxCoroutine);
    this->SetCoroutineEnv(MineSvrMsgLayer::Instance().GetCoroutineEnv());

    m_oAsyncActionPool.Init(pstConfig->m_iAsyncActionNumInit, pstConfig->m_iAsyncActionNumDeta, pstConfig->m_iAsyncActionNumMax);
    m_oMineOrePool.Init(pstConfig->m_iMineOreNumInit, pstConfig->m_iMineOreNumDeta, pstConfig->m_iMineOreNumMax);
    m_oMinePlayerPool.Init(pstConfig->m_iMinePlayerNumInit, pstConfig->m_iMinePlayerNumDeta, pstConfig->m_iMinePlayerNumMax);
    //初始化时间链表
    TLIST_INIT(&m_stOreTimeListHead);
    m_dwOreTimeListSize = 0;
    //初始化脏数据相关
    TLIST_INIT(&m_stOreDirtyListHead);
    m_dwOreDirtyListSize = 0;
    m_dwOreWriteTimeVal = m_pstConfig->m_iUpdateInterval;
    m_ullOreLastWriteTimestamp = CGameTime::Instance().GetCurrSecond();

	TLIST_INIT(&m_stPlayerTimeListHead);
	TLIST_INIT(&m_stPlayerDirtyListHead);
	m_dwPlayerDirtyListSize = 0;
	m_dwPlayerWriteTimeVal = m_pstConfig->m_iUpdateInterval;
	m_ullPlayerLastWriteTimestamp = CGameTime::Instance().GetCurrSecond();



    RESBASIC* poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_ORE_RATE_PARA);
    assert(poResBasic);
    m_dwOreRateOfNormal = poResBasic->m_para[0] * 100;
    m_dwOreRateOfSupper = m_dwOreRateOfNormal + poResBasic->m_para[1] * 100;
    m_dwOreRateOfInvestigate = 100;


	TLIST_INIT(&m_stNormalOreEmptyListHead);
	TLIST_INIT(&m_stSupperOreEmptyListHead);
	m_iNormalOreEmptyNum = 0;
	m_iNormalOreTotalNum = 0;
	m_iSupperOreEmptyNum = 0;
	m_iSupperOreTotalNum = 0;
	m_iInvestigateNum = 0;
    //SendSysInfo();

	if (!m_oMineUidSet.Init("UidList.fd"))
	{
		return false;
	}

	if (!IsMineUidSetEmpty())
	{
		m_oMineUidSet.SettleBegin(); 
		SetGetOreSwitch(true);
	}
    return true;
}

void MineDataMgr::Fini()
{
    MineOre* pstOre = NULL;
    for (m_oOreMapIter = m_oOrerMap.begin(); m_oOreMapIter != m_oOrerMap.end(); )
    {
        pstOre = m_oOreMapIter->second;
        this->_SaveOre(pstOre);
        //LOGWARN("Save TeamInfo<%lu> to DB", pstOre->GetUin());
        m_oOrerMap.erase(m_oOreMapIter++);
        m_oMineOrePool.Release(pstOre);
    }
    MinePlayer* pstPlayer = NULL;
    for (m_oPlayerMapIter = m_oPlayerMap.begin(); m_oPlayerMapIter != m_oPlayerMap.end();)
    {
        pstPlayer = m_oPlayerMapIter->second;
        this->_SavePlayer(pstPlayer);
        m_oPlayerMap.erase(m_oPlayerMapIter++);
        m_oMinePlayerPool.Release(pstPlayer);
    }
	
	m_oMineUidSet.Fini();
    return;
}

void MineDataMgr::Update(bool bIdle)
{
    CoDataFrame::Update();
    this->_WriteDirtyToDB();
	if (IsGetOreSwitchOpen())
	{
		GetOreFromDB();
	}
	m_oMineUidSet.Update(bIdle);
}




MineOre* MineDataMgr::GetMineOre(uint64_t ullUid)
{
    DataKey tmpKey;
    tmpKey.m_bType = DATA_TYPE_ORE;
    tmpKey.m_ullKey = ullUid;
    return (MineOre*)CoDataFrame::GetData((void*)&tmpKey);
}






MinePlayer* MineDataMgr::GetMinePlayer(uint64_t ullUin)
{
    DataKey tmpKey;
    tmpKey.m_bType = DATA_TYPE_PLAYER;
    tmpKey.m_ullKey = ullUin;
    void* poOut = NULL;
    int iRet = CoDataFrame::GetData((void*)&tmpKey, poOut);
    MinePlayer* poPlayer = (MinePlayer*)poOut;
    //数据库中没有,新建玩家的数据
    if (iRet != 0)
    {
        poPlayer = _GetNewPlayer();
        if (poPlayer)
        {
            poPlayer->Init(ullUin);
			this->_SavePlayer(poPlayer);
            _AddPlayerToMap(poPlayer);
			AddToTimeList(poPlayer);
        }
    }
    return poPlayer;
}


MinePlayer* MineDataMgr::GetMinePlayerNoCreate(uint64_t ullUin)
{
	DataKey tmpKey;
	tmpKey.m_bType = DATA_TYPE_PLAYER;
	tmpKey.m_ullKey = ullUin;

	return (MinePlayer*) CoDataFrame::GetData((void*)&tmpKey);
}

void MineDataMgr::_SaveOre(MineOre* pstOre)
{
    SSPKG& rstSsPkg = MineSvrMsgLayer::Instance().GetSsPkgData();
    rstSsPkg.m_stHead.m_wMsgId = SS_MSG_MINE_UPT_ORE_DATA_NTF;
    SS_PKG_MINE_UPT_ORE_DATA_NTF& rstSsNtf = rstSsPkg.m_stBody.m_stMineUptOreDataNtf;
    if (!pstOre->PackToData(rstSsNtf.m_astOreList[0]))
    {
        LOGERR("Save Ore<Uid=%lu>  pack failed ", pstOre->GetUid());
        return ;
    }
    rstSsNtf.m_bOreCount = 1;
    MineSvrMsgLayer::Instance().SendToMineDBSvr(rstSsPkg);
}

void MineDataMgr::_DelOre(MineOre * pstOre)
{
    SSPKG& rstSsPkg = MineSvrMsgLayer::Instance().GetSsPkgData();
    rstSsPkg.m_stHead.m_wMsgId = SS_MSG_MINE_DEL_ORE_DATA_NTF;
    SS_PKG_MINE_DEL_ORE_DATA_NTF& rstSsNtf = rstSsPkg.m_stBody.m_stMineDelOreDataNtf;
    rstSsNtf.m_bOreCount = 1;
    rstSsNtf.m_OreUidList[0] = pstOre->GetUid();
    MineSvrMsgLayer::Instance().SendToMineDBSvr(rstSsPkg);
}


void MineDataMgr::_SavePlayer(MinePlayer* pstPlayer)
{
    SSPKG& rstSsPkg = MineSvrMsgLayer::Instance().GetSsPkgData();
    rstSsPkg.m_stHead.m_wMsgId = SS_MSG_MINE_UPT_PLAYER_DATA_NTF;
    SS_PKG_MINE_UPT_PLAYER_DATA_NTF& rstSsNtf = rstSsPkg.m_stBody.m_stMineUptPlayerDataNtf;
    if (!pstPlayer->PackToData(rstSsNtf.m_astPlayerList[0]))
    {
        LOGERR("Save Player<Uin=%lu>  pack failed ", pstPlayer->GetUin());
        return;
    }
    rstSsNtf.m_bPlayerCount = 1;
    MineSvrMsgLayer::Instance().SendToMineDBSvr(rstSsPkg);
}

void MineDataMgr::_WriteDirtyToDB()
{
    TLISTNODE* pstPos = NULL;
    TLISTNODE* pstNext = NULL;
    TLISTNODE* pstHead = &(m_stOreDirtyListHead);

    MineOre* pstOre = NULL;

    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();
    if (m_dwOreDirtyListSize >= (uint32_t) m_pstConfig->m_iDirtyNumMax ||
        ullCurTime - m_ullOreLastWriteTimestamp >= m_dwOreWriteTimeVal)
    {
        m_ullOreLastWriteTimestamp = ullCurTime;

        TLIST_FOR_EACH_SAFE(pstPos, pstNext, pstHead)
        {
            pstOre = TLIST_ENTRY(pstPos, MineOre, m_stDirtyListNode);
            this->_SaveOre(pstOre);
            this->DelFromDirtyList(pstOre);
            this->AddToTimeList(pstOre);
        }
        TLIST_INIT(pstHead);
    }

	MinePlayer* pstPlayer = NULL;
	pstHead = &(m_stPlayerDirtyListHead);
	if (m_dwPlayerDirtyListSize >= (uint32_t)m_pstConfig->m_iDirtyNumMax ||
		ullCurTime - m_ullPlayerLastWriteTimestamp >= m_dwPlayerWriteTimeVal)
	{
		m_ullPlayerLastWriteTimestamp = ullCurTime;

		TLIST_FOR_EACH_SAFE(pstPos, pstNext, pstHead)
		{
			pstPlayer = TLIST_ENTRY(pstPos, MinePlayer, m_stDirtyListNode);
			this->_SavePlayer(pstPlayer);
			this->DelFromDirtyList(pstPlayer);
			this->AddToTimeList(pstPlayer);
		}
		TLIST_INIT(pstHead);
	}
}

MineOre* MineDataMgr::_GetNewOre()
{
	MineOre* pstOre = NULL;
    //mempool中有空余时
	if (m_oMineOrePool.CanGetNewNode()) //这个判断是避免Pool中的报错信息
	{
		pstOre = m_oMineOrePool.Get();
		if (pstOre)
		{
			return pstOre;
		}	
	}
    //mempool中没有空余时，需要置换，置换采用LRU算法，将最近最少使用的节点回写到数据库
    
	//当回写策略不当时,会出现m_stTimeListHead为空的情况,可以直接return或者回写DirtyList
	if (TLIST_IS_EMPTY(&m_stOreTimeListHead))
	{
		pstOre = container_of(TLIST_PREV(&m_stOreDirtyListHead), MineOre, m_stDirtyListNode);
		LOGERR("the strategy is error!!");
		LOGWARN("exchange MineOre node from DirtyListHead");
	}
	else
	{
		pstOre = container_of(TLIST_PREV(&m_stOreTimeListHead), MineOre, m_stTimeListNode);
		LOGWARN("exchange MineOre node from TimeListHead");
	}
	this->_SaveOre(pstOre);
	this->DelFromTimeList(pstOre);
	this->DelFromDirtyList(pstOre);
	this->_DelOreFromMap(pstOre);
    
    return pstOre;
}


MinePlayer* MineDataMgr::_GetNewPlayer()
{
	MinePlayer* pstPlayer = NULL; 
	//mempool中有空余时
	if (m_oMinePlayerPool.CanGetNewNode())	//这个判断是避免Pool中的报错信息
	{
		pstPlayer = m_oMinePlayerPool.Get();
		if (pstPlayer)
		{
			return pstPlayer;
		}
		
	}
	//mempool中没有空余时，需要置换，置换采用LRU算法，将最近最少使用的节点回写到数据库
	
	
	//当回写策略不当时,会出现m_stTimeListHead为空的情况,可以直接return或者回写DirtyList
	if (TLIST_IS_EMPTY(&m_stPlayerTimeListHead))
	{
		pstPlayer = container_of(TLIST_PREV(&m_stPlayerDirtyListHead), MinePlayer, m_stDirtyListNode);
		LOGERR("pstPlayer the strategy is error!!");
		LOGWARN("exchange MinePlayer node from DirtyListHead");
	}
	else
	{
		pstPlayer = container_of(TLIST_PREV(&m_stPlayerTimeListHead), MinePlayer, m_stTimeListNode);
		LOGWARN("exchange MinePlayer node from TimeListHead");
	}
	this->_SavePlayer(pstPlayer);
	this->DelFromDirtyList(pstPlayer);
	this->DelFromTimeList(pstPlayer);
	this->_DelPlayerToMap(pstPlayer);
	
	return pstPlayer;
}

void MineDataMgr::_AddOreToMap(MineOre* pstOre)
{
    m_oOrerMap.insert(Key2Ore_t::value_type(pstOre->GetUid(), pstOre));
    m_oOreTypeToAddrVectMap[pstOre->GetType()].push_back((size_t)pstOre);
	assert(pstOre->GetUid() != 0);
	m_oMineUidSet.Insert(pstOre->GetUid());
	if (pstOre->IsEmptyOre())
	{
		this->AddToEmptyList(pstOre);
	}
	this->AddOreNum(pstOre);
}

void MineDataMgr::_DelOreFromMap(MineOre* pstOre)
{
	if (pstOre->IsEmptyOre())
	{
		this->DelFromEmptyList(pstOre);
	}
	
	this->SubOreNum(pstOre);
    m_oOreMapIter = m_oOrerMap.find(pstOre->GetUid());
    
    if (m_oOreMapIter != m_oOrerMap.end())
    {
        m_oOrerMap.erase(m_oOreMapIter);
    }
    else
    {
        LOGERR("Del Uid<%lu> pstOre from map failed, not found", pstOre->GetUid());
    }
    vector<size_t>& oOreAddrVector = m_oOreTypeToAddrVectMap[pstOre->GetType()];
    vector<size_t>::iterator oOreAddrVectorIter = find(oOreAddrVector.begin(), oOreAddrVector.end(), (size_t)pstOre);
    if (oOreAddrVectorIter != oOreAddrVector.end())
    {
        oOreAddrVector.erase(oOreAddrVectorIter);
    }
    else
    {
        LOGERR("Del add<%lu> error", (size_t)pstOre);
    }
}



void MineDataMgr::_AddPlayerToMap(MinePlayer* pstPlayer)
{
    m_oPlayerMap.insert(Key2Player_t::value_type(pstPlayer->GetUin(), pstPlayer));

}

void MineDataMgr::_DelPlayerToMap(MinePlayer* pstPlayer)
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


void MineDataMgr::AddToDirtyList(MineOre* pstOre)
{
    TLISTNODE* pstNode = &pstOre->m_stDirtyListNode;
    if (!TLIST_IS_EMPTY(pstNode))
    {
        return;
    }
	if (pstOre->GetType() == ORE_TYPE_INVESTIGATE)
	{
		//调查矿 不参与管理
		return;
	}
    TLIST_INSERT_NEXT(&m_stOreDirtyListHead, pstNode);
    m_dwOreDirtyListSize++;
    //LOGWARN("TeamId<%lu> AddDirtyList size<%u>", pstOre->GetUin(), m_dwDirtyListSize);
}

void MineDataMgr::AddToDirtyList(MinePlayer* pstPlayer)
{
	TLISTNODE* pstNode = &pstPlayer->m_stDirtyListNode;
	if (!TLIST_IS_EMPTY(pstNode))
	{
		return;
	}
	TLIST_INSERT_NEXT(&m_stPlayerDirtyListHead, pstNode);
	m_dwPlayerDirtyListSize++;
	LOGWARN("Player<%lu> AddToDirtyList DirtySize<%u> TimeSize<%u>", pstPlayer->GetUin(), m_dwPlayerDirtyListSize, m_dwPlayerTimeListSize);
}

void MineDataMgr::MoveToTimeListFirst(MineOre * pstOre)
{
    TLISTNODE* pstNode = &pstOre->m_stTimeListNode;
    if (TLIST_IS_EMPTY(pstNode))
    {
        return;
    }
	if (pstOre->GetType() == ORE_TYPE_INVESTIGATE)
	{
		//调查矿 不参与管理
		return;
	}
    TLIST_DEL(pstNode);
    TLIST_INSERT_NEXT(&m_stOreTimeListHead, pstNode);
}


void MineDataMgr::MoveToTimeListFirst(MinePlayer* pstPlayer)
{
	TLISTNODE* pstNode = &pstPlayer->m_stTimeListNode;
	if (TLIST_IS_EMPTY(pstNode))
	{
		return;
	}

	TLIST_DEL(pstNode);
	TLIST_INSERT_NEXT(&m_stPlayerTimeListHead, pstNode);
}

void MineDataMgr::AddToTimeList(MineOre* pstOre)
{
    TLISTNODE* pstNode = &pstOre->m_stTimeListNode;
    if (!TLIST_IS_EMPTY(pstNode))
    {
        return;
    }
	if (pstOre->GetType() == ORE_TYPE_INVESTIGATE)
	{
		//调查矿 不参与管理
		return;
	}
	  
    TLIST_INSERT_NEXT(&m_stOreTimeListHead, pstNode);
    m_dwOreTimeListSize++;
    //LOGWARN("TeamId<%lu> AddTimeList size<%u>", pstOre->GetUin(), m_dwTimeListSize);
}


void MineDataMgr::AddToTimeList(MinePlayer* pstPlayer)
{
	TLISTNODE* pstNode = &pstPlayer->m_stTimeListNode;
	if (!TLIST_IS_EMPTY(pstNode))
	{
		return;
	}
	TLIST_INSERT_NEXT(&m_stPlayerTimeListHead, pstNode);
	m_dwPlayerTimeListSize++;
	LOGWARN("Player<%lu> AddToTimeList DirtySize<%u> TimeSize<%u>", pstPlayer->GetUin(), m_dwPlayerDirtyListSize, m_dwPlayerTimeListSize);

}

void MineDataMgr::DelFromDirtyList(MineOre* pstOre)
{
    TLISTNODE* pstNode = &pstOre->m_stDirtyListNode;
    if (TLIST_IS_EMPTY(pstNode))
    {
        return;
    }
    TLIST_DEL(pstNode);
    m_dwOreDirtyListSize--;
    //LOGWARN("TeamId<%lu> DelDirtyList size<%u>", pstOre->GetUin(), m_dwDirtyListSize);
}


void MineDataMgr::DelFromDirtyList(MinePlayer* pstPlayer)
{
	TLISTNODE* pstNode = &pstPlayer->m_stDirtyListNode;
	if (TLIST_IS_EMPTY(pstNode))
	{
		return;
	}
	TLIST_DEL(pstNode);
	m_dwPlayerDirtyListSize--;
	LOGWARN("Player<%lu> DelFromDirtyList DirtySize<%u> TimeSize<%u>", pstPlayer->GetUin(), m_dwPlayerDirtyListSize, m_dwPlayerTimeListSize);
}


void MineDataMgr::AddToEmptyList(MineOre* pstOre)
{
	TLISTNODE* pstNode = &pstOre->m_stEmptyListNode;
	if (!TLIST_IS_EMPTY(pstNode))
	{
		//在列表中
		return;
	}

	if (pstOre->GetType() == ORE_TYPE_NORMAL)
	{
		TLIST_INSERT_NEXT(&m_stNormalOreEmptyListHead, pstNode);
		m_iNormalOreEmptyNum++;
	}
	else if (pstOre->GetType() == ORE_TYPE_SUPPER)
	{
		TLIST_INSERT_NEXT(&m_stSupperOreEmptyListHead, pstNode);
		m_iSupperOreEmptyNum++;
	}
}


void MineDataMgr::DelFromEmptyList(MineOre* pstOre)
{
	TLISTNODE* pstNode = &pstOre->m_stEmptyListNode;
	if (TLIST_IS_EMPTY(pstNode))
	{
		//不在列表中
		return;
	}

	if (pstOre->GetType() == ORE_TYPE_NORMAL)
	{
		TLIST_DEL(pstNode);
		m_iNormalOreEmptyNum--;
	}
	else if (pstOre->GetType() == ORE_TYPE_SUPPER)
	{
		TLIST_DEL(pstNode);
		m_iSupperOreEmptyNum--;
	}
}

void MineDataMgr::DelFromTimeList(MineOre* pstOre)
{
    TLISTNODE* pstNode = &pstOre->m_stTimeListNode;
    if (TLIST_IS_EMPTY(pstNode))
    {
        return;
    }
    TLIST_DEL(pstNode);
    m_dwOreTimeListSize--;
    //LOGWARN("TeamId<%lu> DelTimeyList size<%u>", pstOre->GetUin(), m_dwTimeListSize);
}


void MineDataMgr::DelFromTimeList(MinePlayer* pstPlayer)
{
	TLISTNODE* pstNode = &pstPlayer->m_stTimeListNode;
	if (TLIST_IS_EMPTY(pstNode))
	{
		return;
	}
	TLIST_DEL(pstNode);
	m_dwPlayerTimeListSize--;
	LOGWARN("Player<%lu> DelFromTimeList DirtySize<%u> TimeSize<%u>", pstPlayer->GetUin(), m_dwPlayerDirtyListSize, m_dwPlayerTimeListSize);

}

bool MineDataMgr::IsInMem(void* pResult)
{
    if (!pResult)
    {
        return false;
    }
    DataResult* pstResult = (DataResult*)pResult;
    if (pstResult->m_bType == DATA_TYPE_ORE)    //kuang
    {
       return m_oOrerMap.find( ((DT_MINE_ORE_DATA*)pstResult->m_pstData)->m_ullOreUid ) != m_oOrerMap.end();
    }
    else if (pstResult->m_bType == DATA_TYPE_PLAYER)
    {
        return m_oPlayerMap.find(((DT_MINE_PLAYER_DATA*)pstResult->m_pstData)->m_ullUin) != m_oPlayerMap.end();
    }
    return false;

}

bool MineDataMgr::SaveInMem(void* pResult)
{
    if (!pResult)
    {
        return false;
    }
    DataResult* pstResult = (DataResult*)pResult;
    if (pstResult->m_bType == DATA_TYPE_ORE)
    {
        DT_MINE_ORE_DATA* poData = (DT_MINE_ORE_DATA*)pstResult->m_pstData;
        MineOre * pstOre = this->_GetNewOre();
        if (!pstOre)
        {
            return false;
        }
        if (!pstOre->InitFromDB(*poData))
        {
            return false;
        }
        this->AddToTimeList(pstOre);
        this->_AddOreToMap(pstOre);
        return true;
    }
    else if (pstResult->m_bType == DATA_TYPE_PLAYER)
    {
        DT_MINE_PLAYER_DATA* poData = (DT_MINE_PLAYER_DATA*)pstResult->m_pstData;
        MinePlayer * pstPlayer = this->_GetNewPlayer();
        if (!pstPlayer)
        {
            return false;
        }
        if (!pstPlayer->InitFromDB(*poData))
        {
            return false;
        }
        this->AddToTimeList(pstPlayer);
        this->_AddPlayerToMap(pstPlayer);
        return true;
    }

    return true;
}



void* MineDataMgr::_GetDataInMem(void* key)
{
    if (!key)
    {
        return NULL;
    }
    DataKey* pstKey = (DataKey*)key;
    if (pstKey->m_bType == DATA_TYPE_ORE)
    {
        m_oOreMapIter = m_oOrerMap.find(pstKey->m_ullKey);
        if (m_oOreMapIter == m_oOrerMap.end())
        {
            return NULL;
        }
        this->MoveToTimeListFirst(m_oOreMapIter->second);
        return (void*)(m_oOreMapIter->second);
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

CoGetDataAction* MineDataMgr::_CreateGetDataAction(void* key)
{
    if (!key)
    {
        return NULL;
    }
    DataKey* pstKey = (DataKey*)key;
    AsyncGetOreDataAction* poAction = m_oAsyncActionPool.Get();
    if (!poAction)
    {
        LOGERR("m_oAsyncActionPool.Get() failed key<%lu>, type<%hhu>", pstKey->m_ullKey, pstKey->m_bType);
        return NULL;
    }
    poAction->SetKey(*pstKey);
    return (CoGetDataAction*)poAction;
}

void MineDataMgr::_ReleaseGetDataAction(CoGetDataAction* poAction)
{
    AsyncGetOreDataAction* poGetDataAction = (AsyncGetOreDataAction*)poAction;
    m_oAsyncActionPool.Release(poGetDataAction);
}









uint64_t MineDataMgr::CreateOreUid()
{
    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();

    if (m_ullLastCreateUinTime != ullCurTime)
    {
        m_ullLastCreateUinTime = ullCurTime;
        m_dwSeq = 0;
    }
    else
    {
        if (++m_dwSeq >= 1024)
        {
            return 0;
        }
    }
    //ID生成算法
    // 64-20(时间戳,单位:秒)  |  10位(当前时间戳内的序号)   |  10位(服务器ID)

    uint64_t ullTimestamp = m_ullLastCreateUinTime;

    uint64_t ullUin = ullTimestamp << 20;
    ullUin |= m_dwSeq << 10;
    ullUin |= m_pstConfig->m_iSvrID;

    return ullUin;
}


MineOre* MineDataMgr::CreateOre(uint8_t bOreType, uint32_t dwOreId, uint32_t dwCreateNum)
{
	uint64_t Uid = 0;
    for (uint32_t i = 0; i < dwCreateNum; i++)
    {
        MineOre* pstOre = _GetNewOre();
        if (pstOre == NULL)
        {
            LOGERR("Get the new Data memory error!");
            return NULL;
        }
		Uid = CreateOreUid();
		if (Uid == 0)
		{
			LOGERR("CreateOreUid error!");
			return NULL;
		}
        pstOre->Init(Uid, bOreType, dwOreId);
        AddToTimeList(pstOre);
        _AddOreToMap(pstOre);

    }
    LOGRUN("CreateOre bOreType<%hhu> OreId<%u> Count<%u> OK!",bOreType, dwOreId, dwCreateNum);
    return NULL;
}





int MineDataMgr::RandomOre(uint64_t ullUin, uint32_t dwTeamLi, OUT uint8_t& rbNum, OUT DT_MINE_ORE_INFO* pstOreInfo)
{
	if (m_oOreTypeToAddrVectMap[ORE_TYPE_NORMAL].size() < MAX_MINE_EXPLORE_ORE_NUM 
		|| m_oOreTypeToAddrVectMap[ORE_TYPE_SUPPER].size() < MAX_MINE_EXPLORE_ORE_NUM
		|| m_oOreTypeToAddrVectMap[ORE_TYPE_INVESTIGATE].size() < MAX_MINE_EXPLORE_ORE_NUM)
	{
		//这种情况一般是 MineSvr宕机重启时,tbus中还缓存着探索的消息, MineSvr会先处理这个消息,然后才会处理MineDBSvr回复的
		//获取矿信息的消息, 这样就会导致矿的数量不够.

		//还有一种隐藏情况, 如果因节点不够,导致回写矿信息,如果连续多个都是同一种矿,可能也会造成数量不够(极小概率) 不处理
		LOGERR("Uin<%lu> random ore error! not enough ore", ullUin);
		return ERR_SYS;
	}
    for (rbNum = 0; rbNum < MAX_MINE_EXPLORE_ORE_NUM; rbNum++)
    {
		uint32_t dwRandom = CFakeRandom::Instance().Random(1, 100);
		uint8_t bTypeSelect = dwRandom < m_dwOreRateOfNormal ? ORE_TYPE_NORMAL :
			dwRandom < m_dwOreRateOfSupper ? ORE_TYPE_SUPPER : ORE_TYPE_INVESTIGATE;
		//bTypeSelect = dwRandom % 3 + 1;
		vector<size_t>& rAddrVector = m_oOreTypeToAddrVectMap[bTypeSelect];

        int iEnd = rAddrVector.size() - 1 - rbNum;

        int iIndex = CFakeRandom::Instance().Random(0, iEnd);
        if (rAddrVector[iIndex] == 0)
        {
            LOGERR("Uin<%lu> RandomOre error!", ullUin);
            return ERR_SYS;
        }
        size_t tmp = rAddrVector[iIndex];
		if ( ((MineOre*)tmp)->GetOwner() == ullUin )
		{
			//是自己的矿 替换成宝箱 
			iIndex = m_oOreTypeToAddrVectMap[ORE_TYPE_INVESTIGATE].size() - 1 - rbNum;
			((MineOre*)m_oOreTypeToAddrVectMap[ORE_TYPE_INVESTIGATE][iIndex])->GetOreInfo(pstOreInfo[rbNum]);
			LOGWARN("Uin<%lu> random own ore, replace investigate ore", ullUin);
		}
		else if (((MineOre*)tmp)->GetOwnerLi() > dwTeamLi)
		{
			//战力比较 高
			MineOre* pstMineOre = GetEmptyOre(bTypeSelect, rbNum);
			if (!pstMineOre)
			{
				LOGERR("Uin<%lu> RandomOre error for GetEmptyOre!", ullUin);
				return ERR_SYS;
			}
			assert(pstMineOre->GetUid() != 0);
			//检查 是否已随机出来这个矿了
			bool bIsExplored = false;
			for (int j = 0 ; j < rbNum; j++)
			{
				if (pstOreInfo[j].m_ullUid == pstMineOre->GetUid())
				{
					bIsExplored = true;
					break;
				}
			}
			if (bIsExplored)
			{
				//已探索出来,给宝箱吧
				iIndex = m_oOreTypeToAddrVectMap[ORE_TYPE_INVESTIGATE].size() - 1 - rbNum;
				((MineOre*)m_oOreTypeToAddrVectMap[ORE_TYPE_INVESTIGATE][iIndex])->GetOreInfo(pstOreInfo[rbNum]);
				LOGWARN("Uin<%lu> RandomOre TeamLi is higher, and the new empty ore is Explored", ullUin);
			}
			else
			{
				pstMineOre->GetOreInfo(pstOreInfo[rbNum]);
				LOGWARN("Uin<%lu> RandomOre TeamLi is higher, get the new empty ore", ullUin);
			}
		}
		else
		{
			rAddrVector[iIndex] = rAddrVector[iEnd];
			rAddrVector[iEnd] = tmp;
			((MineOre*)tmp)->GetOreInfo(pstOreInfo[rbNum]);
		}

    }


    return ERR_NONE;
}



void MineDataMgr::AddOreNum(MineOre* pstMineOre)
{
	if (pstMineOre->GetType() == ORE_TYPE_NORMAL)
	{

		m_iNormalOreTotalNum++;
	}
	else if (pstMineOre->GetType() == ORE_TYPE_SUPPER)
	{

		m_iSupperOreTotalNum++;
	}
	else if (pstMineOre->GetType() == ORE_TYPE_INVESTIGATE)
	{
		m_iInvestigateNum++;
	}
}


void MineDataMgr::SubOreNum(MineOre* pstMineOre)
{
	if (pstMineOre->GetType() == ORE_TYPE_NORMAL)
	{
		m_iNormalOreTotalNum--;
	}
	else if (pstMineOre->GetType() == ORE_TYPE_SUPPER)
	{

		m_iSupperOreTotalNum--;
	}
	else if (pstMineOre->GetType() == ORE_TYPE_INVESTIGATE)
	{
		m_iInvestigateNum--;
	}
}

void MineDataMgr::SubEmptyNum(MineOre* pstMineOre)
{
	if (pstMineOre->GetType() == ORE_TYPE_NORMAL)
	{
		m_iNormalOreEmptyNum--;
	}
	else if (pstMineOre->GetType() == ORE_TYPE_SUPPER)
	{
		m_iSupperOreEmptyNum--;

	}
	
}

void MineDataMgr::AddEmptyNum(MineOre* pstMineOre)
{
	if (pstMineOre->GetType() == ORE_TYPE_NORMAL)
	{
		m_iNormalOreEmptyNum++;
	}
	else if (pstMineOre->GetType() == ORE_TYPE_SUPPER)
	{
		m_iSupperOreEmptyNum++;

	}
	
}

void MineDataMgr::GetOreFromDB()
{
	PKGMETA::SSPKG& rstSsPkg = MineSvrMsgLayer::Instance().GetSsPkgData();
	rstSsPkg.m_stHead.m_wMsgId = PKGMETA::SS_MSG_MINE_GET_ORE_DATA_REQ;
	PKGMETA::SS_PKG_MINE_GET_ORE_DATA_REQ& rstReq = rstSsPkg.m_stBody.m_stMineGetOreDataReq;
	rstReq.m_bOreCount = 0;
	rstReq.m_ullTokenId = 0;
	for (; !m_oMineUidSet.SettleIsEnd() && rstReq.m_bOreCount < MAX_MINE_BATCH_DEAL_ORE_DATA_NUM; m_oMineUidSet.SettleNext())
	{
		rstReq.m_OreUidList[rstReq.m_bOreCount] = m_oMineUidSet.SettleGetCur();
		rstReq.m_bOreCount++;
	}
	if (rstReq.m_bOreCount > 0)
	{
		MineSvrMsgLayer::Instance().SendToMineDBSvr(rstSsPkg);
	}
	if (m_oMineUidSet.SettleIsEnd())
	{
		SetGetOreSwitch(false);
	}
}

MineOre* MineDataMgr::GetEmptyOre(uint8_t bOreType, uint8_t rbNum)
{
	MineOre* pstOre = NULL;
	if (bOreType == ORE_TYPE_SUPPER)
	{
		if (!TLIST_IS_EMPTY(&m_stSupperOreEmptyListHead))
		{
			pstOre = container_of(TLIST_PREV(&m_stSupperOreEmptyListHead), MineOre, m_stEmptyListNode);
			TLISTNODE* pstNode = &pstOre->m_stEmptyListNode;
			TLIST_DEL(pstNode);
			TLIST_INSERT_NEXT(&m_stSupperOreEmptyListHead, pstNode);
			LOGWARN("Get a new empty SupperType ore<%lu> ", pstOre->GetUid());
			return pstOre;
		}
	}
	else if (bOreType == ORE_TYPE_NORMAL)
	{
		if (!TLIST_IS_EMPTY(&m_stNormalOreEmptyListHead))
		{
			pstOre = container_of(TLIST_PREV(&m_stNormalOreEmptyListHead), MineOre, m_stEmptyListNode);
			TLISTNODE* pstNode = &pstOre->m_stEmptyListNode;
			TLIST_DEL(pstNode);
			TLIST_INSERT_NEXT(&m_stNormalOreEmptyListHead, pstNode);
			LOGWARN("Get a new empty NormalType ore<%lu> ", pstOre->GetUid());
			return pstOre;
		}
	}
	//都没有,给宝箱
	LOGWARN("not empty ore, use InvestigateOre<%lu>", pstOre->GetUid());
	int iIndex = m_oOreTypeToAddrVectMap[ORE_TYPE_INVESTIGATE].size() - 1 - rbNum;
	return (MineOre*)m_oOreTypeToAddrVectMap[ORE_TYPE_INVESTIGATE][iIndex];
}

