#include "MatchBase.h"
#include <functional>
#include "GameTime.h"
#include "FakeRandom.h"
#include "../framework/GameObjectPool.h"
#include "../framework/MatchSvrMsgLayer.h"
#include "../module/MatchTimer.h"
#include "../module/MatchInfo.h"

using namespace PKGMETA;

MatchBase::MatchBase()
{}

MatchBase::~MatchBase()
{
    MapKey2Info_t::iterator iterScore = m_oMapScore.begin();
    for (; iterScore != m_oMapScore.end(); iterScore++)
    {
        if (iterScore->second)
        {
            RELEASE_GAMEOBJECT(iterScore->second);
        }
    }
    m_oMapScore.clear();
    m_oMapSection.clear();

    return;
}


void MatchBase::ReleaseTimer(GameTimer* pTimer)
{
    if (!pTimer)
    {
        LOGERR("pTimer is NULL");
        return;
    }

    MatchTimer* pMatchTimer = (MatchTimer*)pTimer;
    if (!pMatchTimer->m_poInfo)
    {
        LOGERR("pMatchTimer->m_poInfo is null");
        return;
    }

    if (!pMatchTimer->m_poInfo->m_bIsMatched)
    {
        // 超时通知客户端取消匹配
        SSPKG stSsPkg;
        stSsPkg.m_stHead.m_wMsgId = SS_MSG_MATCH_CANCEL_RSP;
        stSsPkg.m_stHead.m_ullReservId = (uint64_t)pMatchTimer->m_poInfo->m_stPlayerInfo.m_iZoneSvrId;
        SS_PKG_MATCH_CANCEL_RSP& rstSsPkgBodyRsp = stSsPkg.m_stBody.m_stMatchCancelRsp;

        DT_FIGHT_PLAYER_INFO& rstPlayerInfo = pMatchTimer->m_poInfo->m_stPlayerInfo;
        int iRet = pMatchTimer->m_pstMgr->MatchCancelHandle(rstPlayerInfo.m_ullUin, rstPlayerInfo.m_stMatchInfo.m_dwEfficiency);
        rstSsPkgBodyRsp.m_nErrNo = (int16_t)iRet;
        rstSsPkgBodyRsp.m_ullUin = rstPlayerInfo.m_ullUin;
        rstSsPkgBodyRsp.m_bMatchType = pMatchTimer->m_pstMgr->m_bMatchType;
        MatchSvrMsgLayer::Instance().SendToZoneSvr(stSsPkg);
        LOGRUN("MatchCancelReq_SS Uin<%lu> MatchType<%d> ret<%d>", rstPlayerInfo.m_ullUin, pMatchTimer->m_pstMgr->m_bMatchType, iRet);
    }

    // 释放MatchInfo
    _ReleaseMatchInfo(pMatchTimer->m_poInfo, pMatchTimer->m_pstMgr);

    // 释放Timer
    LOGRUN("release match timer, id=%u", pTimer->GetObjID());
    RELEASE_GAMEOBJECT(pTimer);

    return;
}


void MatchBase::_ReleaseMatchInfo(MatchInfo* pstInfo, MatchBase* pstMgr)
{
    if (!pstInfo || !pstMgr)
    {
        LOGERR("pMatchTimer->m_poInfo is null");
        return;
    }
    MapKey2Info_t& rstMap = pstMgr->m_oMapScore;
    MapKey2Info_t::iterator iter = rstMap.find(pstInfo->m_stPlayerInfo.m_stMatchInfo.m_dwEfficiency);
    if (iter != rstMap.end())
    {
        rstMap.erase(iter);
    }

    LOGRUN("release matchinfo, id=%u", pstInfo->GetObjID());
    RELEASE_GAMEOBJECT(pstInfo);
    return;
}


bool MatchBase::Init()
{
    //调用派生类的初始化
    if (!this->AppInit())
    {
        return false;
    }

    //初始化一些基础信息
    if (!this->_InitBasic())
    {
        return false;
    }

    //初始化定时器管理器
    OnReleaseTimerCb_t fReleaseTimer = MatchBase::ReleaseTimer;
    if (!m_oTimerMgr.Init(DEFAULT_MATCH_TIMER_NUM, fReleaseTimer))
    {
        return false;
    }

    return true;
}

bool MatchBase::_InitBasic()
{
    //匹配区段信息
    Section stTempSec;
    ResMatchMgr_t& rstResMatchMgr = CGameDataMgr::Instance().GetResMatchMgr();

    for (int i = 0; i < rstResMatchMgr.GetResNum(); i++)
    {
        RESMATCH* pResMatch = rstResMatchMgr.GetResByPos(i);
        if (!pResMatch)
        {
            return false;
        }
        if (pResMatch->m_bMatchType != m_bMatchType)
        {
            continue;
        }
        stTempSec.m_iLow = pResMatch->m_bScoreDown;
        stTempSec.m_iHigh = pResMatch->m_bScoreUp;
        m_oMapSection.insert(MapSection2Info_t::value_type(stTempSec, pResMatch));

        if (pResMatch->m_bMatchAIType == MATCH_FAKE_PLAYER_DATA)
        {
            FakeNode* pstNode = GET_GAMEOBJECT(FakeNode, GAMEOBJ_MATCH_FAKENODE);
            assert(pstNode);
            pstNode->m_iCount = 0;
            pstNode->m_oMapPlayer.clear();
            m_oMapFakePlayer.insert(MapSection2Fake_t::value_type(stTempSec, pstNode));
        }
    }

    // 地图信息
    RESBASIC* poBasicTerrainInfo = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_TERRAIN_INFO);
    m_bTerrainNum = (uint8_t)poBasicTerrainInfo->m_para[0];

    return true;
}


void MatchBase::Update()
{
    m_oTimerMgr.Update();

    //调用派生类的update方法
    this->AppUpdate();
}


bool MatchBase::MatchStartHandle(DT_FIGHT_PLAYER_INFO& rstSelfInfo)
{
    Section stTempSec;
    stTempSec.m_iLow = rstSelfInfo.m_stMatchInfo.m_dwScore;
    stTempSec.m_iHigh = rstSelfInfo.m_stMatchInfo.m_dwScore;

    //找到积分对应的配置档
    MapSection2Info_t::iterator iter = m_oMapSection.find(stTempSec);
    if (iter == m_oMapSection.end())
    {
        LOGERR("Player(%s) Uin(%lu) score(%u) section is not found",
                 rstSelfInfo.m_szName, rstSelfInfo.m_ullUin, rstSelfInfo.m_stMatchInfo.m_dwScore);
        return false;
    }

    RESMATCH* pResMatch = iter->second;

    //匹配AI时间为0,说明立即开启假匹配,连败后立即进入假匹配
    if (pResMatch->m_bMatchAIType!=0 &&
        (pResMatch->m_bMatchAITime == 0 ||
         rstSelfInfo.m_stMatchInfo.m_dwLosingStreak >= pResMatch->m_bMatchAILoseTimes))
    {
        _SendFakeMatchStartRsp(rstSelfInfo, pResMatch->m_bMatchAIType);
        return true;
    }

    MapKey2Info_t::iterator iterTarget = m_oMapScore.find(rstSelfInfo.m_stMatchInfo.m_dwEfficiency);
    if (iterTarget != m_oMapScore.end())
    {
        if (iterTarget->second->m_stPlayerInfo.m_ullUin == rstSelfInfo.m_ullUin)
        {
            // 如果匹配到自己，继续等待
            LOGERR("Player(%s) Uin(%lu) match self, keep waiting", rstSelfInfo.m_szName, rstSelfInfo.m_ullUin);
            return false;
        }
        else
        {
            // 匹配到对手,且对手还没被匹配走
            if (!iterTarget->second->m_bIsMatched)
            {
                DT_FIGHT_PLAYER_INFO& rstOpponentInfo = iterTarget->second->m_stPlayerInfo;
                LOGRUN("score match success, (%s) vs (%s)", rstSelfInfo.m_szName, rstOpponentInfo.m_szName);

                // 保存成功匹配的玩家数据，用于假匹配
                _SaveFightPlayerInfo(rstOpponentInfo);
                _SaveFightPlayerInfo(rstSelfInfo);

                // 去FightSvr创建副本，再通知客户端
                _SendCreateDungeonReq(rstSelfInfo, rstOpponentInfo);

                //从池子中删除匹配到的玩家
                iterTarget->second->m_bIsMatched = true;
                m_oTimerMgr.DelTimer(iterTarget->second->m_pTimer->GetObjID());

                return true;
            }
            else
            {
                //TODO
            }
        }
    }
    else
    {
        // 没有匹配到对手，添加自己到等待池中
        MatchInfo* pInfo = GET_GAMEOBJECT(MatchInfo, GAMEOBJ_MATCH_INFO);
        if (!pInfo)
        {
            LOGERR("Uin<%lu> pInfo is null", rstSelfInfo.m_ullUin);
            return false;
        }

        pInfo->m_wWaitCount = 0;
        pInfo->m_stPlayerInfo = rstSelfInfo;
        pInfo->m_pResMatch = pResMatch;
        pInfo->m_ulFakeMatchWaitTimeSec = CGameTime::Instance().GetCurrSecond();

        // 添加定时器
        pInfo->m_pTimer = GET_GAMEOBJECT(MatchTimer, GAMEOBJ_MATCH_TIMER);
        if (!pInfo->m_pTimer)
        {
            RELEASE_GAMEOBJECT(pInfo);
            LOGERR("Uin<%lu> pInfo->m_poTimer is null", rstSelfInfo.m_ullUin);
            return false;
        }
        pInfo->m_pTimer->AttachParam(pInfo, this);

        struct timeval tFirstFireTime = *(CGameTime::Instance().GetCurrTime());
        TvAddMs(&tFirstFireTime, pResMatch->m_bMatchSearchCycle * 1000);
        pInfo->m_pTimer->SetTimerAttr(tFirstFireTime, MATCH_TIMER_CHECK_TIMES, pResMatch->m_bMatchSearchCycle * 1000);

        //加入到定时器管理池和积分匹配池
        m_oTimerMgr.AddTimer((GameTimer*)pInfo->m_pTimer);
        m_oMapScore.insert(MapKey2Info_t::value_type(rstSelfInfo.m_stMatchInfo.m_dwEfficiency, pInfo));

        LOGRUN("Player(%s) Uin(%lu) add timer, id=%u, timer=%u",
                rstSelfInfo.m_szName, rstSelfInfo.m_ullUin, pInfo->GetObjID(), pInfo->m_pTimer->GetObjID());
    }

    // 匹配失败
    return false;
}


void MatchBase::_SendFakeMatchStartRsp(DT_FIGHT_PLAYER_INFO& rstSelfInfo, uint8_t bMatchAIType)
{
    DT_FIGHT_DUNGEON_INFO* pstDungeonInfo;

    if (bMatchAIType == MATCH_FAKE_PEAK_ARENA)
    {
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_DUNGEON_CREATE_REQ;
        m_stSsPkg.m_stHead.m_ullReservId = 0;
        pstDungeonInfo = &(m_stSsPkg.m_stBody.m_stDungeonCreateReq.m_stDungeonInfo);
    }
    else
    {
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_DUNGEON_CREATE_RSP;
        m_stSsPkg.m_stHead.m_ullReservId = (uint64_t) rstSelfInfo.m_iZoneSvrId;
        m_stSsPkg.m_stBody.m_stDungeonCreateRsp.m_ullUin = rstSelfInfo.m_ullUin;
        m_stSsPkg.m_stBody.m_stDungeonCreateRsp.m_nErrNo = ERR_NONE;
        pstDungeonInfo = &(m_stSsPkg.m_stBody.m_stDungeonCreateRsp.m_stDungeonInfo);
    }

    pstDungeonInfo->m_bFakeType = bMatchAIType;
    pstDungeonInfo->m_bMatchType = m_bMatchType;
    pstDungeonInfo->m_bFightPlayerNum = 2;
    pstDungeonInfo->m_astFightPlayerList[0] = rstSelfInfo;
    pstDungeonInfo->m_astFightPlayerList[0].m_chGroup = PLAYER_GROUP_DOWN;
    _GetFakePlayerInfo(rstSelfInfo, pstDungeonInfo->m_astFightPlayerList[1], bMatchAIType);
    pstDungeonInfo->m_astFightPlayerList[1].m_chGroup = PLAYER_GROUP_UP;

    pstDungeonInfo->m_bTerrainId = (uint8_t)CFakeRandom::Instance().Random(1, m_bTerrainNum);
    for (int i=0; i < pstDungeonInfo->m_bFightPlayerNum; i++)
    {
        DT_FIGHT_PLAYER_INFO& rstPlayerInfo = pstDungeonInfo->m_astFightPlayerList[i];
        for (int j=0; j < rstPlayerInfo.m_bTroopNum; j++)
        {
            DT_TROOP_INFO& rstTroopInfo = rstPlayerInfo.m_astTroopList[j];
            rstTroopInfo.m_bId = MAX_TROOP_NUM * rstPlayerInfo.m_chGroup + j + 1;
            if (rstPlayerInfo.m_chGroup == PLAYER_GROUP_DOWN)
            {
                // 回城卡片状态
                rstTroopInfo.m_stInitPos.m_iPosX = 0;
                rstTroopInfo.m_stInitPos.m_iPosY = -2000;
            }
            else
            {
                // 回城卡片状态
                rstTroopInfo.m_stInitPos.m_iPosX = 0;
                rstTroopInfo.m_stInitPos.m_iPosY = 10000;
            }
        }
    }

    LOGRUN("Player(%s) Uin(%lu) fake match success, MatchAIType(%d)",
            rstSelfInfo.m_szName, rstSelfInfo.m_ullUin, bMatchAIType);

    if (bMatchAIType == MATCH_FAKE_PEAK_ARENA)
    {
        MatchSvrMsgLayer::Instance().SendToFightSvr(m_stSsPkg);
    }
    else
    {
        MatchSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    }
}


void MatchBase::_GetFakePlayerInfo(DT_FIGHT_PLAYER_INFO& rstSelfInfo, DT_FIGHT_PLAYER_INFO& rstOpponentInfo, uint8_t& bMatchAIType)
{
    //匹配关卡数据或者是自定义匹配
    if (bMatchAIType == MATCH_FAKE_PVE ||bMatchAIType == MATCH_FAKE_OTHER || bMatchAIType == MATCH_FAKE_PEAK_ARENA)
    {
        rstOpponentInfo = rstSelfInfo;
        rstOpponentInfo.m_ullUin = 10000;
        return;
    }

    Section stTempSec;
    stTempSec.m_iLow = rstSelfInfo.m_stMatchInfo.m_dwScore;
    stTempSec.m_iHigh = rstSelfInfo.m_stMatchInfo.m_dwScore;

    MapSection2Fake_t::iterator IterSec = m_oMapFakePlayer.find(stTempSec);
    if (IterSec == m_oMapFakePlayer.end())
    {
        rstOpponentInfo = rstSelfInfo;
        rstOpponentInfo.m_ullUin = 0;
        bMatchAIType = MATCH_FAKE_PVE;
        return;
    }

    FakeNode* pstNode = IterSec->second;
    if (pstNode->m_iCount <= 0)
    {
        //保存的玩家数据池中还没有玩家，暂时用关卡数据
        rstOpponentInfo = rstSelfInfo;
        rstOpponentInfo.m_ullUin = 0;
        bMatchAIType = MATCH_FAKE_PVE;
        return;
    }

    //从保存的玩家数据池中随机选一个玩家的数据
    int iCursor = CFakeRandom::Instance().Random(pstNode->m_iCount);
    rstOpponentInfo = pstNode->m_astPlayerList[iCursor];
    if (rstOpponentInfo.m_ullUin == rstSelfInfo.m_ullUin)
    {
        rstOpponentInfo = pstNode->m_astPlayerList[(iCursor + 1) % pstNode->m_iCount];
        rstOpponentInfo.m_ullUin = 0;
    }

    return;
}


//  保存玩家数据,以便假PVP使用
void MatchBase::_SaveFightPlayerInfo(DT_FIGHT_PLAYER_INFO& rstPlayerInfo)
{
    Section stTempSec;
    stTempSec.m_iLow = rstPlayerInfo.m_stMatchInfo.m_dwScore;
    stTempSec.m_iHigh = rstPlayerInfo.m_stMatchInfo.m_dwScore;

    MapSection2Fake_t::iterator IterSec = m_oMapFakePlayer.find(stTempSec);
    if (IterSec == m_oMapFakePlayer.end())
    {
        return;
    }
    FakeNode* pstNode = IterSec->second;

    MapUin2FakePlayer_t::iterator IterPlayer = pstNode->m_oMapPlayer.find(rstPlayerInfo.m_ullUin);
    if (IterPlayer != pstNode->m_oMapPlayer.end())
    {
        //已存在于节点中，更新
        pstNode->m_astPlayerList[IterPlayer->second] = rstPlayerInfo;
        return;
    }

    if (pstNode->m_iCount >= FAKE_PLAYER_MAX_NUM)
    {
        int iCursor = CFakeRandom::Instance().Random(FAKE_PLAYER_MAX_NUM);
        IterPlayer = pstNode->m_oMapPlayer.find(pstNode->m_astPlayerList[iCursor].m_ullUin);
        if (IterPlayer != pstNode->m_oMapPlayer.end())
        {
            pstNode->m_oMapPlayer.erase(IterPlayer);
        }

        pstNode->m_astPlayerList[iCursor] = rstPlayerInfo;
        pstNode->m_oMapPlayer.insert(MapUin2FakePlayer_t::value_type(rstPlayerInfo.m_ullUin, iCursor));
    }
    else
    {
        pstNode->m_astPlayerList[pstNode->m_iCount] = rstPlayerInfo;
        pstNode->m_oMapPlayer.insert(MapUin2FakePlayer_t::value_type(rstPlayerInfo.m_ullUin, pstNode->m_iCount));
        pstNode->m_iCount++;
    }

    return;
}


//匹配成功,发送创建战场信息到FightSvr
void MatchBase::_SendCreateDungeonReq(DT_FIGHT_PLAYER_INFO& rstSelfInfo, DT_FIGHT_PLAYER_INFO& rstOpponentInfo)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_DUNGEON_CREATE_REQ;
	m_stSsPkg.m_stHead.m_ullReservId = 0;
    SS_PKG_DUNGEON_CREATE_REQ& rstSsPkgBodyReq = m_stSsPkg.m_stBody.m_stDungeonCreateReq;

    rstSsPkgBodyReq.m_stDungeonInfo.m_bFakeType = MATCH_FAKE_NONE;
    rstSsPkgBodyReq.m_stDungeonInfo.m_bMatchType = m_bMatchType;
    rstSsPkgBodyReq.m_stDungeonInfo.m_bFightPlayerNum = 2;
    rstSsPkgBodyReq.m_stDungeonInfo.m_astFightPlayerList[0] = rstSelfInfo;
    rstSsPkgBodyReq.m_stDungeonInfo.m_astFightPlayerList[0].m_chGroup = PLAYER_GROUP_DOWN;
    rstSsPkgBodyReq.m_stDungeonInfo.m_astFightPlayerList[1] = rstOpponentInfo;
    rstSsPkgBodyReq.m_stDungeonInfo.m_astFightPlayerList[1].m_chGroup = PLAYER_GROUP_UP;

    MatchSvrMsgLayer::Instance().SendToFightSvr(m_stSsPkg);
}


int MatchBase::MatchCancelHandle(uint64_t m_ullUin, uint32_t m_dwScore)
{
    if (m_ullUin == 0)
    {
        return ERR_NOT_EXIST_PLAYER;
    }

    MatchInfo* pstMatchInfo = NULL;
    MapKey2Info_t::iterator iter = m_oMapScore.find(m_dwScore);

    if (iter != m_oMapScore.end())
    {
        pstMatchInfo = iter->second;
        if (pstMatchInfo)
        {
            if (!pstMatchInfo->m_bIsMatched && pstMatchInfo->m_stPlayerInfo.m_ullUin == m_ullUin)
            {
                LOGRUN("cancel matchinfo, timer=%u", pstMatchInfo->GetObjID());
                pstMatchInfo->m_bIsMatched = true;
                m_oTimerMgr.DelTimer(pstMatchInfo->m_pTimer->GetObjID());
                return ERR_NONE;
            }
        }
    }
    return ERR_NOT_EXIST_PLAYER;
}


// 定时器调用
bool MatchBase::MatchStartTimer(MatchInfo& rstInfo, uint32_t dwWaitCount)
{
    //检查 是否进行假PVP匹配
    if (_CheckFakeMatch(rstInfo))
    {
        return true;
    }

    DT_FIGHT_PLAYER_INFO& rstSelfInfo = rstInfo.m_stPlayerInfo;
    // 先找到自己
    MapKey2Info_t::iterator iterSelf = m_oMapScore.find(rstSelfInfo.m_stMatchInfo.m_dwEfficiency);
    if (iterSelf == m_oMapScore.end())
    {
        LOGERR("Player(%s) Uin(%lu) is not found", rstSelfInfo.m_szName, rstSelfInfo.m_ullUin);
        return false;
    }

    bool bIsMatched = false;
    MapKey2Info_t::iterator iterTarget;
    do
    {
        RESMATCH * pResMatch = iterSelf->second->m_pResMatch;

        uint32_t dwDelta = dwWaitCount * pResMatch->m_dwMatchSearchInterval;
        uint32_t dwLowDelta = dwDelta > pResMatch->m_dwMatchSearchDown ? pResMatch->m_dwMatchSearchDown : dwDelta;
        uint32_t dwHighDelta = dwDelta > pResMatch->m_dwMatchSearchUp ?  pResMatch->m_dwMatchSearchUp : dwDelta;
        uint32_t dwRangeLow = rstSelfInfo.m_stMatchInfo.m_dwEfficiency >= dwLowDelta ? rstSelfInfo.m_stMatchInfo.m_dwEfficiency - dwLowDelta : 0;
        uint32_t dwRangeHigh = rstSelfInfo.m_stMatchInfo.m_dwEfficiency + dwHighDelta;

        LOGRUN("Player(%s) Uin(%lu) match search score range low(%u) high(%u)",
                rstSelfInfo.m_szName, rstSelfInfo.m_ullUin, dwRangeLow, dwRangeHigh);

        // 判断积分前后是否有满足条件的对手
        //向上检索一位
        iterTarget = iterSelf;
        iterTarget++;
        if ((iterTarget) != m_oMapScore.end())
        {
            if (iterTarget->second->m_stPlayerInfo.m_stMatchInfo.m_dwEfficiency <= dwRangeHigh)
            {
                bIsMatched = true;
                break;
            }
        }

        //向下检索一位
        iterTarget = iterSelf;
        if (iterTarget != m_oMapScore.begin())
        {
            iterTarget--;
            if (iterTarget->second->m_stPlayerInfo.m_stMatchInfo.m_dwEfficiency >= dwRangeLow)
            {
                bIsMatched = true;
                break;
            }
        }
    } while (false);

    if (bIsMatched)
    {
        // 匹配到对手
        if (!iterTarget->second->m_bIsMatched)
        {
            // 对手还没被匹配走
            DT_FIGHT_PLAYER_INFO& rstOpponentInfo = iterTarget->second->m_stPlayerInfo;
            LOGRUN("score match success, Player(%s) Uin(%lu) vs Player(%s) Uin(%lu)",
                    rstSelfInfo.m_szName, rstSelfInfo.m_ullUin, rstOpponentInfo.m_szName, rstOpponentInfo.m_ullUin);

            // 保存成功匹配的玩家数据，用于假匹配
            _SaveFightPlayerInfo(rstSelfInfo);
            _SaveFightPlayerInfo(rstOpponentInfo);

            // 去FightSvr创建副本，再通知客户端
            _SendCreateDungeonReq(rstSelfInfo, rstOpponentInfo);

            //删除定时器
            iterSelf->second->m_bIsMatched = true;
            m_oTimerMgr.DelTimer(iterSelf->second->m_pTimer->GetObjID());
            iterTarget->second->m_bIsMatched = true;
            m_oTimerMgr.DelTimer(iterTarget->second->m_pTimer->GetObjID());

            return true;
        }
    }

    // 没匹配到
    return false;
}


// 检查能否进行假PVP，有些情况是不能进行假PVP的
bool MatchBase::_CheckFakeMatch(MatchInfo& rstMatchInfo)
{
    //不匹配AI,直接返回
    if (rstMatchInfo.m_pResMatch->m_bMatchAIType == MATCH_FAKE_NONE)
    {
        return false;
    }

    // 超过配置的PVP等待时间，那么进行假PVP
    if (CGameTime::Instance().GetCurrSecond() - rstMatchInfo.m_ulFakeMatchWaitTimeSec >=
       rstMatchInfo.m_pResMatch->m_bMatchAITime)
    {
        return _FakeMatch(rstMatchInfo.m_stPlayerInfo);
    }

    return false;
}


// 假PVP匹配
bool MatchBase::_FakeMatch(DT_FIGHT_PLAYER_INFO& rstPlayerInfo)
{
    MapKey2Info_t::iterator iterSelf;
    iterSelf = m_oMapScore.find(rstPlayerInfo.m_stMatchInfo.m_dwEfficiency);
    if (iterSelf == m_oMapScore.end())
    {
        LOGERR("Player(%s) Uin(%lu) FakeMatch failed, Efficiency(%u) not found",
                 rstPlayerInfo.m_szName, rstPlayerInfo.m_ullUin, rstPlayerInfo.m_stMatchInfo.m_dwEfficiency);
        return false;
    }

    this->_SendFakeMatchStartRsp(rstPlayerInfo, iterSelf->second->m_pResMatch->m_bMatchAIType);

    //将此玩家从匹配池删除
    iterSelf->second->m_bIsMatched = true;
    m_oTimerMgr.DelTimer(iterSelf->second->m_pTimer->GetObjID());

    return true;
}


