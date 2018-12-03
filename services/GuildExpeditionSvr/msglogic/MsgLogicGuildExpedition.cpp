
#include "LogMacros.h"
#include "strutil.h"
#include "./msglogic/MsgLogicGuildExpedition.h"
#include "../framework/GuildExpeditionSvrMsgLayer.h"
#include "../module/DataMgr.h"
#include "../module/LogicMgr.h"


using namespace PKGMETA;


int CSsGuildExpeditionGetPlayerDataRsp::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
	SS_PKG_GUILD_EXPEDITION_GET_PLAYER_DATA_RSP& rstRsp = pstSsPkg->m_stBody.m_stGuildExpeditionGetPlayerDataRsp;
	DataResult tmpResult;
	tmpResult.m_bType = DATA_TYPE_PLAYER;
	tmpResult.m_pstData = &rstRsp.m_astData[0];
	DataMgr::Instance().AsyncGetDataDone(rstRsp.m_ullTokenId, rstRsp.m_nError, (void*)&tmpResult);
	return 0;
}

int CSsGuildExpeditionGetGuildDataRsp::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
	SS_PKG_GUILD_EXPEDITION_GET_GUILD_DATA_RSP& rstRsp = pstSsPkg->m_stBody.m_stGuildExpeditionGetGuildDataRsp;
	DataResult tmpResult;
	tmpResult.m_bType = DATA_TYPE_GUILD;
	tmpResult.m_pstData = &rstRsp.m_astData[0];
	DataMgr::Instance().AsyncGetDataDone(rstRsp.m_ullTokenId, rstRsp.m_nError, (void*)&tmpResult);
	return 0;
}

int CSsGuildExpeditionGetAllInfoReq::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
	SSPKG* pstPkgNew = GuildExpeditionSvrMsgLayer::Instance().GetSendPkg();
	if (pstPkgNew == NULL)
	{
		LOGERR("get send pkg error");
		return -1;
	}
	SS_PKG_GUILD_EXPEDITION_GET_ALL_INFO_REQ& rstReq = pstSsPkg->m_stBody.m_stGuildExpeditionGetAllInfoReq;
	SS_PKG_GUILD_EXPEDITION_GET_ALL_INFO_RSP& rstRsp = pstPkgNew->m_stBody.m_stGuildExpeditionGetAllInfoRsp;
	bzero(&rstRsp, sizeof(rstRsp));
	rstRsp.m_nError = LogicMgr::Instance().GetGuildAllInfo(rstReq.m_ullGuildId, rstRsp.m_stAllInfo);
	pstPkgNew->m_stHead.m_wMsgId = SS_MSG_GUILD_EXPEDITION_GET_ALL_INFO_RSP;
	pstPkgNew->m_stHead.m_ullUin = pstSsPkg->m_stHead.m_ullUin;
	pstPkgNew->m_stHead.m_ullReservId = pstSsPkg->m_stHead.m_iSrcProcId;
	GuildExpeditionSvrMsgLayer::Instance().SendToOtherSvrByGate(pstPkgNew);
	return 0;
}

int CSsGuildExpeditionUploadGuildInfoNtf::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
	SS_PKG_GUILD_EXPEDITION_UPLOAD_GUILD_INFO_NTF& rstNtf = pstSsPkg->m_stBody.m_stGuildExpeditionUploadGulidInfoNtf;

	LogicMgr::Instance().UploadGuildInfo(rstNtf.m_stInfo);
	return 0;
}

int CSsGuildExpeditionSetFightInfoNtf::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
	SS_PKG_GUILD_EXPEDITION_SET_FIGHT_INFO_NTF&  rstNtf = pstSsPkg->m_stBody.m_stGuildExpeditionSetFightInfoNtf;
	LogicMgr::Instance().SetFightInfo(pstSsPkg->m_stHead.m_ullUin, rstNtf.m_ullGuildId, pstSsPkg->m_stHead.m_iSrcProcId,
		rstNtf.m_stFightInfo, rstNtf.m_stShowInfo);
	return 0;
}


int CSsGuildExpeditionFightRequestReq::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
	SSPKG* pstPkgNew = GuildExpeditionSvrMsgLayer::Instance().GetSendPkg();
	if (pstPkgNew == NULL)
	{
		LOGERR("get send pkg error");
		return -1;
	}
	SS_PKG_GUILD_EXPEDITION_FIGHT_REQUEST_REQ& rstReq = pstSsPkg->m_stBody.m_stGuildExpeditionFightRequestReq;
	SS_PKG_GUILD_EXPEDITION_FIGHT_REQUEST_RSP& rstRsp = pstPkgNew->m_stBody.m_stGuildExpeditionFightRequestRsp;
	bzero(&rstRsp, sizeof(rstRsp));
	rstRsp.m_nError = LogicMgr::Instance().FightRequest(pstSsPkg->m_stHead.m_ullUin, rstReq.m_ullGuildId, 
		rstReq.m_ullFoeUin, rstReq.m_ullFoeGuildId, rstRsp.m_stFightInfo);
	pstPkgNew->m_stHead.m_wMsgId = SS_MSG_GUILD_EXPEDITION_FIGHT_REQUEST_RSP;
	pstPkgNew->m_stHead.m_ullUin = pstSsPkg->m_stHead.m_ullUin;
	pstPkgNew->m_stHead.m_ullReservId = pstSsPkg->m_stHead.m_iSrcProcId;
	GuildExpeditionSvrMsgLayer::Instance().SendToOtherSvrByGate(pstPkgNew);

	return 0;
}

int CSsGuildExpeditionFightResultReq::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
	SSPKG* pstPkgNew = GuildExpeditionSvrMsgLayer::Instance().GetSendPkg();
	if (pstPkgNew == NULL)
	{
		LOGERR("get send pkg error");
		return -1;
	}
	SS_PKG_GUILD_EXPEDITION_FIGHT_RESULT_REQ& rstReq = pstSsPkg->m_stBody.m_stGuildExpeditionFightResultReq;
	SS_PKG_GUILD_EXPEDITION_FIGHT_RESULT_RSP& rstRsp = pstPkgNew->m_stBody.m_stGuildExpeditionFightResultRsp;
	rstRsp.m_nError = LogicMgr::Instance().FightResult(pstSsPkg->m_stHead.m_ullUin, rstReq.m_ullGuildId, rstReq.m_bIsWin,
		rstReq.m_ullFoeUin, rstReq.m_ullFoeGuildId);
	pstPkgNew->m_stHead.m_wMsgId = SS_MSG_GUILD_EXPEDITION_FIGHT_RESULT_RSP;
	pstPkgNew->m_stHead.m_ullUin = pstSsPkg->m_stHead.m_ullUin;
	pstPkgNew->m_stHead.m_ullReservId = pstSsPkg->m_stHead.m_iSrcProcId;
	rstRsp.m_bIsWin = rstReq.m_bIsWin;
	rstRsp.m_bSceneNum = rstReq.m_bSceneNum;
	GuildExpeditionSvrMsgLayer::Instance().SendToOtherSvrByGate(pstPkgNew);
	return 0;
}


int CSsGuildExpeditionMatchReq::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
	SSPKG* pstPkgNew = GuildExpeditionSvrMsgLayer::Instance().GetSendPkg();
	if (pstPkgNew == NULL)
	{
		LOGERR("get send pkg error");
		return -1;
	}
	SS_PKG_GUILD_EXPEDITION_MATCH_REQ& rstReq = pstSsPkg->m_stBody.m_stGuildExpeditionMatchReq;
	SS_PKG_GUILD_EXPEDITION_MATCH_RSP& rstRsp = pstPkgNew->m_stBody.m_stGuildExpeditionMatchRsp;
	bzero(&rstRsp, sizeof(rstRsp));
	int iRet = 0;
	do 
	{
		Guild* pstGuild = DataMgr::Instance().GetGuild(rstReq.m_ullGuildId);
		if (pstGuild == NULL)
		{
			iRet = ERR_NOT_FOUND;
			LOGERR("Uin<%lu>  Guild<%lu> match  error", pstSsPkg->m_stHead.m_ullUin, rstReq.m_ullGuildId);
			break;
		}
		iRet = pstGuild->MatchGuild();
		if (iRet == ERR_NONE)
		{
			pstGuild->GetAllInfo(rstRsp.m_stAllInfo);
		}
	} while (0);

	rstRsp.m_nError = iRet;
	pstPkgNew->m_stHead.m_wMsgId = SS_MSG_GUILD_EXPEDITION_MATCH_RSP;
	pstPkgNew->m_stHead.m_ullUin = pstSsPkg->m_stHead.m_ullUin;
	pstPkgNew->m_stHead.m_ullReservId = pstSsPkg->m_stHead.m_iSrcProcId;
	GuildExpeditionSvrMsgLayer::Instance().SendToOtherSvrByGate(pstPkgNew);
	return 0;
}

int CSsGuildExpeditionGuildStateNtf::HandleServerMsg(PKGMETA::SSPKG* pstSsPkg)
{
	SS_PKG_GUILD_EXPEDITION_GUILD_STATE_NTF& rstNtf = pstSsPkg->m_stBody.m_stGuildExpeditionGuildStateNtf;
	
	if (rstNtf.m_bType == 1)
	{
		LogicMgr::Instance().GuildDissovle(rstNtf.m_ullGuildId);
	}
	else if (rstNtf.m_bType == 2)
	{
		LogicMgr::Instance().GuildMemberQuit(rstNtf.m_ullGuildId, rstNtf.m_ullUllMemUin);
	}

	return 0;
}
