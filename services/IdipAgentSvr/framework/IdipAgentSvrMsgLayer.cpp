#include "define.h"
#include "LogMacros.h"
#include "IdipAgentSvr.h"
#include "IdipAgentSvrMsgLayer.h"
#include "IdipAgentMsgLogic.h"
#include "idip_proto.h"
#include "ss_proto.h"
#include "HttpReqMgr.h"

using namespace PKGMETA;

IdipAgentSvrMsgLayer::IdipAgentSvrMsgLayer() : CMsgLayerBase( sizeof(PKGMETA::SCPKG) * 2 , sizeof(PKGMETA::SSPKG) * 2 )
{

}

void IdipAgentSvrMsgLayer::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER(SS_MSG_GM_RSP,                  GmRsp_SS,            m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_IDIP_RSP,				 LogQueryRsp_SS,	  m_oSsMsgHandlerMap);
	REGISTER_MSG_HANDLER(SS_MSG_DATABASE_OP, GM_DATABASE_OP_SS, m_oSsMsgHandlerMap);
}

bool IdipAgentSvrMsgLayer::Init()
{
	m_pstConfig = &(IdipAgentSvr::Instance().GetConfig());
	if (!this->_Init(m_pstConfig->m_iBusGCIMKey, m_pstConfig->m_iProcID))
	{
		return false;
	}

	return true;
}

int IdipAgentSvrMsgLayer::DealPkg()
{
	int iRecvBytes = 0;
	int i = 0;

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

		this->_DealSvrPkg();
	}

	//´¦Àíwork threadsµÄresponse
	_HandleWorkThreadRsp();

	return i;
}

int IdipAgentSvrMsgLayer::_HandleWorkThreadRsp()
{
	int iRet = 0;
	int iDealPkgNum = 0;
	uint32_t dwPkgLen = 0;
	HttpServerThread* poServerThread = NULL;
	CThreadQueue* poRspQueue = NULL;

	//´¦ÀíServerThreadµÄRspÏûÏ¢
	for (int i = 0; i < m_pstConfig->m_iServerThreadNum; i++)
	{
		poServerThread = IdipAgentSvr::Instance().GetServerThread(i);
		poRspQueue = poServerThread->GetRspQueue();
		iRet = poRspQueue->PeekMsg(&m_stSsSendBuf.m_szTdrBuf, &dwPkgLen);
		m_stSsSendBuf.m_uPackLen = dwPkgLen;

		if (iRet == THREAD_Q_EMPTY)
		{
			continue;
		}

		if (iRet != THREAD_Q_SUCESS)
		{
			LOGERR_r("Peek msg from thread(%d) failed! errno(%d)", i, iRet);
			continue;
		}

        if (SS_MSG_GM_REQ == SSPKG_MSGID(m_stSsSendBuf.m_szTdrBuf))
        {
            m_oCommBusLayer.Send(m_pstConfig->m_iZoneSvrID, m_stSsSendBuf); 
        }
		else if (SS_MSG_DATABASE_OP == SSPKG_MSGID(m_stSsSendBuf.m_szTdrBuf))
		{
			m_oCommBusLayer.Send(m_pstConfig->m_iRoleSvrID, m_stSsSendBuf);
		}
        else
        {
            m_oCommBusLayer.Send(m_pstConfig->m_iLogQuerySvrID, m_stSsSendBuf);
        }
		poRspQueue->PopMsg( );
		iDealPkgNum++;
	}

	return iDealPkgNum;
}

IMsgBase* IdipAgentSvrMsgLayer::GetMsgHandler(int iMsgID)
{
	MsgHandlerMap_t::iterator TempMsgHndrIter;
	TempMsgHndrIter = m_oSsMsgHandlerMap.find(iMsgID);
	return (TempMsgHndrIter == m_oSsMsgHandlerMap.end()) ? NULL : TempMsgHndrIter->second;
}

int IdipAgentSvrMsgLayer::SendToQuerySvr(PKGMETA::SSPKG& rstSsPkg)
{
	return SendToServer(m_pstConfig->m_iLogQuerySvrID, rstSsPkg);
}

int IdipAgentSvrMsgLayer::SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(m_pstConfig->m_iZoneSvrID, rstSsPkg);
}
int IdipAgentSvrMsgLayer::SendToRoleSvr(PKGMETA::SSPKG& rstSsPkg)
{
	return SendToServer(m_pstConfig->m_iRoleSvrID, rstSsPkg);
}
