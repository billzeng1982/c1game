
#include "ClusterDBSvrMsgLayer_r.h"
#include "ss_proto.h"
#include "../logic/GuildExpeditionDBMsg.h"


using namespace PKGMETA;

bool ClusterDBSvrMsgLayer_r::Init()
{
    this->_RegistServerMsg();

    return true;
}

void ClusterDBSvrMsgLayer_r::_RegistServerMsg()
{
     REGISTER_MSG_HANDLER_r(SS_MSG_GUILD_EXPEDITION_GET_GUILD_DATA_REQ, GuildExpeditionGetGuildDataReq_SS, m_oSsMsgHandlerMap);
     REGISTER_MSG_HANDLER_r(SS_MSG_GUILD_EXPEDITION_GET_PLAYER_DATA_REQ, GuildExpeditionGetPlayerDataReq_SS, m_oSsMsgHandlerMap);
	 REGISTER_MSG_HANDLER_r(SS_MSG_GUILD_EXPEDITION_UPDATE_PLAYER_DATA_NTF, GuildExpeditionUptPlayerDataNtf_SS, m_oSsMsgHandlerMap);
	 REGISTER_MSG_HANDLER_r(SS_MSG_GUILD_EXPEDITION_UPDATE_GUILD_DATA_NTF, GuildExpeditionUptGuildDataNtf_SS, m_oSsMsgHandlerMap);

}

