#include "MsgLogicCloneBattle.h"
#include "LogMacros.h"
#include "strutil.h"
#include "../framework/CloneBattleSvrMsgLayer.h"
#include "../module/CloneBattleMgr.h"


using namespace PKGMETA;
int CSsCloneBattleGetInfoReq::HandleServerMsg(PKGMETA::SSPKG * pstSsPkg)
{
    SS_PKG_CLONE_BATTLE_GET_INFO_REQ& rstReq = pstSsPkg->m_stBody.m_stCloneBattleGetInfoReq;
    SSPKG& stSsPkgNew = CloneBattleSvrMsgLayer::Instance().GetSsPkg();
    SS_PKG_CLONE_BATTLE_GET_INFO_RSP& rstRsp = stSsPkgNew.m_stBody.m_stCloneBattleGetInfoRsp;
    CloneBattleTeam* pstTeam = NULL;
    if (rstReq.m_ullTeamId != 0)
    {
        pstTeam = CloneBattleMgr::Instance().GetTeamInfo(rstReq.m_ullTeamId);
    }
    
    if (pstTeam)
    {
        pstTeam->GetTeamInfo(rstRsp.m_stTeamInfo);
    }
    else
    {
        rstRsp.m_stTeamInfo.m_ullId = 0;
    }
    CloneBattleMgr::Instance().GetBossInfo(rstRsp.m_stBossInfo);
    stSsPkgNew.m_stHead.m_ullUin = pstSsPkg->m_stHead.m_ullUin;
    stSsPkgNew.m_stHead.m_wMsgId = SS_MSG_CLONE_BATTLE_GET_INFO_RSP;
    rstRsp.m_nError = ERR_NONE;
    CloneBattleSvrMsgLayer::Instance().SendToZoneSvr(stSsPkgNew);

    return 0;
}

int CSsCloneBattleGetDataRsp::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
    SS_PKG_CLONE_BATTLE_GET_DATA_RSP& rstRsp = pstSsPkg->m_stBody.m_stCloneBattleGetDataRsp;
    CloneBattleMgr::Instance().AsyncGetDataDone(rstRsp.m_ullTokenId, rstRsp.m_nError, &rstRsp.m_stCloneBattleData);
    return 0;
}


int CSsCloneBattleJoinTeamReq::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
    SS_PKG_CLONE_BATTLE_JOIN_TEAM_REQ& rstReq = pstSsPkg->m_stBody.m_stCloneBattleJoinTeamReq;
    CloneBattleTeam* pstTeam = NULL;
    int iRet = ERR_NONE;
	LOGRUN("Uin<%lu> TeamId<%lu> Type<%d> BossType<%d> UseProp<%d>", pstSsPkg->m_stHead.m_ullUin,
		rstReq.m_ullTeamId, rstReq.m_bType, rstReq.m_bBossType, rstReq.m_bUseProp);
    do 
    {
        if (rstReq.m_bType == 1)    //创建
        {
            
            pstTeam = CloneBattleMgr::Instance().CreateTeam(rstReq);
        }
        else if (rstReq.m_bType == 3)   //加入指定队伍
        {
            
            pstTeam = CloneBattleMgr::Instance().GetTeamInfo(rstReq.m_ullTeamId);
            if (!pstTeam)
            {
                LOGERR("Uin<%lu> can't find the team<%lu> ", pstSsPkg->m_stHead.m_ullUin, rstReq.m_ullTeamId);
                iRet = ERR_CLONEBATTLE_TEAM_NOT_EXIST;
                break;
            }
            if (pstTeam->GetBossType() != rstReq.m_bBossType)
            {
                LOGERR("Uin<%lu> ERR_CLONEBATTLE_ALREADY_BOSS_TYPE_ERROR <%lu> send BossType<%hhu> TeamBossType<%hhu>", 
                    pstSsPkg->m_stHead.m_ullUin, rstReq.m_ullTeamId, rstReq.m_bBossType, pstTeam->GetBossType());
                iRet = ERR_CLONEBATTLE_ALREADY_BOSS_TYPE_ERROR;
                break;
            }

            //CloneBattleMgr::Instance().DelFromMatchList(pstTeam);
            iRet = pstTeam->AddMateInfo(rstReq.m_stMateInfo);
            //CloneBattleMgr::Instance().AddToMatchList(pstTeam);
        }
        else        //快速匹配
        {
            
            pstTeam = CloneBattleMgr::Instance().MatchTeam(pstSsPkg->m_stHead.m_ullUin, rstReq, iRet);
        }
    } while (false);

    SSPKG& stSsPkgNew = CloneBattleSvrMsgLayer::Instance().GetSsPkg();
    stSsPkgNew.m_stHead.m_ullUin = pstSsPkg->m_stHead.m_ullUin;
    stSsPkgNew.m_stHead.m_wMsgId = SS_MSG_CLONE_BATTLE_JOIN_TEAM_RSP;
    
    SS_PKG_CLONE_BATTLE_JOIN_TEAM_RSP& rstRsp = stSsPkgNew.m_stBody.m_stCloneBattleJoinTeamRsp;
    if (pstTeam && iRet == ERR_NONE)
    {
        pstTeam->GetTeamInfo(rstRsp.m_stTeamInfo);
        rstRsp.m_nError = ERR_NONE;
        rstRsp.m_bUseProp = rstReq.m_bUseProp;
        LOGRUN("Uin<%lu> join the team<%lu>", pstSsPkg->m_stHead.m_ullUin, pstTeam->GetTeamId());
    }
    else if (iRet != ERR_NONE)
    {
        //加入失败,错误码返出去
        rstRsp.m_nError = iRet;
        LOGERR("Uin<%lu> AddTeam<%lu> error<%d>", pstSsPkg->m_stHead.m_ullUin, rstReq.m_ullTeamId, iRet);
    }
    else
    {
        rstRsp.m_nError = ERR_SYS;
        LOGERR("Uin<%lu> the pstTeam<%lu> is NULL", pstSsPkg->m_stHead.m_ullUin, rstReq.m_ullTeamId);
    }
    CloneBattleSvrMsgLayer::Instance().SendToZoneSvr(stSsPkgNew);

    return 0;
}

int CSsCloneBattleFightReq::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
    SS_PKG_CLONE_BATTLE_FIGHT_REQ& rstReq = pstSsPkg->m_stBody.m_stCloneBattleFightReq;
    
    SSPKG& stSsPkgNew = CloneBattleSvrMsgLayer::Instance().GetSsPkg();
    SS_PKG_CLONE_BATTLE_FIGHT_RSP& rstRsp = stSsPkgNew.m_stBody.m_stCloneBattleFightRsp;
    
    int iRet = ERR_NONE;
    CloneBattleTeam* pstTeam = CloneBattleMgr::Instance().GetTeamInfo(rstReq.m_ullTeamId);
    if (pstTeam)
    {
        pstTeam->UptMateInfo(rstReq.m_stMateInfo);
        pstTeam->GetTeamInfo(rstRsp.m_stTeamInfo);
    }
    else
    {
        iRet = ERR_SYS;
        LOGERR("Uin<%lu> the pstTeam<%lu> is NULL", pstSsPkg->m_stHead.m_ullUin, rstReq.m_ullTeamId);
    }
    stSsPkgNew.m_stHead.m_ullUin = pstSsPkg->m_stHead.m_ullUin;
    stSsPkgNew.m_stHead.m_wMsgId = SS_MSG_CLONE_BATTLE_FIGHT_RSP;
    rstRsp.m_nError = iRet;
    CloneBattleSvrMsgLayer::Instance().SendToZoneSvr(stSsPkgNew);
    return 0;
}

int CSsCloneBattleRewardNtf::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{

    SS_PKG_CLONE_BATTLE_REWARD_NTF& rstReq = pstSsPkg->m_stBody.m_stCloneBattleRewardNtf;
    CloneBattleTeam* pstTeam = CloneBattleMgr::Instance().GetTeamInfo(rstReq.m_ullTeamId);
    if (!pstTeam)
    {
        LOGERR("Uin<%lu> pstTeam<%lu> is NULL", pstSsPkg->m_stHead.m_ullUin, rstReq.m_ullTeamId);
        return -1;
    }
    pstTeam->HandleWin(pstSsPkg->m_stHead.m_ullUin);
    return 0;
}

int CSsCloneBattleQuitTeamReq::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
    SS_PKG_CLONE_BATTLE_QUIT_TEAM_REQ& rstReq = pstSsPkg->m_stBody.m_stCloneBattleQuitTeamReq;
    CloneBattleTeam* pstTeam = NULL;
    int iRet = ERR_NONE;
    pstTeam = CloneBattleMgr::Instance().GetTeamInfo(rstReq.m_ullTeamId);
    if (pstTeam)
    {
        bool bFullTag = pstTeam->IsFull();
        iRet = pstTeam->QuitTeam(pstSsPkg->m_stHead.m_ullUin);
        if (iRet == ERR_NONE )
        {
            if (pstTeam->CanDissolve())
            {
                //解散
                CloneBattleMgr::Instance().DissolveTeam(pstTeam);
            }
            else if (bFullTag)
            {
                //从满员队伍退出,需要重新加入到匹配队伍里
                CloneBattleMgr::Instance().AddToMatchList(pstTeam);
            }
        }
    }
    SSPKG& stSsPkgNew = CloneBattleSvrMsgLayer::Instance().GetSsPkg();
    SS_PKG_CLONE_BATTLE_QUIT_TEAM_RSP& rstRsp = stSsPkgNew.m_stBody.m_stCloneBattleQuitTeamRsp;
    stSsPkgNew.m_stHead.m_ullUin = pstSsPkg->m_stHead.m_ullUin;
    stSsPkgNew.m_stHead.m_wMsgId = SS_MSG_CLONE_BATTLE_QUIT_TEAM_RSP;
    rstRsp.m_nError = iRet;
    CloneBattleSvrMsgLayer::Instance().SendToZoneSvr(stSsPkgNew);
    return 0;
}


int CSsCloneBattleSetTeamReq::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{

    SS_PKG_CLONE_BATTLE_SET_TEAM_REQ& rstReq = pstSsPkg->m_stBody.m_stCloneBattleSetTeamReq;
    int iRet = ERR_NONE;
    uint32_t dwGCardId = 0;
    CloneBattleTeam* pstTeam = CloneBattleMgr::Instance().GetTeamInfo(rstReq.m_ullTeamId);
    if (pstTeam)
    {
        if (rstReq.m_bType == 1)    //设置快速匹配
        {
            if (pstTeam->IsCaptain(pstSsPkg->m_stHead.m_ullUin) && (rstReq.m_bMatch == CloneBattleMgr::CONST_MATCH_TYPE_QUICK_ON
                || rstReq.m_bMatch == CloneBattleMgr::CONST_MATCH_TYPE_QUICK_OFF))
            {
                pstTeam->SetMatchType(rstReq.m_bMatch);
                if (rstReq.m_bMatch == CloneBattleMgr::CONST_MATCH_TYPE_QUICK_ON)
                {
                    CloneBattleMgr::Instance().AddToMatchList(pstTeam);
                }
                else
                {
                    CloneBattleMgr::Instance().DelFromMatchList(pstTeam);
                }
                pstTeam->BroadcastMate(CLONE_BATTLE_NTF_TYPE_TEAM_UPT);
            }
            else
            {
                LOGERR("Uin<%lu> match<%hhu> type error!", pstSsPkg->m_stHead.m_ullUin, rstReq.m_bMatch);
                iRet = ERR_WRONG_PARA;
            }
        }
        else    //更换武将
        {
            
            pstTeam->UptMateInfo(rstReq.m_stMateInfo);
            pstTeam->BroadcastMate(CLONE_BATTLE_NTF_TYPE_TEAM_UPT);
            dwGCardId = rstReq.m_stMateInfo.m_stTroopInfo.m_stGeneralInfo.m_dwId;
        }

    }
    else
    {
        iRet = ERR_SYS;
        LOGERR("Uin<%lu> the pstTeam<%lu> is NULL", pstSsPkg->m_stHead.m_ullUin, rstReq.m_ullTeamId);
    }
    SSPKG& stSsPkgNew = CloneBattleSvrMsgLayer::Instance().GetSsPkg();
    SS_PKG_CLONE_BATTLE_SET_TEAM_RSP& rstRsp = stSsPkgNew.m_stBody.m_stCloneBattleSetTeamRsp;
    stSsPkgNew.m_stHead.m_ullUin = pstSsPkg->m_stHead.m_ullUin;
    stSsPkgNew.m_stHead.m_wMsgId = SS_MSG_CLONE_BATTLE_SET_TEAM_RSP;
    rstRsp.m_nError = iRet;
    rstRsp.m_bType = rstReq.m_bType;
    rstRsp.m_dwGCardId = dwGCardId;
    CloneBattleSvrMsgLayer::Instance().SendToZoneSvr(stSsPkgNew);
    return 0;
}

int CSsCloneBattleZoneSvrOnlineNtf::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
    CloneBattleMgr::Instance().SendSysInfo();
    return 0;
}
