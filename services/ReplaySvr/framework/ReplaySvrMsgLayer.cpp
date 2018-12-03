#include "LogMacros.h"
#include "ObjectPool.h"
#include "ReplaySvrMsgLayer.h"
#include "../ReplaySvr.h"
#include "../module/ReplayMgr.h"
#include "../msglogic/MsgLogicReplay.h"

using namespace PKGMETA;

ReplaySvrMsgLayer::ReplaySvrMsgLayer() : CMsgLayerBase(sizeof(PKGMETA::SCPKG) + 256, sizeof(PKGMETA::SSPKG) + 256)
{

}

void ReplaySvrMsgLayer::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER(SS_MSG_UPLOAD_REPLAY_REQ,          UploadReplayReq_SS,     m_oSsMsgHandlerMap);
}

bool ReplaySvrMsgLayer::Init()
{
    REPLAYSVRCFG& rConfig = ReplaySvr::Instance().GetConfig();
    if (!this->_Init(rConfig.m_iBusGCIMKey, rConfig.m_iProcID))
    {
        return false;
    }

    return true;
}

int ReplaySvrMsgLayer::DealPkg()
{
    int iRecvBytes = 0;
    int i = 0;

    for (; i < DEAL_PKG_PER_LOOP; i++)
    {
        iRecvBytes = m_oCommBusLayer.Recv();
        if (iRecvBytes < 0)
        {
            LOGERR("bus recv error!");
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

int ReplaySvrMsgLayer::SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(ReplaySvr::Instance().GetConfig().m_iZoneSvrID, rstSsPkg);
}

