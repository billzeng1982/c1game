#include "ZoneHttpConnMsgLayer.h"
#include "utils/MyTdrBuf.h"
#include "ZoneHttpConn.h"
#include "logic/ZoneHttpConnMsglogic.h"

ZoneHttpConnMsgLayer::ZoneHttpConnMsgLayer() : CMsgLayerBase(sizeof(PKGMETA::SCPKG) * 2, sizeof(PKGMETA::SSPKG) * 2)
{
}

ZoneHttpConnMsgLayer::~ZoneHttpConnMsgLayer()
{
}

bool ZoneHttpConnMsgLayer::Init()
{
    m_stConfig = ZoneHttpConn::Instance().GetConfig();
    if (!this->_Init(m_stConfig.m_iBusGCIMKey, m_stConfig.m_iProcID))
    {
        return false;
    }
    if (!m_oHttpControler.Init(m_stConfig))
    {
        return false;
    }

    return true;
}

int ZoneHttpConnMsgLayer::DealPkg()
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

    m_oHttpControler.Update();

    return i;
}

IMsgBase * ZoneHttpConnMsgLayer::GetMsgHandler(int iMsgID)
{
    MsgHandlerMap_t::iterator TempMsgHndrIter;
    TempMsgHndrIter = m_oSsMsgHandlerMap.find(iMsgID);
    return (TempMsgHndrIter == m_oSsMsgHandlerMap.end()) ? NULL : TempMsgHndrIter->second;
}

void ZoneHttpConnMsgLayer::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER(SS_MSG_ROLE_WEB_TASK_INFO_RSP, GetPlayerWebTaskInfoRsp, m_oSsMsgHandlerMap);
    REGISTER_MSG_HANDLER(SS_MSG_WEB_TASK_GET_RWD_RSP, GetWebTaskRwdRsp, m_oSsMsgHandlerMap);
}

int ZoneHttpConnMsgLayer::SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(m_stConfig.m_iZoneSvrID, rstSsPkg);
}
