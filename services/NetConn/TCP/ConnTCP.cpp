#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include "ConnTCP.h"
#include "workdir.h"
#include "GameTime.h"
#include "oi_misc.h"
#include "comm_func.h"
#include "og_comm.h"
#include "strutil.h"
#include "../utils/FakeRandom.h"
#include "tea.h"
#include "cs_proto.h"

using namespace PKGMETA;

#define CONNTCP_EPOLL_WAIT_MS 5
#define CONNTCP_MAX_BUSMSG_HND_PER_LOOP 1024 // 每次loop最多处理的bus msg数量
#define CONNTCP_CLIENT_CHK_FREQ 500 // ms

CConnTCP::CConnTCP(NETCONNCFG* pstConfig) : m_stCsSendBuf(sizeof(CSPKG)*2), m_stScSendBuf(sizeof(SCPKG)*2)
{
    bzero(&m_stListen, sizeof(m_stListen));
    bzero(&m_tvConnTimePerSec, sizeof(m_tvConnTimePerSec) );
    bzero(&m_stProcStatsInfo,sizeof(m_stProcStatsInfo));
    m_stCsSendBuf.Reset();
	m_stScSendBuf.Reset();
    m_iConnNumPerSec = 0;
    m_ullLastUptTimeMs = 0;
    m_pstConfig = pstConfig;

}

bool CConnTCP::CreateClientPool()
{
    int iNew = m_oClientPool.CreatePool(m_pstConfig->m_stTcpConfig.m_iMaxFD);
	if (iNew < 0)
	{
		LOGERR("Init client pool failed! iRet=%d", iNew);
		return false;
	}
    m_oClientPool.RegisterSlicedIter(&m_oClientUptIter);

	return true;
}

bool CConnTCP::Create()
{
    int iRet = 0;
    // init Epool
    if (!m_oEpoll.Create(m_pstConfig->m_stTcpConfig.m_iMaxFD + 1))
    {
        LOGERR("Init Epoll failed, errinfo=(%s)", m_oEpoll.GetErrMsg());
        return false;
    }

    // init tbus layer
    iRet = m_oBusLayer.Init(m_pstConfig->m_iBusGCIMKey, m_pstConfig->m_iProcID);
    if (iRet < 0)
    {
        LOGERR("Init BusLayer failed, Ret=(%d) %s ", iRet, m_oBusLayer.GetErrMsg());
        return false;
    }

    // init blockPool
    if (m_oBuffBlockPool.CreatePool(m_pstConfig->m_stTcpConfig.m_iMaxFD*2) < 0)
    {
        return false;
    }

    if (!this->CreateClientPool())
    {
        return false;
    }

    // init msg broadcaster
    if (!m_oMsgBroadcaster.Init(this))
    {
        return false;
    }

    int iSock = CreateListenSock(m_pstConfig->m_szListenIP, m_pstConfig->m_stTcpConfig.m_wTcpListenPort, m_pstConfig->m_stTcpConfig.m_wTcpBackLog);
    if (iSock < 0)
    {
        LOGERR("Create listen sock faild! ip=[%s], port=[%u]", m_pstConfig->m_szListenIP, m_pstConfig->m_stTcpConfig.m_wTcpListenPort);
        return false;
    }
    m_stListen.m_iSock = iSock;

    if (!m_oEpoll.EpollAdd(m_stListen.m_iSock, &m_stListen))
    {
        LOGERR("epoll add listen sock failed!");
        tnet_close(m_stListen.m_iSock);
        return false;
    }

    return true;
}

void CConnTCP::InitClientParam(SClient* pstClient, HANDLE iCltHnd, struct timeval* pstCurTime)
{
    pstClient->m_iHnd = iCltHnd;
    pstClient->m_szAccountName[0] = '\0';
    pstClient->m_dwSessionId = (++m_stListen.m_dwSessionId) ? m_stListen.m_dwSessionId : (++m_stListen.m_dwSessionId);
    pstClient->m_dwCltVer = 0;
    pstClient->m_lLoginTime = pstCurTime->tv_sec;
    pstClient->m_lActiveTime = pstCurTime->tv_sec;
    pstClient->m_iState = CLIENT_STATE_NULL;
    pstClient->m_bExitByLogicSvr = false;
    pstClient->m_iRecvLen = 0;
    pstClient->m_iCltPkgLen = 0;
    pstClient->m_lRecvStatsTime = pstCurTime->tv_sec;
    pstClient->m_wRecvStatsPkgs = 0;
    pstClient->m_stBuffBlockQ.Reset();
    pstClient->m_StatsInfo.m_ulRecvBytes = 0;
    pstClient->m_StatsInfo.m_ulRecvPkgs  = 0;
    pstClient->m_StatsInfo.m_ulSendBytes = 0;
    pstClient->m_StatsInfo.m_ulSendPkgs  = 0;
}

bool CConnTCP::AcceptConn()
{
    int iSock = 0;
    struct sockaddr_in stSockAddr = { 0 };
    socklen_t iSockLen = sizeof(stSockAddr);
    SClient* pstClient = NULL;
    HANDLE iCltHnd = 0;
    struct timeval* pstCurTime = CGameTime::Instance().GetCurrTime();

    if (MsPass( pstCurTime, &m_tvConnTimePerSec ) >= 1000) // 每秒限制检测时间刷新 1000ms
    {
        m_tvConnTimePerSec = *pstCurTime;
        m_iConnNumPerSec = 0;
    }

    while (1)
    {
        iSock = TcpAccept( m_stListen.m_iSock, (struct sockaddr*)&stSockAddr, &iSockLen );
        if (iSock < 0)
        {
            LOGERR( "tcp accept error: %s", strerror(errno) );
            return false;
        }

        if (0 == iSock)
        {
            break;
        }

        m_stProcStatsInfo.m_ulCltConnNum++;
        if (m_pstConfig->m_iConnLimitPerSec > 0 && m_iConnNumPerSec > m_pstConfig->m_iConnLimitPerSec)
        {
            // 超过每秒限制
            LOGWARN("Connection limited for new one! LoginSpdPerSec=%d", m_pstConfig->m_iConnLimitPerSec );
            close( iSock );
            break;
        }
        m_iConnNumPerSec++;

        pstClient = m_oClientPool.NewData(&iCltHnd);
        if (!pstClient)
        {
            LOGWARN( "No free client for new connection! MaxNum[%d], FreeNum[%d]", m_oClientPool.GetMaxNum(), m_oClientPool.GetFreeNum() );
            close(iSock);
            return false;
        }

        char *pstIP = inet_ntoa(stSockAddr.sin_addr);
        if (!pstIP)
        {
            LOGERR("Get IP str failed");
            pstClient->m_szIPStr[0] = '\0';
        }
        else
        {
            STRNCPY(pstClient->m_szIPStr, pstIP, 32);
        }

        // 初始化client TCP连接数据
        InitClientParam(pstClient, iCltHnd, pstCurTime);
        pstClient->m_dwIP = tdr_ntoh32( stSockAddr.sin_addr.s_addr );
        pstClient->m_wPort = tdr_ntoh16( stSockAddr.sin_port );
        pstClient->m_iSock = iSock;
        tnet_set_nonblock(iSock, 1);

        if (!m_oEpoll.EpollAdd(iSock, pstClient))
        {
            LOGERR("EpollAdd failed for client [%s:%d]", inet_ntoa(stSockAddr.sin_addr), pstClient->m_wPort);
            close(iSock);
            m_oClientPool.DeleteData(iCltHnd);
        }

        LOGRUN("Client [%s:%u] handle[%d] connect in. ", INET_HTOA(pstClient->m_dwIP), pstClient->m_wPort, pstClient->m_iHnd);
    }

    return 0;
}

int CConnTCP::HandleNetIO()
{
    int iNfds = 0;
    SClient* pstClient = NULL;

    iNfds = m_oEpoll.Wait(CONNTCP_EPOLL_WAIT_MS);
    if (iNfds <= 0)
    {
        return 0;
    }

    for (int i = 0; i < iNfds; i++)
    {
        pstClient = (SClient*)m_oEpoll.GetEpollDataPtr(i);
        assert(pstClient);

        if (pstClient->m_iSock == m_stListen.m_iSock)
        {
            this->AcceptConn();
            continue;
        }

        if (m_oEpoll.IsEvReadable(i))
        {
            if (this->OnReadable(pstClient) < 0)
            {
                continue;
            }
        }

        if (m_oEpoll.IsEvWritable(i))
        {
            this->OnWritable(pstClient);
        }
    }

    return iNfds;
}

int CConnTCP::HandleBusMsg()
{
    int iMsgCnt = 0;
    for (; iMsgCnt < CONNTCP_MAX_BUSMSG_HND_PER_LOOP; iMsgCnt++)
    {
        if ((m_oBusLayer.Recv() <= 0))
        {
            break;
        }

        m_stProcStatsInfo.m_ulRecvBusPkgs++;
        m_stProcStatsInfo.m_ulRecvBusBytes += m_oBusLayer.GetRecvMsgLen();

        if (m_pstConfig->m_stConnSvrInfo.m_iLogicSvrID == m_oBusLayer.GetRecvMsgSrc())
        {
            this->DealLogicSvrPkg();
        }
    }

    return iMsgCnt;
}

/*
	将处理周期平摊给所有client，每个client每次epoll周期最多只能读入一个pkg
		接收并处理client msg, 返回 0: succ, <0: error
*/
int CConnTCP::OnReadable(SClient* pstClient)
{
    assert( pstClient );
    int iRet = 0;

    if (pstClient->m_iRecvLen < (int)sizeof(CSPKGHEAD))
    {
        //接收消息头
        iRet = TcpRecv(pstClient->m_iSock,
                       pstClient->m_szCltPkg + pstClient->m_iRecvLen,
                       sizeof( CSPKGHEAD ) - pstClient->m_iRecvLen);

        if( iRet < 0 )
        {
            if( -3 == iRet )
            {
                // 主动断开连接
                LOGRUN("Client close connect actively. Player[%s] Hnd[%d] IP[%s:%u] SessionID[%u]",
                    pstClient->m_szAccountName, pstClient->m_iHnd, INET_HTOA(pstClient->m_dwIP), pstClient->m_wPort, pstClient->m_dwSessionId);

                this->CloseClient( pstClient, CONNSESSION_REASON_PEER_CLOSE );
            }
            else
            {
                LOGERR( "Recv pkg failed. Player[%s] Hnd[%d] IP[%s] SessionID[%u] Err[%d : %s] iRet=%d",
				    pstClient->m_szAccountName, pstClient->m_iHnd, INET_HTOA(pstClient->m_dwIP), pstClient->m_dwSessionId,
				    errno, strerror(errno), iRet );

                this->CloseClient( pstClient, CONNSESSION_REASON_NETWORK_FAIL );
            }

            return -1;
        }

        pstClient->m_iRecvLen += iRet;
        pstClient->m_StatsInfo.m_ulRecvBytes += iRet;

        m_stProcStatsInfo.m_ulRecvCltBytes += iRet;

        if (pstClient->m_iRecvLen < (int)sizeof(CSPKGHEAD))
        {
            pstClient->m_lActiveTime = CGameTime::Instance().GetCurrSecond();
            return 0;
        }

        //client pkg head 读完
        pstClient->m_iCltPkgLen = CSPKG_LEN( (pstClient->m_szCltPkg) );

        if (pstClient->m_iCltPkgLen < (int)sizeof(CSPKGHEAD) ||
            pstClient->m_iCltPkgLen > (int)sizeof(CSPKG) )
        {
            LOGERR("Invalid client pkg. Player[%s] Hnd[%d] IP[%s] SessionID[%u] TotalRecvBytes[%lu]",
        					pstClient->m_szAccountName, pstClient->m_iHnd, INET_HTOA(pstClient->m_dwIP), pstClient->m_dwSessionId,
        					pstClient->m_StatsInfo.m_ulRecvBytes );
            this->CloseClient(pstClient, CONNSESSION_REASON_BAD_PKGLEN);
            return -1;
        }
    }

    iRet = TcpRecv(pstClient->m_iSock,
                    pstClient->m_szCltPkg + pstClient->m_iRecvLen,
                    pstClient->m_iCltPkgLen - pstClient->m_iRecvLen);
    if( iRet < 0 )
    {
        if( -3 == iRet )
        {
            // 主动断开连接
            LOGRUN("Client close connect actively. Player[%s] Hnd[%d] IP[%s] SessionID[%u]",
                pstClient->m_szAccountName, pstClient->m_iHnd, INET_HTOA(pstClient->m_dwIP), pstClient->m_dwSessionId );
            this->CloseClient( pstClient, CONNSESSION_REASON_PEER_CLOSE );
        }
        else
        {
            LOGERR( "Recv pkg failed. Err[%d : %s]. Player[%s] Hnd[%d] IP[%s] SessionID[%u] ",
                errno, strerror(errno),
                pstClient->m_szAccountName, pstClient->m_iHnd, INET_HTOA(pstClient->m_dwIP), pstClient->m_dwSessionId );
            this->CloseClient( pstClient, CONNSESSION_REASON_NETWORK_FAIL );
        }

        return -1;
    }

    pstClient->m_iRecvLen += iRet;
    pstClient->m_StatsInfo.m_ulRecvBytes += iRet;
    m_stProcStatsInfo.m_ulRecvCltBytes += iRet;

    if( pstClient->m_iRecvLen < pstClient->m_iCltPkgLen )
    {
        return 0;
    }

    // 收完一个包
    pstClient->m_iRecvLen = 0;

    pstClient->m_StatsInfo.m_ulRecvPkgs++;
    m_stProcStatsInfo.m_ulRecvCltPkgs++;

    // 检查上行包率
    if( !(this->CheckCltPkgFreq( pstClient ) ) )
    {
        this->CloseClient( pstClient, CONNSESSION_REASON_EXCEED_LIMIT );
        return -1;
    }

    // 退出状态，drop掉上行包
    if( IS_CLIENT_IN_STATE( pstClient, CLIENT_STATE_LOGOUT ) )
    {
        return 0;
    }

    // 处理已经接收完整的client pkg
    return this->HandleCltPkg( pstClient );
}

/*
	socket可写, 发送缓存的数据
	返回:
	< 0: error
	0: 没发送完(剩余数据缓存起来后续再写)
	1: 发完整个包
*/
int CConnTCP::OnWritable(SClient* pstClient)
{
    assert( pstClient );
    int iRet = 0;
    SBuffBlockQ* pstBufBlkQ = NULL;
    SBuffBlock* pstBufBlk = NULL;

    pstBufBlkQ = &(pstClient->m_stBuffBlockQ);

    TLISTNODE* pstPos = NULL;
    TLISTNODE* pstNext = NULL;
    TLISTNODE* pstHead = &(pstBufBlkQ->m_stBufBlkHead);

    TLIST_FOR_EACH_SAFE(pstPos, pstNext, pstHead)
    {
        pstBufBlk = TLIST_ENTRY(pstPos, SBuffBlock, m_stNode);

        iRet = TcpSend(pstClient->m_iSock,
                        pstBufBlk->m_szBuff + pstBufBlk->m_iHead,
                        pstBufBlk->m_iTail - pstBufBlk->m_iHead);
        if (iRet < 0)
        {
            LOGERR("Send msg to Player[%s] IP[%s] Err[%d : %s]",
			    pstClient->m_szAccountName, INET_HTOA(pstClient->m_dwIP), errno, strerror(errno));
            this->CloseClient(pstClient, CONNSESSION_REASON_NETWORK_FAIL);
            return -1;
        }

        pstBufBlk->m_iHead += iRet;

        pstClient->m_StatsInfo.m_ulSendBytes += iRet;
        m_stProcStatsInfo.m_ulSendCltBytes += iRet;

        if( pstBufBlk->m_iHead == pstBufBlk->m_iTail )
        {
            // 该缓存块发送完毕, 回收资源
            TLIST_DEL( &(pstBufBlk->m_stNode) );
            m_oBuffBlockPool.DeleteData( pstBufBlk );
            pstBufBlkQ->m_iNodeNum--;

            m_stProcStatsInfo.m_ulSendCltPkgs++;
            continue;
        }

        return 0;
    }

    // 缓存的数据全部下发完毕
    assert( 0 == pstBufBlkQ->m_iNodeNum );
    if( pstBufBlkQ->m_bCloseAftSnd )
    {
        this->CloseClient( pstClient, CONNSESSION_REASON_NONE );
    }
    else
    {
        m_oEpoll.EpollMod( pstClient->m_iSock, pstClient, EPOLLIN ); // 去掉EPOLLOUT
    }

    pstBufBlkQ->Reset();
    return 1;
}

// 处理一个已经接收完整的client pkg, 暂不加密
int CConnTCP::HandleCltPkg(SClient* pstClient)
{
    assert( pstClient );
    uint16_t wMsgID = CSPKG_MSGID(pstClient->m_szCltPkg);

	// 连接心跳报文
    if (CS_MSG_HEARTBEAT == wMsgID)
    {
        pstClient->m_lActiveTime = CGameTime::Instance().GetCurrSecond();
        return 0;
    }

    // 登录消息
    if (IS_CLIENT_IN_STATE(pstClient, CLIENT_STATE_NULL))
    {
        if (CS_MSG_ACCOUNT_LOGIN_REQ == wMsgID)
        {
            // 登陆
            return this->OnAccountLogin(pstClient);
        }
        else if (CS_MSG_RECONNECT_REQ == wMsgID)
        {
            //重连
            return this->OnReconnect(pstClient);
        }
        else
        {
            // 来自client的第一个包必须是AccLogin包或Reconnect包
            LOGERR("Fisrt pkg recv from client must login pkg! IP[%s]", INET_HTOA(pstClient->m_dwIP));
            this->CloseClient(pstClient, 0, false);
            return -1;
        }
    }

    if (CS_MSG_ACCOUNT_LOGIN_REQ == wMsgID || CS_MSG_RECONNECT_REQ == wMsgID)
    {
        // 非初始状态下发的登录包不处理
        LOGERR("Client[%s] IP[%s] state [%d], recv Login Msg. Drop",
                pstClient->m_szAccountName, INET_HTOA(pstClient->m_dwIP), pstClient->m_iState );
        return 0;
    }

    // 处在login状态, 不发给LogicSvr处理
    if (IS_CLIENT_IN_STATE( pstClient, CLIENT_STATE_LOGIN ))
    {
        return 0;
    }

    //需要先解密数据包
	if (1 == m_pstConfig->m_stTcpConfig.m_iIsEncrypt && pstClient->m_szAesKey[0] != '\0')
	{
        int iRet = Decrypt(pstClient);
        if (iRet < 0)
        {
            LOGERR("Client[%s] IP[%s] ,decrypt pkg failed", pstClient->m_szAccountName, INET_HTOA(pstClient->m_dwIP));
            return iRet;
        }
	}

    if (!this->SendToLogicSvr(pstClient, pstClient->m_szCltPkg, CONNSESSION_CMD_INPROC))
    {
        LOGERR("Send msg to LogicServer failed!. Cmd[%u] Player[%s] IP[%s]",
                wMsgID, pstClient->m_szAccountName, INET_HTOA(pstClient->m_dwIP) );
        return -1;
    }

    pstClient->m_lActiveTime = CGameTime::Instance().GetCurrSecond();

    return 0;
}


int CConnTCP::Decrypt(SClient* pstClient)
{
    //TEA解密
    int iHeadLen = CSPKG_HEAD_LEN;
    int iBodyLen = CSPKG_BODY_LEN((pstClient->m_szCltPkg));

    if (!TEAHelper::Instance().Decrypt((unsigned char*)(pstClient->m_szCltPkg+iHeadLen), iBodyLen,\
      (unsigned char*)(pstClient->m_szCltPkg+iHeadLen), iBodyLen, (unsigned char*)pstClient->m_szAesKey, MAX_AES_KEY_LEN - 1))
    {
        //解密出错，直接关闭连接
        this->CloseClient(pstClient, CONNSESSION_REASON_BAD_PKGLEN, false);
    }
    return 0;
}


// LogicServer connect 主动断开网络连接
void CConnTCP::CloseClient( SClient* pstClient, char chStopReason, bool bNtfLogicSvr )
{
    assert(pstClient);

    if (bNtfLogicSvr)
    {
        this->ConnStopNotifyLogicSvr( pstClient, chStopReason );
    }

    LOGRUN("Close Client[%s] Hnd[%d] IP[%s:%u] SessionID[%u] StopReason[%d]!",
        pstClient->m_szAccountName, pstClient->m_iHnd, INET_HTOA(pstClient->m_dwIP), pstClient->m_wPort, pstClient->m_dwSessionId, chStopReason);

    SBuffBlockQ* pstBufBlkQ = &(pstClient->m_stBuffBlockQ);
    SBuffBlock*  pstBufBlk  = NULL;

    TLISTNODE* pstPos = NULL;
    TLISTNODE* pstNext = NULL;
    TLISTNODE* pstHead =  &(pstBufBlkQ->m_stBufBlkHead);
    TLIST_FOR_EACH_SAFE( pstPos, pstNext, pstHead )
    {
        pstBufBlk = TLIST_ENTRY( pstPos, SBuffBlock, m_stNode );
        TLIST_DEL( pstPos );
        m_oBuffBlockPool.DeleteData( pstBufBlk );
        pstBufBlkQ->m_iNodeNum--;
    }

    // 关闭socket时,会自动从epoll监听中退出
    close( pstClient->m_iSock );

    // 回收SClientTCP
    assert( 0 == pstBufBlkQ->m_iNodeNum );
    bzero(pstClient, sizeof(SClient));
    m_oClientPool.DeleteData( pstClient );
}

int CConnTCP::SendToLogicSvr(SClient* pstClient, char* pszPkg, char chSessCmd)
{
    assert( pstClient );

    CONNSESSION stConnSession;

    bzero(&stConnSession, sizeof(stConnSession));
    stConnSession.m_chConnType = PROTO_TYPE_TCP;
    stConnSession.m_chCmd = chSessCmd;
    stConnSession.m_dwSessionId = pstClient->m_dwSessionId;
    stConnSession.m_iConnHnd = pstClient->m_iHnd;

    switch( stConnSession.m_chCmd )
    {
        case CONNSESSION_CMD_START:
        {
            stConnSession.m_stCmdData.m_stConnSessStart.m_dwIPAddr = pstClient->m_dwIP;
            stConnSession.m_stCmdData.m_stConnSessStart.m_wPort = pstClient->m_wPort;
            break;
        }
        case CONNSESSION_CMD_STOP:
        {
            break;
        }
        case CONNSESSION_CMD_INPROC:
        {
            break;
        }
        case CONNSESSION_CMD_RECONN:
        {
            break;
        }

        default:
            return -1;
    }

    return this->SendToLogicSvr(stConnSession, pszPkg);
}

int CConnTCP::SendToLogicSvr(CONNSESSION& rstSession, char* pszPkg)
{
    m_stCsSendBuf.Reset();
    TdrError::ErrorType iRet = rstSession.pack( m_stCsSendBuf.m_szTdrBuf, m_stCsSendBuf.m_uSize, &(m_stCsSendBuf.m_uPackLen) );
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("pack CONNSESSION error! cmd <%d>\n", rstSession.m_chCmd);
        return -1;
    }

    if (pszPkg)
    {
        if (m_stCsSendBuf.m_uPackLen + CSPKG_LEN(pszPkg) > m_stCsSendBuf.m_uSize)
        {
            assert(false);
            return false;
        }

        memcpy( m_stCsSendBuf.m_szTdrBuf+m_stCsSendBuf.m_uPackLen, pszPkg, CSPKG_LEN(pszPkg) );
        m_stCsSendBuf.m_uPackLen += CSPKG_LEN(pszPkg);
    }

    return m_oBusLayer.Send(m_pstConfig->m_stConnSvrInfo.m_iLogicSvrID, m_stCsSendBuf);
}

// 发送连线断开消息到LogicServer
void CConnTCP::ConnStopNotifyLogicSvr(SClient* pstClient, char chStopReason)
{
    assert(pstClient);

    if (CLIENT_STATE_NULL == pstClient->m_iState ||
        pstClient->m_bExitByLogicSvr)
    {
        return;
    }

    TRACE("Send SESSION_STOP to LogicSvr with AccountName[%s], Hnd[%d]", pstClient->m_szAccountName, pstClient->m_iHnd );

    CONNSESSION stSession;
    stSession.m_chCmd = CONNSESSION_CMD_STOP;
    stSession.m_dwSessionId = pstClient->m_dwSessionId;
    stSession.m_iConnHnd = pstClient->m_iHnd;
    stSession.m_stCmdData.m_stConnSessStop.m_chStopReason = chStopReason;

    this->SendToLogicSvr(stSession, NULL);
}

// 检查client发包频率,过快有可能使用了外挂，直接踢掉
bool CConnTCP::CheckCltPkgFreq(SClient* pstClient)
{
    assert( pstClient );

    time_t lCurrTime = CGameTime::Instance().GetCurrSecond();
    pstClient->m_wRecvStatsPkgs++;

    if( lCurrTime - pstClient->m_lRecvStatsTime < 1 )
    {
        return true;
    }

    int iPkgPerSec = (int)pstClient->m_wRecvStatsPkgs / (int)(lCurrTime - pstClient->m_lRecvStatsTime);
    // 每秒Client包太多?
    if( iPkgPerSec >= m_pstConfig->m_iRecvPkgLimitPerSec )
    {
        LOGERR("Too much pkg in 1 second. Avg[%d] MaxAllowed[%d]. Player[%s] Hnd[%d] IP[%s] SessionID[%u]",
            iPkgPerSec,
            m_pstConfig->m_iRecvPkgLimitPerSec,
            pstClient->m_szAccountName, pstClient->m_iHnd, INET_HTOA(pstClient->m_dwIP), pstClient->m_dwSessionId );

        return false;
    }

    pstClient->m_wRecvStatsPkgs = 0;
    pstClient->m_lRecvStatsTime = lCurrTime;

    return true;
}

void CConnTCP::DealLogicSvrPkg()
{
    MyTdrBuf* pstTdrBuf = m_oBusLayer.GetRecvTdrBuf();
    char* pRecvPkg = pstTdrBuf->m_szTdrBuf;
    size_t uRecvTotalLen = (int)pstTdrBuf->m_uPackLen;
    size_t uSessLen = 0;

    // 获取LogicServer回传的session信息
    CONNSESSION stSession;
    TdrError::ErrorType iRet = stSession.unpack( pRecvPkg, uRecvTotalLen, &uSessLen );
    if( iRet != TdrError::TDR_NO_ERROR)
    {
       LOGERR("Unpack Session pkg failed!");
       return;
    }

    if (PROTO_TYPE_TCP == stSession.m_chConnType)
    {
        DealLogicSvrPkg(stSession, uSessLen, pstTdrBuf);
    }

    return;
}

void CConnTCP::DealLogicSvrPkg(CONNSESSION& rstSession, size_t uSessLen, MyTdrBuf* pstTdrBuf)
{
    // 有可能只有session (SESSION_STOP)，没有pkg, 此时 iPkgLen==0
    char* pRecvPkg = pstTdrBuf->m_szTdrBuf;
    size_t uRecvTotalLen = (int)pstTdrBuf->m_uPackLen;
    int iPkgLen = uRecvTotalLen - uSessLen;
    char* pszScPkg = pRecvPkg + uSessLen;

    if (rstSession.m_chCmd == CONNSESSION_CMD_BROADCAST)
    {
        if( iPkgLen > 0 )
        {
            // 广播
            m_oMsgBroadcaster.AddMsg( pszScPkg, iPkgLen );
        }

        return;
    }

    SClient* pstClient = m_oClientPool.GetDataByHnd(rstSession.m_iConnHnd);
    if( NULL == pstClient )
    {
        if (rstSession.m_chCmd != CONNSESSION_CMD_STOP)
        {
             rstSession.m_chCmd = CONNSESSION_CMD_STOP;
             rstSession.m_stCmdData.m_stConnSessStop.m_chStopReason = CONNSESSION_REASON_INVALID_CLIENT;
             this->SendToLogicSvr(rstSession, NULL);
        }

        // 如果LogicSvr发来的是CONNSESSION_CMD_STOP,不需要再发CONNSESSION_CMD_STOP给LogicServer
        return;
    }

    // client 已清空
    if (0 == pstClient->m_iHnd)
    {
        LOGRUN( "Client has been cleaned!" );
        return;
    }

    assert( pstClient->m_iSock > 0 );
    // LogicSvr消息对应的Client无效,Conn需对LogicSvr进行通知
    if (pstClient->m_dwSessionId != rstSession.m_dwSessionId)
    {
        //this->CloseClient(pstClient, CONNSESSION_REASON_INVALID_CLIENT);
        LOGERR("DealLogicSvrPkg iHnd[%d], session invaild, Client session[%u], LogicSvr session[%u]",
                rstSession.m_iConnHnd, pstClient->m_dwSessionId, rstSession.m_dwSessionId);
        return;
    }

    // 已是延迟退出状态, 不再处理多余的bus pkg
    if (IS_CLIENT_IN_STATE(pstClient, CLIENT_STATE_LOGOUT))
    {
        return;
    }

    // LogicServer通知对某Client主动断线,会将本次pkg发送给client再断开连接. 注意不一定有pkg
    bool bExitByLogicSvr = false;
    if (CONNSESSION_CMD_STOP == rstSession.m_chCmd)
    {
        bExitByLogicSvr = true;
        SET_CLIENT_IN_STATE( pstClient, CLIENT_STATE_LOGOUT);
        pstClient->m_bExitByLogicSvr = true;
        LOGRUN("Disconnect from LogicServer. Player[%s] IP[%s:%u]",pstClient->m_szAccountName, INET_HTOA(pstClient->m_dwIP), pstClient->m_wPort);
    }

    // iPkgLen检查
    if (iPkgLen < (int)sizeof(SCPKGHEAD) || iPkgLen > (int)sizeof(SCPKG))
    {
        if( !bExitByLogicSvr )
        {
            return;
        }

        SBuffBlockQ* pstBufBlkQ = &(pstClient->m_stBuffBlockQ);
        if( pstBufBlkQ->m_iNodeNum <= 0 )
        {
            // 若对应Client已没有包需要发送，立即断开连接
            this->CloseClient(pstClient, CONNSESSION_REASON_NONE, !(CONNSESSION_CMD_STOP == rstSession.m_chCmd));
        }
        else
        {
            // 当前缓存的最后一个包发送后，断开连接
            SET_CLIENT_IN_STATE(pstClient, CLIENT_STATE_LOGOUT);
            pstBufBlkQ->m_bCloseAftSnd = true;
        }

        return;
    }

    if (CONNSESSION_CMD_RELOGIN == rstSession.m_chCmd)
    {
        SET_CLIENT_IN_STATE(pstClient, CLIENT_STATE_NULL);
    }
    else
    {
        if (SCPKG_MSGID(pszScPkg) == SC_MSG_ACCOUNT_LOGIN_RSP ||
           SCPKG_MSGID(pszScPkg) == SC_MSG_RECONNECT_RSP)
        {
            SET_CLIENT_IN_STATE(pstClient, CLIENT_STATE_INGAME);
        }
    }

    this->SendToClient(pstClient, pszScPkg, bExitByLogicSvr);
}

/*
	发送一个完整的pkg给client
	返回:
	< 0: error
	0: 没发送完整个包(剩余数据缓存起来后续再写)
	1: 发完整个包
*/
int CConnTCP::SendToClient(SClient* pstClient, char* pszScPkg, bool bCloseAftSnd)
{
    assert( pstClient && pszScPkg );

    SBuffBlockQ* pstBufBlkQ = &(pstClient->m_stBuffBlockQ);
    if( pstBufBlkQ->m_bCloseAftSnd)
    {
        // Client等待断线, 来自LogicServer的下行包直接drop
        return 1;
    }
    pstBufBlkQ->m_bCloseAftSnd = bCloseAftSnd;

    m_stScSendBuf.Reset();
    int iPkgLen = SCPKG_LEN(pszScPkg);
	//需要加密
	if (1 == m_pstConfig->m_stTcpConfig.m_iIsEncrypt)
	{
		//登录或重连时,需要生产相应的加密key
		if (SCPKG_MSGID(pszScPkg) == SC_MSG_ACCOUNT_LOGIN_RSP ||
            SCPKG_MSGID(pszScPkg) == SC_MSG_RECONNECT_RSP)
		{
            iPkgLen = this->GenEncryptKey(pstClient, pszScPkg, m_stScSendBuf.m_szTdrBuf);
		}
		else if (pstClient->m_szAesKey[0] != '\0')
		{
            iPkgLen = this->EncryptPkg(pstClient, pszScPkg, m_stScSendBuf.m_szTdrBuf);
		}
        if (iPkgLen < 0)
        {
            LOGERR("EncryptPkg failed, Ret=%d", iPkgLen);
            return -1;
        }
	}
    //不需要加密
	else
	{
		memcpy(m_stScSendBuf.m_szTdrBuf, pszScPkg, iPkgLen);
	}
    m_stScSendBuf.m_uPackLen = (size_t)iPkgLen;


    // 对应的client-socket无缓存数据时, 尝试直接写socket发送数据
    int iSendSize = 0;
    if (0 == pstBufBlkQ->m_iNodeNum)
    {
        iSendSize = TcpSend(pstClient->m_iSock, m_stScSendBuf.m_szTdrBuf, iPkgLen);
        if (iSendSize < 0)
        {
            LOGERR( "Send msg[%u] to Player[%s] Hnd[%d] IP[%s] Err[%d : %s]", SCPKG_MSGID(pszScPkg),
                pstClient->m_szAccountName, pstClient->m_iHnd,INET_HTOA(pstClient->m_dwIP), errno, strerror(errno));
            this->CloseClient( pstClient, CONNSESSION_REASON_NETWORK_FAIL );
            return -1;
        }

        pstClient->m_StatsInfo.m_ulSendBytes += iSendSize;
        m_stProcStatsInfo.m_ulSendCltBytes += iSendSize;

        // 一个包发完
        if (iSendSize == (int)m_stScSendBuf.m_uPackLen)
        {
            m_stProcStatsInfo.m_ulSendCltPkgs++;
            if( pstBufBlkQ->m_bCloseAftSnd )
            {
                this->CloseClient( pstClient, CONNSESSION_REASON_NONE, false );
            }
            return 1;
        }
    }

    // 未写完的数据缓存起来
    int iRet = 0;
    if (iSendSize < (int)m_stScSendBuf.m_uPackLen)
    {
        iRet = this->CachePkg(pstClient, m_stScSendBuf.m_szTdrBuf + iSendSize, (int)m_stScSendBuf.m_uPackLen - iSendSize);
    }
    return iRet;
}


/*缓存未发送的数据*/
int CConnTCP::CachePkg(SClient* pstClient, char* pszScPkg, int iPkgLen)
{
    SBuffBlock* pstBufBlock = NULL;
    SBuffBlockQ* pstBufBlkQ = &(pstClient->m_stBuffBlockQ);
    if (pstBufBlkQ->m_iNodeNum != 0)
    {
        TLISTNODE* psTailNode = TLIST_PREV(&(pstBufBlkQ->m_stBufBlkHead));
        pstBufBlock = TLIST_ENTRY(psTailNode, SBuffBlock, m_stNode);
    }
    else
    {
        //socket增加可写事件通知 !!
        m_oEpoll.EpollMod(pstClient->m_iSock, pstClient, EPOLLIN|EPOLLOUT);
    }

    int iWriteLen = 0;
    do
    {
        if (!pstBufBlock ||(pstBufBlock->m_iTail == MAX_BUFF_BLOCK_SIZE))
        {
            //当前Client节点缓存的包过多
            if (pstBufBlkQ->m_iNodeNum >= PER_CLT_MAX_BUFF_BLOCK)
            {
                this->CloseClient( pstClient, CONNSESSION_REASON_WR_BLOCKED );
                return -1;
            }

            pstBufBlock = m_oBuffBlockPool.NewData();
            if( !pstBufBlock )
            {
                this->CloseClient(pstClient, CONNSESSION_REASON_WR_BLOCKED);
                return -1;
            }
            pstBufBlock->Reset();
            //插入队尾
            TLIST_INSERT_PREV(&(pstBufBlkQ->m_stBufBlkHead), &(pstBufBlock->m_stNode));
            pstBufBlkQ->m_iNodeNum++;
        }

        int iLen = iPkgLen > MAX_BUFF_BLOCK_SIZE-pstBufBlock->m_iTail ? MAX_BUFF_BLOCK_SIZE-pstBufBlock->m_iTail : iPkgLen;

        memcpy(pstBufBlock->m_szBuff+pstBufBlock->m_iTail, pszScPkg+iWriteLen, iLen);

        iPkgLen -= iLen;
        iWriteLen += iLen;
        pstBufBlock->m_iTail += iLen;

    }while(iPkgLen > 0);

    return 0;
}


/*数据包加密*/
int CConnTCP::EncryptPkg(SClient* pstClient, char* pszScPkg, char* pszBuff)
{
    int iHeadLen = SCPKG_HEAD_LEN;
    int iBodyLen = SCPKG_BODY_LEN(pszScPkg);
    int iPkgLen = iHeadLen + iBodyLen;

    SCPKGHEAD stPkgHead;
    size_t uUsedSize;
    int iRet = stPkgHead.unpack(pszScPkg, (size_t)iPkgLen, &uUsedSize);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("unpack ScPkg head failed, errno<%d>", iRet);
        return -1;
    }
    TEAHelper::Instance().Encrypt((unsigned char*)(pszScPkg + iHeadLen), iBodyLen, (unsigned char*)(pszBuff + iHeadLen),
                              iBodyLen, (unsigned char*)pstClient->m_szAesKey, MAX_AES_KEY_LEN - 1);

    stPkgHead.m_wBodyLen = (uint16_t)iBodyLen;
    iRet = stPkgHead.pack(pszBuff, (size_t)iHeadLen, &uUsedSize);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("pack ScPkg head failed, errno<%d>", iRet);
        return -1;
    }

    //加密后的新的包的长度
    iPkgLen = iHeadLen + iBodyLen;

    return iPkgLen;
}


/*生成加密的key*/
int CConnTCP::GenEncryptKey(SClient* pstClient, char* pszScPkg, char* pszBuff)
{
    //登录回包
    SCPKG stScPkg;
    size_t uUsedSize = 0;
    int iPkgLen = SCPKG_LEN(pszScPkg);
    int iRet = stScPkg.unpack(pszScPkg, (size_t)iPkgLen, &uUsedSize);

    if(iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("unpack ScPkg failed, errno<%d>", iRet);
        return -1;
    }

    //生成加密key
    TEAHelper::Instance().GenerKey(pstClient->m_szAesKey, MAX_AES_KEY_LEN);

    if (SCPKG_MSGID(pszScPkg) == SC_MSG_ACCOUNT_LOGIN_RSP)
    {
        StrCpy(stScPkg.m_stBody.m_stAccountLoginRsp.m_szAesKey, pstClient->m_szAesKey, MAX_AES_KEY_LEN);
        stScPkg.m_stBody.m_stAccountLoginRsp.m_bIsEncrypt = 1;
    }
    else if (SCPKG_MSGID(pszScPkg) == SC_MSG_RECONNECT_RSP)
    {
        StrCpy(stScPkg.m_stBody.m_stReconnectRsp.m_szAesKey, pstClient->m_szAesKey, MAX_AES_KEY_LEN);
		stScPkg.m_stBody.m_stReconnectRsp.m_bIsEncrypt = 1;
    }

    iRet = stScPkg.pack(pszBuff, sizeof(SCPKG), &uUsedSize);
    iPkgLen = uUsedSize;
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("pack ScPkg failed, errno<%d>", iRet);
        return -1;
    }

    return iPkgLen;
}


int CConnTCP::OnAccountLogin(SClient* pstClient)
{
    CSPKG stCsPkg;

    // 解登录包
    TdrError::ErrorType iRet = stCsPkg.unpack( pstClient->m_szCltPkg, pstClient->m_iCltPkgLen );
    if( iRet != TdrError::TDR_NO_ERROR)
    {
       LOGERR("Unpack AccLogin pkg failed!");
       this->CloseClient(pstClient, 0, false);
       return -1;
    }

    CS_PKG_ACCOUNT_LOGIN_REQ* pstAccLogin = &(stCsPkg.m_stBody.m_stAccountLoginReq);
    StrCpy( pstClient->m_szAccountName, pstAccLogin->m_szAccountName, sizeof(pstClient->m_szAccountName) );

    if( !this->SendToLogicSvr( pstClient, pstClient->m_szCltPkg, CONNSESSION_CMD_START ) )
    {
        LOGERR("Send account login msg to LogicServer failed!. Player[%s] IP[%s]",
            pstClient->m_szAccountName, INET_HTOA(pstClient->m_dwIP) );
        this->CloseClient( pstClient, 0, false );
        return -1;
    }

    SET_CLIENT_IN_STATE( pstClient, CLIENT_STATE_LOGIN );

    return 0;
}

int CConnTCP::OnReconnect(SClient* pstClient)
{
    CSPKG stCsPkg;

    // 解登录包
    TdrError::ErrorType iRet = stCsPkg.unpack(pstClient->m_szCltPkg, pstClient->m_iCltPkgLen);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
       LOGERR("Unpack Reconn pkg failed!");
       this->CloseClient(pstClient, 0, false);
       return -1;
    }

    if( !this->SendToLogicSvr(pstClient, pstClient->m_szCltPkg, CONNSESSION_CMD_START))
    {
        LOGERR("Send Reconn msg to LogicServer failed!. Player[%s] IP[%s]",
            pstClient->m_szAccountName, INET_HTOA(pstClient->m_dwIP) );
        this->CloseClient(pstClient, 0, false);
        return -1;
    }

    SET_CLIENT_IN_STATE( pstClient, CLIENT_STATE_LOGIN );

    return 0;
}

void CConnTCP::Update(bool bIdle)
{
    m_oMsgBroadcaster.Update(bIdle);

    if (m_oClientPool.GetUsedNum() == 0)
    {
        return;
    }

    uint64_t m_ullCurTimeMs = CGameTime::Instance().GetCurrTimeMs();
    if ((m_ullCurTimeMs -m_ullLastUptTimeMs) < CONNTCP_CLIENT_CHK_FREQ)
    {
        return;
    }

    int iCheckNum = bIdle ? 50 : 10;

    if (m_oClientUptIter.IsEnd())
    {
        m_oClientUptIter.Begin();
    }

    SClient* pstClient = NULL;
    for (int i = 0; i < iCheckNum && !m_oClientUptIter.IsEnd(); i++, m_oClientUptIter.Next())
    {
        pstClient = m_oClientUptIter.CurrItem();
        assert(pstClient);

        // heart beat check
        time_t lCurrSec = CGameTime::Instance().GetCurrSecond();
        if (m_pstConfig->m_stTcpConfig.m_wMaxIdle > 0)
        {
            if (lCurrSec - pstClient->m_lActiveTime >= m_pstConfig->m_stTcpConfig.m_wMaxIdle)
            {
                this->CloseClient(pstClient, CONNSESSION_REASON_IDLE_CLOSE);
                continue;
            }
        }
    }

    if (m_oClientUptIter.IsEnd())
    {
        m_ullLastUptTimeMs = m_ullCurTimeMs;
    }
}
