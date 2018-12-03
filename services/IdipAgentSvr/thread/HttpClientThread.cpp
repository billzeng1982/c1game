#include "oi_misc.h"
#include "LogMacros.h"
#include "HttpClientThread.h"
#include "IdipAgentSvrMsgLayer.h"

using namespace PKGMETA;

const char* IDIP_HTTP_HEADER = "Content-Type: application/x-www-form-urlencoded";

HttpClientThread::HttpClientThread() : m_stSendBuf(sizeof(SSPKG) * 2 + 1)
{

}

void HttpClientThread::SendPkg(SSPKG& rstSsPkg)
{
    rstSsPkg.m_stHead.m_iSrcProcId = m_pstConfig->m_iProcID;

    TdrError::ErrorType iRet = rstSsPkg.pack(m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uSize, &m_stSendBuf.m_uPackLen);

    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("pack ss pkg error! MsgId(%u)", rstSsPkg.m_stHead.m_wMsgId);
        return;
    }

    m_oRspQueue.WriteMsg(m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uPackLen);
}

bool HttpClientThread::_ThreadAppInit(void* pvAppData)
{
    m_pstConfig = (IDIPAGENTSVRCFG*)pvAppData;
    m_oHttpClient.HeaderAppend(IDIP_HTTP_HEADER);
    return true;
}

void HttpClientThread::_ThreadAppFini()
{

}

int HttpClientThread::_ThreadAppProc()
{
    int iRecvBytes = this->Recv(WORK_THREAD);
    if (iRecvBytes < 0)
    {
        return -1;
    }
    else if (0 == iRecvBytes)
    {
        return 0;
    }

    MyTdrBuf* pstRecvBuf = GetRecvBuf(WORK_THREAD);
    TdrError::ErrorType iRet = m_stSsRecvPkg.unpack(pstRecvBuf->m_szTdrBuf, pstRecvBuf->m_uPackLen);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("unpack pkg failed! errno : %d", iRet);
        return -1;
    }

    IMsgBase* poMsgHandler = IdipAgentSvrMsgLayer::Instance().GetMsgHandler(m_stSsRecvPkg.m_stHead.m_wMsgId);
    if( !poMsgHandler )
    {
        LOGERR_r("Can not find msg handler. id <%u>", m_stSsRecvPkg.m_stHead.m_wMsgId);
        return 1;
    }

    poMsgHandler->HandleServerMsg(m_stSsRecvPkg, this);

    return 1;
}
