#include "MsgLogicCloneBattle.h"
#include "LogMacros.h"
#include "common_proto.h"
#include "FakeRandom.h"
#include "../gamedata/GameDataMgr.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "../module/player/PlayerMgr.h"
#include "../module/Match.h"
#include "../module/CloneBattle.h"
#include "../module/Lottery.h"
#include "../module/Item.h"
#include "../module/Props.h"
#include "../module/Task.h"
#include "dwlog_def.h"
#include "ZoneLog.h"

using namespace PKGMETA;


int CloneBattleGetInfoReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }    
    //CS_PKG_CLONE_BATTLE_GET_INFO_REQ& rstCsReq = rstCsPkg.m_stBody.m_stCloneBattleGetInfoReq;
    DT_ROLE_CLONE_BATTLE_INFO& rstCloneBattleInfo = poPlayer->GetPlayerData().GetCloneBattleInfo();

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_CLONE_BATTLE_GET_INFO_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    SS_PKG_CLONE_BATTLE_GET_INFO_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stCloneBattleGetInfoReq;
    rstSsReq.m_ullTeamId = rstCloneBattleInfo.m_ullTeamId;
    ZoneSvrMsgLayer::Instance().SendToCloneBattleSvr(m_stSsPkg);
    return 0;
}

int CloneBattleFightReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    PlayerData& roPData = poPlayer->GetPlayerData();
    DT_ROLE_CLONE_BATTLE_INFO& rstCloneBattleInfo = roPData.GetCloneBattleInfo();
    //CS_PKG_CLONE_BATTLE_FIGHT_REQ& rstCsReq = rstCsPkg.m_stBody.m_stCloneBattleFightReq;
    SS_PKG_CLONE_BATTLE_FIGHT_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stCloneBattleFightReq;
    
    int iRet = ERR_NONE;
    do 
    {
        iRet = CloneBattle::Instance().CheckFight(roPData);
        if (iRet != ERR_NONE)
        {
            break;
        }
        //战斗前在更新一次武将数据
        iRet = CloneBattle::Instance().InitMatInfo(roPData, rstCloneBattleInfo.m_dwGCardId, rstSsReq.m_stMateInfo);
        if (iRet != ERR_NONE)
        {
            LOGERR("Uin<%lu> start fight: InitMateInfo erro, GCard<%u>", poPlayer->GetUin(), rstCloneBattleInfo.m_dwGCardId);
            break;
        }
        if (rstCloneBattleInfo.m_ullTeamId == 0)
        {
            iRet = ERR_CLONEBATTLE_NOT_IN_TEAM;
            LOGERR("Uin<%lu> start fight:isn't in the CloneBattleTeam", poPlayer->GetUin());
            break;
        }
    } while (false);
    if (iRet == ERR_NONE)
    {
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_CLONE_BATTLE_FIGHT_REQ;
        m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
        rstSsReq.m_ullTeamId = rstCloneBattleInfo.m_ullTeamId;
        ZoneSvrMsgLayer::Instance().SendToCloneBattleSvr(m_stSsPkg);
    }
    else
    {
        SC_PKG_CLONE_BATTLE_FIGHT_RSP& rstScRsp = m_stScPkg.m_stBody.m_stCloneBattleFightRsp;
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CLONE_BATTLE_FIGHT_RSP;
        rstScRsp.m_nError = iRet;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }
    return 0;
}

int CloneBattleJoinTeamReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    PlayerData& roPData = poPlayer->GetPlayerData();
    CS_PKG_CLONE_BATTLE_JOIN_TEAM_REQ& rstCsReq = rstCsPkg.m_stBody.m_stCloneBattleJoinTeamReq;
    SS_PKG_CLONE_BATTLE_JOIN_TEAM_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stCloneBattleJoinTeamReq;
    DT_ROLE_CLONE_BATTLE_INFO& rstCloneBattleInfo = roPData.GetCloneBattleInfo();
    int iRet = ERR_NONE;
	uint8_t bIsUseProp = 0;
    do 
    {
        if (rstCsReq.m_bBossType >= MAX_NUM_CLONE_BATTLE_BOSS)
        {
            LOGERR("Uin<%lu> player cheats, m_bBossType<%hhu> error", poPlayer->GetUin(), rstCsReq.m_bBossType);
            iRet = ERR_WRONG_PARA;
            break;
        }
        if ( (iRet = CloneBattle::Instance().CheckLvLimit(roPData)) != ERR_NONE )
        {
            break;
        }
		bool bHaveTicket = CloneBattle::Instance().IsHaveTicket(roPData);
		bool bHaveBossCard = CloneBattle::Instance().IsHaveBossGCard(roPData, rstCsReq.m_bBossType);
		if (!bHaveBossCard && !bHaveTicket)
		{
			iRet = ERR_NOT_ENOUGH_PROPS;
			LOGERR("Uin<%lu> join team , have no ticket", poPlayer->GetUin());
			break;
		}
		if (!bHaveBossCard)
		{
			//使用道具
			bIsUseProp = 1;
		}

        rstCloneBattleInfo.m_dwGCardId = roPData.GetMajestyInfo().m_dwHighGCardLiId;
        iRet = CloneBattle::Instance().InitMatInfo(roPData, rstCloneBattleInfo.m_dwGCardId, rstSsReq.m_stMateInfo);
        if (iRet != ERR_NONE)
        {
            LOGERR("Uin<%lu> join team, InitMateInfo erro, GCard<%u>", poPlayer->GetUin(), rstCloneBattleInfo.m_dwGCardId);
            break;
        }

        rstSsReq.m_ullTeamId = rstCsReq.m_ullTeamId;
        rstSsReq.m_bType = rstCsReq.m_bType;
        rstSsReq.m_bBossType = rstCsReq.m_bBossType;
        rstSsReq.m_bUseProp = bIsUseProp;
        
    } while (false);
    if (iRet != ERR_NONE)
    {
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CLONE_BATTLE_JOIN_TEAM_RSP;
        m_stScPkg.m_stBody.m_stCloneBattleJoinTeamRsp.m_nError = iRet;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }
    else
    {
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_CLONE_BATTLE_JOIN_TEAM_REQ;
        m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
        ZoneSvrMsgLayer::Instance().SendToCloneBattleSvr(m_stSsPkg);

    }
    return 0;
}

int CloneBattleRewardReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    PlayerData& roPData = poPlayer->GetPlayerData();
    //CS_PKG_CLONE_BATTLE_REWARD_REQ& rstCsReq = rstCsPkg.m_stBody.m_stCloneBattleRewardReq;
    SC_PKG_CLONE_BATTLE_REWARD_RSP& rstScRsp = m_stScPkg.m_stBody.m_stCloneBattleRewardRsp;
    DT_ROLE_CLONE_BATTLE_INFO& rstCloneBattleInfo = roPData.GetCloneBattleInfo();
    rstScRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    uint32_t dwBossId = roPData.m_dwCloneBattleFightBossId;
    int iRet = ERR_NONE;
    do 
    {
        if (rstCloneBattleInfo.m_ullTeamId == 0)
        {
            iRet = ERR_CLONEBATTLE_NOT_IN_TEAM;
            LOGERR("Uin<%lu> not in the CloneBattleTeam", poPlayer->GetUin());
            break;
        }
        RESCLONEBATTLEFIGHTREWARD* pResReward = CGameDataMgr::Instance().GetResCloneBattleFightRewardMgr().Find(dwBossId);
        if (!pResReward)
        {
            LOGERR("Uin<%lu> the RESCLONEBATTLEFIGHTREWARD<%u> is NULL", poPlayer->GetUin(), dwBossId);
            iRet = ERR_SYS;
            break;
        }
        if (rstCloneBattleInfo.m_bRewardMap == 0)
        {
            //结束后首次选箱子 检查次数,记录,并通知CloneBattleSvr处理
            iRet = CloneBattle::Instance().CheckFight(roPData);
            if (iRet != ERR_NONE)
            {
                LOGERR("Uin<%lu> have no fight num, player cheated or the team dissovled", poPlayer->GetUin());
                break;
            }
            //首次战斗胜利
            if (rstCloneBattleInfo.m_bWinNum == 0)
            {
                if (rstCloneBattleInfo.m_bUseProp == 1)
                {
                    CloneBattle::Instance().ConsumeTicket(poPlayer->GetPlayerData(), rstScRsp.m_stSyncItemInfo);
                }
            }
            uint8_t bSelect = CFakeRandom::Instance().Random(1, 500) % 2;
            iRet = CloneBattle::Instance().OpenRewardBox(roPData, dwBossId, bSelect, rstScRsp.m_stSyncItemInfo);
            if (iRet != ERR_NONE)
            {
                LOGERR("Uin<%lu> OpenRewardBox error, bossId<%u>", poPlayer->GetUin(), dwBossId);
                break;
            }

            rstCloneBattleInfo.m_bRewardMap |= 1 << bSelect;
            rstCloneBattleInfo.m_bWinNum++;

            m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_CLONE_BATTLE_REWARD_NTF;
            m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
            m_stSsPkg.m_stBody.m_stCloneBattleRewardNtf.m_ullTeamId = rstCloneBattleInfo.m_ullTeamId;
            ZoneSvrMsgLayer::Instance().SendToCloneBattleSvr(m_stSsPkg);

        }
        else if (rstCloneBattleInfo.m_bRewardMap == 3)
        {
            iRet = ERR_CLONEBATTLE_ALREADY_REWARD_ALL;
            LOGERR("Uin<%lu> have got the award", poPlayer->GetUin());
            break;
        }
        else
        {
            //购买剩下的奖励
            RESBASIC *poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(CLONE_GENERAL_BASIC);
            assert(poResBasic);
            int iDiamond = poResBasic->m_para[1];
            if (!Item::Instance().IsEnough(&roPData, ITEM_TYPE_DIAMOND, 0 ,iDiamond))
            {
                LOGERR("Uin<%lu> get extra award: have no diamond<%d>", roPData.m_ullUin, iDiamond);
                iRet = ERR_NOT_ENOUGH_DIAMOND;
                break;
            }
            uint8_t bUnSelect = (~rstCloneBattleInfo.m_bRewardMap & 3) - 1;
            iRet = CloneBattle::Instance().OpenRewardBox(roPData, dwBossId, bUnSelect, rstScRsp.m_stSyncItemInfo);
            if (iRet != ERR_NONE)
            {
                LOGERR("Uin<%lu> OpenRewardBox error, bossId<%u>", poPlayer->GetUin(), dwBossId);
                break;
            }
            Item::Instance().ConsumeItem(&roPData, ITEM_TYPE_DIAMOND, 0,  -iDiamond, rstScRsp.m_stSyncItemInfo, DWLOG::METHOD_CLONEBATTLE_REWARD);
            rstCloneBattleInfo.m_bRewardMap |= 1 << bUnSelect;
            break;
        }

        //完成胜利任务
        Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_PVP, 1, 7, 1);
    } while (false);
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CLONE_BATTLE_REWARD_RSP;
    rstScRsp.m_nError = iRet;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int CloneBattleQuitTeamReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    
    SS_PKG_CLONE_BATTLE_QUIT_TEAM_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stCloneBattleQuitTeamReq;
    rstSsReq.m_ullTeamId = poPlayer->GetPlayerData().GetCloneBattleInfo().m_ullTeamId;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_CLONE_BATTLE_QUIT_TEAM_REQ;
    ZoneSvrMsgLayer::Instance().SendToCloneBattleSvr(m_stSsPkg);
    return 0;
}

int CloneBattleSetTeamReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    CS_PKG_CLONE_BATTLE_SET_TEAM_REQ& rstCsReq = rstCsPkg.m_stBody.m_stCloneBattleSetTeamReq;
    SS_PKG_CLONE_BATTLE_SET_TEAM_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stCloneBattleSetTeamReq;
    DT_ROLE_CLONE_BATTLE_INFO& rstCloneBattleInfo = poPlayer->GetPlayerData().GetCloneBattleInfo();
    int iRet = ERR_NONE;
    do
    {
        if (rstCloneBattleInfo.m_ullTeamId == 0)
        {
            iRet = ERR_CLONEBATTLE_NOT_IN_TEAM;
            LOGERR("Uin<%lu> set team: isn't is the CloneBattleTeam", poPlayer->GetUin());
            break;
        }
        
        if (rstCsReq.m_bType == 2)  //更换武将
        {
            iRet = CloneBattle::Instance().InitMatInfo(poPlayer->GetPlayerData(), rstCsReq.m_dwGCardId, rstSsReq.m_stMateInfo);
            if (iRet != ERR_NONE)
            {
                LOGERR("Uin<%lu> set team: InitMateInfo erro,GarcId<%u>", poPlayer->GetUin(), rstCsReq.m_dwGCardId);
                break;
            }
        }
        rstSsReq.m_ullTeamId = rstCloneBattleInfo.m_ullTeamId;
        rstSsReq.m_bMatch = rstCsReq.m_bMatch;
        rstSsReq.m_bType = rstCsReq.m_bType;
    } while (false);

    if (iRet != ERR_NONE)
    {
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CLONE_BATTLE_SET_TEAM_RSP;
        m_stScPkg.m_stBody.m_stCloneBattleSetTeamRsp.m_nError = iRet;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }
    else
    {
        m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_CLONE_BATTLE_SET_TEAM_REQ;
        ZoneSvrMsgLayer::Instance().SendToCloneBattleSvr(m_stSsPkg);
    }
    return 0;
}






int CloneBattleQuitTeamRsp_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer<%lu>  is null, MsgId<%hu>", rstSsPkg.m_stHead.m_ullUin, rstSsPkg.m_stHead.m_wMsgId);
        return -1;
    }
    SS_PKG_CLONE_BATTLE_QUIT_TEAM_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stCloneBattleQuitTeamRsp;
    SC_PKG_CLONE_BATTLE_QUIT_TEAM_RSP& rstScRsp = m_stScPkg.m_stBody.m_stCloneBattleQuitTeamRsp;
    if (rstSsRsp.m_nError == ERR_NONE)
    {
        poPlayer->GetPlayerData().GetCloneBattleInfo().m_ullTeamId = 0;

    }
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CLONE_BATTLE_QUIT_TEAM_RSP;
    rstScRsp.m_nError = rstSsRsp.m_nError;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}



int CloneBattleJoinTeamRsp_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer<%lu>  is null, MsgId<%hu>", rstSsPkg.m_stHead.m_ullUin, rstSsPkg.m_stHead.m_wMsgId);
        return -1;
    }

    SS_PKG_CLONE_BATTLE_JOIN_TEAM_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stCloneBattleJoinTeamRsp;
    SC_PKG_CLONE_BATTLE_JOIN_TEAM_RSP& rstScRsp = m_stScPkg.m_stBody.m_stCloneBattleJoinTeamRsp;

    rstScRsp.m_nError = rstSsRsp.m_nError;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CLONE_BATTLE_JOIN_TEAM_RSP;
    rstScRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    memcpy(&rstScRsp.m_stTeamInfo, &rstSsRsp.m_stTeamInfo, sizeof(rstScRsp.m_stTeamInfo));
    if (rstSsRsp.m_nError == ERR_NONE)
    {
        poPlayer->GetPlayerData().GetCloneBattleInfo().m_ullTeamId = rstSsRsp.m_stTeamInfo.m_ullId;
        poPlayer->GetPlayerData().GetCloneBattleInfo().m_ullLastUptTime = CloneBattle::Instance().GetLastUptTimesMs();
        poPlayer->GetPlayerData().GetCloneBattleInfo().m_bUseProp = rstSsRsp.m_bUseProp;

        
    }
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int CloneBattleFightRsp_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer<%lu>  is null, MsgId<%hu>", rstSsPkg.m_stHead.m_ullUin, rstSsPkg.m_stHead.m_wMsgId);
        return -1;
    }
    DT_ROLE_CLONE_BATTLE_INFO& rstCloneBattleInfo = poPlayer->GetPlayerData().GetCloneBattleInfo();
    SS_PKG_CLONE_BATTLE_FIGHT_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stCloneBattleFightRsp;
    SC_PKG_CLONE_BATTLE_FIGHT_RSP& rstScRsp = m_stScPkg.m_stBody.m_stCloneBattleFightRsp;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CLONE_BATTLE_FIGHT_RSP;
    rstScRsp.m_nError = rstSsRsp.m_nError;
    for (int i = 0; i < MAX_NUM_CLONE_BATTLE_MATE; i++)
    {
        memcpy(&rstScRsp.m_astTroopList[i], &rstSsRsp.m_stTeamInfo.m_astMateInfo[i].m_stTroopInfo, sizeof(DT_TROOP_INFO));
    }
    if (rstSsRsp.m_nError == ERR_NONE)
    {
        //重置结算
        rstCloneBattleInfo.m_bRewardMap = 0;

        //修改参与任务
        Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_PVP, 1, 7, 3);
    }
    poPlayer->GetPlayerData().m_dwCloneBattleFightBossId = rstSsRsp.m_stTeamInfo.m_dwBossId;
    rstScRsp.m_dwBossId = rstSsRsp.m_stTeamInfo.m_dwBossId;
    ZoneLog::Instance().WriteCloneBattleLog(poPlayer, rstSsRsp.m_stTeamInfo.m_bCount, rstScRsp.m_dwBossId);
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int CloneBattleBroadcastNtf_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    SS_PKG_CLONE_BATTLE_BROADCAST_NTF& rstSsNtf = rstSsPkg.m_stBody.m_stCloneBattleBroadCastNtf;
    switch (rstSsNtf.m_bNtfType)
    {
    case CLONE_BATTLE_NTF_TYPE_TEAM_UPT:
    {
        CloneBattle::Instance().BroadcastMate(rstSsNtf);
        break;
    }
    case CLONE_BATTLE_NTF_TYPE_TEAM_REWARD:
    {
        CloneBattle::Instance().BroadcastMate(rstSsNtf);
        CloneBattle::Instance().SendTeamReward(rstSsNtf.m_stTeamInfo);
        break;
    }
    case CLONE_BATTLE_NTF_TYPE_TEAM_DEL:
    {
        //此通知只有系统解散才有,玩家解散是会走QuitTeamRsp
        CloneBattle::Instance().HandleSysDissovleTeam(rstSsNtf.m_stTeamInfo);
        break;
    }
    default:
        LOGERR("SS_PKG_CLONE_BATTLE_BROADCAST_NTF type<%hhu> error", rstSsNtf.m_bNtfType);
        break;
    }
    return 0;
}

int CloneBattleGetInfoRsp_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{

    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer<%lu>  is null, MsgId<%hu>", rstSsPkg.m_stHead.m_ullUin, rstSsPkg.m_stHead.m_wMsgId);
        return -1;
    }

    SS_PKG_CLONE_BATTLE_GET_INFO_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stCloneBattleGetInfoRsp;
    
    SC_PKG_CLONE_BATTLE_GET_INFO_RSP& rstScRsp = m_stScPkg.m_stBody.m_stCloneBattleGetInfoRsp;
    

    if (rstSsRsp.m_nError == ERR_NONE)
    {
        memcpy(&rstScRsp.m_stBossInfo, &rstSsRsp.m_stBossInfo, sizeof(DT_CLONE_BATTLE_BOSS_INFO));
        memcpy(&rstScRsp.m_stTeamInfo, &rstSsRsp.m_stTeamInfo, sizeof(DT_CLONE_BATTLE_TEAM_INFO));
        
    }
    rstScRsp.m_nError = rstSsRsp.m_nError;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CLONE_BATTLE_GET_INFO_RSP;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}




int CloneBattleSetTeamRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer<%lu>  is null, MsgId<%hu>", rstSsPkg.m_stHead.m_ullUin, rstSsPkg.m_stHead.m_wMsgId);
        return -1;
    }
    SS_PKG_CLONE_BATTLE_SET_TEAM_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stCloneBattleSetTeamRsp;
    SC_PKG_CLONE_BATTLE_SET_TEAM_RSP& rstScRsp = m_stScPkg.m_stBody.m_stCloneBattleSetTeamRsp;
    if (rstSsRsp.m_nError == ERR_NONE)
    {
        //更换武将成功
        if (rstSsRsp.m_bType == 2)
        {
            poPlayer->GetPlayerData().GetCloneBattleInfo().m_dwGCardId = rstSsRsp.m_dwGCardId;
        }
    }
    rstScRsp.m_nError = rstSsRsp.m_nError;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_CLONE_BATTLE_SET_TEAM_RSP;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}


int CloneBattleUptSysInfoNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
    SS_PKG_CLONE_BATTLE_UPT_SYSINFO_NTF& rstSsNtf = rstSsPkg.m_stBody.m_stCloneBattleUptSysInfoNtf;
    CloneBattle::Instance().UptSysInfo(rstSsNtf);
    return 0;
}

