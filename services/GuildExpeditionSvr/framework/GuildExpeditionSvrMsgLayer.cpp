#include "GuildExpeditionSvrMsgLayer.h"
#include "log/LogMacros.h"
#include "../GuildExpeditionSvr.h"
#include "../msglogic/MsgLogicGuildExpedition.h"

using namespace PKGMETA;

GuildExpeditionSvrMsgLayer::GuildExpeditionSvrMsgLayer() : CMsgLayerBase_c(sizeof(PKGMETA::SSPKG) + 256)
{
    m_pstConfig = NULL;
}

bool GuildExpeditionSvrMsgLayer::Init()
{
	m_pstConfig = &GuildExpeditionSvr::Instance().GetConfig();
    if (_Init(m_pstConfig->m_iBusGCIMKey, m_pstConfig->m_iProcID) < 0)
    {
        return false;
    }
	m_SendSsPkgPool.Init(20, 10, 200);

    return true;
}

void GuildExpeditionSvrMsgLayer::_RegistServerMsg()
{
	REGISTER_MSG_HANDLER_c(SS_MSG_GUILD_EXPEDITION_GET_ALL_INFO_REQ, CSsGuildExpeditionGetAllInfoReq, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER_c(SS_MSG_GUILD_EXPEDITION_UPLOAD_GUILD_INFO_NTF, CSsGuildExpeditionUploadGuildInfoNtf, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER_c(SS_MSG_GUILD_EXPEDITION_GET_PLAYER_DATA_RSP, CSsGuildExpeditionGetPlayerDataRsp, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER_c(SS_MSG_GUILD_EXPEDITION_GET_GUILD_DATA_RSP, CSsGuildExpeditionGetGuildDataRsp, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER_c(SS_MSG_GUILD_EXPEDITION_FIGHT_REQUEST_REQ, CSsGuildExpeditionFightRequestReq, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER_c(SS_MSG_GUILD_EXPEDITION_FIGHT_RESULT_REQ, CSsGuildExpeditionFightResultReq, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER_c(SS_MSG_GUILD_EXPEDITION_SET_FIGHT_INFO_NTF, CSsGuildExpeditionSetFightInfoNtf, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER_c(SS_MSG_GUILD_EXPEDITION_MATCH_REQ, CSsGuildExpeditionMatchReq, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER_c(SS_MSG_GUILD_EXPEDITION_GUILD_STATE_NTF, CSsGuildExpeditionGuildStateNtf, m_oSsMsgHandlerMap);

	

}

int GuildExpeditionSvrMsgLayer::DealPkg()
{
    int iRecvBytes = 0;
    int iDealPkgNum = 0;
    int i = 0;

    for (; i<DEAL_PKG_PER_LOOP; i++ )
    {
        iRecvBytes = m_oCommBusLayer.Recv();
        if( iRecvBytes < 0 )
        {
            LOGERR_r("bus recv error %s", m_oCommBusLayer.GetErrMsg());
            return -1;
        }
        if( 0 == iRecvBytes )
        {
            break;
        }

        this->_DealSvrPkg();
        iDealPkgNum++;
    }

    return iDealPkgNum;
}

int GuildExpeditionSvrMsgLayer::SendToOtherSvrByGate(PKGMETA::SSPKG* pstSsPkg)
{
	pstSsPkg->m_stHead.m_ullReservId |= (uint64_t)PROC_PASS_THROUGH << 32;
	int iRet = SendToServer(GuildExpeditionSvr::Instance().GetConfig().m_iClusterGateID, *pstSsPkg);
	m_SendSsPkgPool.Release(pstSsPkg);
	return iRet;
}



int GuildExpeditionSvrMsgLayer::SendToClusterDBSvr(PKGMETA::SSPKG* pstSsPkg)
{
    int iRet = SendToServer(GuildExpeditionSvr::Instance().GetConfig().m_iClusterDBSvrID, *pstSsPkg);
	m_SendSsPkgPool.Release(pstSsPkg);
	return iRet;
}




