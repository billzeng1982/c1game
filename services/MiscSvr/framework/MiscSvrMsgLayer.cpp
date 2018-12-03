
#include "define.h"
#include "LogMacros.h"
#include "../MiscSvr.h"
#include "MiscSvrMsgLayer.h"
// #include "../logic/LevelRrecordMsgLogic.h"
//#include "../logic/CloneBattleDBMsg.h"


using namespace PKGMETA;

MiscSvrMsgLayer::MiscSvrMsgLayer() : CMsgLayerBase(sizeof(PKGMETA::SCPKG) * 2, sizeof(PKGMETA::SSPKG) * 2)
{

}




bool MiscSvrMsgLayer::Init()
{
	m_pstConfig = &(MiscSvr::Instance().GetConfig());
	if (!this->_Init(m_pstConfig->m_iBusGCIMKey, m_pstConfig->m_iProcID))
	{
		return false;
	}
    if (m_pstConfig->m_iWorkThreadNum <= 0)
    {
        assert(false);
        return false;
    }

    m_astWorkThreads = new CDBWorkThread[m_pstConfig->m_iWorkThreadNum];
    if (!m_astWorkThreads)
    {
        return false;
    }

    key_t iShmKey = m_pstConfig->m_iThreadQBaseShmKey;
    bool bRet = 0;
    for (int i = 0; i < m_pstConfig->m_iWorkThreadNum; i++)
    {
        bRet = m_astWorkThreads[i].InitThread(i, m_pstConfig->m_dwThreadQSize, THREAD_QUEUE_DUPLEX, (void*)m_pstConfig, &iShmKey);
        if (!bRet)
        {
            LOGERR_r("Init thread <%d> failed", i);
            return false;
        }
    }
 
	return true;
}

int MiscSvrMsgLayer::DealPkg()
{
    int iRecvBytes = 0;
    int iDealPkgNum = 0;
    

    for (int i = 0; i < DEAL_PKG_PER_LOOP; i++)
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

        MyTdrBuf* pstTdrBuf = m_oCommBusLayer.GetRecvTdrBuf();
        this->_ForwardToWorkThread(pstTdrBuf);

        iDealPkgNum++;
    }

    /*处理work threads的response*/
    CDBWorkThread* poWorkThread = NULL;
    CThreadQueue* poRspQueue = NULL;
    int iRet = 0;
    uint32_t dwPkgLen = 0;

    for (int i = 0; i < m_pstConfig->m_iWorkThreadNum; i++)
    {
        poWorkThread = GetWorkThread(i);
        poRspQueue = poWorkThread->GetRspQueue();
        iRet = poRspQueue->PeekMsg(&m_stSsSendBuf.m_szTdrBuf, &dwPkgLen);
        m_stSsSendBuf.m_uPackLen = dwPkgLen;
        if (iRet != THREAD_Q_SUCESS)
        {
            continue;
        }

        if (!m_stSsSendBuf.m_szTdrBuf)
        {
            continue;
        }
        m_oCommBusLayer.Send(SSPKG_DST_ADDR(m_stSsSendBuf.m_szTdrBuf), m_stSsSendBuf);
        poRspQueue->PopMsg();
        iDealPkgNum++;
    }
    return iDealPkgNum;
}



bool MiscSvrMsgLayer::Fini()
{
    for (int i = 0; i < m_pstConfig->m_iWorkThreadNum; i++)
    {
        m_astWorkThreads[i].FiniThread();
    }
    return true;
}

void MiscSvrMsgLayer::_ForwardToWorkThread(MyTdrBuf* pstTdrBuf)
{
    assert(pstTdrBuf);

    int iThreadIdx = -1;
    
    int iRet = 0;

    /* select work thread */
    iThreadIdx = (int)(SSPKG_HEAD_UIN(pstTdrBuf->m_szTdrBuf) % m_pstConfig->m_iWorkThreadNum);
    CDBWorkThread* poWorkThread = GetWorkThread(iThreadIdx);
    if (!poWorkThread)
    {
        LOGERR_r("get thread error. iThreadIdx: %d", iThreadIdx);
        return;
    }
    iRet = poWorkThread->GetReqQueue()->WriteMsg(pstTdrBuf->m_szTdrBuf, pstTdrBuf->m_uPackLen);
    if (iRet != THREAD_Q_SUCESS)
    {
        LOGERR_r("Write msg to worker %d failed! errno <%d>", iThreadIdx, iRet);
        return;
    }
}

