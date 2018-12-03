#include "MineSvrMsgLayer.h"
#include "LogMacros.h"
#include "../MineSvr.h"
#include "../msglogic/MsgLogicMine.h"
#include "define.h"
using namespace PKGMETA;

MineSvrMsgLayer::MineSvrMsgLayer() : CMsgLayerBase_c(sizeof(PKGMETA::SSPKG) + 256)
{
    m_pstConfig = NULL;
}

bool MineSvrMsgLayer::Init()
{
    MINESVRCFG& rstDBSvrCfg = MineSvr::Instance().GetConfig();
    if (_Init(rstDBSvrCfg.m_iBusGCIMKey, rstDBSvrCfg.m_iProcID) < 0)
    {
        return false;
    }

    m_pstConfig = &rstDBSvrCfg;

    return true;
}

void MineSvrMsgLayer::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER_c(SS_MSG_MINE_GET_INFO_REQ,            CSsMineGetInfoReq,       m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER_c(SS_MSG_MINE_EXPLORE_REQ, CSsMineExploreReq, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER_c(SS_MSG_MINE_DEAL_ORE_REQ, CSsMineDealOreReq, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER_c(SS_MSG_MINE_GET_AWARD_REQ, CSsMineGetAwardReq, m_oSsMsgHandlerMap);

    REGISTER_MSG_HANDLER_c(SS_MSG_MINE_GET_REVENGER_INFO_REQ, CSsMineGetRevengerInfoReq, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER_c(SS_MSG_MINE_FIGHT_RESULT_REQ, CSsMineFightResultReq, m_oSsMsgHandlerMap);

    REGISTER_MSG_HANDLER_c(SS_MSG_MINE_GET_ORE_DATA_RSP, CSsMineGetOreDataRsp, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER_c(SS_MSG_MINE_GET_PLAYER_DATA_RSP, CSsMineGetPlayerDataRsp, m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER_c(SS_MSG_MINE_SVR_COMBINE_UPT_NTF, CSsMineSvrCombineUptNtf, m_oSsMsgHandlerMap);
}

int MineSvrMsgLayer::DealPkg()
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


int MineSvrMsgLayer::SendToClusterGate(PKGMETA::SSPKG& rstSsPkg)
{
    //直接给ZoneSvr调试
	rstSsPkg.m_stHead.m_ullReservId |= (uint64_t)PROC_ZONE_SVR << 32;
    return SendToServer(MineSvr::Instance().GetConfig().m_iClusterGateID, rstSsPkg);
}





int MineSvrMsgLayer::SendToMineDBSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(MineSvr::Instance().GetConfig().m_iMineDBSvrID, rstSsPkg);
}

