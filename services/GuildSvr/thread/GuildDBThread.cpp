#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "GuildDBThread.h"
#include "oi_misc.h"
#include "TableDefine.h"
#include "LogMacros.h"

using namespace PKGMETA;

GuildDBThread::GuildDBThread() : m_stSendBuf(sizeof(DT_GUILD_DB_REQ)*2+1)
{

}

void GuildDBThread::SendReqPkg(DT_GUILD_DB_REQ& rstDBReq)
{
    TdrError::ErrorType iRet = rstDBReq.pack(m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uSize, &m_stSendBuf.m_uPackLen );
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("pack GUILD_DB_REQ pkg error! operate type(%d)", rstDBReq.m_bType);
        return;
    }

    m_oReqQueue.WriteMsg(m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uPackLen);
}

void GuildDBThread::SendRspPkg(DT_GUILD_DB_RSP& rstDBRsp)
{
    TdrError::ErrorType iRet = rstDBRsp.pack(m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uSize, &m_stSendBuf.m_uPackLen);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("pack GUILD_DB_RSP pkg error! operate type(%d)", rstDBRsp.m_bType);
        return;
    }

    m_oRspQueue.WriteMsg(m_stSendBuf.m_szTdrBuf, m_stSendBuf.m_uPackLen);
}

bool GuildDBThread::_ThreadAppInit(void* pvAppData)
{
    GUILDSVRCFG* pstConfig = (GUILDSVRCFG*)pvAppData;

    // init mysql hanlder
    if( !m_oMysqlHandler.ConnectDB( pstConfig->m_szDBAddr,
                                 pstConfig->m_wPort,
                                 pstConfig->m_szDBName,
                                 pstConfig->m_szUser,
                                 pstConfig->m_szPassword) )
    {
        LOGERR_r( "Connect mysql failed!" );
        return false;
    }

    if(!m_oGuildTable.Init(GUILD_TABLE_NAME, &m_oMysqlHandler, pstConfig->m_iGuildTableNum))
    {
        LOGERR_r( "GuilTable init failed!" );
        return false;
    }

    m_iPingFreq = pstConfig->m_iPingFreq;

    return true;
}

void GuildDBThread::_ThreadAppFini()
{
    int iRet = _HandleMsg();
    while (iRet > 0)
    {
        iRet = _HandleMsg();
    }

    if (iRet <0)
    {
        LOGERR_r("_HandleMsg failed, iRet=%d", iRet);
    }
}

int GuildDBThread::_ThreadAppProc()
{
    int iRet = _CheckMysqlConn();
    if (iRet <= 0)
    {
        return -1;
    }

    iRet = _HandleMsg();
    if (iRet <= 0)
    {
        return -1;
    }

    return 0;
}

int GuildDBThread::_HandleMsg()
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
    switch(m_stReqPkg.m_bType)
    {
        case GUILD_DB_UPDATE:
            iOpRet = m_oGuildTable.UpdateGuildWholeData(m_stReqPkg.m_stData);
            return 1;
        case GUILD_DB_DEL:
            iOpRet = m_oGuildTable.DeleteGuild(m_stReqPkg.m_stData.m_stBaseInfo.m_ullGuildId);
            return 1;
        case GUILD_DB_CREATE:
            iOpRet = m_oGuildTable.CreateGuild(m_stReqPkg.m_stData);
            break;
        case GUILD_DB_GET:
            iOpRet = m_oGuildTable.GetGuildWholeData(m_stReqPkg.m_stData.m_stBaseInfo.m_ullGuildId, m_stReqPkg.m_stData);
            break;
        case GUILD_DB_GET_BY_NAME:
            iOpRet = m_oGuildTable.GetGuildWholeData(m_stReqPkg.m_stData.m_stBaseInfo.m_szName, m_stReqPkg.m_stData);
            break;
        default:
            LOGERR_r("GuildTable handlemsg error, msg type(%d) is error", m_stReqPkg.m_bType);
            return -3;
    }

    m_stRspPkg.m_nErrNo = iOpRet;
    m_stRspPkg.m_bType = m_stReqPkg.m_bType;
    m_stRspPkg.m_ullActionId = m_stReqPkg.m_ullActionId;
    m_stRspPkg.m_stData = m_stReqPkg.m_stData;

    SendRspPkg(m_stRspPkg);

    return 1;
}

int GuildDBThread::_CheckMysqlConn()
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

