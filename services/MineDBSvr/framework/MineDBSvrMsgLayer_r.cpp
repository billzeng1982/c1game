
#include "MineDBSvrMsgLayer_r.h"
#include "ss_proto.h"
#include "../logic/MineDBMsg.h"
using namespace PKGMETA;

bool MineDBSvrMsgLayer_r::Init()
{
    this->_RegistServerMsg();

    return true;
}

void MineDBSvrMsgLayer_r::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER_r(SS_MSG_MINE_GET_ORE_DATA_REQ, MineGetOreDataReq_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER_r(SS_MSG_MINE_UPT_ORE_DATA_NTF, MineUptOreDataNtf_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER_r(SS_MSG_MINE_DEL_ORE_DATA_NTF, MineDelOreDataNtf_SS, m_oSsMsgHandlerMap);

    REGISTER_MSG_HANDLER_r(SS_MSG_MINE_GET_PLAYER_DATA_REQ, MineGetPlayerDataReq_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER_r(SS_MSG_MINE_UPT_PLAYER_DATA_NTF, MineUptPlayerDataNtf_SS, m_oSsMsgHandlerMap);


}

