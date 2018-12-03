#include "MsgLayerBase.h"
#include "CpuSampleStats.h"
#include "MsgBase.h"
#include "LogMacros.h"
CMsgLayerBase::CMsgLayerBase( size_t uScSize, size_t uSsSize ) : m_stScSendBuf(uScSize), m_stSsSendBuf(uSsSize)
{

}

bool CMsgLayerBase::_Init(int iBusCGIMKey, int iProcId )
{
	m_iProcId     = iProcId;
	m_iBusGCIMKey = iBusCGIMKey;

	if( m_oCommBusLayer.Init(m_iBusGCIMKey, m_iProcId) < 0 )
	{
		return false;
	}

	this->_RegistClientMsg();
	this->_RegistServerMsg();
	//this->_RegistOssMsg();

	return true;
}

void CMsgLayerBase::_DealSvrPkg()
{
	MyTdrBuf* pstTdrBuf  = m_oCommBusLayer.GetRecvTdrBuf();

	TdrError::ErrorType iRet = m_stSsRecvPkg.unpack( pstTdrBuf->m_szTdrBuf, pstTdrBuf->m_uPackLen );
	if( iRet != TdrError::TDR_NO_ERROR)
	{
		return;
	}

	IMsgBase* poMsgHandler = this->GetServerMsgHandler( m_stSsRecvPkg.m_stHead.m_wMsgId );
	if( !poMsgHandler )
	{
		LOGERR_r("Can not find msg handler. id <%u> SrcProc <%u>", m_stSsRecvPkg.m_stHead.m_wMsgId, m_stSsRecvPkg.m_stHead.m_iSrcProcId);
		return;
	}

       
    CpuSampleStats::Instance().BeginSample("CMsgLayerBase::_DealSvrPk::%hu", m_stSsRecvPkg.m_stHead.m_wMsgId);
	poMsgHandler->HandleServerMsg(m_stSsRecvPkg);
    CpuSampleStats::Instance().EndSample();
        

    

	return;
}

void CMsgLayerBase::DealSvrPkg(SSPKG& rstSsPkg)
{
	IMsgBase* poMsgHandler = this->GetServerMsgHandler( rstSsPkg.m_stHead.m_wMsgId );
	if( !poMsgHandler )
	{
		LOGERR_r("Can not find msg handler. id <%u>", rstSsPkg.m_stHead.m_wMsgId );
		return;
	}

    CpuSampleStats::Instance().BeginSample("CMsgLayerBase::_DealSvrPk::%hu", rstSsPkg.m_stHead.m_wMsgId);
	poMsgHandler->HandleServerMsg(rstSsPkg);
    CpuSampleStats::Instance().EndSample();
	return;
}

int CMsgLayerBase::SendToServer( int iAddr, PKGMETA::SSPKG& rstSsPkg )
{
	rstSsPkg.m_stHead.m_iSrcProcId = m_iProcId;
	rstSsPkg.m_stHead.m_iDstProcId = iAddr;

	TdrError::ErrorType iRet = rstSsPkg.pack( m_stSsSendBuf.m_szTdrBuf, m_stSsSendBuf.m_uSize, &m_stSsSendBuf.m_uPackLen );
	if( iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR_r("pack ss pkg error! cmd <%u>, <%d> \n", rstSsPkg.m_stHead.m_wMsgId, (int)iRet);
		return -1;
	}

	return m_oCommBusLayer.Send(iAddr, m_stSsSendBuf);
}

int CMsgLayerBase::SendToServer(int iAddr, MyTdrBuf* pstTdrBuf)
{
	return m_oCommBusLayer.Send(iAddr, *pstTdrBuf);
}

int CMsgLayerBase::_SendToClient( int iConnAddr, const PKGMETA::CONNSESSION* pstConnSess, PKGMETA::SCPKG* pstScPkg, char cSessCmd, uint16_t wCutVersion)
{
	if( !pstConnSess )
	{
		assert( false );
		return -1;
	}

	if( (0 == pstConnSess->m_iConnHnd || 0 == pstConnSess->m_dwSessionId)  &&
        (pstConnSess->m_chCmd != PKGMETA::CONNSESSION_CMD_BROADCAST))
	{
		return 0;
	}

	PKGMETA::CONNSESSION stSendSess;
	bzero( &stSendSess, sizeof(stSendSess) );
	stSendSess.m_chCmd = cSessCmd;
	stSendSess.m_dwSessionId = pstConnSess->m_dwSessionId;
	stSendSess.m_iConnHnd = pstConnSess->m_iConnHnd;
	stSendSess.m_stCmdData = pstConnSess->m_stCmdData;
	stSendSess.m_chConnType = pstConnSess->m_chConnType;

	m_stScSendBuf.Reset();
	size_t uUsedSize = 0;
	TdrError::ErrorType iRet = stSendSess.pack( m_stScSendBuf.m_szTdrBuf, m_stScSendBuf.m_uSize, &uUsedSize );
	if( iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR_r("pack CONNSESSION error! cmd <%d>\n", cSessCmd);
		return -1;
	}
	m_stScSendBuf.m_uPackLen = uUsedSize;

	if( pstScPkg )
	{
		char* pPkg =  m_stScSendBuf.m_szTdrBuf + uUsedSize;
		iRet = pstScPkg->pack( pPkg, m_stScSendBuf.m_uSize-uUsedSize, &uUsedSize, wCutVersion);
		if( iRet != TdrError::TDR_NO_ERROR)
		{
			LOGERR_r("pack s->c pkg error! cmd-%u iRet-%d\n", pstScPkg->m_stHead.m_wMsgId, iRet);
			return -1;
		}
		m_stScSendBuf.m_uPackLen +=  uUsedSize;
	}

	return m_oCommBusLayer.Send( iConnAddr, m_stScSendBuf );
}

IMsgBase* CMsgLayerBase::GetClientMsgHandler( int iMsgId )
{
	MsgHandlerMap_t::iterator it = m_oCsMsgHandlerMap.find( iMsgId );
	return ( it == m_oCsMsgHandlerMap.end()) ? NULL : it->second;
}


IMsgBase* CMsgLayerBase::GetServerMsgHandler( int iMsgId )
{
	MsgHandlerMap_t::iterator it = m_oSsMsgHandlerMap.find( iMsgId );
	return ( it == m_oSsMsgHandlerMap.end()) ? NULL : it->second;
}

/*IMsgBase* CMsgLayerBase::GetOssMsgHandler( int iMsgId )
{
	MsgHandlerMap_t::iterator it = m_oOssMsgHandlerMap.find( iMsgId );
	return ( it == m_oOssMsgHandlerMap.end()) ? NULL : it->second;
}*/

PKGMETA::SSPKG* CMsgLayerBase::UnpackSsPkg( MyTdrBuf* pstTdrBuf )
{
	if( !pstTdrBuf )
	{
		return NULL;
	}

	int iRet = m_stSsRecvPkg.unpack( pstTdrBuf->m_szTdrBuf, pstTdrBuf->m_uPackLen );
	if( iRet != TdrError::TDR_NO_ERROR)
	{
		return NULL;
	}

	return &m_stSsRecvPkg;
}

