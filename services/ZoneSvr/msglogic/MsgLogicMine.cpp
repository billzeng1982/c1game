
#include "LogMacros.h"
#include "ss_proto.h"
#include "cs_proto.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "../module/player/PlayerMgr.h"
#include "../module/Mine.h"
#include "MsgLogicMine.h"
#include "../module/MasterSkill.h"
#include "../module/ZoneLog.h"
#include "../module/Task.h"

int MineGetInfoReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    SS_PKG_MINE_GET_INFO_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stMineGetInfoReq;
    StrCpy(rstSsReq.m_szName, poPlayer->GetRoleName(), sizeof(rstSsReq.m_szName));
	rstSsReq.m_wLv = poPlayer->GetRoleLv();
	rstSsReq.m_wIconId = poPlayer->GetRoleIconId();
	rstSsReq.m_dwLeaderValue = poPlayer->GetLeaderValue();
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MINE_GET_INFO_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
	m_stSsPkg.m_stHead.m_ullReservId = 0;
    ZoneSvrMsgLayer::Instance().SendToMineSvr(m_stSsPkg);

    return 0;
}

int MineExploreReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    SS_PKG_MINE_EXPLORE_REQ& rstReq = m_stSsPkg.m_stBody.m_stMineExploreReq;
	rstReq.m_bOwnOreCount = 0;
    DT_ROLE_MINE_INFO& rstMineInfo = poPlayer->GetPlayerData().GetMineInfo();
    int iRet = ERR_NONE;
    do 
    {
        if (!Mine::Instance().IsOpen())
        {
            iRet = ERR_MODULE_NOT_OPEN;
            break;
        }
		if (!Mine::Instance().IsLvEnough(poPlayer->GetPlayerData()))
		{
			iRet = ERR_LEVEL_LIMIT;
			break;
		}
        if (!Mine::Instance().IsExploreCountEnough(rstMineInfo))
        {
            iRet = ERR_NOT_SATISFY_COND;
            break;
        }
        
    } while (0);

    if (iRet == ERR_NONE)
    {
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MINE_EXPLORE_REQ;
        m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
		m_stSsPkg.m_stHead.m_ullReservId = 0;
		rstReq.m_dwTeamLi = (uint32_t) (poPlayer->GetPlayerData().GetMajestyInfo().m_dwHighGCardLi 
			* Mine::Instance().GetTeamLiRate() + 0.5);
        ZoneSvrMsgLayer::Instance().SendToMineSvr(m_stSsPkg);
    }
    else
    {
        LOGERR("Uin<%lu> MineExploreReq_CSerror<%d> ", poPlayer->GetUin(), iRet);
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MINE_EXPLORE_RSP;

        m_stScPkg.m_stBody.m_stMineGetInfoRsp.m_nErrNo = iRet;
        m_stScPkg.m_stBody.m_stMineGetInfoRsp.m_bAwardCount = 0;
        m_stScPkg.m_stBody.m_stMineGetInfoRsp.m_bExploreOreCount = 0;
        m_stScPkg.m_stBody.m_stMineGetInfoRsp.m_bOwnOreCount = 0;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }
    return 0;
}

int MineDealOreReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_MINE_DEAL_ORE_REQ& rstCsReq = rstCsPkg.m_stBody.m_stMineDealOreReq;
    SS_PKG_MINE_DEAL_ORE_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stMineDealOreReq;
    bzero(&rstSsReq, sizeof(rstSsReq));
    PlayerData& rstPlayerData = poPlayer->GetPlayerData();
    int iRet = ERR_NONE;
    do
    {
        if (!Mine::Instance().IsOpen())
        {
            iRet = ERR_MODULE_NOT_OPEN;
            break;
        }
		if (!Mine::Instance().IsLvEnough(poPlayer->GetPlayerData()))
		{
			iRet = ERR_LEVEL_LIMIT;
			break;
		}
        if (rstCsReq.m_bDealType == MINE_ORE_DEAL_TYPE_OCCPUY)  //占领
        {
            if ((iRet = Mine::Instance().GetTroopInfo(rstPlayerData, rstCsReq.m_bMSId, rstCsReq.m_TroopFormation, rstSsReq))
                != ERR_NONE)
            {
                break;
            }
            
        }
        else if (rstCsReq.m_bDealType == MINE_ORE_DEAL_TYPE_DROP)	//放弃矿
        {
        }
        else if (rstCsReq.m_bDealType == MINE_ORE_DEAL_TYPE_MODFIY)	//修改上阵武将
        {
            DT_ITEM_MSKILL* pstMSkill = MasterSkill::Instance().GetMSkillInfo(&rstPlayerData, rstCsReq.m_bMSId);
            if (!pstMSkill)
            {
                iRet = ERR_WRONG_PARA;
                break;
            }
            memcpy(&rstSsReq.m_stMasterSkill, pstMSkill, sizeof(DT_ITEM_MSKILL));
        }
        else if (rstCsReq.m_bDealType == MINE_ORE_DEAL_TYPE_CHALLENGE_REQUEST)	//挑战
        {
            if (!Mine::Instance().IsChallengeCountEnough(rstPlayerData.GetMineInfo()))
            {
                iRet = ERR_MINE_COUNT_NOT_ENOUGH;
                break;
            }
        }
        else if (rstCsReq.m_bDealType == MINE_ORE_DEAL_TYPE_GIVE_UP_GRAB)	//放弃占领矿 (高级矿)
        {
        }
        else if (rstCsReq.m_bDealType == MINE_ORE_DEAL_TYPE_GRAB)	//占领矿 (高级矿)
        {
            if ((iRet = Mine::Instance().GetTroopInfo(rstPlayerData, rstCsReq.m_bMSId, rstCsReq.m_TroopFormation, rstSsReq))
                != ERR_NONE)
            {
                break;
            }
            StrCpy(rstSsReq.m_szName, rstPlayerData.GetRoleName(), sizeof(rstSsReq.m_szName));
        }
        else if (rstCsReq.m_bDealType == MINE_ORE_DEAL_TYPE_INVESTIGATE)	//调查
        {
        }
		else if (rstCsReq.m_bDealType == MINE_ORE_DEAL_TYPE_REVENGE)		//复仇
		{
			if (!Mine::Instance().IsRevengeCountEnough(rstPlayerData.GetMineInfo()))
			{
				iRet = ERR_MINE_COUNT_NOT_ENOUGH;
				break;
			}
		}
        else
        {
            iRet = ERR_WRONG_PARA;
            break;
        }
    } while (0);
    if (iRet == ERR_NONE)
    {
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MINE_DEAL_ORE_REQ;
        m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
		m_stSsPkg.m_stHead.m_ullReservId = 0;
        rstSsReq.m_ullOreUid = rstCsReq.m_ullOreUid;
        rstSsReq.m_bDealType = rstCsReq.m_bDealType;
        ZoneSvrMsgLayer::Instance().SendToMineSvr(m_stSsPkg);
    }
    else
    {
        LOGERR("Uin<%lu> MineDealOreReq_CS error<%d> OreUid<%lu> DealType<%hhu>, MSId<%hhu>", poPlayer->GetUin(), 
            iRet, rstCsReq.m_ullOreUid, rstCsReq.m_bDealType, rstCsReq.m_bMSId);
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MINE_DEAL_ORE_RSP;
        bzero(&m_stScPkg.m_stBody.m_stMineDealOreRsp, sizeof(m_stScPkg.m_stBody.m_stMineDealOreRsp));
        m_stScPkg.m_stBody.m_stMineDealOreRsp.m_nErrNo = iRet;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    }
    return 0;

}

int MineGetAwardReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_MINE_GET_AWARD_REQ& rstCsReq = rstCsPkg.m_stBody.m_stMineGetAwardReq;
    SS_PKG_MINE_GET_AWARD_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stMineGetAwardReq;
    if (rstCsReq.m_bType == 0 || rstCsReq.m_bType == 1)
    {
        m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
		m_stSsPkg.m_stHead.m_ullReservId = 0;
        rstSsReq.m_bIndex = rstCsReq.m_bIndex;
        rstSsReq.m_bType = rstCsReq.m_bType;
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MINE_GET_AWARD_REQ;
        ZoneSvrMsgLayer::Instance().SendToMineSvr(m_stSsPkg);
    }
    else
    {
        m_stScPkg.m_stBody.m_stMineGetAwardRsp.m_nErrNo = ERR_SYS;
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MINE_GET_AWARD_RSP;
        m_stScPkg.m_stBody.m_stMineDealOreRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }

    return 0;

}


int MineBuyCountReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    CS_PKG_MINE_BUY_COUNT_REQ& rstCsReq = rstCsPkg.m_stBody.m_stMineBuyCountReq;
    SC_PKG_MINE_BUY_COUNT_RSP& rstScRsp = m_stScPkg.m_stBody.m_stMineBuyCountRsp;
    rstScRsp.m_bType = rstCsReq.m_bType;
	rstScRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    if (rstCsReq.m_bType == 1)  //探索
    {
        rstScRsp.m_nErrNo = Mine::Instance().BuyExploreCount(poPlayer->GetPlayerData(), rstScRsp.m_stSyncItemInfo);
    }
    else if (rstCsReq.m_bType == 2) //挑战
    {
        rstScRsp.m_nErrNo = Mine::Instance().BuyChallengeCount(poPlayer->GetPlayerData(), rstScRsp.m_stSyncItemInfo);
    }
    else if (rstCsReq.m_bType ==  3)    //复仇
    {
        rstScRsp.m_nErrNo = Mine::Instance().BuyRevengeCount(poPlayer->GetPlayerData(), rstScRsp.m_stSyncItemInfo);
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MINE_BUY_COUNT_RSP;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;

}

int MineFightResultReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (NULL == poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}
	LOGWARN("Uin<%lu> Name<%s> MineFightResultReq_CS", poPlayer->GetUin(), poPlayer->GetRoleName());
	CS_PKG_MINE_FIGHT_RESULT_REQ& rstCsReq = rstCsPkg.m_stBody.m_stMineFightResultReq;
	SS_PKG_MINE_FIGHT_RESULT_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stMineFightResultReq;
	rstSsReq.m_bWinFlag = rstCsReq.m_bWinFlag;
	rstSsReq.m_ullObjOreUid = rstCsReq.m_ullObjOreUid;
	rstSsReq.m_ullObjUin = rstCsReq.m_ullObjUin;
	rstSsReq.m_bFightType = rstCsReq.m_bFightType;
	rstSsReq.m_bRevengeLogIndex = rstCsReq.m_bRevengeLogIndex;
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MINE_FIGHT_RESULT_REQ;
	m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
	m_stSsPkg.m_stHead.m_ullReservId = 0;
	ZoneSvrMsgLayer::Instance().SendToMineSvr(m_stSsPkg);
	return 0;
}

int MineGetRevengerInfoReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (NULL == poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

	CS_PKG_MINE_GET_REVENGER_INFO_REQ& rstCsReq = rstCsPkg.m_stBody.m_stMineGetRevengerInfoReq;
	SS_PKG_MINE_GET_REVENGER_INFO_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stMineGetRevengerInfoReq;
	rstSsReq.m_ullObjUin = rstCsReq.m_ullObjUin;

	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MINE_GET_REVENGER_INFO_REQ;
	m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
	m_stSsPkg.m_stHead.m_ullReservId = 0;
	ZoneSvrMsgLayer::Instance().SendToMineSvr(m_stSsPkg);
	return 0;
}

//*** SS 协议

int MineGetInfoRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer is null");
        return -1;
    }
    //填写回复包
    SS_PKG_MINE_GET_INFO_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stMineGetInfoRsp;
    SC_PKG_MINE_GET_INFO_RSP& rstScRsp = m_stScPkg.m_stBody.m_stMineGetInfoRsp;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MINE_GET_INFO_RSP;
    rstScRsp.m_nErrNo = rstSsRsp.m_nErrNo;

    if (rstScRsp.m_nErrNo == ERR_NONE)
    {
        rstScRsp.m_bOwnOreCount = rstSsRsp.m_bOwnOreCount;
        rstScRsp.m_bExploreOreCount = rstSsRsp.m_bExploreOreCount;
        rstScRsp.m_bAwardCount = rstSsRsp.m_bAwardCount;
		rstScRsp.m_bComLogCount = rstSsRsp.m_bComLogCount;
		rstScRsp.m_bRevengeLogCount = rstSsRsp.m_bRevengeLogCount;
        memcpy(rstScRsp.m_astOwnOreList, rstSsRsp.m_astOwnOreList, sizeof(DT_MINE_ORE_INFO) * rstSsRsp.m_bOwnOreCount);
        memcpy(rstScRsp.m_astExploreOreList, rstSsRsp.m_astExploreOreList, sizeof(DT_MINE_ORE_INFO) * rstSsRsp.m_bExploreOreCount);
        memcpy(rstScRsp.m_astAwardList, rstSsRsp.m_astAwardList, sizeof(DT_MINE_AWARD) * rstSsRsp.m_bAwardCount);
		memcpy(rstScRsp.m_astComLogList, rstSsRsp.m_astComLogList, sizeof(DT_MINE_COM_LOG)* rstSsRsp.m_bComLogCount);
		memcpy(rstScRsp.m_astRevengeLogList, rstSsRsp.m_astRevengeLogList, sizeof(DT_MINE_REVENGE_LOG) * rstSsRsp.m_bRevengeLogCount);
		Mine::Instance().RefreshGCardState(poPlayer->GetPlayerData(), rstSsRsp.m_bOwnOreCount, rstSsRsp.m_astOwnOreList);
    }
	else
	{
		rstScRsp.m_bOwnOreCount = 0;
		rstScRsp.m_bExploreOreCount = 0;
		rstScRsp.m_bAwardCount = 0;
		rstScRsp.m_bComLogCount = 0;
		rstScRsp.m_bRevengeLogCount = 0;
	}

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int MineExploreRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer is null");
        return -1;
    }
    SS_PKG_MINE_EXPLORE_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stMineExploreRsp;
    if (rstSsRsp.m_nErrNo == ERR_NONE)
    {
        Mine::Instance().ConsumeExploreCount(poPlayer->GetPlayerData().GetMineInfo());
    }

    //填写回复包
    SC_PKG_MINE_EXPLORE_RSP& rstScRsp = m_stScPkg.m_stBody.m_stMineExploreRsp;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MINE_EXPLORE_RSP;
    rstScRsp.m_nErrNo = rstSsRsp.m_nErrNo;
    rstScRsp.m_bExploreOreCount = rstSsRsp.m_bExploreOreCount;
    memcpy(rstScRsp.m_astExploreOreList, rstSsRsp.m_astExploreOreList, sizeof(rstScRsp.m_astExploreOreList));
    ZoneLog::Instance().WriteMineExploreLog(&(poPlayer->GetPlayerData()), rstSsRsp);
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int MineDealOreRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer is null");
        return -1;
    }
    SS_PKG_MINE_DEAL_ORE_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stMineDealOreRsp;
    SC_PKG_MINE_DEAL_ORE_RSP& rstScRsp = m_stScPkg.m_stBody.m_stMineDealOreRsp;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MINE_DEAL_ORE_RSP;
    rstScRsp.m_nErrNo = rstSsRsp.m_nErrNo;
    rstScRsp.m_bDealType = rstSsRsp.m_bDealType;
    rstScRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    bzero(&rstScRsp.m_stOreInfo, sizeof(rstScRsp.m_stOreInfo));
    PlayerData& rstData = poPlayer->GetPlayerData();
    if (rstSsRsp.m_nErrNo == ERR_NONE)
    {
        if (rstSsRsp.m_bDealType == MINE_ORE_DEAL_TYPE_OCCPUY)
        {
			Mine::Instance().ConsumeChallengeCount(rstData.GetMineInfo());
            memcpy(&rstScRsp.m_stOreInfo, &rstSsRsp.m_stOre, sizeof(DT_MINE_ORE_INFO));
            Mine::Instance().SetGCardState(rstData, rstSsRsp.m_stOre, true);
			if (rstScRsp.m_nErrNo == ERR_NONE)
			{
				//占领矿
				Task::Instance().ModifyData(&rstData, TASK_VALUE_TYPE_MINE, 1, 3);
			}
        }
        else if (rstSsRsp.m_bDealType == MINE_ORE_DEAL_TYPE_DROP)
        {
            Mine::Instance().SetGCardState(rstData, rstSsRsp.m_stOre, false);
        }
        else if (rstSsRsp.m_bDealType == MINE_ORE_DEAL_TYPE_INVESTIGATE)
        {
            //这里需要返回错误号
            rstScRsp.m_nErrNo  = Mine::Instance().InvestigateOre(rstData, rstSsRsp.m_stOre, rstScRsp.m_stSyncItemInfo);
			if (rstScRsp.m_nErrNo == ERR_NONE)
			{
				//调查矿
				Task::Instance().ModifyData(&rstData, TASK_VALUE_TYPE_MINE, 1, 4); 
			}
        }
        else if (rstSsRsp.m_bDealType == MINE_ORE_DEAL_TYPE_MODFIY)
        {
			Mine::Instance().SetGCardState(rstData, rstSsRsp.m_stOre, true);
        }
        else if (rstSsRsp.m_bDealType == MINE_ORE_DEAL_TYPE_CHALLENGE_REQUEST)
        {
			Mine::Instance().ConsumeChallengeCount(rstData.GetMineInfo()); 
			Mine::Instance().ConsumeGCardAP(rstData);
        }
        else if (rstSsRsp.m_bDealType == MINE_ORE_DEAL_TYPE_GIVE_UP_GRAB)
        {
        }
        else if (rstSsRsp.m_bDealType == MINE_ORE_DEAL_TYPE_GRAB)
        {
            memcpy(&rstScRsp.m_stOreInfo, &rstSsRsp.m_stOre, sizeof(DT_MINE_ORE_INFO));
            Mine::Instance().SetGCardState(rstData, rstSsRsp.m_stOre, true);

        }
		else if (rstSsRsp.m_bDealType == MINE_ORE_DEAL_TYPE_REVENGE)
		{
			Mine::Instance().ConsumeRevengeCount(rstData.GetMineInfo());
			Mine::Instance().ConsumeGCardAP(rstData);
		}
    }

    if (rstScRsp.m_bDealType == MINE_ORE_DEAL_TYPE_OCCPUY ||
        rstScRsp.m_bDealType == MINE_ORE_DEAL_TYPE_DROP ||
        rstScRsp.m_bDealType == MINE_ORE_DEAL_TYPE_GRAB ||
        rstScRsp.m_bDealType == MINE_ORE_DEAL_TYPE_GIVE_UP_GRAB)
    {
        ZoneLog::Instance().WriteMineOpLog(&rstData, rstSsRsp);
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;


}

int MineGetAwardRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer is null");
        return -1;
    }
    SS_PKG_MINE_GET_AWARD_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stMineGetAwardRsp;
    SC_PKG_MINE_GET_AWARD_RSP& rstScRsp = m_stScPkg.m_stBody.m_stMineGetAwardRsp;
    rstScRsp.m_nErrNo = rstSsRsp.m_nErrNo;
    rstScRsp.m_bType = rstSsRsp.m_bType;
    rstScRsp.m_bIndex = rstSsRsp.m_bIndex;
    rstScRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    if (rstSsRsp.m_nErrNo == ERR_NONE)
    {
        Mine::Instance().GetAwardByAwardLog(poPlayer->GetPlayerData(), rstSsRsp.m_bAwardCount, rstSsRsp.m_astAwardList, rstScRsp);
    }
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MINE_GET_AWARD_RSP;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

int MineFightResultRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
	if (!poPlayer)
	{
		LOGERR("poPlayer is null");
		return -1;
	}

	SS_PKG_MINE_FIGHT_RESULT_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stMineFightResultRsp;
	SC_PKG_MINE_FIGHT_RESULT_RSP& rstScRsp = m_stScPkg.m_stBody.m_stMineFightResultRsp;
	rstScRsp.m_nErrNo = rstSsRsp.m_nErrNo;
	if (rstScRsp.m_nErrNo == ERR_NONE)
	{
		if (rstScRsp.m_bFightType == 2)	//复仇
		{
			Mine::Instance().ConsumeRevengeCount(poPlayer->GetPlayerData().GetMineInfo());
		}
		else	//正常挑战
		{
			//尝试掠夺矿
			Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_MINE, 1, 2);
			if (rstSsRsp.m_bWinFlag == 1)
			{
				//成功掠夺
				Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_MINE, 1, 1);
			}
		}
		
	}
	rstScRsp.m_bWinFlag = rstSsRsp.m_bWinFlag;
	rstScRsp.m_bFightType = rstSsRsp.m_bFightType;
	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MINE_FIGHT_RESULT_RSP;
	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
	return 0;
}


int MineInfoNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        //LOGERR("poPlayer is null");
        return -1;
    }

    SS_PKG_MINE_INFO_NTF& rstSsNtf = rstSsPkg.m_stBody.m_stMineInfoNtf;
    SC_PKG_MINE_INFO_NTF& rstScNtf = m_stScPkg.m_stBody.m_stMineInfoNtf;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MINE_INFO_NTF;
    rstScNtf.m_bType = rstSsNtf.m_bType;
    if (rstSsNtf.m_bType == MINE_NTF_TYPE_ADD_AWARD)
    {
        DT_MINE_AWARD& rstAward = rstSsNtf.m_stInfoNtf.m_stAddAward;
        LOGWARN("Uin<%lu> CreateTime<%lu> State<%hhu>,PropsNum<%hhu>, ItemType<%hhu> ItemNum<%u>", rstSsPkg.m_stHead.m_ullUin, rstAward.m_ullCreateTime, rstAward.m_bState,
            rstAward.m_bPropNum, rstAward.m_astPropList[0].m_bItemType, rstAward.m_astPropList[0].m_dwItemNum);
    }
    memcpy(&rstScNtf.m_stInfoNtf, &rstSsNtf.m_stInfoNtf, sizeof(rstScNtf.m_stInfoNtf));
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}


int MineGetRevengerInfoRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
	if (!poPlayer)
	{
		LOGERR("poPlayer is null");
		return -1;
	}
	SS_PKG_MINE_GET_REVENGER_INFO_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stMineGetRevengerInfoRsp;
	SC_PKG_MINE_GET_REVENGER_INFO_RSP& rstScRsp = m_stScPkg.m_stBody.m_stMineGetRevengerInfoRsp;
	rstScRsp.m_nErrNo = rstSsRsp.m_nErrNo;
	if (rstSsRsp.m_nErrNo == ERR_NONE)
	{
		rstScRsp.m_bObjCount = rstSsRsp.m_bObjCount;
		memcpy(rstScRsp.m_astObjOwnOreList, rstSsRsp.m_astObjOwnOreList, sizeof(DT_MINE_ORE_INFO) * rstSsRsp.m_bObjCount);
	}
	else
	{
		rstScRsp.m_bObjCount = 0;
	}
	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MINE_GET_REVENGER_INFO_RSP;
	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
	return 0;
}

int MineSeasonSettleNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
	SS_PKG_MINE_SEASON_SETTLE_NTF& rstNtf = rstSsPkg.m_stBody.m_stMineSeasonSettleNtf;
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
	SS_PKG_MAIL_ADD_REQ& rstMailAddReq = m_stSsPkg.m_stBody.m_stMailAddReq;
	rstMailAddReq.m_nUinCount = 1;
	rstMailAddReq.m_UinList[0] = rstNtf.m_ullObjUin;

	DT_MAIL_DATA& rstMailData = rstMailAddReq.m_stMailData;
	rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
	rstMailData.m_bState = MAIL_STATE_UNOPENED;
	rstMailData.m_ullFromUin = 0;
	rstMailData.m_ullTimeStampMs = 0;
	RESPRIMAIL* poResPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(10100);
	assert(poResPriMail);

	StrCpy(rstMailData.m_szTitle, poResPriMail->m_szTitle, MAX_MAIL_TITLE_LEN);
	StrCpy(rstMailData.m_szContent, poResPriMail->m_szContent, MAX_MAIL_CONTENT_LEN);
	uint8_t& rCount = rstMailData.m_bAttachmentCount;
	for (rCount = 0; rCount < rstNtf.m_bPropNum && rCount < MAX_MAIL_ATTACHMENT_NUM; rCount++)
	{
		rstMailData.m_astAttachmentList[rCount].m_bItemType = rstNtf.m_astPropList[rCount].m_bItemType;
		rstMailData.m_astAttachmentList[rCount].m_dwItemId = rstNtf.m_astPropList[rCount].m_dwItemId;
		rstMailData.m_astAttachmentList[rCount].m_iValueChg = rstNtf.m_astPropList[rCount].m_dwItemNum;
	}
	ZoneSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
	return 0;
}



