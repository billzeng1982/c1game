#include "FakeRandom.h"
#include "GameTime.h"
#include "../gamedata/GameDataMgr.h"
#include "CloneBattleMgr.h"

using namespace PKGMETA;



bool CloneBattleMgr::Init(CLONEBATTLESVRCFG* pstConfig)
{
    if (!pstConfig)
    {
        LOGERR("pstConfig is null");
        return false;
    }
    m_pstConfig = pstConfig;

    //初始化CoDataFrame类
    this->BaseInit(300);
    this->SetCoroutineEnv(CloneBattleSvrMsgLayer::Instance().GetCoroutineEnv());

    
    //初始化内存池
    m_dwCurCacheSize = 0;
    m_dwMaxCacheSize = m_pstConfig->m_dwMaxTeamCacheNum;
    if (m_oTeamPool.CreatePool(m_pstConfig->m_dwMaxTeamCacheNum) < 0)
    {
        LOGERR("Create m_oTeamPool pool[num=%u] failed.", m_pstConfig->m_dwMaxTeamCacheNum);
        return false;
    }
    m_oTeamPool.RegisterSlicedIter(&m_oTeamPoolIter);
    m_oAsyncActionPool.Init(30, 10, 500);
    //初始化时间链表
    TLIST_INIT(&m_stTimeListHead);
    m_dwTimeListSize = 0;
    //初始化脏数据相关
    TLIST_INIT(&m_stDirtyListHead);
    m_dwDirtyListSize = 0;
    m_dwWriteTimeVal = m_pstConfig->m_iUpdateInterval;
    m_ullLastWriteTimestamp = CGameTime::Instance().GetCurrSecond();


    RESBASIC* poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(COMMON_UPDATE_TIME);
    assert(poResBasic);
    m_iUptHour = (int) poResBasic->m_para[0];

    poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(CLONE_GENERAL_BASIC);
    assert(poResBasic);
    m_dwTeamFinishNum = (uint32_t)poResBasic->m_para[0];
    m_dwMateFinishNum = (uint32_t)poResBasic->m_para[3];
    m_ullLastUptTimeMs = CGameTime::Instance().GetSecOfCycleHourInCurrday(m_iUptHour) * 1000; //毫秒
    poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(CLONE_GENERAL_BOSS_ACT_TIME);
    assert(poResBasic);
    char szDataStr[255] = { 0 };
    snprintf(szDataStr, 255, "%d/%d/%d %d:00:00", (int)poResBasic->m_para[0],
        (int)poResBasic->m_para[1], (int)poResBasic->m_para[2], m_iUptHour);
    time_t ullBeginTime = 0;
    if (CGameTime::Instance().ConvStrToTime(szDataStr, ullBeginTime) != 0 )
    {
        LOGERR("convert the TimeStr to time_t error");
        return false;
    }
    m_ullActBeginTime = ullBeginTime;
    m_ullActEndTime = ullBeginTime + (int)poResBasic->m_para[4] * SECONDS_OF_HOUR;

    //初始Boss数据
    RandBoss();


//     m_oBossInfo.m_BossId[0] = 1;
//     m_oBossInfo.m_BossId[1] = 2;
//     m_oBossInfo.m_BossId[2] = 3;
//     m_oBossInfo.m_BossId[3] = 4;
//     m_oBossInfo.m_BossId[4] = 5;
//     m_oBossInfo.m_BossId[5] = 6;
    SendSysInfo();

    LOGRUN("Act Begin<%lu> End<%lu>, LastUptTimeMs<%lu> BossId<%u,%u,%u,%u,%u,%u>",
        m_ullActBeginTime, m_ullActEndTime, m_ullLastUptTimeMs, m_oBossInfo.m_BossId[0], m_oBossInfo.m_BossId[1],
        m_oBossInfo.m_BossId[2], m_oBossInfo.m_BossId[3], m_oBossInfo.m_BossId[4], m_oBossInfo.m_BossId[5]);
    return true;
}

void CloneBattleMgr::Fini()
{
    CloneBattleTeam* pstTeamInfo = NULL;
    int i = 0;
    for (m_oTeamMapIter = m_oTeamMap.begin(); m_oTeamMapIter != m_oTeamMap.end(); m_oTeamMapIter++, i++)
    {
        pstTeamInfo = m_oTeamMapIter->second;
        this->_SaveTeamInfo(pstTeamInfo);
        LOGWARN("Save TeamInfo<%lu> to DB", pstTeamInfo->GetTeamId());
    }
    m_oTeamMap.clear();
    m_oTeamPool.DestroyPool();
    return;
}

void CloneBattleMgr::Update(bool bIdle)
{
    CoDataFrame::Update();
    this->_WriteDirtyToDB();
    this->UpdateToDissolve(bIdle);

}

void CloneBattleMgr::UpdateToDissolve(bool bIdle)
{
    uint64_t ullUpdateTmMs = 0;
    if (!CGameTime::Instance().IsNeedUpdateByHour(m_ullLastUptTimeMs, m_iUptHour, ullUpdateTmMs))
    {
        return;
        
    }
    m_ullLastUptTimeMs = ullUpdateTmMs;
    SSPKG& rstSsPkg = CloneBattleSvrMsgLayer::Instance().GetSsPkgData();
    rstSsPkg.m_stHead.m_wMsgId = SS_MSG_CLONE_BATTLE_DEL_DATA_NTF;
    SS_PKG_CLONE_BATTLE_DEL_DATA_NTF& rstSsNtf = rstSsPkg.m_stBody.m_stCloneBattleDelDataNtf;
    rstSsNtf.m_dwCount = 0;
    
    //解散队伍
    CloneBattleTeam* pstTeamInfo = NULL;
    //int iCheckNum = bIdle ? 200 : 100;
    for (m_oTeamPoolIter.Begin(); !m_oTeamPoolIter.IsEnd();  m_oTeamPoolIter.Next())
    {
        pstTeamInfo = m_oTeamPoolIter.CurrItem();
        //过期解散
        if (pstTeamInfo->GetCreateTime() * 1000 < m_ullLastUptTimeMs)
        {
            this->DissolveTeam(pstTeamInfo, true, false);
        }
        rstSsNtf.m_TeamIdList[rstSsNtf.m_dwCount++] = pstTeamInfo->GetTeamId();
        if (rstSsNtf.m_dwCount >= 50)
        {
            CloneBattleSvrMsgLayer::Instance().SendToMiscSvr(rstSsPkg);
            rstSsNtf.m_dwCount = 0;
        }
    }
    if (rstSsNtf.m_dwCount > 0)
    {
        CloneBattleSvrMsgLayer::Instance().SendToMiscSvr(rstSsPkg);
    }
    RandBoss();
    SendSysInfo();
    LOGRUN("Update::Act Begin<%lu> End<%lu>, LastUptTimeMs<%lu> BossId<%u,%u,%u,%u,%u,%u>",
        m_ullActBeginTime, m_ullActEndTime, m_ullLastUptTimeMs, m_oBossInfo.m_BossId[0], m_oBossInfo.m_BossId[1],
        m_oBossInfo.m_BossId[2], m_oBossInfo.m_BossId[3], m_oBossInfo.m_BossId[4], m_oBossInfo.m_BossId[5]);
}



CloneBattleTeam* CloneBattleMgr::GetTeamInfo(uint64_t ullTeamId)
{
    return (CloneBattleTeam*)CoDataFrame::GetData((void*)&ullTeamId);
}

void CloneBattleMgr::_SaveTeamInfo(CloneBattleTeam* pstTeamInfo)
{
    SSPKG& rstSsPkg = CloneBattleSvrMsgLayer::Instance().GetSsPkgData();
    rstSsPkg.m_stHead.m_wMsgId = SS_MSG_CLONE_BATTLE_UPT_DATA_NTF;
    SS_PKG_CLONE_BATTLE_UPT_DATA_NTF& rstSsNtf = rstSsPkg.m_stBody.m_stCloneBattleUptDataNtf;

    if (!pstTeamInfo->PackToData(rstSsNtf.m_stCloneBattleData))
    {
        LOGERR("Save Uin<%lu> MailBox pack failed ", pstTeamInfo->GetTeamId());
        return ;
    }
    CloneBattleSvrMsgLayer::Instance().SendToMiscSvr(rstSsPkg);
}

void CloneBattleMgr::_DelTeamInfo(CloneBattleTeam * pstTeamInfo)
{
    SSPKG& rstSsPkg = CloneBattleSvrMsgLayer::Instance().GetSsPkgData();
    rstSsPkg.m_stHead.m_wMsgId = SS_MSG_CLONE_BATTLE_DEL_DATA_NTF;
    SS_PKG_CLONE_BATTLE_DEL_DATA_NTF& rstSsNtf = rstSsPkg.m_stBody.m_stCloneBattleDelDataNtf;
    rstSsNtf.m_dwCount = 1;
    rstSsNtf.m_TeamIdList[0] = pstTeamInfo->GetTeamId();
    CloneBattleSvrMsgLayer::Instance().SendToMiscSvr(rstSsPkg);
}

void CloneBattleMgr::_WriteDirtyToDB()
{
    TLISTNODE* pstPos = NULL;
    TLISTNODE* pstNext = NULL;
    TLISTNODE* pstHead = &(m_stDirtyListHead);

    CloneBattleTeam* pstTeamInfo = NULL;

    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();

    if (m_dwDirtyListSize >= (uint32_t) m_pstConfig->m_iDirtyNumMax ||
        ullCurTime - m_ullLastWriteTimestamp >= m_dwWriteTimeVal)
    {
        m_ullLastWriteTimestamp = ullCurTime;

        TLIST_FOR_EACH_SAFE(pstPos, pstNext, pstHead)
        {
            pstTeamInfo = TLIST_ENTRY(pstPos, CloneBattleTeam, m_stDirtyListNode);
            this->_SaveTeamInfo(pstTeamInfo);
            this->DelFromDirtyList(pstTeamInfo);
            this->AddToTimeList(pstTeamInfo);
        }
        TLIST_INIT(pstHead);
    }
}

CloneBattleTeam* CloneBattleMgr::_GetNewTeamInfo()
{
    CloneBattleTeam* pstTeamInfo = NULL;
    //mempool中有空余时
    if (m_oTeamPool.GetFreeNum() > 0)
    {
        pstTeamInfo = m_oTeamPool.NewData();
        if (!pstTeamInfo)
        {
            LOGERR("pstTeamInfo is NULL, get pstTeamInfo from pool failed");
            return NULL;
        }
    }
    //mempool中没有空余时，需要置换，置换采用LRU算法，将最近最少使用的节点回写到数据库
    else
    {
        //当回写策略不当时,会出现m_stTimeListHead为空的情况,可以直接return或者回写DirtyList
        if (TLIST_IS_EMPTY(&m_stTimeListHead))
        {
            pstTeamInfo = container_of(TLIST_PREV(&m_stDirtyListHead), CloneBattleTeam, m_stTimeListNode);
            LOGERR("the strategy is error!!");
        }
        else
        {
            pstTeamInfo = container_of(TLIST_PREV(&m_stTimeListHead), CloneBattleTeam, m_stTimeListNode);
        }
        this->_SaveTeamInfo(pstTeamInfo);
        this->_DelTeamInfoFromMap(pstTeamInfo);
        this->DelFromTimeList(pstTeamInfo);
        this->DelFromDirtyList(pstTeamInfo);
    }
    return pstTeamInfo;
}

void CloneBattleMgr::_AddTeamInfoToMap(CloneBattleTeam* pstTeamInfo)
{
    m_oTeamMap.insert(Id2Team_t::value_type(pstTeamInfo->GetTeamId(), pstTeamInfo));
    m_arrMatch[pstTeamInfo->GetBossType()].Insert(pstTeamInfo);
}

void CloneBattleMgr::_DelTeamInfoFromMap(CloneBattleTeam* pstTeamInfo)
{
    m_oTeamMapIter = m_oTeamMap.find(pstTeamInfo->GetTeamId());
    if (m_oTeamMapIter != m_oTeamMap.end())
    {
        m_oTeamMap.erase(m_oTeamMapIter);
    }
    else
    {
        LOGERR("Del Id(%lu) pstTeamInfo from map failed, not found", pstTeamInfo->GetTeamId());
    }
    m_arrMatch[pstTeamInfo->GetBossType()].Delete(pstTeamInfo);
}



void CloneBattleMgr::AddToDirtyList(CloneBattleTeam* pstTeamInfo)
{
    TLISTNODE* pstNode = &pstTeamInfo->m_stDirtyListNode;
    if (!TLIST_IS_EMPTY(pstNode))
    {
        return;
    }
    TLIST_INSERT_NEXT(&m_stDirtyListHead, pstNode);
    m_dwDirtyListSize++;
    //LOGWARN("TeamId<%lu> AddDirtyList size<%u>", pstTeamInfo->GetTeamId(), m_dwDirtyListSize);
}

void CloneBattleMgr::MoveToTimeListFirst(CloneBattleTeam * pstTeamInfo)
{
    TLISTNODE* pstNode = &pstTeamInfo->m_stTimeListNode;
    if (TLIST_IS_EMPTY(pstNode))
    {
        return;
    }
    TLIST_DEL(pstNode);
    TLIST_INSERT_NEXT(&m_stTimeListHead, pstNode);
}

void CloneBattleMgr::AddToTimeList(CloneBattleTeam* pstTeamInfo)
{
    TLISTNODE* pstNode = &pstTeamInfo->m_stTimeListNode;
    if (!TLIST_IS_EMPTY(pstNode))
    {
        return;
    }

    TLIST_INSERT_NEXT(&m_stTimeListHead, pstNode);
    m_dwTimeListSize++;
    //LOGWARN("TeamId<%lu> AddTimeList size<%u>", pstTeamInfo->GetTeamId(), m_dwTimeListSize);
}

void CloneBattleMgr::DelFromDirtyList(CloneBattleTeam* pstTeamInfo)
{
    TLISTNODE* pstNode = &pstTeamInfo->m_stDirtyListNode;
    if (TLIST_IS_EMPTY(pstNode))
    {
        return;
    }
    TLIST_DEL(pstNode);
    m_dwDirtyListSize--;
    //LOGWARN("TeamId<%lu> DelDirtyList size<%u>", pstTeamInfo->GetTeamId(), m_dwDirtyListSize);
}


void CloneBattleMgr::DelFromTimeList(CloneBattleTeam* pstTeamInfo)
{
    TLISTNODE* pstNode = &pstTeamInfo->m_stTimeListNode;
    if (TLIST_IS_EMPTY(pstNode))
    {
        return;
    }
    TLIST_DEL(pstNode);
    m_dwTimeListSize--;
    //LOGWARN("TeamId<%lu> DelTimeyList size<%u>", pstTeamInfo->GetTeamId(), m_dwTimeListSize);
}

bool CloneBattleMgr::IsInMem(void* pResult)
{
    if (!pResult)
    {
        return false;
    }
    DT_CLONE_BATTLE_TEAM_DATA* poData = (DT_CLONE_BATTLE_TEAM_DATA*)pResult;

    return m_oTeamMap.find(poData->m_stBaseInfo.m_ullId) != m_oTeamMap.end();
}

bool CloneBattleMgr::SaveInMem(void* pResult)
{
    if (!pResult)
    {
        return false;
    }
    DT_CLONE_BATTLE_TEAM_DATA* poData = (DT_CLONE_BATTLE_TEAM_DATA*)pResult;
    CloneBattleTeam * pstTeamInfo = this->_GetNewTeamInfo();
    if (!pstTeamInfo)
    {
        return false;
    }
    if (!pstTeamInfo->InitFromDB(*poData))
    {
        return false;
    }
    this->AddToTimeList(pstTeamInfo);
    this->AddToMatchList(pstTeamInfo);
    this->_AddTeamInfoToMap(pstTeamInfo);
    return true;
}


void CloneBattleMgr::GetBossInfo(DT_CLONE_BATTLE_BOSS_INFO& rstBossInfo)
{
    memcpy(&rstBossInfo, &m_oBossInfo, sizeof(rstBossInfo));
}

void* CloneBattleMgr::_GetDataInMem(void* key)
{
    if (!key)
    {
        return NULL;
    }
    uint64_t ullTeamId = *(uint64_t*)key;
    m_oTeamMapIter = m_oTeamMap.find(ullTeamId);
    if (m_oTeamMapIter == m_oTeamMap.end())
    {
        return NULL;
    }
    this->MoveToTimeListFirst(m_oTeamMapIter->second);
    return (void*)(m_oTeamMapIter->second);
}

CoGetDataAction* CloneBattleMgr::_CreateGetDataAction(void* key)
{
    if (!key)
    {
        return NULL;
    }
    AsyncGetDataAction* poAction = m_oAsyncActionPool.Get();
    if (!poAction)
    {
        LOGERR("m_oAsyncActionPool.Get() failed key<%lu>", *(uint64_t*)key);
        return NULL;
    }
    poAction->SetTeamId(*(uint64_t*)key);
    return (CoGetDataAction*)poAction;
}

void CloneBattleMgr::_ReleaseGetDataAction(CoGetDataAction* poAction)
{
    AsyncGetDataAction* poGetDataAction = (AsyncGetDataAction*)poAction;
    m_oAsyncActionPool.Release(poGetDataAction);
}


void CloneBattleMgr::RandBoss()
{
    CFakeRandom::Instance().SetSeed((uint32_t)(m_ullLastUptTimeMs * 0.001));
    RESCLONEBATTLEPOOL* poResPool = NULL; 
    int iBossIndex = 0;
    for (int i = 0; i < MAX_NUM_CLONE_BATTLE_BOSS; i++)
    {
        poResPool = CGameDataMgr::Instance().GetResCloneBattlePoolMgr().GetResByPos(i);
        assert(poResPool);
        assert(poResPool->m_bGeneralCount >= 1);
        iBossIndex = CFakeRandom::Instance().Random(1, poResPool->m_bGeneralCount) - 1;
        m_oBossInfo.m_BossId[i] = poResPool->m_generalList[iBossIndex];
    }

    //再设置一次,防止MAX_NUM_CLONE_BATTLE_BOSS 变化引起结果不同
    CFakeRandom::Instance().SetSeed((uint32_t)(m_ullLastUptTimeMs * 0.001 + 1));
    if (CGameTime::Instance().IsContainCur(m_ullActBeginTime, m_ullActEndTime))
    {
        poResPool = CGameDataMgr::Instance().GetResCloneBattlePoolMgr().Find(CONST_HIDE_POOL_ID);
        assert(poResPool);
        assert(poResPool->m_bGeneralCount >= 1);
        iBossIndex = CFakeRandom::Instance().Random(1, poResPool->m_bGeneralCount) - 1;
        m_oBossInfo.m_BossId[0] = poResPool->m_generalList[iBossIndex];
    }
}

CloneBattleTeam* CloneBattleMgr::CreateTeam(SS_PKG_CLONE_BATTLE_JOIN_TEAM_REQ& rstReq)
{
    CloneBattleTeam * pstTeamInfo = this->_GetNewTeamInfo();
    if (!pstTeamInfo)
    {
        return NULL;
    }
    pstTeamInfo->InitTeamInfo(CreateTeamId(), rstReq.m_bBossType, m_oBossInfo.m_BossId[rstReq.m_bBossType], rstReq.m_stMateInfo);
    LOGRUN("Uin<%lu> create a team<%lu>", rstReq.m_stMateInfo.m_ullUin, pstTeamInfo->GetTeamId());
    
    this->_AddTeamInfoToMap(pstTeamInfo);
    this->AddToTimeList(pstTeamInfo);
    this->AddToMatchList(pstTeamInfo);
    //保存
    this->_SaveTeamInfo(pstTeamInfo);
    
    return pstTeamInfo;
}


CloneBattleTeam* CloneBattleMgr::MatchTeam(uint64_t ullUin, SS_PKG_CLONE_BATTLE_JOIN_TEAM_REQ& rstReq, int& rRet)
{

    CloneBattleMatch& rstMatch = m_arrMatch[rstReq.m_bBossType];
    CloneBattleTeam* poTeamInfo = rstMatch.Match(rstReq.m_stMateInfo.m_stTroopInfo.m_stGeneralInfo.m_dwLi);
    if (!poTeamInfo)
    {
		poTeamInfo = CreateTeam(rstReq);
		if (!poTeamInfo)
		{
			LOGERR("Uin<%lu> create a team error", ullUin);
			rRet = ERR_SYS;
			return NULL;
		}
        return poTeamInfo;
    }
	rRet = poTeamInfo->AddMateInfo(rstReq.m_stMateInfo);
    if (rRet != 0)
    {
		LOGERR("Uin<%lu> add to team<%lu> error<%d>", ullUin, poTeamInfo->GetTeamId(),rRet);
        return NULL;
    }
    //满了从匹配池中删除
    if (poTeamInfo->IsFull())
    {
        CloneBattleMgr::Instance().DelFromMatchList(poTeamInfo);
    }
    return poTeamInfo;
    
}

int CloneBattleMgr::DissolveTeam(CloneBattleTeam * pstTeamInfo, bool bBroadcast, bool bSendDelTag)
{
    if (bBroadcast)
    {
        pstTeamInfo->BroadcastMate(CLONE_BATTLE_NTF_TYPE_TEAM_DEL);
    }
    if (bSendDelTag)
    {
        this->_DelTeamInfo(pstTeamInfo);
    }
    
    this->_DelTeamInfoFromMap(pstTeamInfo);
    this->DelFromTimeList(pstTeamInfo);
    this->DelFromDirtyList(pstTeamInfo);
    m_oTeamPool.DeleteData(pstTeamInfo);
    return 0;
}




uint64_t CloneBattleMgr::CreateTeamId()
{
    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();

    if (m_ullLastCreateUinTime != ullCurTime)
    {
        m_ullLastCreateUinTime = ullCurTime;
        m_dwSeq = 0;
    }
    else
    {
        if (++m_dwSeq >= 200)
        {
            return 0;
        }
    }

    //ID生成算法
    // 26位(时间戳,单位:秒)  |  8位(当前时间戳内的序号)   |  10位(服务器ID)

    uint64_t ullTimestamp = m_ullLastCreateUinTime;

    uint64_t ullUin = ullTimestamp << 18;
    ullUin |= m_dwSeq << 10;
    ullUin |= m_pstConfig->m_iSvrID;

    return ullUin;
}

void CloneBattleMgr::SendSysInfo()
{
    //下发通知,离线玩家在登录时刷新重置数据
    SSPKG& rstSsPkg = CloneBattleSvrMsgLayer::Instance().GetSsPkg();
    rstSsPkg.m_stHead.m_wMsgId = SS_MSG_CLONE_BATTLE_UPT_SYSINFO_NTF;
    SS_PKG_CLONE_BATTLE_UPT_SYSINFO_NTF& rstNtf = rstSsPkg.m_stBody.m_stCloneBattleUptSysInfoNtf;
    this->GetBossInfo(rstNtf.m_stBossInfo);
    rstNtf.m_ullLastUptTimeMs = m_ullLastUptTimeMs;
    CloneBattleSvrMsgLayer::Instance().SendToZoneSvr(rstSsPkg);
}


void CloneBattleMgr::AddToMatchList(CloneBattleTeam* pstTeam)
{
    if (pstTeam->IsFull() || pstTeam->GetMatchType() == CONST_MATCH_TYPE_QUICK_OFF)
    {
        return;
    }
    m_arrMatch[pstTeam->GetBossType()].Insert(pstTeam);
}


void CloneBattleMgr::DelFromMatchList(CloneBattleTeam* pstTeam)
{
//     if (pstTeam->IsFull())
//     {
//         return;
//     }
    m_arrMatch[pstTeam->GetBossType()].Delete(pstTeam);
}


































