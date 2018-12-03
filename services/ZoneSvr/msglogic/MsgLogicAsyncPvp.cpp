#include "MsgLogicAsyncPvp.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "LogMacros.h"
#include "ss_proto.h"
#include "cs_proto.h"
#include "../module/AsyncPvp.h"
#include "../module/player/PlayerMgr.h"
#include "../module/Item.h"
#include "../gamedata/GameDataMgr.h"
#include "../module/Task.h"
#include "../module/Majesty.h"
#include "../module/Message.h"
#include "dwlog_def.h"
#include "ZoneLog.h"

using namespace PKGMETA;

int AsyncpvpStartReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    CS_PKG_ASYNC_PVP_START_REQ& rstCsReq = rstCsPkg.m_stBody.m_stAsyncpvpStartReq;

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ASYNC_PVP_START_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    SS_PKG_ASYNC_PVP_START_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stAsyncpvpStartReq;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ASYNC_PVP_START_RSP;
    SC_PKG_ASYNC_PVP_START_RSP& rstScRsp = m_stScPkg.m_stBody.m_stAsyncpvpStartRsp;

	rstScRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    int iRet = 0;
    do
    {
        iRet = AsyncPvp::Instance().CheckFight(&poPlayer->GetPlayerData());
        if (iRet != ERR_NONE)
        {
            break;
        }

		iRet = AsyncPvp::Instance().CheckColdTime(&poPlayer->GetPlayerData());
		if(iRet != ERR_NONE)
		{
			iRet = AsyncPvp::Instance().CheckResetColdTime(&poPlayer->GetPlayerData());
			if(iRet != ERR_NONE)
			{
				break;
			}
		}

        rstSsReq.m_ullUin = poPlayer->GetUin();
        rstSsReq.m_dwOpponent = rstCsReq.m_dwOpponent;
    }while(false);

    if (iRet == ERR_NONE)
    {
        ZoneSvrMsgLayer::Instance().SendToAsyncPvpSvr(m_stSsPkg);
    }
    else
    {
        rstScRsp.m_nErrNo = iRet;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }

    return 0;
}


int AsyncpvpSettleReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    CS_PKG_ASYNC_PVP_SETTLE_REQ& rstCsReq = rstCsPkg.m_stBody.m_stAsyncpvpSettleReq;

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ASYNC_PVP_SETTLE_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    SS_PKG_ASYNC_PVP_SETTLE_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stAsyncpvpSettleReq;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ASYNC_PVP_SETTLE_RSP;
    SC_PKG_ASYNC_PVP_SETTLE_RSP& rstScRsp = m_stScPkg.m_stBody.m_stAsyncpvpSettleRsp;

    int iRet = 0;
    do
    {
        DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = poPlayer->GetPlayerData().GetELOInfo().m_stAsyncPvpInfo;
        //检查是否开启
        if (rstAsyncPvpInfo.m_bOpenFlag == 0)
        {
            iRet = ERR_NOT_SATISFY_COND;
            break;
        }

        rstSsReq.m_ullRaceNo = rstCsReq.m_ullRaceNo;
        rstSsReq.m_bWinGroup = rstCsReq.m_bWinGroup;
    }while(false);

    if (iRet == ERR_NONE)
    {
        ZoneSvrMsgLayer::Instance().SendToAsyncPvpSvr(m_stSsPkg);
    }
    else
    {
        rstScRsp.m_nErrNo = iRet;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }

    return 0;
}


int AsyncpvpRefreshOpponentReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ASYNC_PVP_REFRESH_OPPONENT_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    SS_PKG_ASYNC_PVP_REFRESH_OPPONENT_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stAsyncpvpRefreshOpponentReq;
    rstSsReq.m_bIsAuto = 0;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ASYNC_PVP_REFRESH_OPPONENT_RSP;
    SC_PKG_ASYNC_PVP_REFRESH_OPPONENT_RSP& rstScRsp = m_stScPkg.m_stBody.m_stAsyncpvpRefreshOpponentRsp;

    int iRet = 0;
    do
    {
        iRet = AsyncPvp::Instance().CheckRefresh(&poPlayer->GetPlayerData());
        if (iRet != ERR_NONE)
        {
            break;
        }

        rstSsReq.m_ullUin = poPlayer->GetUin();
    }while(false);

    if (iRet == ERR_NONE)
    {
        ZoneSvrMsgLayer::Instance().SendToAsyncPvpSvr(m_stSsPkg);
    }
    else
    {
        rstScRsp.m_nErrNo = iRet;

		DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = poPlayer->GetPlayerData().GetELOInfo().m_stAsyncPvpInfo;
		rstScRsp.m_bRefreshTimes = rstAsyncPvpInfo.m_bRefreshTimes;

        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }

    return 0;
}


int AsyncpvpGetDataReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    CS_PKG_ASYNC_PVP_GET_DATA_REQ& rstCsReq = rstCsPkg.m_stBody.m_stAsyncpvpGetDataReq;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ASYNC_PVP_GET_DATA_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    SS_PKG_ASYNC_PVP_GET_DATA_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stAsyncpvpGetDataReq;
    rstSsReq.m_stShowData.m_bCount = 0;
    rstSsReq.m_stTeamData.m_bTroopNum = 0;

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ASYNC_PVP_GET_DATA_RSP;
    SC_PKG_ASYNC_PVP_GET_DATA_RSP& rstScRsp = m_stScPkg.m_stBody.m_stAsyncpvpGetDataRsp;
    rstScRsp.m_bOpponentCount = 0;
    rstScRsp.m_bRecordCount = 0;

    int iRet = 0;
    do
    {
        DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = poPlayer->GetPlayerData().GetELOInfo().m_stAsyncPvpInfo;

        rstSsReq.m_bIsFirst = (rstAsyncPvpInfo.m_bOpenFlag == 0) ? 1 : 0;
        rstSsReq.m_ullUin = poPlayer->GetUin();

        if (rstSsReq.m_bIsFirst)
        {
            iRet = Majesty::Instance().SetBattleGeneralList(&poPlayer->GetPlayerData(), rstCsReq.m_bCount, rstCsReq.m_GeneralList, BATTLE_ARRAY_TYPE_ASYNCPVP|0x80);
            if (iRet != ERR_NONE)
            {
                break;
            }

            iRet = AsyncPvp::Instance().GetShowData(&poPlayer->GetPlayerData(), rstSsReq.m_stShowData);
            if (iRet != ERR_NONE)
            {
                break;
            }

            iRet = AsyncPvp::Instance().GetTeamData(&poPlayer->GetPlayerData(), rstSsReq.m_stTeamData);
            if (iRet != ERR_NONE)
            {
                break;
            }
        }
    }while(false);

    if (iRet != ERR_NONE)
    {
        rstScRsp.m_nErrNo = iRet;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }
    else
    {
        ZoneSvrMsgLayer::Instance().SendToAsyncPvpSvr(m_stSsPkg);
    }

    return 0;
}


int AsyncpvpChgTeamReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    CS_PKG_ASYNC_PVP_CHG_TEAM_REQ& rstCsReq = rstCsPkg.m_stBody.m_stAsyncpvpChgTeamReq;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ASYNC_PVP_CHG_TEAM_RSP;
    SC_PKG_ASYNC_PVP_CHG_TEAM_RSP& rstScRsp = m_stScPkg.m_stBody.m_stAsyncpvpChgTeamRsp;

    do
    {
        DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = poPlayer->GetPlayerData().GetELOInfo().m_stAsyncPvpInfo;
        //检查是否开启
        if (rstAsyncPvpInfo.m_bOpenFlag == 0)
        {
            rstScRsp.m_nErrNo = ERR_NOT_SATISFY_COND;
            break;
        }

		rstScRsp.m_nErrNo = Majesty::Instance().SetBattleGeneralList(&poPlayer->GetPlayerData(), rstCsReq.m_bCount, rstCsReq.m_GeneralList, BATTLE_ARRAY_TYPE_ASYNCPVP);
        if (rstScRsp.m_nErrNo == ERR_NONE)
        {
            AsyncPvp::Instance().UptToAsyncSvr(&poPlayer->GetPlayerData());
        }
    }while(false);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}


int AsyncpvpScoreRewardReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    CS_PKG_ASYNC_PVP_SCORE_REWARD_REQ& rstCsReq = rstCsPkg.m_stBody.m_stAsyncpvpScoreRewardReq;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ASYNC_PVP_SCORE_REWARD_RSP;
    SC_PKG_ASYNC_PVP_SCORE_REWARD_RSP& rstScRsp = m_stScPkg.m_stBody.m_stAsyncpvpScoreRewardRsp;
    rstScRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;


    DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = poPlayer->GetPlayerData().GetELOInfo().m_stAsyncPvpInfo;

    rstScRsp.m_nErrNo = ERR_NONE;
    for (uint8_t i=0; i<rstCsReq.m_bRewardCnt; i++)
    {
        if (rstAsyncPvpInfo.m_dwScoreRewardFlag & (1<<(rstCsReq.m_szRewardList[i] -1)))
        {
            rstScRsp.m_nErrNo = ERR_NOT_SATISFY_COND;
            break;
        }

        RESASYNCPVPSCOREREWARD* pResReward = CGameDataMgr::Instance().GetResAsyncPvpScoreRewardMgr().Find(rstCsReq.m_szRewardList[i]);
        if (!pResReward)
        {
            rstScRsp.m_nErrNo = ERR_SYS;
            break;
        }

        if (rstAsyncPvpInfo.m_wScore < pResReward->m_dwScore)
        {
            rstScRsp.m_nErrNo = ERR_NOT_SATISFY_COND;
            break;
        }
    }

    if (rstScRsp.m_nErrNo != ERR_NONE)
    {
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return -1;
    }

    for (uint8_t i=0; i<rstCsReq.m_bRewardCnt; i++)
    {
        rstAsyncPvpInfo.m_dwScoreRewardFlag = rstAsyncPvpInfo.m_dwScoreRewardFlag | (1<<(rstCsReq.m_szRewardList[i] -1));

        RESASYNCPVPSCOREREWARD* pResReward = CGameDataMgr::Instance().GetResAsyncPvpScoreRewardMgr().Find(rstCsReq.m_szRewardList[i]);
        for (uint8_t j=0; j<pResReward->m_bCount; j++)
        {
            Item::Instance().RewardItem(&poPlayer->GetPlayerData(), pResReward->m_szPropsType[j], pResReward->m_propsId[j], pResReward->m_propsNum[j],
                                        rstScRsp.m_stSyncItemInfo, DWLOG::METHOD_ASYNC_PVP_SCORE_REWARD, Item::CONST_SHOW_PROPERTY_NORMAL, true);
        }
    }
    rstScRsp.m_dwRewardFlag = rstAsyncPvpInfo.m_dwScoreRewardFlag;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}


int AsyncpvpRankRewardReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    CS_PKG_ASYNC_PVP_RANK_REWARD_REQ& rstCsReq = rstCsPkg.m_stBody.m_stAsyncpvpRankRewardReq;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ASYNC_PVP_RANK_REWARD_RSP;
    SC_PKG_ASYNC_PVP_RANK_REWARD_RSP& rstScRsp = m_stScPkg.m_stBody.m_stAsyncpvpRankRewardRsp;
    rstScRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;


    do
    {
        DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = poPlayer->GetPlayerData().GetELOInfo().m_stAsyncPvpInfo;
        if (rstAsyncPvpInfo.m_ullRankRewardFlag & (1<<(rstCsReq.m_bRewardId -1)))
        {
            rstScRsp.m_nErrNo = ERR_NOT_SATISFY_COND;
            break;
        }

        RESASYNCPVPHIGHESTRANKREWARD* pResReward = CGameDataMgr::Instance().GetResAsyncPvpRankRewardMgr().Find(rstCsReq.m_bRewardId);
        if (!pResReward)
        {
            rstScRsp.m_nErrNo = ERR_SYS;
            break;
        }

        if (rstAsyncPvpInfo.m_dwRankHighest > pResReward->m_dwRank || rstAsyncPvpInfo.m_dwRankHighest == 0)
        {
            rstScRsp.m_nErrNo = ERR_NOT_SATISFY_COND;
            break;
        }

        rstAsyncPvpInfo.m_ullRankRewardFlag = rstAsyncPvpInfo.m_ullRankRewardFlag | (1<<(rstCsReq.m_bRewardId -1));

        for (uint8_t i=0; i<pResReward->m_bCount; i++)
        {
            Item::Instance().RewardItem(&poPlayer->GetPlayerData(), pResReward->m_szPropsType[i], pResReward->m_propsId[i], pResReward->m_propsNum[i],
                                         rstScRsp.m_stSyncItemInfo, DWLOG::METHOD_ASYNC_PVP_RANK_REWARD);
        }

        rstScRsp.m_ullRewardFlag = rstAsyncPvpInfo.m_ullRankRewardFlag;
        rstScRsp.m_nErrNo = ERR_NONE;
    }while(false);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}


int AsyncpvpFightTimesBuyReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ASYNC_PVP_FIGHT_TIMES_BUY_RSP;
    SC_PKG_ASYNC_PVP_FIGHT_TIMES_BUY_RSP& rstScRsp = m_stScPkg.m_stBody.m_stAsyncpvpFightTimesBuyRsp;
    rstScRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    int iRet = AsyncPvp::Instance().FightTimesBuy(&poPlayer->GetPlayerData(), 1, rstScRsp.m_stSyncItemInfo);
    if (iRet > 0)
    {
        rstScRsp.m_nErrNo = ERR_NONE;
        rstScRsp.m_bFightTimesBuy = (uint8_t)iRet;
    }
    else
    {
        rstScRsp.m_nErrNo = iRet;
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}


int AsyncpvpGetTopListReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    CS_PKG_ASYNC_PVP_GET_TOPLIST_REQ& rstCsReq = rstCsPkg.m_stBody.m_stAsyncpvpGetTopListReq;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ASYNC_PVP_GET_TOPLIST_RSP;
    SC_PKG_ASYNC_PVP_GET_TOPLIST_RSP& rstScRsp = m_stScPkg.m_stBody.m_stAsyncpvpGetTopListRsp;

    rstScRsp.m_ullTimeStamp = rstCsReq.m_ullTimeStamp;
    rstScRsp.m_nErrNo = AsyncPvp::Instance().GetTopList(rstScRsp.m_astTopList, rstScRsp.m_ullTimeStamp);
    if (rstScRsp.m_nErrNo == ERR_NONE)
    {
        rstScRsp.m_bTopListCnt = MAX_RANK_TOP_NUM;
    }
    else
    {
        rstScRsp.m_bTopListCnt = 0;
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int AsyncpvpWorshipReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (NULL == poPlayer)
	{
		LOGRUN("player not exist.");
		return -1;
	}

	CS_PKG_ASYNC_PVP_WORSHIP_REQ& rstCsReq = rstCsPkg.m_stBody.m_stAsyncpvpWorshipReq;
	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ASYNC_PVP_WORSHIP_RSP;
	SC_PKG_ASYNC_PVP_WORSHIP_RSP& rstScRsp = m_stScPkg.m_stBody.m_stAsyncpvpWorshipRsp;
	rstScRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

	rstScRsp.m_nErrNo = AsyncPvp::Instance().Worship(&poPlayer->GetPlayerData(), rstCsReq, rstScRsp);

	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

	return 0;
}


int AsyncpvpGetWorshipGoldReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (NULL == poPlayer)
	{
		LOGRUN("player not exist.");
		return -1;
	}

	//发送消息给asyncsvr，取被膜拜的金币
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ASYNC_PVP_GET_WORSHIP_GOLD_REQ;
	SS_PKG_ASYNC_PVP_GET_WORSHIP_GOLD_REQ& rstSsPkg = m_stSsPkg.m_stBody.m_stAsyncpvpGetWorshipGoldReq;
	rstSsPkg.m_ullUin = poPlayer->GetPlayerData().GetRoleBaseInfo().m_ullUin;
	ZoneSvrMsgLayer::Instance().SendToAsyncPvpSvr(m_stSsPkg);

	return 0;
}

int AsyncpvpResetCdReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (NULL == poPlayer)
	{
		LOGRUN("player not exist.");
		return -1;
	}

	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ASYNC_PVP_RESET_CD_RSP;
	SC_PKG_ASYNC_PVP_RESET_CD_RSP& rstScRsp = m_stScPkg.m_stBody.m_stAsyncpvpResetCdRsp;
	rstScRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

	rstScRsp.m_nErrNo = AsyncPvp::Instance().ResetCdTime(&poPlayer->GetPlayerData(), rstScRsp.m_stSyncItemInfo);

	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

	return 0;
}

int AsyncpvpSkipFightReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    CS_PKG_ASYNC_PVP_SKIP_FIGHT_REQ& rstCsReq = rstCsPkg.m_stBody.m_stAsyncpvpSkipFightReq;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ASYNC_PVP_SKIP_FIGHT_RSP;
    SC_PKG_ASYNC_PVP_SKIP_FIGHT_RSP& rstScRsp = m_stScPkg.m_stBody.m_stAsyncpvpSkipFightRsp;
    rstScRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    rstScRsp.m_nErrNo = AsyncPvp::Instance().SkipFight(&poPlayer->GetPlayerData(), rstCsReq.m_bFightCount, rstScRsp.m_stSyncItemInfo);
    DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = poPlayer->GetPlayerData().GetELOInfo().m_stAsyncPvpInfo;
    rstScRsp.m_wScore = rstAsyncPvpInfo.m_wScore;
    rstScRsp.m_ullCdTimeStamp = rstAsyncPvpInfo.m_ullCdTimeStamp;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int AsyncpvpGetPlayerInfoReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGRUN("player not exist.");
        return -1;
    }

    CS_PKG_ASYNC_PVP_GET_PLAYER_INFO_REQ& rstCsReq = rstCsPkg.m_stBody.m_stAsyncpvpGetPlayerInfoReq;

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ASYNC_PVP_GET_PLAYER_INFO_REQ;
    m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
    SS_PKG_ASYNC_PVP_GET_PLAYER_INFO_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stAsyncpvpGetPlayerInfoReq;
    rstSsReq.m_ullUin = poPlayer->GetUin();
    rstSsReq.m_dwOpponent = rstCsReq.m_dwOpponent;

    ZoneSvrMsgLayer::Instance().SendToAsyncPvpSvr(m_stSsPkg);

    return 0;
}


/*--------------------------------------------------------------------------------------------------------------------------------------*/

int AsyncpvpStartRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_ASYNC_PVP_START_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stAsyncpvpStartRsp;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ASYNC_PVP_START_RSP;
    SC_PKG_ASYNC_PVP_START_RSP& rstScRsp = m_stScPkg.m_stBody.m_stAsyncpvpStartRsp;
	rstScRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    rstScRsp.m_nErrNo = rstSsRsp.m_nErrNo;
    rstScRsp.m_ullRaceNo = rstSsRsp.m_ullRaceNo;
    rstScRsp.m_stOpponentInfo = rstSsRsp.m_stOpponentInfo;

    if (rstScRsp.m_nErrNo == ERR_NONE)
    {
        AsyncPvp::Instance().AfterStartFight(&poPlayer->GetPlayerData());
    }

	//扣重置的钱
	int iRet = AsyncPvp::Instance().CheckColdTime(&poPlayer->GetPlayerData());
	if(iRet != ERR_NONE)
	{
		AsyncPvp::Instance().SettleResetDia(&poPlayer->GetPlayerData(), rstScRsp.m_stSyncItemInfo);
	}
	//添加冷却时间戳
	AsyncPvp::Instance().UpdateCdTime(&poPlayer->GetPlayerData(), rstScRsp.m_ullCdTimeStamp);

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}


int AsyncpvpSettleRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_ASYNC_PVP_SETTLE_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stAsyncpvpSettleRsp;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ASYNC_PVP_SETTLE_RSP;
    SC_PKG_ASYNC_PVP_SETTLE_RSP& rstScRsp = m_stScPkg.m_stBody.m_stAsyncpvpSettleRsp;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    rstScRsp.m_nErrNo = rstSsRsp.m_nErrNo;
    if (rstScRsp.m_nErrNo == ERR_NONE)
    {
        DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = poPlayer->GetPlayerData().GetELOInfo().m_stAsyncPvpInfo;
		DT_ROLE_MAJESTY_INFO& rstMajestyInfo = poPlayer->GetPlayerData().GetMajestyInfo();

        //结算积分
        uint16_t wScore = 0;
		bool bIsAvoidLoss = false;
		RESBASIC* pstResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(ASYNCPVP_POINT_LOSE_AVOID);
		uint8_t bVipLv = (uint8_t)pstResBasic->m_para[0];
		uint16_t wLevel = (uint16_t)pstResBasic->m_para[1];

		if (rstMajestyInfo.m_bVipLv >= bVipLv || rstMajestyInfo.m_wLevel >= wLevel || rstAsyncPvpInfo.m_bFirstFlag == 0)
		{
			bIsAvoidLoss = true;
		}
        rstAsyncPvpInfo.m_bFirstFlag = 1;

		int16_t wWinScore = (uint16_t)CGameDataMgr::Instance().GetResBasicMgr().Find((int)ASYNCPVP_SUCESS_GAIN_SCORE)->m_para[0];
		int16_t wLoseScore = (uint16_t)CGameDataMgr::Instance().GetResBasicMgr().Find((int)ASYNCPVP_FAIL_GAIN_SCORE)->m_para[0];

        if (rstSsRsp.m_stRecord.m_ullWinPlayerId == poPlayer->GetUin())
        {
            wScore = wWinScore;
            //  胜利场次
            Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_ASYNCPVP, 1, 3, 1);

            if (rstSsRsp.m_astOpponentList[0].m_stBaseInfo.m_dwRank > rstSsRsp.m_astOpponentList[1].m_stBaseInfo.m_dwRank)
            {
                //系统分发通知等
                if (rstSsRsp.m_stSelfBaseInfo.m_dwRank >= 4 && rstSsRsp.m_stSelfBaseInfo.m_dwRank <= 10)
                {
                    Message::Instance().AutoSendSysMessage(1001, "Name=%s|Name=%s|Rank=%u", poPlayer->GetRoleName(), rstSsRsp.m_stRecord.m_astPlayerList[1].m_szName, rstSsRsp.m_stSelfBaseInfo.m_dwRank);
                }
                else if (rstSsRsp.m_stSelfBaseInfo.m_dwRank == 3)
                {
                    Message::Instance().AutoSendSysMessage(1002, "Name=%s|Name=%s", poPlayer->GetRoleName(), rstSsRsp.m_stRecord.m_astPlayerList[1].m_szName);
                    Message::Instance().AutoSendWorldMessage(&poPlayer->GetPlayerData(), 1005, "Name=%s", rstSsRsp.m_stRecord.m_astPlayerList[1].m_szName);
                }
                else if (rstSsRsp.m_stSelfBaseInfo.m_dwRank == 2)
                {
                    Message::Instance().AutoSendSysMessage(1003, "Name=%s|Name=%s", poPlayer->GetRoleName(), rstSsRsp.m_stRecord.m_astPlayerList[1].m_szName);
                    Message::Instance().AutoSendWorldMessage(&poPlayer->GetPlayerData(), 1006, "Name=%s", rstSsRsp.m_stRecord.m_astPlayerList[1].m_szName);
                }
                else if (rstSsRsp.m_stSelfBaseInfo.m_dwRank == 1)
                {
                    Message::Instance().AutoSendSysMessage(1004, "Name=%s|Name=%s", poPlayer->GetRoleName(), rstSsRsp.m_stRecord.m_astPlayerList[1].m_szName);
                    Message::Instance().AutoSendWorldMessage(&poPlayer->GetPlayerData(), 1007, "Name=%s", rstSsRsp.m_stRecord.m_astPlayerList[1].m_szName);
                }
            }
        }
        else
        {
			if (bIsAvoidLoss)
			{
				wScore = wWinScore;
			}
			else
			{
				wScore = wLoseScore;
			}

            //  失败场次
            Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_ASYNCPVP, 1, 3, 2);
        }
        rstAsyncPvpInfo.m_wScore += wScore;

        //刷新最高排名
        if (rstSsRsp.m_stSelfBaseInfo.m_dwRank < rstAsyncPvpInfo.m_dwRankHighest)
        {
            rstAsyncPvpInfo.m_dwRankHighest = rstSsRsp.m_stSelfBaseInfo.m_dwRank;

        }
        //  参与场次
        Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_ASYNCPVP, 1, 3, 3);
        //  达到积分
        Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_ASYNCPVP, wScore, 1);
        //  达到排名
        Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_ASYNCPVP, rstSsRsp.m_stSelfBaseInfo.m_dwRank, 2);

        rstScRsp.m_bOpponentCount = rstSsRsp.m_bOpponentCount;
        for (uint8_t i = 0; i < rstScRsp.m_bOpponentCount; i++)
        {
            rstScRsp.m_astOpponentList[i] = rstSsRsp.m_astOpponentList[i];
        }

		AsyncPvp::Instance().GetWorshipInfo(&poPlayer->GetPlayerData(), rstScRsp.m_astOpponentList, rstScRsp.m_bOpponentCount);

        rstScRsp.m_stRecord = rstSsRsp.m_stRecord;
        rstScRsp.m_stSelfBaseInfo = rstSsRsp.m_stSelfBaseInfo;

        if (rstSsRsp.m_stRecord.m_ullWinPlayerId == rstSsPkg.m_stHead.m_ullUin)
        {
            uint32_t dwOldRank = 0;
            uint32_t dwNewRank = 0;
            if (rstSsRsp.m_stRecord.m_astPlayerList[0].m_ullUin == rstSsPkg.m_stHead.m_ullUin)
            {
                dwOldRank = rstSsRsp.m_stRecord.m_astPlayerList[1].m_dwRank;
                dwNewRank = rstSsRsp.m_stRecord.m_astPlayerList[0].m_dwRank;
            }
            else
            {
                dwOldRank = rstSsRsp.m_stRecord.m_astPlayerList[0].m_dwRank;
                dwNewRank = rstSsRsp.m_stRecord.m_astPlayerList[1].m_dwRank;
            }
            ZoneLog::Instance().WriteFightPvPLog(&poPlayer->GetPlayerData(), rstAsyncPvpInfo.m_wScore - wScore,
                rstAsyncPvpInfo.m_wScore, dwOldRank, dwNewRank);
			if (dwNewRank > dwOldRank)
			{
				//战胜被自己高的排名
				Task::Instance().ModifyData(&poPlayer->GetPlayerData(), TASK_VALUE_TYPE_ASYNCPVP, 1, 4);
			}
			
        }
        else
        {
            ZoneLog::Instance().WriteFightPvPLog(&poPlayer->GetPlayerData(), rstAsyncPvpInfo.m_wScore - wScore,
                rstAsyncPvpInfo.m_wScore, rstSsRsp.m_stSelfBaseInfo.m_dwRank, rstSsRsp.m_stSelfBaseInfo.m_dwRank);
        }
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    //如果被攻击者在线,需要将战报同步给被攻击者
    Player* poDefender= PlayerMgr::Instance().GetPlayerByUin(rstSsRsp.m_stRecord.m_astPlayerList[1].m_ullUin);
    if (!poDefender)
    {
        return 0;
    }
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ASYNC_PVP_RECORD_NTF;
    SC_PKG_ASYNC_PVP_RECORD_NTF& rstScNtf = m_stScPkg.m_stBody.m_stAsyncpvpRecordNtf;
    rstScNtf.m_stRecord = rstSsRsp.m_stRecord;

    ZoneSvrMsgLayer::Instance().SendToClient(poDefender, &m_stScPkg);

    return 0;
}


int AsyncpvpRefreshOpponentRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_ASYNC_PVP_REFRESH_OPPONENT_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stAsyncpvpRefreshOpponentRsp;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ASYNC_PVP_REFRESH_OPPONENT_RSP;
    SC_PKG_ASYNC_PVP_REFRESH_OPPONENT_RSP& rstScRsp = m_stScPkg.m_stBody.m_stAsyncpvpRefreshOpponentRsp;

    rstScRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    rstScRsp.m_nErrNo = rstSsRsp.m_nErrNo;
    if (rstScRsp.m_nErrNo == ERR_NONE)
    {
        if (rstSsRsp.m_bIsAuto != 1)
        {
            AsyncPvp::Instance().AfterRefresh(&poPlayer->GetPlayerData(), rstScRsp.m_stSyncItemInfo);
        }

        rstScRsp.m_bOpponentCount = rstSsRsp.m_bOpponentCount;
        for (uint8_t i = 0; i < rstScRsp.m_bOpponentCount; i++)
        {
            rstScRsp.m_astOpponentList[i] = rstSsRsp.m_astOpponentList[i];
        }

		AsyncPvp::Instance().GetWorshipInfo(&poPlayer->GetPlayerData(), rstScRsp.m_astOpponentList, rstScRsp.m_bOpponentCount);
    }

	DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = poPlayer->GetPlayerData().GetELOInfo().m_stAsyncPvpInfo;
	rstScRsp.m_bRefreshTimes = rstAsyncPvpInfo.m_bRefreshTimes;
    rstScRsp.m_dwMyRank = rstSsRsp.m_dwMyRank;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}


int AsyncpvpGetDataRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_ASYNC_PVP_GET_DATA_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stAsyncpvpGetDataRsp;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ASYNC_PVP_GET_DATA_RSP;
    SC_PKG_ASYNC_PVP_GET_DATA_RSP& rstScRsp = m_stScPkg.m_stBody.m_stAsyncpvpGetDataRsp;

    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGRUN("poPlayer is null");
        return -1;
    }

    DT_ROLE_ASYNC_PVP_INFO& rstAsyncPvpInfo = poPlayer->GetPlayerData().GetELOInfo().m_stAsyncPvpInfo;
    rstScRsp.m_nErrNo = rstSsRsp.m_nErrNo;
    if (rstScRsp.m_nErrNo == ERR_NONE)
    {
        rstAsyncPvpInfo.m_bOpenFlag = 1;

        //更新一下历史最高排名
        if (rstSsRsp.m_stSelfInfo.m_stBaseInfo.m_dwRank < rstAsyncPvpInfo.m_dwRankHighest
            ||rstAsyncPvpInfo.m_dwRankHighest == 0)
        {
            rstAsyncPvpInfo.m_dwRankHighest = rstSsRsp.m_stSelfInfo.m_stBaseInfo.m_dwRank;
        }

        rstScRsp.m_stSelfInfo = rstSsRsp.m_stSelfInfo;
        rstScRsp.m_bRecordCount = rstSsRsp.m_bRecordCount;
        for (uint8_t i = 0; i < rstScRsp.m_bRecordCount; i++)
        {
            rstScRsp.m_astRecordList[i] = rstSsRsp.m_astRecordList[i];
        }

        rstScRsp.m_bOpponentCount = rstSsRsp.m_bOpponentCount;
        for (uint8_t i = 0; i < rstScRsp.m_bOpponentCount; i++)
        {
           rstScRsp.m_astOpponentList[i] = rstSsRsp.m_astOpponentList[i];
        }

		AsyncPvp::Instance().GetWorshipInfo(&poPlayer->GetPlayerData(), rstScRsp.m_astOpponentList, rstScRsp.m_bOpponentCount);
    }
    else
    {
        rstAsyncPvpInfo.m_bOpenFlag = 0;
    }

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}

int AsyncpvpRefreshTopListNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_ASYNC_PVP_REFRESH_TOPLIST_NTF& rstSsNtf = rstSsPkg.m_stBody.m_stAsyncpvpRefreshTopListNtf;

    AsyncPvp::Instance().RefreshTopList(rstSsNtf.m_astTopList, rstSsNtf.m_ullTimeStamp);

    return 0;
}

int AsyncpvpGetWorshipGoldRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
	if (!poPlayer)
	{
		LOGRUN("poPlayer is null");
		return -1;
	}

	SS_PKG_ASYNC_PVP_GET_WORSHIP_GOLD_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stAsyncpvpGetWorshipGoldRsp;
	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ASYNC_PVP_GET_WORSHIP_GOLD_RSP;
	SC_PKG_ASYNC_PVP_GET_WORSHIP_GOLD_RSP& rstScRsp = m_stScPkg.m_stBody.m_stAsyncpvpGetWorshipGoldRsp;
	rstScRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

	rstSsRsp.m_nErrNo = ERR_NONE;

	if (0 != rstSsRsp.m_iGoldTaken)
	{
		rstSsRsp.m_nErrNo = AsyncPvp::Instance().AddWorshipGold(&poPlayer->GetPlayerData(), rstSsRsp.m_iGoldTaken, rstScRsp);
	}

	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

	return 0;
}

int AsyncpvpGetPlayerInfoRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
	if (!poPlayer)
	{
		LOGRUN("poPlayer is null");
		return -1;
	}

	SS_PKG_ASYNC_PVP_GET_PLAYER_INFO_RSP& rstSsRsp = rstSsPkg.m_stBody.m_stAsyncpvpGetPlayerInfoRsp;
	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_ASYNC_PVP_GET_PLAYER_INFO_RSP;
	SC_PKG_ASYNC_PVP_GET_PLAYER_INFO_RSP& rstScRsp = m_stScPkg.m_stBody.m_stAsyncpvpGetPlayerInfoRsp;

	rstScRsp.m_nErrNo = rstSsRsp.m_nErrNo;
    rstScRsp.m_stOpponentInfo = rstSsRsp.m_stOpponentInfo;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

    return 0;
}


