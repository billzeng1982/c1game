

#include "GuildExpeditionDBMsg.h"
#include "../framework/ClusterDBSvrMsgLayer.h"
#include "../thread/DBWorkThread.h"



int GuildExpeditionGetPlayerDataReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
	CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
	SS_PKG_GUILD_EXPEDITION_GET_PLAYER_DATA_REQ& rstReq = rstSsPkg.m_stBody.m_stGuildExpeditionGetPlayerDataReq;
	SS_PKG_GUILD_EXPEDITION_GET_PLAYER_DATA_RSP& rstRsp = m_stSsPkg.m_stBody.m_stGuildExpeditionGetPlayerDataRsp;
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_EXPEDITION_GET_PLAYER_DATA_RSP;
	rstRsp.m_ullTokenId = rstReq.m_ullTokenId;
	rstRsp.m_nError = poWorkThread->GetGuildExpeditionPlayerTable().GetData(rstReq.m_Key, rstReq.m_chCount, 
		 rstRsp.m_astData, &rstRsp.m_chCount);
	poWorkThread->SendToGuildExpeditionSvr(m_stSsPkg);
	return 0;
}

int GuildExpeditionGetGuildDataReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
	CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
	SS_PKG_GUILD_EXPEDITION_GET_GUILD_DATA_REQ& rstReq = rstSsPkg.m_stBody.m_stGuildExpeditionGetGuildDataReq;
	SS_PKG_GUILD_EXPEDITION_GET_GUILD_DATA_RSP& rstRsp = m_stSsPkg.m_stBody.m_stGuildExpeditionGetGuildDataRsp;
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_EXPEDITION_GET_GUILD_DATA_RSP;
	rstRsp.m_ullTokenId = rstReq.m_ullTokenId;
	rstRsp.m_nError = poWorkThread->GetGuildExpeditionGuildTable().GetData(rstReq.m_Key, rstReq.m_chCount,
		rstRsp.m_astData, &rstRsp.m_chCount);
	poWorkThread->SendToGuildExpeditionSvr(m_stSsPkg);
	return 0;
}

int GuildExpeditionUptPlayerDataNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
	CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
	SS_PKG_GUILD_EXPEDITION_UPDATE_PLAYER_DATA_NTF& rstNtf = rstSsPkg.m_stBody.m_stGuildExpeditionUpdatePlayerDataNtf;
	poWorkThread->GetGuildExpeditionPlayerTable().UptData(rstNtf.m_astPlayerData, rstNtf.m_chCount);
	return 0;
}

int GuildExpeditionUptGuildDataNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
	CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
	SS_PKG_GUILD_EXPEDITION_UPDATE_GUILD_DATA_NTF& rstNtf = rstSsPkg.m_stBody.m_stGuildExpeditionUpdateGuildDataNtf;
	poWorkThread->GetGuildExpeditionGuildTable().UptData(rstNtf.m_astGuildData, rstNtf.m_chCount);
	return 0;
}

int GuildExpeditionDelGuildDataNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
	CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
	SS_PKG_GUILD_EXPEDITION_DEL_GUILD_DATA_NTF& rstNtf = rstSsPkg.m_stBody.m_stGuildExpeditionDelGuildDataNtf;
	poWorkThread->GetGuildExpeditionGuildTable().DelData(rstNtf.m_Key, rstNtf.m_chCount);
}
