#include "CloneBattleTeam.h"
#include "PKGMETA_metalib.h"
#include "LogMacros.h"
#include "ss_proto.h"
#include "../framework/CloneBattleSvrMsgLayer.h"
#include "CloneBattleMgr.h"
using namespace PKGMETA;

void CloneBattleTeam::Clear()
{
    TLIST_INIT(&m_stDirtyListNode);
    TLIST_INIT(&m_stTimeListNode);
    m_dwAvrLi = 0;
    bzero(&m_stTeamInfo, sizeof(m_stTeamInfo));
}

bool CloneBattleTeam::InitFromDB(IN PKGMETA::DT_CLONE_BATTLE_TEAM_DATA& rstData)
{
    this->Clear();
    size_t ulUseSize = 0;
    uint32_t dwVersion = rstData.m_stBaseInfo.m_dwVersion;
    int iRet = m_stTeamInfo.unpack((char*)rstData.m_stTeamBlob.m_szData, sizeof(rstData.m_stTeamBlob.m_szData), &ulUseSize, dwVersion);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("Id(%lu) unpack m_stTeamBlob failed, Ret(%d)", rstData.m_stBaseInfo.m_ullId, iRet);
        return false;
    }

    for (int i = 0; i < MAX_NUM_CLONE_BATTLE_MATE; i++)
    {
        if (m_stTeamInfo.m_astMateInfo->m_ullUin == 0)
        {
            continue;
        }
        m_dwAvrLi += m_stTeamInfo.m_astMateInfo->m_stTroopInfo.m_stGeneralInfo.m_dwLi;
    }
    if (m_stTeamInfo.m_bCount == 0)
    {
        m_dwAvrLi = 1;
        LOGERR("Id<%lu>error:m_stTeamInfo.m_bCount is 0", rstData.m_stBaseInfo.m_ullId);
        return false;
    }
    m_dwAvrLi /= m_stTeamInfo.m_bCount;
    return true;
}

bool CloneBattleTeam::PackToData(OUT PKGMETA::DT_CLONE_BATTLE_TEAM_DATA& rstData)
{
    size_t ulUseSize = 0;
    rstData.m_stBaseInfo.m_ullId = GetTeamId();
    rstData.m_stBaseInfo.m_dwVersion = MetaLib::getVersion();
    int iRet = m_stTeamInfo.pack((char*)rstData.m_stTeamBlob.m_szData, sizeof(rstData.m_stTeamBlob.m_szData), &ulUseSize);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("Id(%lu) pack m_stTeamBlob failed! Ret(%d) ", rstData.m_stBaseInfo.m_ullId, iRet);
        return false;
    }
    rstData.m_stTeamBlob.m_iLen = (int)ulUseSize;
    return true;
}

void CloneBattleTeam::GetTeamInfo(OUT PKGMETA::DT_CLONE_BATTLE_TEAM_INFO& rstTeamInfo)
{
    memcpy(&rstTeamInfo, &m_stTeamInfo, sizeof(rstTeamInfo));
}

bool CloneBattleTeam::InitTeamInfo(uint64_t ullTeamId, uint8_t bBossType, uint32_t dwBossId, DT_CLONE_BATTLE_MATE_INFO& rstMateInfo)
{
    this->Clear();
    m_stTeamInfo.m_ullId = ullTeamId;
    m_stTeamInfo.m_bBossType = bBossType;
    m_stTeamInfo.m_dwBossId = dwBossId;
    m_stTeamInfo.m_ullCaptain = rstMateInfo.m_ullUin;
    memcpy(&m_stTeamInfo.m_astMateInfo[0], &rstMateInfo, sizeof(DT_CLONE_BATTLE_MATE_INFO));
    m_stTeamInfo.m_astMateInfo[0].m_bProgress = 0;
    m_stTeamInfo.m_bCount = 1;
    m_stTeamInfo.m_ullCreateTime = CGameTime::Instance().GetCurrSecond();
    m_dwAvrLi = rstMateInfo.m_stTroopInfo.m_stGeneralInfo.m_dwLi;
    m_stTeamInfo.m_bMatch = CloneBattleMgr::CONST_MATCH_TYPE_QUICK_ON;
    return true;
        
}

void CloneBattleTeam::UptMateInfo(PKGMETA::DT_CLONE_BATTLE_MATE_INFO & rstMateInfo)
{
    bool bUptTag = false;
    for (int i = 0; i < MAX_NUM_CLONE_BATTLE_MATE; i++)
    {
        if (m_stTeamInfo.m_astMateInfo[i].m_ullUin == rstMateInfo.m_ullUin)
        {
            memcpy(&m_stTeamInfo.m_astMateInfo[i].m_stTroopInfo, &rstMateInfo.m_stTroopInfo, sizeof(DT_TROOP_INFO));
            _ChangeAvrLi(m_stTeamInfo.m_astMateInfo[i].m_stTroopInfo.m_stGeneralInfo.m_dwLi, 3,
                rstMateInfo.m_stTroopInfo.m_stGeneralInfo.m_dwLi);
            CloneBattleMgr::Instance().DelFromTimeList(this);
            CloneBattleMgr::Instance().AddToDirtyList(this);
            bUptTag = true;
        }
    }
    if (!bUptTag)
    {
        LOGERR("Uin<%lu> UptMateInfo error, not the mate TeamId<%lu>", rstMateInfo.m_ullUin, m_stTeamInfo.m_ullId);
    }
}

int CloneBattleTeam::AddMateInfo(PKGMETA::DT_CLONE_BATTLE_MATE_INFO& rstMateInfo)
{
    if (IsFull())
    {
        LOGERR("Uin<%lu> the team<%lu>  is full", rstMateInfo.m_ullUin, m_stTeamInfo.m_ullId);
        return ERR_CLONEBATTLE_TEAM_FULL;
    }

    int iAddIndex = 0;

    for (int i = 0; i < MAX_NUM_CLONE_BATTLE_MATE; i++)
    {
        if (m_stTeamInfo.m_astMateInfo[i].m_ullUin == rstMateInfo.m_ullUin)
        {
            LOGERR("Uin<%lu> have joined in the team<%lu>", rstMateInfo.m_ullUin, m_stTeamInfo.m_ullId);
            return ERR_CLONEBATTLE_ALREADY_IN_TEAM;
            
        }
        if (m_stTeamInfo.m_astMateInfo[i].m_ullUin == 0)
        {
            iAddIndex = i;
            break;
        }
    }

    memcpy(&m_stTeamInfo.m_astMateInfo[iAddIndex], &rstMateInfo, sizeof(DT_CLONE_BATTLE_MATE_INFO));
    m_stTeamInfo.m_astMateInfo[iAddIndex].m_bProgress = 0;
    m_stTeamInfo.m_bCount++;
    _ChangeAvrLi(rstMateInfo.m_stTroopInfo.m_stGeneralInfo.m_dwLi, 1);
    
    CloneBattleMgr::Instance().DelFromTimeList(this);
    CloneBattleMgr::Instance().AddToDirtyList(this);
    BroadcastMate(CLONE_BATTLE_NTF_TYPE_TEAM_UPT);
    return ERR_NONE;
}

int CloneBattleTeam::QuitTeam(uint64_t ullUin)
{
    int iRet = ERR_NONE;
    bool bIsCaptain = false;
    uint64_t ullNextCaptainUin = 0;
    for (int i = MAX_NUM_CLONE_BATTLE_MATE-1; i >=0; i--)
    {
        if (m_stTeamInfo.m_astMateInfo[i].m_ullUin != 0)
        {
            //队长候选人
            ullNextCaptainUin = m_stTeamInfo.m_astMateInfo[i].m_ullUin;
        }
        if (m_stTeamInfo.m_astMateInfo[i].m_ullUin == ullUin)
        {
            //退出队伍条件检查
            if (m_stTeamInfo.m_astMateInfo[i].m_bProgress > 0)
            {
                LOGERR("Uin<%lu> quit team error, m_bProgress<%hhu> != 0", ullUin, m_stTeamInfo.m_astMateInfo[i].m_bProgress);
                iRet = ERR_NOT_SATISFY_COND;
                break;
            }
            //退出处理
            
            m_stTeamInfo.m_bCount--;
            _ChangeAvrLi(m_stTeamInfo.m_astMateInfo[i].m_stTroopInfo.m_stGeneralInfo.m_dwLi, 2);
            bzero(&m_stTeamInfo.m_astMateInfo[i], sizeof(DT_CLONE_BATTLE_MATE_INFO));
            CloneBattleMgr::Instance().DelFromTimeList(this);
            CloneBattleMgr::Instance().AddToDirtyList(this);
            if (ullUin == m_stTeamInfo.m_ullCaptain)
            {
                bIsCaptain = true;
            }
        }
    }
    if (bIsCaptain)
    {
        m_stTeamInfo.m_ullCaptain = ullNextCaptainUin;
    }

    return iRet;
}

bool CloneBattleTeam::CanDissolve()
{
    return m_stTeamInfo.m_bCount == 0;
}

void CloneBattleTeam::HandleWin(uint64_t ullUin)
{
    DT_CLONE_BATTLE_MATE_INFO* pstMateInfo = _FindMate(ullUin);
    if (!pstMateInfo)
    {
        LOGERR("Uin<%lu> pstMateInfo is NULL, TeamId<%lu>", ullUin, m_stTeamInfo.m_ullId);
        return;
    }

    if (++pstMateInfo->m_bProgress >= CloneBattleMgr::Instance().GetMateFinishNum())
    {

        if (++m_stTeamInfo.m_bFinishCount >= CloneBattleMgr::Instance().GetTeamFinishNum())
        {
            BroadcastMate(CLONE_BATTLE_NTF_TYPE_TEAM_REWARD);
        }
        
    }
    else
    {
        BroadcastMate(CLONE_BATTLE_NTF_TYPE_TEAM_UPT);
    }
    LOGRUN("Uin<%lu> TeamId<%lu> handle win process<%hhu> ok TeamFinishNum<%hhu>", 
        ullUin, m_stTeamInfo.m_ullId, pstMateInfo->m_bProgress, m_stTeamInfo.m_bFinishCount);
    
}

void CloneBattleTeam::BroadcastMate(uint8_t bNtfType)
{
    SSPKG stSsPkg;
    stSsPkg.m_stHead.m_wMsgId = SS_MSG_CLONE_BATTLE_BROADCAST_NTF;
    SS_PKG_CLONE_BATTLE_BROADCAST_NTF& rstNtf = stSsPkg.m_stBody.m_stCloneBattleBroadCastNtf;
    rstNtf.m_bNtfType = bNtfType;
    GetTeamInfo(rstNtf.m_stTeamInfo);
    CloneBattleSvrMsgLayer::Instance().SendToZoneSvr(stSsPkg);
}

void CloneBattleTeam::SetMatchType(uint8_t bMatchType)
{
    m_stTeamInfo.m_bMatch = bMatchType;
}


uint8_t CloneBattleTeam::GetMatchType()
{
    return m_stTeamInfo.m_bMatch;
}

bool CloneBattleTeam::IsFull()
{
    return m_stTeamInfo.m_bCount >= MAX_NUM_CLONE_BATTLE_MATE;
}


void CloneBattleTeam::_ChangeAvrLi(uint32_t dwLi, uint8_t bType, uint32_t dwOldLi)
{
    CloneBattleMgr::Instance().DelFromMatchList(this);
    if (m_stTeamInfo.m_bCount == 0)
    {
        m_dwAvrLi = 0;
        return;
    }
    if (bType == 1)
    {
        //增加
        m_dwAvrLi += (dwLi - m_dwAvrLi) / m_stTeamInfo.m_bCount;
    }

    else if (bType == 2)
    {

        //删除
        m_dwAvrLi -= (dwLi - m_dwAvrLi) / m_stTeamInfo.m_bCount;
    }
    else
    {
        //更新
        m_dwAvrLi += (dwLi - dwOldLi) / m_stTeamInfo.m_bCount;
    }
    CloneBattleMgr::Instance().AddToMatchList(this);
}

PKGMETA::DT_CLONE_BATTLE_MATE_INFO* CloneBattleTeam::_FindMate(uint64_t ullUin)
{
    for (int i = 0; i < MAX_NUM_CLONE_BATTLE_MATE; i++)
    {
        if (m_stTeamInfo.m_astMateInfo[i].m_ullUin == ullUin)
        {
            return &m_stTeamInfo.m_astMateInfo[i];
        }
    }
    return NULL;
}

