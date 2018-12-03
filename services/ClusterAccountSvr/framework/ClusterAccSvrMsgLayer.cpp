#include "ClusterAccSvrMsgLayer.h"
#include "LogMacros.h"
#include "ObjectPool.h"
#include "hash_func.cpp"
#include "../ClusterAccountSvr.h"
#include "../logic/ThreadMsgLogic.h"
#include "FakeRandom.h"
#include <time.h>

using namespace PKGMETA;

ClusterAccSvrMsgLayer::ClusterAccSvrMsgLayer() : CMsgLayerBase( sizeof(PKGMETA::SCPKG)*2 , sizeof(PKGMETA::SSPKG)*2 )
{
    m_pstConfig = NULL;
}


bool ClusterAccSvrMsgLayer::Init()
{
    CLUSTERACCOUNTSVRCFG& rstDBSvrCfg =  ClusterAccountSvr::Instance().GetConfig();
    if( _Init(rstDBSvrCfg.m_iBusGCIMKey, rstDBSvrCfg.m_iProcID) < 0 )
    {
        return false;
    }

    m_pstConfig = &rstDBSvrCfg;

    if( !m_oHttpControler.Init(m_pstConfig) )
    {
        return false;
    }

    CFakeRandom::Instance().SetSeed( (uint32_t)time(NULL) );
    
    return true;
}

int ClusterAccSvrMsgLayer::DealPkg()
{
    int iRecvBytes = 0;
    int iDealPkgNum = 0;
    int i = 0;

    for (; i<DEAL_PKG_PER_LOOP; i++ )
    {
        iRecvBytes = m_oCommBusLayer.Recv();
        if( iRecvBytes < 0 )
        {
            LOGERR_r("bus recv error: %s", m_oCommBusLayer.GetErrMsg());
            return -1;
        }
        if( 0 == iRecvBytes )
        {
            break;
        }

        MyTdrBuf* pstTdrBuf  = m_oCommBusLayer.GetRecvTdrBuf();
        this->ForwardToWorkThread( pstTdrBuf );

        iDealPkgNum++;
    }

    /*处理work threads的response*/
    ClusterAccountSvr& roClusterAccSvr = ClusterAccountSvr::Instance();
    CDBWorkThread* poWorkThread = NULL;
    CThreadQueue* poRspQueue = NULL;
    int iRet = 0;
    uint32_t dwPkgLen = 0;

    for( int i=0; i < m_pstConfig->m_iWorkThreadNum; i++ )
    {
        poWorkThread = roClusterAccSvr.GetWorkThread(i);
        poRspQueue = poWorkThread->GetRspQueue();
        iRet = poRspQueue->PeekMsg( &m_stSsSendBuf.m_szTdrBuf, &dwPkgLen );
        m_stSsSendBuf.m_uPackLen = dwPkgLen;
        if( iRet != THREAD_Q_SUCESS )
        {
            continue;
        }

        if( !m_stSsSendBuf.m_szTdrBuf )
        {
            continue;
        }

        // 处理多线程返还的消息 
        uint16_t wMsgID = SSPKG_MSGID( m_stSsSendBuf.m_szTdrBuf );
        if( SS_MSG_GET_ONE_CLUSTER_ACC_INFO_RSP == wMsgID )
        {
            m_oHttpControler.SendHttpRsp(&m_stSsSendBuf);
        }
       
        poRspQueue->PopMsg( );
        iDealPkgNum++;
    }

    // 处理http 请求
    m_oHttpControler.Update();
    
    return iDealPkgNum;
}

bool ClusterAccSvrMsgLayer::ForwardToWorkThread( MyTdrBuf* pstTdrBuf )
{
    assert( pstTdrBuf );

    int iThreadIdx = -1;
    CDBWorkThread* poWorkThread = NULL;
    ClusterAccountSvr& roClusterAccSvr = ClusterAccountSvr::Instance();
    int iRet = 0;

    /* select work thread，随机选取 */
    iThreadIdx = (int)CFakeRandom::Instance().Random( m_pstConfig->m_iWorkThreadNum );
    
    poWorkThread = roClusterAccSvr.GetWorkThread(iThreadIdx);
    if (!poWorkThread)
    {
        LOGERR_r("get thread error. iThreadIdx: %d", iThreadIdx);
        return false;
    }

    iRet = poWorkThread->GetReqQueue()->WriteMsg( pstTdrBuf->m_szTdrBuf, pstTdrBuf->m_uPackLen );
    if( iRet != THREAD_Q_SUCESS )
    {
        LOGERR_r("Write msg to worker %d failed! errno <%d>", iThreadIdx, iRet );
        return false;
    }

    return true;
}


