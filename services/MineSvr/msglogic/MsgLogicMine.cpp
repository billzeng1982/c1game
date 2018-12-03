
#include "LogMacros.h"
#include "strutil.h"
#include "./MsgLogicMine.h"
#include "../framework/MineSvrMsgLayer.h"
#include "../module/MineLogicMgr.h"
#include "../module/MineDataMgr.h"
using namespace PKGMETA;


int CSsMineGetInfoReq::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
    SS_PKG_MINE_GET_INFO_REQ& rstReq = pstSsPkg->m_stBody.m_stMineGetInfoReq;
    SSPKG& rstPkgNew = MineSvrMsgLayer::Instance().GetSsPkg();
    SS_PKG_MINE_GET_INFO_RSP& rstRsp = rstPkgNew.m_stBody.m_stMineGetInfoRsp;
	bzero(&rstRsp, sizeof(rstRsp));

    rstRsp.m_nErrNo = MineLogicMgr::Instance().GetInfo(pstSsPkg->m_stHead.m_ullUin, rstReq.m_szName,
		pstSsPkg->m_stHead.m_iSrcProcId, rstReq.m_wLv, rstReq.m_wIconId, rstReq.m_dwLeaderValue, rstRsp);

    rstPkgNew.m_stHead.m_wMsgId = SS_MSG_MINE_GET_INFO_RSP;
    rstPkgNew.m_stHead.m_ullReservId = pstSsPkg->m_stHead.m_iSrcProcId; //目标ZoneSvr
    rstPkgNew.m_stHead.m_ullUin = pstSsPkg->m_stHead.m_ullUin;
    LOGWARN("Uin<%lu> AwardCnt<%hhu> OwnOreCnt<%hhu> ExploreOreCnt<%hhu>", pstSsPkg->m_stHead.m_ullUin,
        rstRsp.m_bAwardCount, rstRsp.m_bOwnOreCount, rstRsp.m_bExploreOreCount);
    MineSvrMsgLayer::Instance().SendToClusterGate(rstPkgNew);
    return 0;
}

int CSsMineExploreReq::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
    SS_PKG_MINE_EXPLORE_REQ& rstReq = pstSsPkg->m_stBody.m_stMineExploreReq;
    SSPKG& rstPkgNew = MineSvrMsgLayer::Instance().GetSsPkg();
    SS_PKG_MINE_EXPLORE_RSP& rstRsp = rstPkgNew.m_stBody.m_stMineExploreRsp;
    rstRsp.m_bExploreOreCount = 0;
    rstRsp.m_nErrNo = MineLogicMgr::Instance().Explore(pstSsPkg->m_stHead.m_ullUin, rstReq.m_dwTeamLi, rstRsp);
    rstPkgNew.m_stHead.m_wMsgId = SS_MSG_MINE_EXPLORE_RSP;
    rstPkgNew.m_stHead.m_ullReservId = pstSsPkg->m_stHead.m_iSrcProcId; //目标ZoneSvr
    rstPkgNew.m_stHead.m_ullUin = pstSsPkg->m_stHead.m_ullUin;
    MineSvrMsgLayer::Instance().SendToClusterGate(rstPkgNew);
    return 0;
}

int CSsMineDealOreReq::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
    SS_PKG_MINE_DEAL_ORE_REQ& rstReq = pstSsPkg->m_stBody.m_stMineDealOreReq;
    SSPKG& rstPkgNew = MineSvrMsgLayer::Instance().GetSsPkg();
    SS_PKG_MINE_DEAL_ORE_RSP& rstRsp = rstPkgNew.m_stBody.m_stMineDealOreRsp;
    bzero(&rstRsp.m_stOre, sizeof(rstRsp.m_stOre));
	LOGWARN("Uin<%lu> dealOre Type<%hhu>", pstSsPkg->m_stHead.m_ullUin, rstReq.m_bDealType);
    if (rstReq.m_bDealType == MINE_ORE_DEAL_TYPE_OCCPUY)    //占领
    {
        rstRsp.m_nErrNo = MineLogicMgr::Instance().Occupy(pstSsPkg->m_stHead.m_ullUin, rstReq, rstRsp.m_stOre);
    }
    else if (rstReq.m_bDealType == MINE_ORE_DEAL_TYPE_DROP) //放弃
    {
        rstRsp.m_nErrNo = MineLogicMgr::Instance().Drop(pstSsPkg->m_stHead.m_ullUin, rstReq.m_ullOreUid, rstRsp.m_stOre);
    }
    else if (rstReq.m_bDealType == MINE_ORE_DEAL_TYPE_MODFIY)   //修改
    {
        rstRsp.m_nErrNo = MineLogicMgr::Instance().Modify(pstSsPkg->m_stHead.m_ullUin, rstReq.m_ullOreUid, rstReq.m_stMasterSkill, rstRsp.m_stOre);
    }
    else if (rstReq.m_bDealType == MINE_ORE_DEAL_TYPE_INVESTIGATE)  //调查
    {
        rstRsp.m_nErrNo = MineLogicMgr::Instance().Investigate(pstSsPkg->m_stHead.m_ullUin, rstReq.m_ullOreUid, rstRsp.m_stOre);
    }
    else if (rstReq.m_bDealType == MINE_ORE_DEAL_TYPE_CHALLENGE_REQUEST)    //挑战请求
    {
        rstRsp.m_nErrNo = MineLogicMgr::Instance().ChallengeRequest(pstSsPkg->m_stHead.m_ullUin, rstReq.m_ullOreUid, rstRsp.m_stOre);
    }
    else if (rstReq.m_bDealType == MINE_ORE_DEAL_TYPE_GRAB) //抢占矿
    {
        rstRsp.m_nErrNo = MineLogicMgr::Instance().Grab(pstSsPkg->m_stHead.m_ullUin, rstReq, rstRsp.m_stOre);
    }
	else if (rstReq.m_bDealType == MINE_ORE_DEAL_TYPE_GIVE_UP_GRAB) //放弃抢占矿
	{
		rstRsp.m_nErrNo = MineLogicMgr::Instance().GiveUpGrab(pstSsPkg->m_stHead.m_ullUin, rstReq, rstRsp.m_stOre);
	}
	else if (rstReq.m_bDealType == MINE_ORE_DEAL_TYPE_REVENGE)	//复仇请求
	{
		rstRsp.m_nErrNo = MineLogicMgr::Instance().RevengeRequest(pstSsPkg->m_stHead.m_ullUin, rstReq.m_ullOreUid, rstRsp.m_stOre);
	}

    else
    {
        LOGERR("Uin<%lu> CSsMineDealOreReq DealType<%hhu> error", pstSsPkg->m_stHead.m_ullUin, rstReq.m_bDealType);
        rstRsp.m_nErrNo = ERR_SYS;
    }
    rstPkgNew.m_stHead.m_wMsgId = SS_MSG_MINE_DEAL_ORE_RSP;
    rstPkgNew.m_stHead.m_ullReservId = pstSsPkg->m_stHead.m_iSrcProcId; //目标ZoneSvr
    rstPkgNew.m_stHead.m_ullUin = pstSsPkg->m_stHead.m_ullUin;
    rstRsp.m_bDealType = rstReq.m_bDealType;
    MineSvrMsgLayer::Instance().SendToClusterGate(rstPkgNew);

    return 0;
}

int CSsMineGetAwardReq::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
    SS_PKG_MINE_GET_AWARD_REQ& rstReq = pstSsPkg->m_stBody.m_stMineGetAwardReq;
    SSPKG& rstPkgNew = MineSvrMsgLayer::Instance().GetSsPkg();
    SS_PKG_MINE_GET_AWARD_RSP& rstRsp = rstPkgNew.m_stBody.m_stMineGetAwardRsp;
    rstRsp.m_bAwardCount = 0;

    rstRsp.m_nErrNo = MineLogicMgr::Instance().GetAwardLog(pstSsPkg->m_stHead.m_ullUin, rstReq.m_bType, 
		rstReq.m_bIndex, rstRsp);
    rstRsp.m_bIndex = rstReq.m_bIndex;
    rstRsp.m_bType = rstRsp.m_bType;

    rstPkgNew.m_stHead.m_wMsgId = SS_MSG_MINE_GET_AWARD_RSP;
    rstPkgNew.m_stHead.m_ullReservId = pstSsPkg->m_stHead.m_iSrcProcId; //目标ZoneSvr
    rstPkgNew.m_stHead.m_ullUin = pstSsPkg->m_stHead.m_ullUin;
    MineSvrMsgLayer::Instance().SendToClusterGate(rstPkgNew);
    return 0;

}



int CSsMineGetRevengerInfoReq::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
    SS_PKG_MINE_GET_REVENGER_INFO_REQ& rstReq = pstSsPkg->m_stBody.m_stMineGetRevengerInfoReq;
    SSPKG& rstPkgNew = MineSvrMsgLayer::Instance().GetSsPkg();
    rstPkgNew.m_stHead.m_wMsgId = SS_MSG_MINE_GET_REVENGER_INFO_RSP;
    rstPkgNew.m_stHead.m_ullReservId = pstSsPkg->m_stHead.m_iSrcProcId; //目标ZoneSvr
    rstPkgNew.m_stHead.m_ullUin = pstSsPkg->m_stHead.m_ullUin;

    SS_PKG_MINE_GET_REVENGER_INFO_RSP& rstRsp = rstPkgNew.m_stBody.m_stMineGetRevengerInfoRsp;
    rstRsp.m_bObjCount = 0;

    
    int iRet = ERR_NONE;
    do 
    {
        if (rstReq.m_ullObjUin == 0)
        {
            LOGERR("<%lu> revenger ObjUin is 0 ", pstSsPkg->m_stHead.m_ullUin);
            iRet = ERR_SYS;
            break;
        }
        MinePlayer* pstPlayer = MineDataMgr::Instance().GetMinePlayer(rstReq.m_ullObjUin);
        if (!pstPlayer)
        {
            LOGERR(" Uin<%lu> can't find the revenger ObjUin ", pstSsPkg->m_stHead.m_ullUin);
            break;
        }
        iRet = pstPlayer->GetOwnOreInfo(rstRsp.m_bObjCount, rstRsp.m_astObjOwnOreList);
    } while (0);
    rstRsp.m_nErrNo = iRet;
    MineSvrMsgLayer::Instance().SendToClusterGate(rstPkgNew);

    return 0;
}

int CSsMineFightResultReq::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
    SS_PKG_MINE_FIGHT_RESULT_REQ& rstReq = pstSsPkg->m_stBody.m_stMineFightResultReq;
    SSPKG& rstPkgNew = MineSvrMsgLayer::Instance().GetSsPkg();
    rstPkgNew.m_stHead.m_wMsgId = SS_MSG_MINE_FIGHT_RESULT_RSP;
    rstPkgNew.m_stHead.m_ullReservId = pstSsPkg->m_stHead.m_iSrcProcId; //目标ZoneSvr
    rstPkgNew.m_stHead.m_ullUin = pstSsPkg->m_stHead.m_ullUin;

    SS_PKG_MINE_FIGHT_RESULT_RSP& rstRsp = rstPkgNew.m_stBody.m_stMineFightResultRsp;

	if (rstReq.m_bFightType == 2)	//复仇
	{
		rstRsp.m_nErrNo = MineLogicMgr::Instance().Revenge(pstSsPkg->m_stHead.m_ullUin, rstReq.m_ullObjUin, rstReq.m_ullObjOreUid,
			rstReq.m_bWinFlag, rstReq.m_bRevengeLogIndex);
	}
	else							//正常挑战
	{
		rstRsp.m_nErrNo = MineLogicMgr::Instance().Challenge(pstSsPkg->m_stHead.m_ullUin, rstReq.m_ullObjOreUid, rstReq.m_bWinFlag);
	}

	rstRsp.m_bWinFlag = rstReq.m_bWinFlag;
	rstRsp.m_bFightType = rstReq.m_bFightType;
    MineSvrMsgLayer::Instance().SendToClusterGate(rstPkgNew);
    return 0;
}

int CSsMineSvrCombineUptNtf::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
	SS_PKG_MINE_SVR_COMBINE_UPT_NTF& rstNtf = pstSsPkg->m_stBody.m_stMineSvrCombineUptNtf;
	MinePlayer* pstPlayer = MineDataMgr::Instance().GetMinePlayerNoCreate(rstNtf.m_ullObjUin);
	if (NULL == pstPlayer)
	{
		return 0;
	}
	pstPlayer->UpdateAddr(rstNtf.m_dwAddr);
	MineDataMgr::Instance().DelFromTimeList(pstPlayer);
	MineDataMgr::Instance().AddToDirtyList(pstPlayer);

	return 0;

}

int CSsMineGetOreDataRsp::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{

	SS_PKG_MINE_GET_ORE_DATA_RSP& rstRsp = pstSsPkg->m_stBody.m_stMineGetOreDataRsp;

	DataResult tmpResult;
	tmpResult.m_bType = DATA_TYPE_ORE;
	if (rstRsp.m_ullTokenId == 0)
	{
		for (int i = 0; i < rstRsp.m_bOreCount; i++)
		{
			tmpResult.m_pstData = &rstRsp.m_astOreList[i];
			if (!MineDataMgr::Instance().IsInMem((void*)&tmpResult))
			{
				MineDataMgr::Instance().SaveInMem((void*)&tmpResult);
			}

		}
	}
	else
	{
		tmpResult.m_pstData = &rstRsp.m_astOreList[0];
		MineDataMgr::Instance().AsyncGetDataDone(rstRsp.m_ullTokenId, rstRsp.m_nErrNo, (void*)&tmpResult);
	}

	
	
	return 0;
}


int CSsMineGetPlayerDataRsp::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
	SS_PKG_MINE_GET_PLAYER_DATA_RSP& rstRsp = pstSsPkg->m_stBody.m_stMineGetPlayerDataRsp;

	DataResult tmpResult;
	tmpResult.m_bType = DATA_TYPE_PLAYER;
	tmpResult.m_pstData = &rstRsp.m_stPlayer;
	MineDataMgr::Instance().AsyncGetDataDone(rstRsp.m_ullTokenId, rstRsp.m_nErrNo, (void*)&tmpResult);
	return 0;
}




