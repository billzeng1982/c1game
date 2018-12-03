#include "SdkDMSvrMsgLayer.h"
#include "LogMacros.h"
#include "../SdkDMSvr.h"
#include "../msglogic/SdkDMSvrMsg.h"

using namespace PKGMETA;

SdkDMSvrMsgLayer::SdkDMSvrMsgLayer() : CMsgLayerBase( sizeof(PKGMETA::SCPKG)*2 , sizeof(PKGMETA::SSPKG)*2 )
{
    m_pstConfig = NULL;
}

bool SdkDMSvrMsgLayer::Init()
{
    SDKDMSVRCFG& rstDBSvrCfg =  SdkDMSvr::Instance().GetConfig();
    if( _Init(rstDBSvrCfg.m_iBusGCIMKey, rstDBSvrCfg.m_iProcID) < 0 )
    {
        return false;
    }

    m_pstConfig = &rstDBSvrCfg;

    return true;
}


void SdkDMSvrMsgLayer::_RegistServerMsg()
{
     REGISTER_MSG_HANDLER(SS_MSG_SDKDM_TDATA_SEND_ORDER_NTF,          SdkDMTDataSendOrderNtf,        m_oSsMsgHandlerMap);
}

int SdkDMSvrMsgLayer::DealPkg()
{
    int iRecvBytes = 0;
    int i = 0;

    for (; i < DEAL_PKG_PER_LOOP; i++)
    {
        iRecvBytes = m_oCommBusLayer.Recv();
        if (iRecvBytes < 0)
        {
            LOGERR_r("bus recv error!");
            return -1;
        }

        if (0 == iRecvBytes)
        {
            break;
        }

        this->_DealSvrPkg();
    }
    return i;
}

int SdkDMSvrMsgLayer::SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg)
{
    //return SendToServer(m_pstConfig->m_iZoneSvrID, rstSsPkg);
    return 0;
}

