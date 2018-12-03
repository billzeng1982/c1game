
#include "LogMacros.h"
#include "ss_proto.h"
#include "cs_proto.h"
#include "GameTime.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "../module/player/Player.h"
#include "../module/player/PlayerMgr.h"
#include "../module/Guild.h"
#include "MsgLogicGuildExpedition.h"
#include "PKGMETA_metalib.h"

using namespace PKGMETA;


int GuildExpeditionGetAllInfoReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (NULL == poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}
	uint64_t ullGuildId = poPlayer->GetPlayerData().GetGuildId();

	if (ullGuildId != 0 && Guild::Instance().IsExpeditionOpen())
	{
		m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_EXPEDITION_GET_ALL_INFO_REQ;
		m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
		SS_PKG_GUILD_EXPEDITION_GET_ALL_INFO_REQ& rstReq = m_stSsPkg.m_stBody.m_stGuildExpeditionGetAllInfoReq;
		rstReq.m_ullGuildId = ullGuildId;
		m_stSsPkg.m_stHead.m_ullReservId = 0;
		ZoneSvrMsgLayer::Instance().SendToGuildExpeditionSvr(m_stSsPkg);

	}
	else
	{
		m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_EXPEDITION_GET_ALL_INFO_RSP;
		SC_PKG_GUILD_EXPEDITION_GET_ALL_INFO_RSP& rstRsp = m_stScPkg.m_stBody.m_stGuildExpeditionGetAllInfoRsp;
		bzero(&rstRsp, sizeof(rstRsp));
		rstRsp.m_nError = Guild::Instance().IsExpeditionOpen() ? ERR_NOT_HAVE_GUILD  : ERR_MODULE_NOT_OPEN;
		ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
	}
	return 0;

}

int GuildExpeditionSetBattleArrayReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (NULL == poPlayer)
	{
		LOGERR("player not exist.");
		return -1;
	}

	m_stSsPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_EXPEDITION_SET_BATTLE_ARRAY_RSP;
	m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
	CS_PKG_GUILD_EXPEDITION_SET_BATTLE_ARRAY_REQ& rstReq = rstCsPkg.m_stBody.m_stGuildExpeditionSetBattleArrayReq;
	SC_PKG_GUILD_EXPEDITION_SET_BATTLE_ARRAY_RSP& rstRsp = m_stScPkg.m_stBody.m_stGuildExpeditionSetBattleArrayRsp;

	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_EXPEDITION_SET_BATTLE_ARRAY_RSP;
	rstRsp.m_nError = Guild::Instance().SetExpeditionArray(&poPlayer->GetPlayerData(), rstReq);
	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
	return 0;
}






int GuildExpeditionGetFightRequestReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGRUN("poPlayer is null");
		return -1;
	}

	CS_PKG_GUILD_EXPEDITION_FIGHT_REQUEST_REQ& rstCSReq = rstCsPkg.m_stBody.m_stGuildExpeditionFgihtRequestReq;
	
	int iRet = Guild::Instance().ExpeditionFightRequest(&poPlayer->GetPlayerData());
	if (ERR_NONE != iRet)
	{
		SC_PKG_GUILD_EXPEDITION_FIGHT_REQUEST_RSP& rstSCRsp = m_stScPkg.m_stBody.m_stGuildExpeditionFightRequestRsp;
		m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_EXPEDITION_FIGHT_REQUEST_RSP;
		rstSCRsp.m_nError = iRet;
		bzero(&rstSCRsp.m_stFightInfo, sizeof(rstSCRsp.m_stFightInfo));
		ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
	}
	else
	{
		SS_PKG_GUILD_EXPEDITION_FIGHT_REQUEST_REQ& rstSSReq = m_stSsPkg.m_stBody.m_stGuildExpeditionFightRequestReq;
		m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_EXPEDITION_FIGHT_REQUEST_REQ;
		m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
		rstSSReq.m_ullFoeUin = rstCSReq.m_ullFoeUin;
		rstSSReq.m_ullGuildId = poPlayer->GetPlayerData().GetGuildId();
		rstSSReq.m_ullFoeGuildId = rstCSReq.m_ullFoeGuildId;
		m_stSsPkg.m_stHead.m_ullReservId = 0;
		ZoneSvrMsgLayer::Instance().SendToGuildExpeditionSvr(m_stSsPkg);
	}
	return 0;
}

int GuildExpeditionGetFightResultReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGRUN("poPlayer is null");
		return -1;
	}
	CS_PKG_GUILD_EXPEDITION_FIGHT_RESULT_REQ& rstCSReq = rstCsPkg.m_stBody.m_stGuildExpeditionFightResultReq;
	int iRet = Guild::Instance().ExpeditionFightResult(&poPlayer->GetPlayerData(), rstCSReq);
	if (iRet != ERR_NONE)
	{
		m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_EXPEDITION_FIGHT_RESULT_RSP;
		m_stScPkg.m_stBody.m_stGuildExpeditionFightResultRsp.m_nError = iRet;
		m_stScPkg.m_stBody.m_stGuildExpeditionFightResultRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
		ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
	}
	return 0;

}

int GuildExpeditionMatchReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara /*= NULL*/)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
	if (!poPlayer)
	{
		LOGRUN("poPlayer is null");
		return -1;
	}
	CS_PKG_GUILD_EXPEDITION_MATCH_REQ& rstCSReq = rstCsPkg.m_stBody.m_stGuildExpeditionMatchReq;
	int iRet = Guild::Instance().ExpeditionMatch(&poPlayer->GetPlayerData());
	if (iRet != ERR_NONE)
	{
		m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_EXPEDITION_MATCH_RSP;
		m_stScPkg.m_stBody.m_stGuildExpeditionMatchRsp.m_nError = iRet;
		bzero(&m_stScPkg.m_stBody.m_stGuildExpeditionMatchRsp.m_stAllInfo, sizeof(m_stScPkg.m_stBody.m_stGuildExpeditionMatchRsp.m_stAllInfo));
		ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
	}
	return 0;
}



int GuildExpeditionGetAllInfoRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{

	Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
	if (!poPlayer)
	{
		LOGRUN("poPlayer is null");
		return -1;
	}
	SS_PKG_GUILD_EXPEDITION_GET_ALL_INFO_RSP& rstSSRsp = rstSsPkg.m_stBody.m_stGuildExpeditionGetAllInfoRsp;
	SC_PKG_GUILD_EXPEDITION_GET_ALL_INFO_RSP& rstSCRsp = m_stScPkg.m_stBody.m_stGuildExpeditionGetAllInfoRsp;


	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_EXPEDITION_GET_ALL_INFO_RSP;
	rstSCRsp.m_nError = rstSSRsp.m_nError;
	memcpy(&rstSCRsp.m_stAllInfo, &rstSSRsp.m_stAllInfo, sizeof(rstSCRsp.m_stAllInfo));
	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
	return 0;
}

int GuildExpeditionGetFightRequestRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
	if (!poPlayer)
	{
		LOGRUN("poPlayer is null");
		return -1;
	}
	SS_PKG_GUILD_EXPEDITION_FIGHT_REQUEST_RSP& rstSSRsp = rstSsPkg.m_stBody.m_stGuildExpeditionFightRequestRsp;
	SC_PKG_GUILD_EXPEDITION_FIGHT_REQUEST_RSP& rstSCRsp = m_stScPkg.m_stBody.m_stGuildExpeditionFightRequestRsp;
	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_EXPEDITION_FIGHT_REQUEST_RSP;
	rstSCRsp.m_nError = rstSSRsp.m_nError;
	memcpy(&rstSCRsp.m_stFightInfo, &rstSSRsp.m_stFightInfo, sizeof(rstSCRsp.m_stFightInfo));
	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
	return 0;
}


int GuildExpeditionGetFightResultRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
	if (!poPlayer)
	{
		LOGRUN("poPlayer is null");
		return -1;
	}
	SS_PKG_GUILD_EXPEDITION_FIGHT_RESULT_RSP& rstSSRsp = rstSsPkg.m_stBody.m_stGuildExpeditionFightResultRsp;
	SC_PKG_GUILD_EXPEDITION_FIGHT_RESULT_RSP& rstSCRsp = m_stScPkg.m_stBody.m_stGuildExpeditionFightResultRsp;
	rstSCRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
	
	if (rstSSRsp.m_nError == ERR_NONE)
	{
		rstSCRsp.m_nError = Guild::Instance().SettleExpeditionFight(&poPlayer->GetPlayerData(), rstSSRsp, rstSCRsp.m_stSyncItemInfo);
	}
	else
	{
		rstSCRsp.m_nError = rstSSRsp.m_nError;
	}
	Guild::Instance().GetRoleExpeditionInfo(&poPlayer->GetPlayerData(), rstSCRsp.m_stRoleExpeditionInfo);
	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_EXPEDITION_FIGHT_RESULT_RSP;
	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
	return 0;
}

int GuildExpeditionMatchRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
	Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
	if (!poPlayer)
	{
		LOGRUN("poPlayer is null");
		return -1;
	}
	SS_PKG_GUILD_EXPEDITION_MATCH_RSP& rstSSRsp = rstSsPkg.m_stBody.m_stGuildExpeditionMatchRsp;
	SC_PKG_GUILD_EXPEDITION_MATCH_RSP& rstSCRsp = m_stScPkg.m_stBody.m_stGuildExpeditionMatchRsp;

	rstSCRsp.m_nError = rstSSRsp.m_nError;
	memcpy(&rstSCRsp.m_stAllInfo, &rstSSRsp.m_stAllInfo, sizeof(rstSCRsp.m_stAllInfo));
	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GUILD_EXPEDITION_MATCH_RSP;
	ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
	return 0;

}


