#include "SerialNumSvrMsgLayer.h"
#include "LogMacros.h"
#include "ObjectPool.h"
#include "hash_func.cpp"
#include "../SerialNumSvr.h"
#include "../logic/SerialNumTableMsg.h"

using namespace PKGMETA;

SerialNumSvrMsgLayer::SerialNumSvrMsgLayer() : CMsgLayerBase(sizeof(PKGMETA::SCPKG)*2 , sizeof(PKGMETA::SSPKG)*2)
{
    m_pstConfig = NULL;
    m_iCursor = 0;
}

bool SerialNumSvrMsgLayer::Init()
{
    SERIALNUMSVRCFG& rstDBSvrCfg =  SerialNumSvr::Instance().GetConfig();

    if( _Init(rstDBSvrCfg.m_iBusGCIMKey, rstDBSvrCfg.m_iProcID) < 0 )
    {
        return false;
    }

    m_pstConfig = &rstDBSvrCfg;

    return true;
}

void SerialNumSvrMsgLayer::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER(SS_MSG_CHECK_SERIAL_NUM_REQ,   CSsCheckSerialNumReq,   m_oSsMsgHandlerMap);
}

int SerialNumSvrMsgLayer::DealPkg()
{
    int iRecvBytes = 0;
    int i = 0;
	int iDealPkgNum = 0;
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
		MyTdrBuf* pstTdrBuf  = m_oCommBusLayer.GetRecvTdrBuf();
		this->_ForwardToWorkThread(pstTdrBuf);
		iDealPkgNum++;
    }

	/*´¦Àíwork threadsµÄresponse*/
	RedisWorkThread* poWorkThread = NULL;
	int iRet = 0;
	uint32_t dwPkgLen = 0;
	for (int i = 0; i < m_pstConfig->m_iRedisWorkThreadNum; i++)
	{
		poWorkThread = SerialNumSvr::Instance().GetWorkThread(i);
		iRet = poWorkThread->GetRspQueue()->PeekMsg(&m_stSsSendBuf.m_szTdrBuf, &dwPkgLen);
		m_stSsSendBuf.m_uPackLen = dwPkgLen;
		if(iRet != THREAD_Q_SUCESS || !m_stSsSendBuf.m_szTdrBuf)
		{
			continue;
		}
		m_oCommBusLayer.Send(m_pstConfig->m_iClusterGateID, m_stSsSendBuf);
		poWorkThread->GetRspQueue()->PopMsg( );
		iDealPkgNum++;
	}

	return iDealPkgNum;
}

void SerialNumSvrMsgLayer::_ForwardToWorkThread(MyTdrBuf* pstTdrBuf)
{
    m_iCursor = (m_iCursor + 1) % m_pstConfig->m_iRedisWorkThreadNum;

	RedisWorkThread* poWorkThread = SerialNumSvr::Instance().GetWorkThread(m_iCursor);

	int iRet = poWorkThread->GetReqQueue()->WriteMsg(pstTdrBuf->m_szTdrBuf, pstTdrBuf->m_uPackLen);

	if (iRet != THREAD_Q_SUCESS)
	{
		LOGERR_r("Write msg to worker %d failed! errno <%d>", m_iCursor, iRet );
		return;
	}
}

IMsgBase* SerialNumSvrMsgLayer::GetMsgHandler(int iMsgID)
{
	MsgHandlerMap_t::iterator it = m_oSsMsgHandlerMap.find( iMsgID );
	return ( it == m_oSsMsgHandlerMap.end()) ? NULL : it->second;
}

