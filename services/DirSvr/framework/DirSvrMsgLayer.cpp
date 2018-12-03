#include "LogMacros.h"
#include "../DirSvr.h"
#include "ObjectPool.h"
#include "DirSvrMsgLayer.h"
#include "../module/ZoneSvrMgr.h"

using namespace PKGMETA;

DirSvrMsgLayer::DirSvrMsgLayer() : CMsgLayerBase(sizeof(PKGMETA::SCPKG) + 256, sizeof(PKGMETA::SSPKG) + 256)
{

}

bool DirSvrMsgLayer::Init()
{
    DIRSVRCFG& rConfig = DirSvr::Instance().GetConfig();
    if (!this->_Init(rConfig.m_iBusGCIMKey, rConfig.m_iProcID))
    {
        return false;
    }

    if (!m_oUdpRelaySvr.Init("0.0.0.0", rConfig.m_wUdpSvrPort, UdpRelaySvr::DEFAULT_THREAD_Q_SIZE))
    {
        LOGERR_r("m_oUdpRelaySvr init failed");
        return false;
    }

    return true;
}

int DirSvrMsgLayer::DealPkg()
{
    int iRecvBytes = 0;
    int i = 0;

    for (; i < DEAL_PKG_PER_LOOP; i++)
    {
        iRecvBytes = m_oUdpRelaySvr.Recv(CThreadFrame::MAIN_THREAD);
        if (iRecvBytes < 0)
        {
            LOGERR("udp recv error!");
            return -1;
        }

        if (0 == iRecvBytes)
        {
            break;
        }

        MyTdrBuf* pTdrBuf = m_oUdpRelaySvr.GetRecvBuf(CThreadFrame::MAIN_THREAD);
        TdrError::ErrorType iRet = m_stSvrInfo.unpack(pTdrBuf->m_szTdrBuf, pTdrBuf->m_uPackLen);
        if ( iRet != TdrError::TDR_NO_ERROR )
        {
            LOGERR("unpack pkg failed! errno : %d\n", iRet);
            return -1;
        }

        ZoneSvrMgr::Instance().HandleSvrStatNtf(m_stSvrInfo);
    }

    return i;
}

