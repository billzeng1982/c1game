#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "TeamDBThread.h"
#include "oi_misc.h"
#include "TableDefine.h"
#include "LogMacros.h"

using namespace PKGMETA;

TeamDBThread::TeamDBThread() : m_stSendBuf(sizeof(DT_ASYNC_PVP_TEAM_DB_REQ)*2+1)
{

}

void TeamDBThread::SendReqPkg(DT_ASYNC_PVP_TEAM_DB_REQ& rstDBReq)
{
    TdrError::ErrorType iRet = rstDBReq.pack(m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uSize, &m_stSendBuf.m_uPackLen );
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("pack DT_ASYNC_PVP_TEAM_DB_REQ pkg error! operate type(%d)", rstDBReq.m_bOpType);
        return;
    }

    m_oReqQueue.WriteMsg(m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uPackLen);
}

void TeamDBThread::SendRspPkg(DT_ASYNC_PVP_TEAM_DB_RSP& rstDBRsp)
{
    TdrError::ErrorType iRet = rstDBRsp.pack(m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uSize, &m_stSendBuf.m_uPackLen );
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("pack DT_ASYNC_PVP_TEAM_DB_RSP pkg error! operate type(%d)", rstDBRsp.m_bOpType);
        return;
    }

    m_oRspQueue.WriteMsg(m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uPackLen);
}

bool TeamDBThread::_ThreadAppInit(void* pvAppData)
{
    ASYNCPVPSVRCFG* pstConfig = (ASYNCPVPSVRCFG*)pvAppData;

    // init mysql hanlder
    if (!m_oMysqlHandler.ConnectDB( pstConfig->m_szDBAddr,
                                 pstConfig->m_wPort,
                                 pstConfig->m_szDBName,
                                 pstConfig->m_szUser,
                                 pstConfig->m_szPassword) )
    {
        LOGERR_r("Connect mysql failed!");
        return false;
    }

    if (!m_oTeamTable.CInit(ASYNCPVP_PLAYER_TABLE_NAME, &m_oMysqlHandler, pstConfig))
    {
        LOGERR_r("TeamTable Init failed");
        return false;
    }

    m_iPingFreq = pstConfig->m_iPingFreq;

    return true;
}

void TeamDBThread::_ThreadAppFini()
{
    int iRet;
    do
    {
        iRet = _HandleMsg();
    }while(iRet > 0);

    if (iRet <0)
    {
        LOGERR_r("_HandleMsg failed, iRet=%d", iRet);
    }
}

int TeamDBThread::_ThreadAppProc()
{
    int iRet = _CheckMysqlConn();
    if (iRet <= 0)
    {
        return -1;
    }

    iRet = _HandleMsg();
    if (iRet <= 0)
    {
        return iRet;
    }

    return 0;
}

int TeamDBThread::_HandleMsg()
{
    int iRecvBytes = this->Recv(WORK_THREAD);
    if (iRecvBytes < 0)
    {
        LOGERR_r("DBWorkThread Recv Failed, iRecvBytes=%d", iRecvBytes);
        return -1;
    }
    else if (0 == iRecvBytes)
    {
        return 0;
    }

    MyTdrBuf* pstRecvBuf = GetRecvBuf(WORK_THREAD);
    TdrError::ErrorType iRet = m_stReqPkg.unpack(pstRecvBuf->m_szTdrBuf, pstRecvBuf->m_uPackLen);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("unpack pkg failed! errno : %d", iRet);
        return -2;
    }

    int iOpRet = 0;
    switch(m_stReqPkg.m_bOpType)
    {
        //Update和Delete不用回复
        case DB_UPDATE:
            iOpRet = m_oTeamTable.UpdateTeam(m_stReqPkg.m_stData);
            return 1;
        case DB_DEL:
            iOpRet = m_oTeamTable.DeleteTeam(m_stReqPkg.m_stData.m_ullUin);
            return 1;
        case DB_CREATE:
            iOpRet = m_oTeamTable.CreateTeam(m_stReqPkg.m_stData);
            break;
        case DB_GET:
            iOpRet = m_oTeamTable.GetTeam(m_stReqPkg.m_stData.m_ullUin, m_stReqPkg.m_stData);
            break;
        default:
            LOGERR_r("TeamDBThread handle msg error, msg type(%d) is error", m_stReqPkg.m_bOpType);
            return -3;
    }

    m_stRspPkg.m_nErrNo = iOpRet;
    m_stRspPkg.m_bOpType = m_stReqPkg.m_bOpType;
    m_stRspPkg.m_ullActionId = m_stReqPkg.m_ullActionId;
    m_stRspPkg.m_stData = m_stReqPkg.m_stData;

    SendRspPkg(m_stRspPkg);

    return 1;
}

int TeamDBThread::_CheckMysqlConn()
{
    // check alive
    if (!m_oMysqlHandler.IsConnAlive())
    {
        if (!m_oMysqlHandler.ReconnectDB())
        {
            LOGERR_r("ReconnectDB failed");
            return -1; //change to idle
        }
    }

    time_t lCurTime = CGameTime::Instance().GetCurrSecond();
    if (lCurTime - m_oMysqlHandler.GetLastPingTime() >= m_iPingFreq)
    {
        // ping mysql
        m_oMysqlHandler.Ping();
        m_oMysqlHandler.SetLastPingTime(lCurTime);
    }

    return 1;
}

