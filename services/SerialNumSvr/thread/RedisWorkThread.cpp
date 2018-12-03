#include "RedisWorkThread.h"
#include "../framework/SerialNumSvrMsgLayer.h"

using namespace PKGMETA;

RedisWorkThread::RedisWorkThread() : m_stSendBuf(sizeof(SSPKG) * 2 + 1)
{
    m_pstConfig = NULL;
}

bool RedisWorkThread::_ThreadAppInit(void* pvPara)
{
	m_pstConfig = (SERIALNUMSVRCFG*)pvPara;

	if (!m_oRedisHandler.Init(m_pstConfig->m_szRedisAddr, m_pstConfig->m_wRedisPort))
	{
		LOGERR_r("Init redis failed!");
		return false;
	}

    if (!m_oRedisHandler.Connect())
    {
        LOGERR_r("Connect redis failed!");
		return false;
    }

	if (!m_stSerialTable.Init(&m_oRedisHandler))
	{
		LOGERR_r("Init ServialTable failed!");
		return false;
	}

	return true;
}

void RedisWorkThread::_ThreadAppFini()
{
    m_oRedisHandler.Close();
}

int RedisWorkThread::_ThreadAppProc()
{
	if (_CheckRedisConn() <= 0)
	{
		return -1;
	}

	if (_HandleSvrMsg() <= 0)
	{
		return -1;
	}

	return 0;
}

int RedisWorkThread::_HandleSvrMsg()
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
		return 1;
	}

	IMsgBase* poMsgHandler = SerialNumSvrMsgLayer::Instance().GetMsgHandler(m_stSsRecvPkg.m_stHead.m_wMsgId);
	if (!poMsgHandler)
	{
		LOGERR_r("Can not find msg handler. id <%u>", m_stSsRecvPkg.m_stHead.m_wMsgId );
		return 1;
	}

	poMsgHandler->HandleServerMsg(m_stSsRecvPkg, this);

	return 1;
}

void RedisWorkThread::SendPkg(PKGMETA::SSPKG& rstSsPkg)
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






