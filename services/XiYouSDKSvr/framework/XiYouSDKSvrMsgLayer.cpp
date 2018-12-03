#include "XiYouSDKSvrMsgLayer.h"
#include "define.h"
#include "LogMacros.h"
#include "../XiYouSDKSvr.h"
#include "../logic/XiYouSDKMsgLogic.h"
#include "../cfg/XiYouSDKSvrCfgDesc.h"

using namespace PKGMETA;

XiYouSDKSvrMsgLayer::XiYouSDKSvrMsgLayer() : CMsgLayerBase( sizeof(PKGMETA::SCPKG)*2 , sizeof(PKGMETA::SSPKG)*2 )
{

}

void XiYouSDKSvrMsgLayer::_RegistServerMsg()
{
    //REGISTER_MSG_HANDLER(SS_MSG_SDK_ACCOUNT_LOGIN_REQ,        SDKAccountLoginReq_SS, m_oSsMsgHandlerMap);
    //REGISTER_MSG_HANDLER(SS_MSG_SDK_GET_ORDERID_REQ,        SDKGetOrderIDReq_SS, m_oSsMsgHandlerMap);
}

bool XiYouSDKSvrMsgLayer::Init()
{
    m_pstConfig = &(XiYouSDKSvr::Instance().GetConfig());
    if (!this->_Init(m_pstConfig->m_iBusGCIMKey, m_pstConfig->m_iProcID))
    {
        return false;
    }
    m_iThreadIdx = 0;

    return true;
}

int XiYouSDKSvrMsgLayer::DealPkg()
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

        //将收到的消息分发给线程处理
        MyTdrBuf* pstTdrBuf = m_oCommBusLayer.GetRecvTdrBuf();
        this->_ForwardToWorkThread(pstTdrBuf);
    }

     //处理work threads的response
     _HandleWorkThreadRsp();

    return i;
}

void XiYouSDKSvrMsgLayer::_ForwardToWorkThread(MyTdrBuf* pstTdrBuf)
{
    m_iThreadIdx = (m_iThreadIdx + 1) % m_pstConfig->m_iClientThreadNum;

    HttpClientThread* poWorkThread = XiYouSDKSvr::Instance().GetClientThread(m_iThreadIdx);
    if (!poWorkThread)
    {
        LOGERR_r("get thread error. iThreadIdx: %d", m_iThreadIdx);
        return;
    }

    int iRet = poWorkThread->GetReqQueue()->WriteMsg(pstTdrBuf->m_szTdrBuf, pstTdrBuf->m_uPackLen);
    if( iRet != THREAD_Q_SUCESS )
    {
        LOGERR_r("Write msg to thread(%d) failed! errno(%d)", m_iThreadIdx, iRet);
        return;
    }
}

int XiYouSDKSvrMsgLayer::_HandleWorkThreadRsp()
{
    int iRet = 0;
    int iDealPkgNum = 0;
    uint32_t dwPkgLen = 0;
    HttpClientThread* poClientThread = NULL;
    HttpServerThread* poServerThread = NULL;
    CThreadQueue* poRspQueue = NULL;

    //处理ClientThread的Rsp消息
    for (int i=0; i<m_pstConfig->m_iClientThreadNum; i++)
    {
        poClientThread = XiYouSDKSvr::Instance().GetClientThread(i);
        poRspQueue = poClientThread->GetRspQueue();
        iRet = poRspQueue->PeekMsg(&m_stSsSendBuf.m_szTdrBuf, &dwPkgLen);
        m_stSsSendBuf.m_uPackLen = dwPkgLen;
        if (iRet == THREAD_Q_EMPTY)
        {
            continue;
        }
        if (iRet != THREAD_Q_SUCESS)
        {
            LOGERR_r("Peek msg from thread(%d) failed! errno(%d)", i, iRet);
            continue;
        }

        m_oCommBusLayer.Send(m_pstConfig->m_iZoneSvrID, m_stSsSendBuf);
        poRspQueue->PopMsg( );
        iDealPkgNum++;
    }

    //处理ServerThread的Rsp消息
    for (int i=0; i<m_pstConfig->m_iServerThreadNum; i++)
    {
        poServerThread = XiYouSDKSvr::Instance().GetServerThread(i);
        poRspQueue = poServerThread->GetRspQueue();
        iRet = poRspQueue->PeekMsg(&m_stSsSendBuf.m_szTdrBuf, &dwPkgLen);
        m_stSsSendBuf.m_uPackLen = dwPkgLen;
        if (iRet == THREAD_Q_EMPTY)
        {
            continue;
        }
        if (iRet != THREAD_Q_SUCESS)
        {
            LOGERR_r("Peek msg from thread(%d) failed! errno(%d)", i, iRet);
            continue;
        }

        m_oCommBusLayer.Send(m_pstConfig->m_iZoneSvrID, m_stSsSendBuf);
        poRspQueue->PopMsg( );
        iDealPkgNum++;
    }
    return iDealPkgNum;
}


IMsgBase* XiYouSDKSvrMsgLayer::GetMsgHandler(int iMsgID)
{
    MsgHandlerMap_t::iterator TempMsgHndrIter;
    TempMsgHndrIter = m_oSsMsgHandlerMap.find(iMsgID);
    return (TempMsgHndrIter == m_oSsMsgHandlerMap.end()) ? NULL : TempMsgHndrIter->second;
}

int XiYouSDKSvrMsgLayer::SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(m_pstConfig->m_iZoneSvrID, rstSsPkg);
}

