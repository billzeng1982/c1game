#include <string.h>
#include "CommBusLayer.h"

//int CCommBusLayer::m_iBusHandle = 1;

CCommBusLayer::CCommBusLayer() : m_stRecvBuf( )
{
    m_szErrMsg[0] = '\0';
    m_iBusHandle = 0;

    m_iLocalAddr = 0;
   
    m_iSrc = 0;
    m_iDst = 0;
    m_uiDyeID = 0;
}

CCommBusLayer::~CCommBusLayer()
{
    if (m_iBusHandle >= 0)
    {
        tbus_delete(&m_iBusHandle);
    }

    tbus_fini();
}


int CCommBusLayer::Init(int iGCIMKey, int iAppAddr, int iFlag)
{
    char  szTBusGCIMKey[32];
    snprintf( szTBusGCIMKey, sizeof(szTBusGCIMKey), "%d", iGCIMKey );

    int iRet = tbus_init_ex( szTBusGCIMKey, iFlag );
    if ( TBUS_SUCCESS != iRet )
	{
		snprintf( m_szErrMsg, sizeof(m_szErrMsg), "tbus init error, [errno:%d][errstr:%s]", 
				  TBUS_ERR_GET_ERROR_CODE(iRet), tbus_error_string(iRet));
		return	-1;
	}

	iRet = tbus_new(&m_iBusHandle);
	if (TBUS_SUCCESS != iRet)
	{
		snprintf( m_szErrMsg, sizeof(m_szErrMsg),"tbus new error, [errno:%d][errstr:%s]", 
				  TBUS_ERR_GET_ERROR_CODE(iRet), tbus_error_string(iRet) );
		return	-2;
	}

    m_iLocalAddr = iAppAddr;
   
    iRet = tbus_bind(m_iBusHandle, m_iLocalAddr);
	if (TBUS_SUCCESS != iRet)
	{
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "tbus bind error, [errno:%d][errstr:%s]", 
				 TBUS_ERR_GET_ERROR_CODE(iRet), tbus_error_string(iRet) );
		return	-3;
	}

	//tbus_set_logpriority(TLOG_PRIORITY_TRACE);

	return	0;
}


/*
    使用tbus_peek_msg，减少一次数据copy，提高效率
    返回收到包的字节数
	0 - no msg
	<0 - error
*/
int CCommBusLayer::Recv( )
{
    int iRet = 0;

    // delete pre msg if exists
    if( m_stRecvBuf.m_szTdrBuf )
    {
        iRet = tbus_delete_msg( m_iBusHandle, m_iSrc, m_iDst );
        if (TBUS_SUCCESS != iRet)
        {
            snprintf(m_szErrMsg, sizeof(m_szErrMsg),"tbus delete msg error, [errno:%d][errstr:%s]", 
                     TBUS_ERR_GET_ERROR_CODE(iRet), tbus_error_string(iRet) );

            return  -1;
        }

        // clean up
        m_stRecvBuf.Reset( );
        m_iSrc = m_iDst = 0;
        m_uiDyeID = 0;
    }

    // peek msg
    iRet = tbus_peek_msg( m_iBusHandle, &m_iSrc, &m_iDst, (const char**)&m_stRecvBuf.m_szTdrBuf, &m_stRecvBuf.m_uPackLen, 0 );
    if (TBUS_SUCCESS != iRet)
    {
        if ( TBUS_ERR_GET_ERROR_CODE(iRet) == TBUS_ERROR_CHANNEL_IS_EMPTY)
        {
            // no msg in channel
            return  0;
        }

        snprintf(m_szErrMsg, sizeof(m_szErrMsg),"tbus peek msg error, [errno:%d][errstr:%s]", 
                 TBUS_ERR_GET_ERROR_CODE(iRet), tbus_error_string(iRet));

        return  -1;
    }
    
    //printf( "!!!!CommBus recv msg len %lu\n",  m_stRecvBuf.m_uPackLen );

    // 获取染色消息特征码 可选
    m_uiDyeID = tbus_get_dyedmsgid();

    return  (int)m_stRecvBuf.m_uPackLen;
}


int CCommBusLayer::Send( TBUSADDR iDstAddr, const MyTdrBuf& rstTdrBuf )
{
	int iRet = tbus_send( m_iBusHandle, &m_iLocalAddr, &iDstAddr, (void*)rstTdrBuf.m_szTdrBuf, rstTdrBuf.m_uPackLen, 0 );
	if (TBUS_SUCCESS != iRet)
	{
		snprintf(m_szErrMsg, sizeof(m_szErrMsg),"tbus send msg error, dest[%d] [errno:%d][errstr:%s]", 
				 iDstAddr, TBUS_ERR_GET_ERROR_CODE(iRet), tbus_error_string(iRet));
		return	-1;
	}

	return	(int)rstTdrBuf.m_uPackLen;
}


int CCommBusLayer::Send( TBUSADDR iDstAddr, char* pszPackBuff, size_t uPackLen )
{
    assert( pszPackBuff );

    int iRet = tbus_send( m_iBusHandle, &m_iLocalAddr, &iDstAddr, (void*)pszPackBuff, uPackLen, 0 );
	if (TBUS_SUCCESS != iRet)
	{
		snprintf(m_szErrMsg, sizeof(m_szErrMsg),"tbus send msg error, dest[%d] [errno:%d][errstr:%s]", 
				 iDstAddr, TBUS_ERR_GET_ERROR_CODE(iRet), tbus_error_string(iRet));
		return	-1;
	}

	return	(int)uPackLen;
}


bool CCommBusLayer::Backward( char* pszPkg, size_t uPkgLen )
{
    int iDst = 0, iSrc = 0;

    int iRet = tbus_backward( m_iBusHandle, &iSrc, &iDst, pszPkg, uPkgLen, 0 );
    if (TBUS_SUCCESS != iRet)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "tbus backward msg error, [errno:%d][errstr:%s]", 
                 TBUS_ERR_GET_ERROR_CODE(iRet), tbus_error_string(iRet));
        return	false;
    }

    return true;
}


bool CCommBusLayer::Backward()
{
    if (!m_stRecvBuf.m_szTdrBuf)
    {
        assert(false);
        return  false;
    }

    int iDst = 0, iSrc = 0;
    int iRet = tbus_backward(m_iBusHandle,&iSrc, &iDst, m_stRecvBuf.m_szTdrBuf, m_stRecvBuf.m_uPackLen, 0 );
    if (TBUS_SUCCESS != iRet)
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "tbus backward msg error, [errno:%d][errstr:%s]", 
        		 TBUS_ERR_GET_ERROR_CODE(iRet), tbus_error_string(iRet) );
        return	false;
    }

    return	true;
}


bool CCommBusLayer::Forward(TBUSADDR iDstAddr, char* pszPkg, size_t uPkgLen )
{
	int	iRet = 0;

	iRet = tbus_forward(m_iBusHandle, &m_iLocalAddr, &iDstAddr, pszPkg, uPkgLen, 0);
	if (TBUS_SUCCESS != iRet)
	{
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "tbus forward msg error, [errno:%d][errstr:%s]", 
				 TBUS_ERR_GET_ERROR_CODE(iRet), tbus_error_string(iRet));
		return	false;
	}
	
	return	true;
}


bool CCommBusLayer::Forward( TBUSADDR iDstAddr )
{
    if (!m_stRecvBuf.m_szTdrBuf)
    {
        assert(false);
        return  false;
    }
    
	int iRet = tbus_forward( m_iBusHandle, &m_iLocalAddr, &iDstAddr, 
                             m_stRecvBuf.m_szTdrBuf, m_stRecvBuf.m_uPackLen, 0 );
	if (TBUS_SUCCESS != iRet)
	{
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "tbus forward msg error, [errno:%d][errstr:%s]", 
				 TBUS_ERR_GET_ERROR_CODE(iRet), tbus_error_string(iRet));
		return	false;
	}

	return true;
}

bool CCommBusLayer::RefreshHandle()
{
    int iRet = tbus_refresh_handle(m_iBusHandle);

    if (TBUS_SUCCESS != iRet)
	{
		snprintf(m_szErrMsg, sizeof(m_szErrMsg), "tbus forward msg error, [errno:%d][errstr:%s]", 
				 TBUS_ERR_GET_ERROR_CODE(iRet), tbus_error_string(iRet));
		return	false;
	}

	return true;
}

