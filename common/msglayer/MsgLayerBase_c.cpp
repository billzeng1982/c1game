#include "MsgLayerBase_c.h"
#include "LogMacros.h"

using namespace PKGMETA;

void CoroutineFunc_SS(void * pArgs)
{
    CoArgs_SS stCorArgs = *(CoArgs_SS*)pArgs;
    stCorArgs.m_oMsgBase->HandleServerMsg(stCorArgs.m_pstSsPkg);
    stCorArgs.m_pstMsgLayer->ReleaseSsPkg(stCorArgs.m_pstSsPkg);
}

CMsgLayerBase_c::CMsgLayerBase_c(size_t uSsSize) :  m_stSsSendBuf(uSsSize)
{ }

bool CMsgLayerBase_c::_Init(int iBusCGIMKey, int iProcId)
{
	m_iProcId = iProcId;
	m_iBusGCIMKey = iBusCGIMKey;

	if (m_oCommBusLayer.Init(m_iBusGCIMKey, m_iProcId) < 0)
	{
		return false;
	}

    if (!m_oCoEnv.Init(MAX_COROUTINE_NUM))
    {
        return false;
    }

    if (!m_oSsPkgPool.Init(SSPKG_INIT_NUM, SSPKG_DELTA_NUM, MAX_COROUTINE_NUM))
    {
        return false;
    }

	this->_RegistServerMsg();

	return true;
}

void CMsgLayerBase_c::_DealSvrPkg()
{
	MyTdrBuf* pstTdrBuf = m_oCommBusLayer.GetRecvTdrBuf();

    SSPKG* pstSsPkg = m_oSsPkgPool.Get();
    if (!pstSsPkg)
    {
        LOGERR_r("Can not get SSPKG from pool");
        return;
    }

	TdrError::ErrorType iRet = pstSsPkg->unpack(pstTdrBuf->m_szTdrBuf, pstTdrBuf->m_uPackLen);
	if( iRet != TdrError::TDR_NO_ERROR)
	{
        m_oSsPkgPool.Release(pstSsPkg);
		return;
	}

	IMsgBase_c* poMsgHandler = this->GetServerMsgHandler(pstSsPkg->m_stHead.m_wMsgId);
	if( !poMsgHandler )
	{
		LOGERR_r("Can not find msg handler. msgid<%u> SrcProc<%u>", pstSsPkg->m_stHead.m_wMsgId, pstSsPkg->m_stHead.m_iSrcProcId);
        m_oSsPkgPool.Release(pstSsPkg);
		return;
	}

    m_oCoArgs.m_oMsgBase = poMsgHandler;
    m_oCoArgs.m_pstSsPkg = pstSsPkg;
    m_oCoArgs.m_pstMsgLayer = this;

    int iCoRet = m_oCoEnv.StartCoroutine(CoroutineFunc_SS, (void*)&m_oCoArgs);
    if (iCoRet < 0)
    {
        LOGERR_r("StartCoroutine failed. msgid<%u> Ret<%d>", pstSsPkg->m_stHead.m_wMsgId, iCoRet);
        m_oSsPkgPool.Release(pstSsPkg);
        return;
    }

	return;
}

void CMsgLayerBase_c::DealSvrPkg(SSPKG& rstSsPkg)
{
	IMsgBase_c* poMsgHandler = this->GetServerMsgHandler(rstSsPkg.m_stHead.m_wMsgId);
	if( !poMsgHandler )
	{
		LOGERR_r("Can not find msg handler. id<%u>", rstSsPkg.m_stHead.m_wMsgId );
		return;
	}

	//poMsgHandler->HandleServerMsg(rstSsPkg);
	return;
}

int CMsgLayerBase_c::SendToServer(int iAddr, SSPKG& rstSsPkg)
{
	rstSsPkg.m_stHead.m_iSrcProcId = m_iProcId;
	rstSsPkg.m_stHead.m_iDstProcId = iAddr;

	TdrError::ErrorType iRet = rstSsPkg.pack( m_stSsSendBuf.m_szTdrBuf, m_stSsSendBuf.m_uSize, &m_stSsSendBuf.m_uPackLen );
	if (iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR_r("pack ss pkg error! cmd<%u>, ret<%d> \n", rstSsPkg.m_stHead.m_wMsgId, (int)iRet);
		return -1;
	}

	return m_oCommBusLayer.Send(iAddr, m_stSsSendBuf);
}

int CMsgLayerBase_c::SendToServer(int iAddr, MyTdrBuf* pstTdrBuf)
{
	return m_oCommBusLayer.Send(iAddr, *pstTdrBuf);
}

IMsgBase_c* CMsgLayerBase_c::GetServerMsgHandler(int iMsgId)
{
	MsgHandlerMap_c_t::iterator it = m_oSsMsgHandlerMap.find(iMsgId);
	return (it == m_oSsMsgHandlerMap.end()) ? NULL : it->second;
}

SSPKG* CMsgLayerBase_c::UnpackSsPkg(MyTdrBuf* pstTdrBuf)
{
	if (!pstTdrBuf)
	{
		return NULL;
	}

	int iRet = m_stSsRecvPkg.unpack(pstTdrBuf->m_szTdrBuf, pstTdrBuf->m_uPackLen);
	if (iRet != TdrError::TDR_NO_ERROR)
	{
		return NULL;
	}

	return &m_stSsRecvPkg;
}

