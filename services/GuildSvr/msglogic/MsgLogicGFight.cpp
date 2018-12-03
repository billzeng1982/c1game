#include <string.h>
#include "strutil.h"
#include "LogMacros.h"
#include "MsgLogicGFight.h"
#include "ss_proto.h"
#include "../framework/GuildSvrMsgLayer.h"
#include "../module/Guild/GuildMgr.h"
#include "../module/Fight/GFightArenaMgr.h"
#include "../module/Fight/GuildFightPoint.h"
#include "../module/Fight/FightPlayerStateMachine.h"


// 加入军团战
int GFightArenaJoinReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_FIGHT_ARENA_JOIN_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;

    SS_PKG_GUILD_FIGHT_ARENA_JOIN_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildFightArenaJoinReq;
    SS_PKG_GUILD_FIGHT_ARENA_JOIN_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stGuildFightArenaJoinRsp;

    GuildFightArena* poArena = GFightArenaMgr::Instance().GetArenaByGuildId(rstSsPkgReq.m_stJoinerInfo.m_ullGuildId);
    if (poArena == NULL)
    {
        rstSsPkgRsp.m_nErrNo = ERR_NOT_FOUND_ARENA;
    }
    else
    {
        rstSsPkgRsp.m_nErrNo = poArena->Join(rstSsPkgReq, rstSsPkgRsp.m_stArenaInfo, rstSsPkgRsp.m_ullTimeStamp);
    }

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return 0;
}


// 退出军团战
int GFightArenaQuitReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_FIGHT_ARENA_QUIT_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;

    SS_PKG_GUILD_FIGHT_ARENA_QUIT_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildFightArenaQuitReq;
    SS_PKG_GUILD_FIGHT_ARENA_QUIT_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stGuildFightArenaQuitRsp;


    GuildFightArena* poArena = GFightArenaMgr::Instance().GetArenaByGuildId(rstSsPkgReq.m_stQuiterInfo.m_ullGuildId);
    if (poArena == NULL)
    {
        return 1;
    }
    else
    {
        rstSsPkgRsp.m_nErrNo = poArena->Quit(rstSsPkgReq.m_stQuiterInfo.m_ullUin);
    }

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);

    return 0;
}


int GFightArenaMoveReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_FIGHT_ARENA_MOVE_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;

    SS_PKG_GUILD_FIGHT_ARENA_MOVE_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildFightArenaMoveReq;
    SS_PKG_GUILD_FIGHT_ARENA_MOVE_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stGuildFightArenaMoveRsp;

    GuildFightArena* poArena = GFightArenaMgr::Instance().GetArenaByGuildId(rstSsPkgReq.m_stPlayerInfo.m_ullGuildId);
    if (poArena == NULL)
    {
        rstSsPkgRsp.m_nErrNo = ERR_NOT_FOUND;
        GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
        return 0;
    }

    int iRet = poArena->Move(rstSsPkgReq.m_stPlayerInfo.m_ullUin, rstSsPkgReq.m_wCampId);
    if (iRet < 0)
    {
        rstSsPkgRsp.m_nErrNo = iRet;
    }
    else
    {
        rstSsPkgRsp.m_nErrNo = ERR_NONE;
        rstSsPkgRsp.m_wTime = (uint16_t)iRet;
    }

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return 0;
}


int GFightQuitMatchReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_FIGHT_QUIT_MATCH_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;

    SS_PKG_GUILD_FIGHT_QUIT_MATCH_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildFightQuitMatchReq;
    SS_PKG_GUILD_FIGHT_QUIT_MATCH_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stGuildFightQuitMatchRsp;
    rstSsPkgRsp.m_nErrNo = ERR_NONE;

    do
    {
        GuildFightArena* poArena = GFightArenaMgr::Instance().GetArenaByGuildId(rstSsPkgReq.m_stQuiterInfo.m_ullGuildId);
        if (poArena == NULL)
        {
            break;
        }

        FightPlayer* poPlayer = poArena->GetPlayer(rstSsPkgReq.m_stQuiterInfo.m_ullUin);
        if (poPlayer == NULL)
        {
            break;
        }

        if (poPlayer->m_stPlayerInfo.m_wState != GUILD_FIGHT_PLAYER_STATE_MATCH_END)
        {
            LOGERR_r("Player(%s) state(%d) is not match end", poPlayer->m_stPlayerInfo.m_szName, poPlayer->m_stPlayerInfo.m_wState);
            break;
        }

        if (poPlayer->m_iFightPvpResult == GUILD_FIGHT_PVP_LOSE)
        {
            FightPlayerStateMachine::Instance().ChangeState(poPlayer, GUILD_FIGHT_PLAYER_STATE_DEAD);
        }
        else if (poPlayer->m_iFightPvpResult == GUILD_FIGHT_PVP_WIN)
        {
            FightPlayerStateMachine::Instance().ChangeState(poPlayer, GUILD_FIGHT_PLAYER_STATE_GOD);
            poPlayer->m_poGuildFightPoint->AddPlayer(poPlayer);
        }
        else
        {
            LOGERR_r("Player(%s) last fightpvp result(%d) is error", poPlayer->m_stPlayerInfo.m_szName, poPlayer->m_iFightPvpResult);
        }

    }while(false);

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return 0;
}

int GFightCancleGodReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    bzero(&m_stSsPkg, sizeof(m_stSsPkg));
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_FIGHT_CANCLE_GOD_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;

    SS_PKG_GUILD_FIGHT_QUIT_MATCH_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildFightQuitMatchReq;
    SS_PKG_GUILD_FIGHT_QUIT_MATCH_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stGuildFightQuitMatchRsp;
    rstSsPkgRsp.m_nErrNo = ERR_NONE;

    do
    {
        GuildFightArena* poArena = GFightArenaMgr::Instance().GetArenaByGuildId(rstSsPkgReq.m_stQuiterInfo.m_ullGuildId);
        if (poArena == NULL)
        {
            break;
        }

        FightPlayer* poPlayer = poArena->GetPlayer(rstSsPkgReq.m_stQuiterInfo.m_ullUin);
        if (poPlayer == NULL)
        {
            break;
        }

        if (poPlayer->m_stPlayerInfo.m_wState != GUILD_FIGHT_PLAYER_STATE_GOD)
        {
            LOGERR_r("Player(%s) state(%d) is not in god state!", poPlayer->m_stPlayerInfo.m_szName, poPlayer->m_stPlayerInfo.m_wState);
            break;
        }
        //  无敌时间置0,其他逻辑由状态机调用处理
        poPlayer->m_iGodLeftTimeMs = 0;
    }while(false);

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return 0;

}


int GFightPvpMatchRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    SS_PKG_GUILD_FIGHT_PVP_MATCH_RSP& rstSsPkgRsp = rstSsPkg.m_stBody.m_stGuildFightPvpMatchRsp;

    GuildFightArena* poArena = GFightArenaMgr::Instance().GetArenaByGuildId(rstSsPkgRsp.m_stPlayerInfo1.m_ullGuildId);
    if (poArena == NULL)
    {
        LOGERR_r("poArena not found,GuildId=%lu, Player1Id=%lu, Player2Id=%lu",rstSsPkgRsp.m_stPlayerInfo1.m_ullGuildId,
                    rstSsPkgRsp.m_stPlayerInfo1.m_ullUin, rstSsPkgRsp.m_stPlayerInfo2.m_ullUin);
        return -1;
    }

    FightPlayer* poPlayer1 = poArena->GetPlayer(rstSsPkgRsp.m_stPlayerInfo1.m_ullUin);
    FightPlayer* poPlayer2 = poArena->GetPlayer(rstSsPkgRsp.m_stPlayerInfo2.m_ullUin);

    if (rstSsPkgRsp.m_nErrNo != ERR_NONE)
    {
        // 可能存在一直匹配的问题，需要关注匹配问题出在哪儿！
        if (poPlayer1!=NULL)
        {
            FightPlayerStateMachine::Instance().ChangeState(poPlayer1, GUILD_FIGHT_PLAYER_STATE_DEAD);
        }

        if (poPlayer2!= NULL)
        {
            FightPlayerStateMachine::Instance().ChangeState(poPlayer2, GUILD_FIGHT_PLAYER_STATE_DEAD);
        }
    }

    return 1;
}


int GFightPvpSettle_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    SS_PKG_GUILD_FIGHT_PVP_SETTLE_NTF& rstSsPkgNtf = rstSsPkg.m_stBody.m_stGuildFightPvpSettleNtf;

    GuildFightArena* poArena = GFightArenaMgr::Instance().GetArenaByGuildId(rstSsPkgNtf.m_stPlayerInfo1.m_ullGuildId);
    if (poArena == NULL)
    {
        LOGERR_r("poArena not found,GuildId=%lu, Player1Id=%lu, Player2Id=%lu",rstSsPkgNtf.m_stPlayerInfo1.m_ullGuildId,
                    rstSsPkgNtf.m_stPlayerInfo1.m_ullUin, rstSsPkgNtf.m_stPlayerInfo2.m_ullUin);
        return -1;
    }

    FightPlayer* poPlayer1 = poArena->GetPlayer(rstSsPkgNtf.m_stPlayerInfo1.m_ullUin);
    FightPlayer* poPlayer2 = poArena->GetPlayer(rstSsPkgNtf.m_stPlayerInfo2.m_ullUin);

    // 更新军团战玩家状态
    if (poPlayer1 != NULL)
    {
       poPlayer1->MatchSettle(rstSsPkgNtf.m_ullWinerUin);
    }
    else
    {
        LOGERR_r("poPlayer not found in Arena, Uin=%lu", rstSsPkgNtf.m_stPlayerInfo1.m_ullUin);
    }

    // 更新军团战玩家状态
    if (poPlayer2 != NULL)
    {
        poPlayer2->MatchSettle(rstSsPkgNtf.m_ullWinerUin);
    }
    else
    {
        LOGERR_r("poPlayer not found in Arena, Uin=%lu", rstSsPkgNtf.m_stPlayerInfo2.m_ullUin);
    }

    return 1;
}

int GFightCreateDungeonRsp_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    SS_PKG_GUILD_FIGHT_PVP_CREATE_DUNGEON_RSP& rstSsPkgRsp = rstSsPkg.m_stBody.m_stGuildFightPvpDunGeonRsp;
    if (rstSsPkgRsp.m_nErrNo == ERR_NONE)
    {
        return 0;
    }

    LOGERR_r("GFight CreateDungeon failed, Player1Id=%lu, Player2Id=%lu", rstSsPkgRsp.m_stPlayerInfo1.m_ullUin, rstSsPkgRsp.m_stPlayerInfo2.m_ullUin);

    GuildFightArena* poArena = GFightArenaMgr::Instance().GetArenaByGuildId(rstSsPkgRsp.m_stPlayerInfo1.m_ullGuildId);
    //战场可能已经结束，不用处理了
    if (poArena == NULL)
    {
        LOGERR_r("poArena not found, GuildId=%lu", rstSsPkgRsp.m_stPlayerInfo1.m_ullGuildId);
        return -1;
    }

    FightPlayer* poPlayer1 = poArena->GetPlayer(rstSsPkgRsp.m_stPlayerInfo1.m_ullUin);
    FightPlayer* poPlayer2 = poArena->GetPlayer(rstSsPkgRsp.m_stPlayerInfo2.m_ullUin);

    // 创建战场失败
    if (poPlayer1!=NULL)
    {
        FightPlayerStateMachine::Instance().ChangeState(poPlayer1, GUILD_FIGHT_PLAYER_STATE_DEAD);
    }

    if (poPlayer2!= NULL)
    {
        FightPlayerStateMachine::Instance().ChangeState(poPlayer2, GUILD_FIGHT_PLAYER_STATE_DEAD);
    }

    return 1;
}




