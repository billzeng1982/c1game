#include "HttpControler.h"
#include <assert.h>
#include <LogMacros.h>
#include "strutil.h"
#include "../framework/ClusterAccSvrMsgLayer.h"

using namespace PKGMETA;

static void HttpGetOneAccInfoCb( struct evhttp_request *req, void *arg )
{
    HttpControler* poHttpControler = (HttpControler*)arg;
    CHttpServer* poHttpServer = poHttpControler->GetHttpServer();

    if (evhttp_request_get_command(req) == EVHTTP_REQ_POST) 
    {
        struct evbuffer *evbuf;
        evbuf = evhttp_request_get_input_buffer(req);
        int n = evbuffer_copyout(evbuf, poHttpServer->m_szReqDataBuf, (CHttpServer::HTTP_REQ_DATA_BUF_SIZE-1) );
        if( n <= 0 )
        {
            LOGERR_r("copy out req data error! n=%d\n",n);
            evhttp_request_free(req);
            return;
        }
        poHttpServer->m_szReqDataBuf[n]='\0';
  
        poHttpControler->OnHttpGetOneAccInfo( req, poHttpServer->m_szReqDataBuf );      
        return;
	}
}

HttpControler::HttpControler() : m_stTdrBuf( 2048 )
{
    m_pstConfig = NULL;
    m_pstMetaLib = NULL;
    m_dwSeqID = 0;
}

uint32_t HttpControler::_GetNextReqID()
{
    ++m_dwSeqID;
    if( 0 == m_dwSeqID )
        ++m_dwSeqID;
    return m_dwSeqID;
}

bool HttpControler::Init( CLUSTERACCOUNTSVRCFG* pstConfig )
{
    assert( pstConfig );
    m_pstConfig = pstConfig;

    bool bRet = m_oHttpServer.Init( m_pstConfig->m_szHttpAddr, m_pstConfig->m_wHttpPort);
    if( !bRet )
    {
        LOGERR_r( "init error! http addr: %s, port: %d", m_pstConfig->m_szHttpAddr, m_pstConfig->m_wHttpPort );
        return false;
    }

    m_oHttpServer.AddRequestCb( m_pstConfig->m_szGetOneAccInfoUri, HttpGetOneAccInfoCb, this );

    int iRet = tdr_load_metalib(&m_pstMetaLib, "../../protocol/ss_proto.bin" );
    if (TDR_ERR_IS_ERROR(iRet))
    {
       LOGERR_r("load metalib idip_proto.bin failed! (%s)", tdr_error_string(iRet));
       return false;
    }

    return true;
}

void HttpControler::Update()
{
    m_oHttpServer.RecvAndHandle();
}

void HttpControler::OnHttpGetOneAccInfo( struct evhttp_request *req, char* szReqJson )
{
    assert( req != NULL && !StrUtil::IsStringNull(szReqJson) );

    // json 2 tdr
    LPTDRMETA pstMeta = tdr_get_meta_by_name(m_pstMetaLib, "SS_PKG_GET_ONE_CLUSTER_ACC_INFO_REQ");
    assert(pstMeta);
   
    TDRDATA stHost, stJson;
    stJson.pszBuff = szReqJson;
    stJson.iBuff = CHttpServer::HTTP_REQ_DATA_BUF_SIZE;
    stHost.pszBuff = (char*)&m_stSsPkg.m_stBody.m_stGetOneClusterAccInfoReq;
    stHost.iBuff = sizeof(m_stSsPkg.m_stBody.m_stGetOneClusterAccInfoReq);
         
    int iRet = tdr_input_json_ex( pstMeta, &stHost, &stJson, 0);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        LOGERR_r("json(%s) to bin failed. (%s)", szReqJson, tdr_error_string(iRet));
        // send errmsg!
        this->_SendErrRsp(req, ERR_WRONG_PARA);
        return;
    }

    // send to db thread
    bzero(&m_stSsPkg.m_stHead, sizeof(m_stSsPkg.m_stHead));
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GET_ONE_CLUSTER_ACC_INFO_REQ;
    m_stSsPkg.m_stHead.m_ullReservId = this->_GetNextReqID();
    
    if( _Send2DbThread( m_stSsPkg ) )
    {
        this->_Add2HttpReqMap( (uint32_t)m_stSsPkg.m_stHead.m_ullReservId, req);
    }else
    {
        this->_SendErrRsp(req, ERR_SYS);
    }
}

void HttpControler::_SendErrRsp( struct evhttp_request *req, int iErrNo )
{
    char szJsonRsp[512];
    snprintf(szJsonRsp, sizeof(szJsonRsp), "{\"ErrNo\":%d}", iErrNo);
    this->_SendHttpRsp( req, szJsonRsp ); 
}

void HttpControler::_Add2HttpReqMap( uint32_t dwReqID, struct evhttp_request *req)
{
    if (NULL == req)
    {
		assert(false);
		return;
    }

	m_oHttpReqMap.insert(HttpReqMap_t::value_type( dwReqID, req ));
	return;
}

struct evhttp_request* HttpControler::_FindHttpReq( uint32_t dwReqID )
{
    HttpReqMap_t::iterator it = m_oHttpReqMap.find(dwReqID);
    if ( it  != m_oHttpReqMap.end())
    {
		return it->second;
    }
    return NULL;
}

void HttpControler::_DelFromHttpReq( uint32_t dwReqID )
{
    if( 0 == dwReqID )
        return;

    HttpReqMap_t::iterator it = m_oHttpReqMap.find(dwReqID);
    if (it != m_oHttpReqMap.end())
    {
        m_oHttpReqMap.erase(it);
    }
}

bool HttpControler::_Send2DbThread(SSPKG& rstSsPkg)
{
    TdrError::ErrorType iRet = rstSsPkg.pack( m_stTdrBuf.m_szTdrBuf, m_stTdrBuf.m_uSize, &m_stTdrBuf.m_uPackLen );
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("pack ss pkg error! cmd <%u>", rstSsPkg.m_stHead.m_wMsgId );
        return false;  
    }

    return ClusterAccSvrMsgLayer::Instance().ForwardToWorkThread( &m_stTdrBuf );
}

void HttpControler::_SendGetOneAccInfoHttpRsp( uint32_t dwReqID, SS_PKG_GET_ONE_CLUSTER_ACC_INFO_RSP& rstGetOneAccInfoRsp )
{
    struct evhttp_request* req = this->_FindHttpReq(dwReqID);
    if( !req )
    {
        return;
    }

    TDRDATA stHost, stJson;
    char szJsonRsp[512];
    stJson.pszBuff = szJsonRsp;
    stJson.iBuff = sizeof(szJsonRsp);
    stHost.pszBuff = (char*)&rstGetOneAccInfoRsp;
    stHost.iBuff = sizeof(rstGetOneAccInfoRsp);
    LPTDRMETA pstMeta = tdr_get_meta_by_name(m_pstMetaLib, "SS_PKG_GET_ONE_CLUSTER_ACC_INFO_RSP");
    int iRet = tdr_output_json(pstMeta, &stJson, &stHost, 0);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        assert(false);
        LOGERR_r("bin to json failed. (%s)", tdr_error_string(iRet));
        snprintf(szJsonRsp, sizeof(szJsonRsp), "{\"ErrNo\":%d}", ERR_SYS);
    }

    this->_SendHttpRsp( req, szJsonRsp );        

    this->_DelFromHttpReq(dwReqID);
}


void HttpControler::_SendHttpRsp( struct evhttp_request *req, char* szRspStr )
{
    assert( req );
    evbuffer_add(m_oHttpServer.m_pEvRspData, szRspStr, strlen(szRspStr) );
    /*
        After calling evhttp_send_reply() databuf will be empty, but the buffer is still
        owned by the caller and needs to be deallocated by the caller if necessary.
    */
	evhttp_send_reply( req, HTTP_OK, "ok", m_oHttpServer.m_pEvRspData ); 
}

void HttpControler::SendHttpRsp( MyTdrBuf* pstTdrBuf )
{
    int iRet = m_stSsPkg.unpack( pstTdrBuf->m_szTdrBuf, pstTdrBuf->m_uPackLen );
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("unpack rsp pkg failed!");
        return;
    }

    if( SS_MSG_GET_ONE_CLUSTER_ACC_INFO_RSP == m_stSsPkg.m_stHead.m_wMsgId )
    {
        this->_SendGetOneAccInfoHttpRsp( (uint32_t)m_stSsPkg.m_stHead.m_ullReservId, m_stSsPkg.m_stBody.m_stGetOneClusterAccInfoRsp );
    }
}


