#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "DBWorkThread.h"
#include "oi_misc.h"
#include "TableDefine.h"
#include "LogMacros.h"
#include "ThreadFrame.h"
#include "../framework/MiscSvrMsgLayer.h"

CDBWorkThread::CDBWorkThread() : m_stSendBuf(sizeof(PKGMETA::SSPKG)*2+1)
{
    m_pstConfig = NULL;
}

void CDBWorkThread::SendPkg( PKGMETA::SSPKG& rstSsPkg )
{
    TdrError::ErrorType iRet = rstSsPkg.pack( m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uSize, &m_stSendBuf.m_uPackLen );
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("pack ss pkg error! cmd <%u>", rstSsPkg.m_stHead.m_wMsgId );
        return;  
    }
    m_oRspQueue.WriteMsg(m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uPackLen);
}


void CDBWorkThread::SendToCloneBattleSvr(PKGMETA::SSPKG& rstSsPkg)
{
    rstSsPkg.m_stHead.m_iDstProcId = m_pstConfig->m_iCloneBattleSvrID;
    rstSsPkg.m_stHead.m_iSrcProcId = m_pstConfig->m_iProcID;
    SendPkg(rstSsPkg);
}


void CDBWorkThread::SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg)
{
    rstSsPkg.m_stHead.m_iDstProcId = m_pstConfig->m_iZoneSvrID;
    rstSsPkg.m_stHead.m_iSrcProcId = m_pstConfig->m_iProcID;
    SendPkg(rstSsPkg);
}

bool CDBWorkThread::_ThreadAppInit(void* pvAppData)
{        
    m_pstConfig = (MISCSVRCFG*)pvAppData;
    
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

    m_oMsgLayer.Init();

    m_oCloneBattleTable.CInit("tbl_clonebattle_team", &m_oMysqlHandler, m_pstConfig);
    m_oLevelRankTabke.Init(m_pstConfig);
     
    return true;
}

void CDBWorkThread::_ThreadAppFini()
{

}

int CDBWorkThread::_ThreadAppProc()
{
    int iRet = _CheckMysqlConn();
    if (iRet <= 0)
    {
        return -1;
    }
    
    iRet = _HandleSvrMsg();
    if (iRet <= 0)
    {
        return -1;
    }
    
    return 0;
}

int CDBWorkThread::_HandleSvrMsg()
{
    int iRecvBytes = this->Recv( WORK_THREAD );
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
    IMsgBase_r* poMsgHandler = m_oMsgLayer.GetServerMsgHandler( m_stSsRecvPkg.m_stHead.m_wMsgId );
    if( !poMsgHandler )
    {
        LOGERR_r("Can not find msg handler. id <%u>", m_stSsRecvPkg.m_stHead.m_wMsgId);
        return 1;
    }

    poMsgHandler->HandleServerMsg(m_stSsRecvPkg, this);
    return 1;
}

int CDBWorkThread::_CheckMysqlConn()
{
    // check alive
    if (!m_oMysqlHandler.IsConnAlive())
    {
        if (!m_oMysqlHandler.ReconnectDB())
        {
            return -1; //change to idle
        }
    }
    
    time_t lCurTime = CGameTime::Instance().GetCurrSecond();
    if (lCurTime - m_oMysqlHandler.GetLastPingTime() >= m_pstConfig->m_iPingFreq)
    {
        // ping mysql
        m_oMysqlHandler.Ping();
        m_oMysqlHandler.SetLastPingTime(lCurTime);
    }

    return 1;
}

