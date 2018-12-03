#include "RedisWorkThread.h"




using namespace PKGMETA;




bool RedisWorkThread::_ThreadAppInit(void* pvAppData)
{        
	m_pstConfig = (FRIENDSVRCFG*)pvAppData;
	if ( !(m_oRedisHandler.Init(m_pstConfig->m_szRedisAddr, m_pstConfig->m_wRedisPort) &&  m_oRedisHandler.Connect()) )
	{
		LOGERR_r( "Connect redis failed!" );
		return false;
	}
	if ( !m_stPlayerInfoTable.Init(&m_oRedisHandler) )
	{
		LOGERR_r( "Init m_stPlayerInfoTable failed!" );
		return false;
	}
	return true;
}


int RedisWorkThread::_ThreadAppProc()
{
    if (_CheckRedisConn() <= 0)
    {
        return -1;
    }
	if (_HandleReqMsg() <= 0)
	{
		return -1;
	}

	return 0;
}

//处理外部发来的 数据库请求
int RedisWorkThread::_HandleReqMsg()
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
	bzero(&m_stReq, sizeof(m_stReq));
	TdrError::ErrorType iRet = m_stReq.unpack(pstRecvBuf->m_szTdrBuf, pstRecvBuf->m_uPackLen);
	if (iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR_r("unpack pkg failed! errno : %d", iRet);
		return 1;
	}
	
	switch (m_stReq.m_bType)
	{
	case FRIEND_REDIS_TYPE_GET_PLAYER_INFO:
		GetPlayerInfo(m_stReq);
		break;;
	case FRIEND_REDIS_TYPE_SAVE_PLAYER_INFO:
		SavePlayerInfo(m_stReq);
		break;
	case FRIEND_REDIS_TYPE_GET_AGREE_APPLY_PLAYER_INFO:
		GetListPlayerInfo(m_stReq);
        break;
	default:
        LOGERR_r("m_stReq.m_bType err : %u", m_stReq.m_bType);
		break;
	}
	return 1;
}

void RedisWorkThread::GetPlayerInfo(DT_FRIEND_PLAYERINFO_REDIS_REQ& rstReq)
{
	bzero(&m_stRsp, sizeof(m_stRsp));
	m_stRsp.m_stPlayerInfo.m_ullUin = rstReq.m_stPlayerInfo.m_ullUin;
	m_stRsp.m_nErrNo = m_stPlayerInfoTable.GetPlayerInfo(m_stRsp.m_stPlayerInfo);
	m_stRsp.m_bType = rstReq.m_bType;
	m_stRsp.m_ullToken = rstReq.m_ullToken;
	SendRsp(m_stRsp);
}

void RedisWorkThread::SavePlayerInfo(DT_FRIEND_PLAYERINFO_REDIS_REQ& rstReq)
{
	bzero(&m_stRsp, sizeof(m_stRsp));
	m_stRsp.m_stPlayerInfo.m_ullUin = rstReq.m_stPlayerInfo.m_ullUin;
	m_stPlayerInfoTable.SavePlayerInfo(rstReq.m_stPlayerInfo);
}

void RedisWorkThread::GetListPlayerInfo(DT_FRIEND_PLAYERINFO_REDIS_REQ& rstReq)
{
	bzero(&m_stRsp, sizeof(m_stRsp));
	m_stRsp.m_nErrNo = m_stPlayerInfoTable.GetListPlayerInfo(rstReq.m_stPlayerInfo.m_ullUin, rstReq.m_stAgreeInfo, rstReq.m_stApplyInfo, m_stRsp.m_stWholeDataFront);
	m_stRsp.m_bType = rstReq.m_bType;
	m_stRsp.m_ullToken = rstReq.m_ullToken;
	m_stRsp.m_stPlayerInfo.m_ullUin = rstReq.m_stPlayerInfo.m_ullUin;
	SendRsp(m_stRsp);
}


void RedisWorkThread::SendReq(DT_FRIEND_PLAYERINFO_REDIS_REQ& rstReq)
{
	TdrError::ErrorType iRet = rstReq.pack(m_stSendReqBuf.m_szTdrBuf, m_stSendReqBuf.m_uSize, &m_stSendReqBuf.m_uPackLen );
	if( iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR_r("pack DT_FRIEND_PLAYERINFO_REDIS_REQ pkg error! Type<%d>, Token<%lu> Uin<%lu>", rstReq.m_bType, rstReq.m_ullToken, rstReq.m_stPlayerInfo.m_ullUin);
		return;
	}
	m_oReqQueue.WriteMsg(m_stSendReqBuf.m_szTdrBuf, m_stSendReqBuf.m_uPackLen);
	//@TODO_DEBUG_DEL  删除
	//LOGRUN_r("SendReq to Redis! Type<%d>, Token<%lu> Uin<%lu>", rstReq.m_bType, rstReq.m_ullToken, rstReq.m_stPlayerInfo.m_ullUin);
	return;
}

void RedisWorkThread::SendRsp(DT_FRIEND_PLAYERINFO_REDIS_RSP& rstRsp)
{
	TdrError::ErrorType iRet = rstRsp.pack(m_stSendRspBuf.m_szTdrBuf, m_stSendRspBuf.m_uSize, &m_stSendRspBuf.m_uPackLen);
	if( iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR_r("pack DT_FRIEND_PLAYERINFO_REDIS_RSP pkg error! Type<%d>, Token<%lu> Uin<%lu>", rstRsp.m_bType, rstRsp.m_ullToken, rstRsp.m_stPlayerInfo.m_ullUin);
		return;
	}

	m_oRspQueue.WriteMsg(m_stSendRspBuf.m_szTdrBuf, m_stSendRspBuf.m_uPackLen);
	//@TODO_DEBUG_DEL  删除
	//LOGRUN_r("Redis SendRsp to Logic! Type<%d>, Token<%lu> Uin<%lu>", rstRsp.m_bType, rstRsp.m_ullToken, rstRsp.m_stPlayerInfo.m_ullUin);
	return;
}

int RedisWorkThread::_CheckRedisConn()
{
    if (!m_oRedisHandler.IsAlive())
    {
        if (!m_oRedisHandler.Reconnect())
        {
            LOGERR_r("redis reconnect faild");
            return -1;
        }
        LOGRUN_r("redis reconnect ok");
    }
    time_t lCurTime = CGameTime::Instance().GetCurrSecond();
    if ( (lCurTime - m_tLastPingTime) >= m_pstConfig->m_iPingFreq) 
    {
        m_oRedisHandler.Ping();
        m_tLastPingTime = lCurTime;
    }
    return 1;
}






