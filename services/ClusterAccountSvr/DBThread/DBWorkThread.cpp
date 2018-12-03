#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "DBWorkThread.h"
#include "oi_misc.h"
#include "TableDefine.h"
#include "LogMacros.h"
#include "ThreadFrame.h"
#include "../framework/ClusterAccSvrMsgLayer.h"

CDBWorkThread::CDBWorkThread() : m_stSendBuf( sizeof(PKGMETA::SSPKG) )
{
    m_pstConfig = NULL;
}

void CDBWorkThread::SendPkg( PKGMETA::SSPKG& rstSsPkg )
{
    rstSsPkg.m_stHead.m_iSrcProcId = m_pstConfig->m_iProcID;

    TdrError::ErrorType iRet = rstSsPkg.pack( m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uSize, &m_stSendBuf.m_uPackLen );
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("pack ss pkg error! cmd <%u>", rstSsPkg.m_stHead.m_wMsgId );
        return;  
    }

    m_oRspQueue.WriteMsg(m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uPackLen);
}

bool CDBWorkThread::_ThreadAppInit(void* pvAppData)
{        
    m_pstConfig = (CLUSTERACCOUNTSVRCFG*)pvAppData;
    
    // init mysql hanlder
    if( !m_oMysqlHandler.ConnectDB( m_pstConfig->m_szDBAddr, 
                                    m_pstConfig->m_wPort, 
                                    m_pstConfig->m_szDBName, 
                                    m_pstConfig->m_szUser, 
                                    m_pstConfig->m_szPassword ) )
    {
        LOGERR_r( "Connect mysql failed!" );
        return false;
    }
    m_oMysqlHandler.SetPingFreq( m_pstConfig->m_iPingFreq );
    
    m_oMsgLayer.Init();

    m_oClusterAccTable.SetConfig(m_pstConfig);
    bool bRet = m_oClusterAccTable.Init( CLUSTER_ACCOUNT_TABLE_NAME, &m_oMysqlHandler, m_pstConfig->m_iAccTableNum);
    return bRet;
}

void CDBWorkThread::_ThreadAppFini()
{

}

int CDBWorkThread::_ThreadAppProc()
{
    int iRet = m_oMysqlHandler.CheckConn();
    if (iRet < 0)
    {   
        // not alive
        return -1;
    }
    
    return _HandleClusterAccSvrMsg();
}

int CDBWorkThread::_HandleClusterAccSvrMsg()
{
    int iRecvBytes = Recv(WORK_THREAD);
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
        return 1;
    }     

    //IMsgBase* poMsgHandler = ClusterAccSvrMsgLayer::Instance().GetMsgHandler( m_stSsRecvPkg.m_stHead.m_wMsgId );
    IMsgBase_r* poMsgHandler = m_oMsgLayer.GetServerMsgHandler( m_stSsRecvPkg.m_stHead.m_wMsgId );
    if (!poMsgHandler)
    {
        LOGERR_r("Can not find msg handler. id <%u>", m_stSsRecvPkg.m_stHead.m_wMsgId );
        return 1;
    }

    poMsgHandler->HandleServerMsg(m_stSsRecvPkg, this);
    return 1;
}

