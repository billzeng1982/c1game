
#include "MiscSvrMsgLayer_r.h"
#include "ss_proto.h"
#include "../logic/CloneBattleDBMsg.h"
#include "../logic/LevelRrecordMsgLogic.h"
using namespace PKGMETA;

bool MiscSvrMsgLayer_r::Init()
{
    this->_RegistServerMsg();

    return true;
}

void MiscSvrMsgLayer_r::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER_r(SS_MSG_LEVEL_RECORD_READ_REQ, LevelRecordReadReq_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER_r(SS_MSG_LEVEL_RECORD_SAVE_REQ, LevelRecordSaveReq_SS, m_oSsMsgHandlerMap);

    REGISTER_MSG_HANDLER_r(SS_MSG_CLONE_BATTLE_DEL_DATA_NTF, CloneBattleDelDataNtf_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER_r(SS_MSG_CLONE_BATTLE_UPT_DATA_NTF, CloneBattleUptDataNtf_SS, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER_r(SS_MSG_CLONE_BATTLE_GET_DATA_REQ, CloneBattleGetDataReq_SS, m_oSsMsgHandlerMap);

}

