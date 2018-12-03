#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include "ConnUDP.h"
#include "workdir.h"
#include "GameTime.h"
#include "oi_misc.h"
#include "comm_func.h"
#include "og_comm.h"
#include "strutil.h"
#include "cs_proto.h"
#include "FakeRandom.h"
#include "../framework/GameObjectPool.h"
#include "UDPClientBuffIndex.h"
#include "GameTime.h"
#include <map>
#include <list>

using namespace PKGMETA;
using namespace std;

#define CONNUDP_EPOLL_WAIT_MS                   (5)
#define CONNUDP_MAX_BUSMSG_HND_PER_LOOP         (1024)  // 每次loop最多处理的bus msg数量
#define CONNUDP_CLIENT_CHK_FREQ                 (500)   // ms
#define MAX_SEND_PKG_NUM_PER_LOOP               (20)
#define CHG_SENDBUFF_RTO_DELTA                  (100)

CConnUDP::CConnUDP(NETCONNCFG* pstConfig) : m_stCsSendBuf(sizeof(PKGMETA::CSPKG)*2), m_stScAckBuf( ACK_BUF_SIZE ),m_stScCloseBuf(ClOSE_BUF_SIZE),
	m_stTdrBufListen(sizeof(PKGMETA::CSPKG))
{
	bzero(&m_stCliListen, sizeof(m_stCliListen));
	bzero(&m_tvConnTimePerSec, sizeof(m_tvConnTimePerSec) );
	bzero(&m_stProcStatsInfo,sizeof(m_stProcStatsInfo));
	m_stCsSendBuf.Reset();
	m_stScAckBuf.Reset();
	m_iConnNumPerSec = 0;
	m_pstConfig = pstConfig;

	m_oPortFreeList.clear();
}

CConnUDP::~CConnUDP()
{

}

void CConnUDP::InitUDPParam()
{
    strcpy(m_stListenParam.szIP, m_pstConfig->m_szListenIP);
	m_stListenParam.wPort = m_pstConfig->m_stUdpConfig.m_wUdpBindPort;
    m_stListenParam.iRecvBufSizeByte = m_pstConfig->m_stUdpConfig.m_dwUdpListenRecvBuffSize;
    m_stListenParam.iSendBufSizeByte = m_pstConfig->m_stUdpConfig.m_dwUdpListenSendBuffSize;

    strcpy(m_stClientParam.szIP, m_pstConfig->m_szListenIP);
    m_stClientParam.iRecvBufSizeByte = m_pstConfig->m_stUdpConfig.m_dwUdpClientRecvBuffSize;
    m_stClientParam.iSendBufSizeByte = m_pstConfig->m_stUdpConfig.m_dwUdpClientSendBuffSize;
}

bool CConnUDP::Create()
{
	LOGRUN("Time sec-%lu", (long)(CGameTime::Instance().GetCurrSecond()*1000 + CGameTime::Instance().GetCurrMsInSec()));

	if (!m_oEpoll.Create( m_pstConfig->m_stUdpConfig.m_iMaxFD + 1))
	{
		LOGERR("%s", m_oEpoll.GetErrMsg() );
		return false;
	}

	if (m_oBusLayer.Init( m_pstConfig->m_iBusGCIMKey, m_pstConfig->m_iProcID ) < 0)
	{
		return false;
	}

	if (m_oUDPBlockPool.Init(MAX_BUFF_BLOCK) < 0)
	{
		return false;
	}

    this->InitUDPParam();

	OnReleaseTimerCb_t fReleaseTimer = CConnUDP::ReleaseTimer;
	if (!m_oUDPTimerMgr.Init(MAX_BUFF_BLOCK, fReleaseTimer))
	{
		return false;
	}

	this->InitPortList();

	if (!this->CreateClientPool())
	{
		return false;
	}

	int iSock = m_stCliListen.m_oSocketUDP.Init(m_stListenParam);

	if( iSock < 0 )
	{
		LOGERR( "Create bind sock faild! ip=[%s], port=[%u]",
			m_pstConfig->m_szListenIP, m_pstConfig->m_stUdpConfig.m_wUdpBindPort );
		return false;
	}

	m_stCliListen.m_iSock = iSock;
	if( !m_oEpoll.EpollAdd( m_stCliListen.m_iSock, &m_stCliListen ) )
	{
		LOGERR("epoll add udp listen sock failed!");
		tnet_close(m_stCliListen.m_iSock);
		return false;
	}

	return true;
}

bool CConnUDP::CreateClientPool()
{
	int iNew = m_oClientPool.CreatePool( m_pstConfig->m_stUdpConfig.m_iMaxFD );
	if (iNew < 0)
	{
		LOGERR("Init client pool failed! iRet=%d", iNew);
		return false;
	}

    m_oClientPool.RegisterSlicedIter(&m_oClientPoolIter);

	return true;
}

void CConnUDP::LogStatsInfo(){}

void CConnUDP::InitPortList()
{
	for (uint16_t wPort = m_pstConfig->m_stUdpConfig.m_wUdpPortRangeBegin; wPort <= m_pstConfig->m_stUdpConfig.m_wUdpPortRangeEnd; wPort++)
	{
		m_oPortFreeList.push_back(wPort);
	}
}

void CConnUDP::InitClientParam(SClient* pstClient, HANDLE iCltHnd, struct timeval* pstCurTime)
{
	// 初始化连接数据，生成该连接TokenID
	pstClient->m_wTokenId = (uint16_t)CFakeRandom::Instance().Random();
	pstClient->m_iHnd = iCltHnd;
	pstClient->m_szAccountName[0] = '\0';
	pstClient->m_dwSessionId = (++m_stCliListen.m_dwSessionId) ?  m_stCliListen.m_dwSessionId : (++m_stCliListen.m_dwSessionId);
	pstClient->m_dwCurSendSeq = 1;    // 默认从1开始
	pstClient->m_dwCurRecvSeq = 1;    // 默认从1开始

	// 先使用客户请求时端口
	pstClient->m_dwCltVer = 0;
	pstClient->m_iState = CLIENT_STATE_NULL;
	pstClient->m_bExitByLogicSvr = false;
	pstClient->m_iRecvLen = 0;
	pstClient->m_iCltPkgLen = 0;
	pstClient->m_wRecvStatsPkgs = 0;
	pstClient->m_stBuffBlockQ.Reset();
	pstClient->m_StatsInfo.m_ulRecvBytes = 0;
	pstClient->m_StatsInfo.m_ulRecvPkgs  = 0;
	pstClient->m_StatsInfo.m_ulSendBytes = 0;
	pstClient->m_StatsInfo.m_ulSendPkgs  = 0;

	pstClient->m_lLoginTime = pstCurTime->tv_sec;
	pstClient->m_lActiveTime = pstCurTime->tv_sec;
	pstClient->m_lRecvStatsTime = pstCurTime->tv_sec;
	pstClient->m_bResendTimeOut = false;
    pstClient->m_wRTT= 0;
    pstClient->m_wRTO= m_pstConfig->m_stUdpConfig.m_iMinResendInterval;
    pstClient->m_wPeerCurRecvSeq = 0;
    pstClient->m_bRepeatSeqCount = 0;
}

bool CConnUDP::AcceptConn()
{
	SClient* pstClient = NULL;
	HANDLE iCltHnd = 0;
	struct timeval* pstCurTime = CGameTime::Instance().GetCurrTime();

	if (MsPass( pstCurTime, &m_tvConnTimePerSec ) >= 1000) // 每秒限制检测时间刷新 1000ms
	{
		m_tvConnTimePerSec = *pstCurTime;
		m_iConnNumPerSec = 0;
	}

	m_stProcStatsInfo.m_ulCltConnNum++;
	if( m_pstConfig->m_iConnLimitPerSec > 0 &&
		m_iConnNumPerSec > m_pstConfig->m_iConnLimitPerSec )
	{
		// 超过每秒限制
		LOGWARN("Connection limited for new one! LoginSpdPerSec=%d", m_pstConfig->m_iConnLimitPerSec );
		return false;
	}
	m_iConnNumPerSec++;

	int iRet = m_stCliListen.m_oSocketUDP.Recv(m_stCliListen.m_szCltPkg, sizeof(m_stCliListen.m_szCltPkg));
	if (iRet < 0)
	{
		LOGERR("udp listen pkg recv err. ret: %d", iRet);
		return true;
	}

	if (CSPKG_MSGID(m_stCliListen.m_szCltPkg) != CS_MSG_UDP_CONN_REQ)
	{
		return false;
	}     

     m_stCsPkg.unpack( m_stCliListen.m_szCltPkg, iRet );    

#if 0
     // debug
    {
        static bool bDebugFlag = true;
        if( bDebugFlag )
        {
            bDebugFlag = false;
            m_stCsPkg.unpack( m_stCliListen.m_szCltPkg, iRet );
            if( m_stCsPkg.m_stBody.m_stUDPConnReq.m_ullUin == 8236370493444)
            {
                return false;
            }
        }
    }
#endif

	pstClient = m_oClientPool.NewData(&iCltHnd);
	if (!pstClient)
	{
		LOGERR("No free client for new connection! MaxNum[%d], FreeNum[%d]", m_oClientPool.GetMaxNum(), m_oClientPool.GetFreeNum());
		return false;
	}    
	bzero(pstClient, sizeof(SClient));

     pstClient->m_ullUin = m_stCsPkg.m_stBody.m_stUDPConnReq.m_ullUin;

     // 创建新的连接，将地址:端口发送客户端
	if (!m_oPortFreeList.empty())
	{
        // 获取新的端口
        m_stClientParam.wPort = m_oPortFreeList.front();
		pstClient->m_oSocketUDP.Init(m_stClientParam);
		m_oPortFreeList.pop_front();
	}
	else
	{
		LOGERR("allow port failed");
		m_oClientPool.DeleteData(pstClient);
		return false;
	}

	pstClient->m_poUDPBuffIndex = (void*)GET_GAMEOBJECT( UDPClientBuffIndex, GAMEOBJ_UDPBUFFINDEX );
	if (!pstClient->m_poUDPBuffIndex)
	{
		LOGERR("pstClient->m_poUDPBuffIndex is null");
		m_oClientPool.DeleteData(pstClient);
		return false;
	}

	// 初始化客户端参数
	InitClientParam(pstClient, iCltHnd, pstCurTime);

	// 记录peer ip and port
	pstClient->m_dwIP = ntohl(m_stCliListen.m_oSocketUDP.m_stCliAddr_Recv.sin_addr.s_addr);
	pstClient->m_wPort = ntohs(m_stCliListen.m_oSocketUDP.m_stCliAddr_Recv.sin_port);

	UDPClientBuffIndex* poBuffIndex = (UDPClientBuffIndex*)pstClient->m_poUDPBuffIndex;
	iRet = this->NotifyClientConn(pstClient, ntohs(pstClient->m_oSocketUDP.m_stSvrAddr.sin_port));
	if (iRet < 0)
	{
		m_oClientPool.DeleteData(iCltHnd);
		RELEASE_GAMEOBJECT(poBuffIndex);
		return false;
	}

	if (!m_oEpoll.EpollAdd(pstClient->m_oSocketUDP.m_SockFd, pstClient))
	{
		LOGERR("EpollAdd failed for client");
		pstClient->m_oSocketUDP.Close();
		RELEASE_GAMEOBJECT(poBuffIndex);
		m_oClientPool.DeleteData( iCltHnd );
	}

	if (!m_oEpoll.EpollMod(pstClient->m_oSocketUDP.m_SockFd, pstClient, EPOLLIN))
	{
		LOGERR("EpollMod failed for client");
		pstClient->m_oSocketUDP.Close();
		RELEASE_GAMEOBJECT(poBuffIndex);
		m_oClientPool.DeleteData( iCltHnd );
	}

	// 设置iSock
	pstClient->m_iSock = pstClient->m_oSocketUDP.m_SockFd;

	return 0;
}

int CConnUDP::NotifyClientConn(SClient* pstClient, uint16_t wPort)
{
	assert( pstClient );

	bzero(&m_stScPKGNotifyConn.m_stBody.m_stUDPConnRsp, sizeof(m_stScPKGNotifyConn.m_stBody.m_stUDPConnRsp));

	m_stScPKGNotifyConn.m_stHead.m_wMsgId = SC_MSG_UDP_CONN_RSP;
	m_stScPKGNotifyConn.m_stHead.m_wTokenId = pstClient->m_wTokenId;
	m_stScPKGNotifyConn.m_stHead.m_bHeadLen = sizeof(m_stScPKGNotifyConn.m_stHead);
	m_stScPKGNotifyConn.m_stHead.m_wBodyLen = sizeof(m_stScPKGNotifyConn.m_stBody.m_stUDPConnRsp);
	m_stScPKGNotifyConn.m_stBody.m_stUDPConnRsp.m_wPort = wPort; // server端新分配的
	m_stTdrBufListen.Reset();
	int iRet = m_stScPKGNotifyConn.pack(m_stTdrBufListen.m_szTdrBuf, m_stTdrBufListen.m_uSize, &m_stTdrBufListen.m_uPackLen);
	if (iRet < 0)
	{
		LOGERR("notify client conn pack failed.");
		return -1;
	}

	// 注意要用listen sock回应答包
	iRet = m_stCliListen.m_oSocketUDP.Send(m_stTdrBufListen.m_szTdrBuf, m_stTdrBufListen.m_uPackLen, pstClient->m_dwIP, pstClient->m_wPort);
	if (iRet < 0)
	{
		return -2;
	}

	LOGRUN("NotifyClientConn send len-%u, ip-%u, port-%u", (uint32_t)m_stTdrBufListen.m_uPackLen, pstClient->m_dwIP, pstClient->m_wPort);
	return 0;
}


int CConnUDP::HandleNetIO()
{
	int iNfds = 0;
	SClient* pstClient = NULL;

	m_oUDPTimerMgr.Update();

	iNfds = m_oEpoll.Wait(CONNUDP_EPOLL_WAIT_MS);
	if (iNfds <= 0)
	{
		return 0;
	}

	for (int i = 0; i < iNfds; i++)
	{
		pstClient = (SClient*)m_oEpoll.GetEpollDataPtr(i);
		if (NULL == pstClient)
		{
			assert( false );
			continue;
		}

		if (pstClient->m_iSock == m_stCliListen.m_iSock)
		{
			this->AcceptConn();
			continue;
		}

		if (m_oEpoll.IsEvReadable(i))
		{
			if( this->OnReadable(pstClient) < 0 )
			{
				continue;
			}
		}
	}

	return iNfds;
}

int CConnUDP::HandleBusMsg()
{
	int iMsgCnt = 0;

	for( ; iMsgCnt < CONNUDP_MAX_BUSMSG_HND_PER_LOOP; iMsgCnt++ )
	{
		if( ( m_oBusLayer.Recv() <= 0 ) )
		{
			break;
		}

		m_stProcStatsInfo.m_ulRecvBusPkgs++;
		m_stProcStatsInfo.m_ulRecvBusBytes += m_oBusLayer.GetRecvMsgLen();

		if( m_pstConfig->m_stConnSvrInfo.m_iLogicSvrID == m_oBusLayer.GetRecvMsgSrc() )
		{
			this->DealLogicSvrPkg();
		}
	}

	return iMsgCnt;
}

int CConnUDP::OnReadable(SClient* pstClient)
{
	assert(pstClient);
	int iRet = 0;

	CSocketUDP& roSock = pstClient->m_oSocketUDP;
	iRet = roSock.Recv(pstClient->m_szCltPkg, sizeof(PKGMETA::CSPKG));
	if( iRet < 0 )
	{
		LOGERR("Recv pkg failed. Err[%d : %s]. Player[%s] Hnd[%d] IP[%s] SessionID[%u] ",
			errno, strerror(errno),
			pstClient->m_szAccountName, pstClient->m_iHnd,
			INET_HTOA(pstClient->m_dwIP), pstClient->m_dwSessionId);
		this->CloseClient(pstClient, CONNSESSION_REASON_NETWORK_FAIL);

		return -1;
	}

#if 0
    // debug
    {
        static bool bDebugFlag = true;
        if( bDebugFlag )
        {
            //bDebugFlag = false;
            if( pstClient->m_ullUin == 8236370493444)
            {
                return -1;
            }
        }
    }
#endif 

	// length校验
	if( iRet < (int)CSPKG_HEAD_LEN )
	{
		LOGERR("Recv pkg len <%d> is less than header length!", iRet);
		this->CloseClient(pstClient, CONNSESSION_REASON_NETWORK_FAIL);
		return -1;
	}

	if( iRet != (int)CSPKG_LEN(pstClient->m_szCltPkg) )
	{
		LOGERR("Recved pkg len<%d> is not equal to pkg's actual len<%d>", iRet, (int)CSPKG_LEN(pstClient->m_szCltPkg) );
		this->CloseClient(pstClient, CONNSESSION_REASON_NETWORK_FAIL);
		return -1;
	}

	// 确认TokenId
	if (pstClient->m_wTokenId != CSPKG_TOKENID(pstClient->m_szCltPkg))
	{
		LOGERR("tokenid is err, pass. tokenid-%u, msgid-%u", CSPKG_TOKENID(pstClient->m_szCltPkg), CSPKG_MSGID(pstClient->m_szCltPkg));
		return -2;
	}

	pstClient->m_iCltPkgLen = iRet;
	pstClient->m_StatsInfo.m_ulRecvBytes += iRet;
	m_stProcStatsInfo.m_ulRecvCltBytes += iRet;
	pstClient->m_StatsInfo.m_ulRecvPkgs++;
	m_stProcStatsInfo.m_ulRecvCltPkgs++;

	// 更新回包IP和端口
	uint32_t dwIp = ntohl(pstClient->m_oSocketUDP.m_stCliAddr_Recv.sin_addr.s_addr);
    uint16_t wPort = ntohs(pstClient->m_oSocketUDP.m_stCliAddr_Recv.sin_port);

    if (dwIp != pstClient->m_dwIP)
    {
        LOGRUN("client sid-%u ip change from %u to %u", pstClient->m_dwSessionId, pstClient->m_dwIP, dwIp);
        pstClient->m_dwIP = dwIp;
    }

    if (wPort != pstClient->m_wPort)
    {
        LOGRUN("client sid-%u port change from %u to %u", pstClient->m_dwSessionId, pstClient->m_wPort, wPort);
        pstClient->m_wPort = wPort;
    }

	// 检查上行包率
	if (!(this->CheckCltPkgFreq(pstClient)))
	{
		LOGERR("CONNSESSION_REASON_EXCEED_LIMIT");
		this->CloseClient(pstClient, CONNSESSION_REASON_EXCEED_LIMIT);
		return -1;
	}

	// 处理已经接收完整的client pkg
	return this->HandleCltPkg(pstClient);
}

int CConnUDP::HandleCltPkgForFilter(SClient* pstClient, uint16_t wMsgID)
{
	// 处在login状态, 不发给LogicSvr处理
	if ( IS_CLIENT_IN_STATE( pstClient, CLIENT_STATE_LOGIN ) )
	{
		LOGERR("login state");
		return -1;
	}

	if ( PKGMETA::CS_MSG_PEER_CLOSE == wMsgID )
	{
		LOGERR("close client");
		this->CloseClient( pstClient, CONNSESSION_REASON_PEER_CLOSE, true);
		return -1;
	}

	return HandleCltPkgForLogin(pstClient, wMsgID);
}

int CConnUDP::HandleCltPkgForLogin(SClient* pstClient, uint16_t wMsgID)
{
	// 登录消息
	if( CLIENT_STATE_NULL == pstClient->m_iState )
	{
		if( PKGMETA::CS_MSG_FIGHT_LOGIN_REQ == wMsgID )
		{
			LOGRUN("Fight Login");
			return this->OnAccountLogin(pstClient);
		}
		else
		{
			// 来自client的第一个包必须是FightLogin包或Reconnect包
			LOGERR("Fightlogin, client state[%d] error! IP[%s]", pstClient->m_iState, INET_HTOA(pstClient->m_dwIP) );
			this->CloseClient( pstClient, 0, false );
			return -1;
		}
	}

	return 0;
}

int CConnUDP::HandleCltPkg(SClient* pstClient)
{
	assert(pstClient);
	uint16_t wMsgID = CSPKG_MSGID(pstClient->m_szCltPkg);
	uint8_t bProtoType = CSPKG_PROTOTYPE(pstClient->m_szCltPkg);
    uint32_t dwSeqNum = CSPKG_SEQNUM(pstClient->m_szCltPkg);

	// 连接心跳报文
	pstClient->m_lActiveTime = CGameTime::Instance().GetCurrSecond();
	if ( PKGMETA::CS_MSG_HEARTBEAT == wMsgID )
	{
		return 0;
	}

	// 正常收包流程
	UDPClientBuffIndex* pClientBuffIndex = (UDPClientBuffIndex*)pstClient->m_poUDPBuffIndex;
	if (!pClientBuffIndex)
	{
		LOGERR("pClientBuffIndex is null");
		return -1;
	}

	if (bProtoType == PROTO_TYPE_UDP_N)
	{
		// 过滤处理的消息
		if (HandleCltPkgForFilter(pstClient, wMsgID) < 0)
		{
			return -1;
		}

		if (!this->SendToLogicSvr( pstClient, pstClient->m_szCltPkg,  PKGMETA::CONNSESSION_CMD_INPROC))
		{
			LOGERR("Send msg to LogicServer failed!. Cmd[%u] Player[%s] IP[%s]",
				wMsgID, pstClient->m_szAccountName, INET_HTOA(pstClient->m_dwIP) );

			return -1;
		}

		return 0;
	}

	// 收到ACK包，直接处理
	if ( PKGMETA::COM_MSG_UDP_ACK_NTF == wMsgID )
	{
		// 缓存中直接删除，并删除其持有的定时器
		TdrError::ErrorType iRet = m_stCsPkg.unpack( pstClient->m_szCltPkg, pstClient->m_iCltPkgLen );
		if( iRet != TdrError::TDR_NO_ERROR)
		{
			LOGERR("Unpack pkg failed!");
			return -1;
		}

        return this->_HandleAckPkg( pstClient, m_stCsPkg.m_stBody.m_stUdpAckNtf);
	}
	else
	{
        LOGRUN( "recv client sid-%u, seq-%u, msgid-%u, CurRecvSeq-%u, ackmapSize-%u",
            pstClient->m_dwSessionId, dwSeqNum, wMsgID, pstClient->m_dwCurRecvSeq, (int)pClientBuffIndex->m_oAckList.size());

        // 收到客户端发送的数据包，触发返回ACK逻辑 (pClientBuffIndex->m_ullTimestampMs > 0)
		if (pClientBuffIndex->m_ullTimestampMs == 0)
		{
			pClientBuffIndex->m_ullTimestampMs = CGameTime::Instance().GetCurrTimeMs();
		}

        // 检查当前最后处理的Seq，是否在缓存中，存在则发送给逻辑服务器处理
		if ( dwSeqNum < pstClient->m_dwCurRecvSeq)
		{
			// 重复收包，不处理
			LOGRUN("repeat pkg, pass.");
			return 0;
		}

        if (pstClient->m_dwCurRecvSeq == dwSeqNum)
		{
			//LOGRUN("handle pkg seq-%u", pstClient->m_dwCurRecvSeq);
			pstClient->m_dwCurRecvSeq++;

			// 过滤处理的消息
			if (HandleCltPkgForFilter(pstClient, wMsgID) < 0)
				return -1;

			if (!this->SendToLogicSvr( pstClient, pstClient->m_szCltPkg,  PKGMETA::CONNSESSION_CMD_INPROC))
			{
				LOGERR("Send msg to LogicServer failed!. Cmd[%u] Player[%s] IP[%s]",
					wMsgID, pstClient->m_szAccountName, INET_HTOA(pstClient->m_dwIP) );

				return -1;
			}

			while(true)
			{
				// 判断缓存中是否存在后续的包
				map<uint32_t, UDPBlock*>::iterator iterBlock = pClientBuffIndex->m_oRecvMap.find(pstClient->m_dwCurRecvSeq);
				if (iterBlock != pClientBuffIndex->m_oRecvMap.end())
				{
					LOGRUN("handle cached pkg seq-%u", pstClient->m_dwCurRecvSeq);
					pstClient->m_dwCurRecvSeq++;

                    int iRet = -1;
                    do
                    {
                        if (HandleCltPkgForFilter(pstClient, wMsgID) < 0)
                            break;

                        if (!iterBlock->second || !iterBlock->second->m_pszBuff)
    					{
    						LOGERR("iterBlock have null pointer.");
    						break;
    					}

                        if (!this->SendToLogicSvr(pstClient, iterBlock->second->m_pszBuff, PKGMETA::CONNSESSION_CMD_INPROC))
    					{
    						LOGERR("Send msg to LogicServer failed!. Cmd[%u] Player[%s] IP[%s]",
    							wMsgID, pstClient->m_szAccountName, INET_HTOA(pstClient->m_dwIP) );
    						break;
    					}

                        iRet = 0;
                    }while(0);

                    // 从处理队列中删除发送给逻辑服务器的包
                    m_oUDPBlockPool.ReleaseBlock(iterBlock->second);
					pClientBuffIndex->m_oRecvMap.erase(iterBlock);

                    if( iRet < 0 )
                    {
                        return iRet;
                    }
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			// 还未处理的包，暂时缓存起来 (Msg N+x 先于 Msg N 到来)
            LOGRUN("Pkg arrive in advance, cache it! SeqNum-%u, MsgID-%u, CurRecvSeq-%u",
                    dwSeqNum, wMsgID, pstClient->m_dwCurRecvSeq );

            // 乱序的包才加入到AckList中
    		pClientBuffIndex->m_oAckList.push_back( dwSeqNum );
    		if (pClientBuffIndex->m_oAckList.size() >= m_pstConfig->m_stUdpConfig.m_dwUdpAckRspNum)
    		{
    			this->SendAckToClient(pstClient);

                // 清除收包时间戳
                pClientBuffIndex->m_ullTimestampMs = 0;
    		}

			int iPkgLen = CSPKG_LEN(pstClient->m_szCltPkg);
			UDPBlock* pBlock = m_oUDPBlockPool.GetBlock(iPkgLen);
			if (!pBlock)
			{
				LOGERR("pBlock is null.");
				return -1;
			}

			// copy data
			memcpy(pBlock->m_pszBuff, pstClient->m_szCltPkg, iPkgLen);
            struct timeval tCurrentTime = *(CGameTime::Instance().GetCurrTime());
            pBlock->m_tBlockTimeStamp = tCurrentTime;
			pClientBuffIndex->m_oRecvMap.insert(std::pair<uint32_t, UDPBlock*>(CSPKG_SEQNUM(pstClient->m_szCltPkg), pBlock));
		}
	}
	return 0;
}


int CConnUDP::_HandleAckPkg( SClient* pstClient, PKGMETA::COM_PKG_UDP_ACK_NTF& rstAckPkg )
{
    UDPClientBuffIndex* pClientBuffIndex = (UDPClientBuffIndex*)pstClient->m_poUDPBuffIndex;

	// 缓存中直接删除，并删除其持有的定时器
    struct timeval tCurrentTime = *(CGameTime::Instance().GetCurrTime());
    uint16_t wOldRTO = pstClient->m_wRTO;

	for (int8_t i=0; i<rstAckPkg.m_chCount; i++)
	{
		map<uint32_t, UDPBlock*>::iterator iterBlock = pClientBuffIndex->m_oSendedMap.find(rstAckPkg.m_SeqNum[i]);
		if (iterBlock != pClientBuffIndex->m_oSendedMap.end())
		{
            uint16_t wRTT = MsPass(&tCurrentTime, &iterBlock->second->m_tBlockTimeStamp);
            RTOEstimator(iterBlock->second, wRTT);

            LOGRUN("recv ack, client sid-%u, pkg seq-%u, timer id-%d, wRTT-%d, avgRTT-%d, RTO-%d, PeerRecvedSeq-%u",
                        pstClient->m_dwSessionId, rstAckPkg.m_SeqNum[i], iterBlock->second->m_dwTimerId, wRTT, pstClient->m_wRTT, pstClient->m_wRTO, rstAckPkg.m_wCurRecvedSeq);

            m_oUDPTimerMgr.DelTimer(iterBlock->second->m_dwTimerId); // ->ReleaseTimer
            // 删除发送队列索引
            pClientBuffIndex->m_oSendedMap.erase(iterBlock);
		}
		else
		{
			LOGRUN("recv ack, client sid-%u, pkg seq-%u :not found, do nothing",
                pstClient->m_dwSessionId, rstAckPkg.m_SeqNum[i] );
		}
	}

    // 小于等于对端Recved Seq的pkg表示对端已收到, 从resend缓存中删除
    map<uint32_t, UDPBlock*>::iterator iter;
    LOGRUN("recv ack, client sid-%u, RecvedSeq-%d",pstClient->m_dwSessionId,rstAckPkg.m_wCurRecvedSeq);
    while( (iter = pClientBuffIndex->m_oSendedMap.begin()) != pClientBuffIndex->m_oSendedMap.end() )
    {
        if(iter->first <= rstAckPkg.m_wCurRecvedSeq)
        {
            uint16_t wRTT = MsPass(&tCurrentTime, &iter->second->m_tBlockTimeStamp);
            RTOEstimator(iter->second, wRTT);

            m_oUDPTimerMgr.DelTimer(iter->second->m_dwTimerId); // ->ReleaseTimer
            pClientBuffIndex->m_oSendedMap.erase(iter);

            LOGRUN("client sid-%u, pkg seq-%u already recved! peer RecvedSeq - %d, ResendQueue size - %ld, wRTT-%d, avgRTT-%d, RTO-%d",
                pstClient->m_dwSessionId, iter->first, rstAckPkg.m_wCurRecvedSeq, pClientBuffIndex->m_oSendedMap.size(),
                wRTT, pstClient->m_wRTT, pstClient->m_wRTO);
        }
        else
        {
            break;
        }
    }

    if (pstClient->m_wRTO < (wOldRTO-CHG_SENDBUFF_RTO_DELTA))
    {
        ChgSendBufferRTO(pClientBuffIndex, pstClient->m_wRTO);
    }

    if (m_pstConfig->m_stUdpConfig.m_iQuickResendSwitch && rstAckPkg.m_chCount > 0)
    {
        QuicklyResend(pstClient, rstAckPkg.m_wCurRecvedSeq);
    }

    return 0;
}

void CConnUDP::CloseClient( SClient* pstClient, char chStopReason, bool bNtfLogicSvr )
{
	assert(pstClient);

	m_oPortFreeList.push_back(pstClient->m_wPort);

	UDPClientBuffIndex* pClientBuffIndex = (UDPClientBuffIndex*)pstClient->m_poUDPBuffIndex;
	if (!pClientBuffIndex)
	{
		LOGERR("pClientBuffIndex is null");
		return;
	}

	if( bNtfLogicSvr )
	{
		this->ConnStopNotifyLogicSvr( pstClient, chStopReason );
	}

    //服务器主动关闭连接时通知Client
    if (chStopReason != CONNSESSION_REASON_PEER_CLOSE)
    {
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PEER_CLOSE;
        m_stScPkg.m_stBody.m_stPeerClose.m_chPad = chStopReason;
        TdrError::ErrorType iErrNo = m_stScPkg.pack(m_stScCloseBuf.m_szTdrBuf, m_stScCloseBuf.m_uSize, &(m_stScCloseBuf.m_uPackLen));
    	if (iErrNo != TdrError::TDR_NO_ERROR)
    	{
    		LOGERR("Pack close notify failed");
    	}
        int iRet = pstClient->m_oSocketUDP.Send(m_stScCloseBuf.m_szTdrBuf, m_stScCloseBuf.m_uPackLen, pstClient->m_dwIP, pstClient->m_wPort);
    	if (iRet < 0)
    	{
    		LOGERR("Close client[%s] sid-%u notify send failed", pstClient->m_szAccountName, pstClient->m_dwSessionId);
    	}
    }


	LOGERR( "Close Client[%s] Hnd[%d] IP[%s] SessionID[%u] StopReason[%d]!",
		pstClient->m_szAccountName, pstClient->m_iHnd, INET_HTOA(pstClient->m_dwIP), pstClient->m_dwSessionId, chStopReason );

	// 关闭socket时,会自动从epoll监听中退出
	//LOGRUN("close fd-%d", pstClient->m_oSocketUDP.m_SockFd);
	pstClient->m_oSocketUDP.Close();

    LOGRUN( "client sid-%u SendMapSize[%d]!", pstClient->m_dwSessionId, (int)pClientBuffIndex->m_oSendedMap.size());
	// 清空发送Timer，这时会将Timer关联的Block块放入m_oUDPBlockFreeList中，需要在关连接前主动清一次
	std::map<uint32_t, UDPBlock*>::iterator iterSendMap = pClientBuffIndex->m_oSendedMap.begin();
	for (; iterSendMap != pClientBuffIndex->m_oSendedMap.end(); iterSendMap++)
	{
        if (iterSendMap->second->m_pszBuff!=NULL)
        {
            LOGRUN("clear Sendmap client sid-%u, seq-%u, msgid-%u, timerId-%d, pBlock-%p",
                    pstClient->m_dwSessionId, SCPKG_SEQNUM(iterSendMap->second->m_pszBuff),
                    SCPKG_MSGID(iterSendMap->second->m_pszBuff), iterSendMap->second->m_dwTimerId, iterSendMap->second);
        }
		if (iterSendMap->second->m_dwTimerId > 0)
		{
			m_oUDPTimerMgr.DelTimer(iterSendMap->second->m_dwTimerId);
		}
	}
	pClientBuffIndex->m_oSendedMap.clear();

	// 清空接受缓存
	LOGRUN( "client sid-%u RecvMapSize[%d]!", pstClient->m_dwSessionId, (int)pClientBuffIndex->m_oRecvMap.size());
	map<uint32_t, UDPBlock*>::iterator iterRecvMap = pClientBuffIndex->m_oRecvMap.begin();
	for (; iterRecvMap != pClientBuffIndex->m_oRecvMap.end(); iterRecvMap++)
	{
        if (iterRecvMap->second->m_pszBuff!=NULL)
        {
            LOGRUN("clear Recvmap client sid-%u, seq-%u, msgid-%u, timerId-%d, pBlock-%p",
                    pstClient->m_dwSessionId, SCPKG_SEQNUM(iterRecvMap->second->m_pszBuff),
                    SCPKG_MSGID(iterRecvMap->second->m_pszBuff), iterRecvMap->second->m_dwTimerId, iterRecvMap->second);
        }
		m_oUDPBlockPool.ReleaseBlock(iterRecvMap->second);
	}
	pClientBuffIndex->m_oRecvMap.clear();

	// 清空ACK队列
	pClientBuffIndex->m_ullTimestampMs = 0;
	pClientBuffIndex->m_oAckList.clear();

	// 释放pool资源
	UDPClientBuffIndex* poBuffIndex = (UDPClientBuffIndex*)pstClient->m_poUDPBuffIndex;
	RELEASE_GAMEOBJECT(poBuffIndex);

	// 回收SClient
	bzero(pstClient, sizeof(SClient));
	m_oClientPool.DeleteData( pstClient );
}

int CConnUDP::SendToLogicSvr(SClient* pstClient, char* pszPkg, char chSessCmd)
{
	assert(pstClient);
	bzero(&m_stSessionToSvr, sizeof(m_stSessionToSvr));
	m_stSessionToSvr.m_chCmd = chSessCmd;
	m_stSessionToSvr.m_dwSessionId = pstClient->m_dwSessionId;
	m_stSessionToSvr.m_iConnHnd = pstClient->m_iHnd;

	switch( m_stSessionToSvr.m_chCmd )
	{
	case PKGMETA::CONNSESSION_CMD_START:
		{
			m_stSessionToSvr.m_stCmdData.m_stConnSessStart.m_dwIPAddr = pstClient->m_dwIP;
			m_stSessionToSvr.m_stCmdData.m_stConnSessStart.m_wPort = pstClient->m_wPort;
			break;
		}
	case PKGMETA::CONNSESSION_CMD_STOP:
		{
			break;
		}
	case PKGMETA::CONNSESSION_CMD_INPROC:
		{
			break;
		}
	case PKGMETA::CONNSESSION_CMD_RECONN:
		{
			break;
		}

	default:
		return -1;
	}

	//LOGRUN("SendToLogicSvr sessionID %d, cmd-%d", pstClient->m_dwSessionId, (int)chSessCmd);
	return this->SendToLogicSvr(m_stSessionToSvr, pszPkg);
}

int CConnUDP::SendToLogicSvr(PKGMETA::CONNSESSION& rstSession, char* pszPkg)
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
void CConnUDP::ConnStopNotifyLogicSvr(SClient* pstClient, char chStopReason)
{
	assert(pstClient);

	if (CLIENT_STATE_NULL == pstClient->m_iState ||
		pstClient->m_bExitByLogicSvr)
	{
		return;
	}

	TRACE("Send SESSION_STOP to client [%s], Hnd[%d]", pstClient->m_szAccountName, pstClient->m_iHnd );

	bzero(&m_stSessionToSvr, sizeof(m_stSessionToSvr));
	m_stSessionToSvr.m_chConnType = PROTO_TYPE_UDP_R;
	m_stSessionToSvr.m_chCmd = PKGMETA::CONNSESSION_CMD_STOP;
	m_stSessionToSvr.m_dwSessionId = pstClient->m_dwSessionId;
	m_stSessionToSvr.m_iConnHnd = pstClient->m_iHnd;
	m_stSessionToSvr.m_stCmdData.m_stConnSessStop.m_chStopReason = chStopReason;

	this->SendToLogicSvr(m_stSessionToSvr, NULL);
}


// 检查client发包频率,过快有可能使用了外挂，直接踢掉
bool CConnUDP::CheckCltPkgFreq( SClient* pstClient )
{
	assert( pstClient );

	time_t lCurrTime = CGameTime::Instance().GetCurrSecond();
	pstClient->m_wRecvStatsPkgs++;

	if( lCurrTime - pstClient->m_lRecvStatsTime < 1 )
	{
		return true;
	}

#if 1
	int iPkgPerSec = (int)pstClient->m_wRecvStatsPkgs / (int)(lCurrTime - pstClient->m_lRecvStatsTime);
	// 每秒Client包太多
	if( iPkgPerSec >= m_pstConfig->m_iRecvPkgLimitPerSec )
	{
		LOGERR("Too much pkg in 1 second. Avg[%d] MaxAllowed[%d]. Player[%s] Hnd[%d] IP[%s] SessionID[%u]",
			iPkgPerSec,
			m_pstConfig->m_iRecvPkgLimitPerSec,
			pstClient->m_szAccountName, pstClient->m_iHnd, INET_HTOA(pstClient->m_dwIP), pstClient->m_dwSessionId );

		return false;
	}
#endif

	pstClient->m_wRecvStatsPkgs = 0;
	pstClient->m_lRecvStatsTime = lCurrTime;

	return true;
}

void CConnUDP::DealLogicSvrPkg()
{
	MyTdrBuf* pstTdrBuf = m_oBusLayer.GetRecvTdrBuf();
	char* pRecvPkg = pstTdrBuf->m_szTdrBuf;
	size_t uRecvTotalLen = (int)pstTdrBuf->m_uPackLen;
	size_t uSessLen = 0;

	// 获取LogicServer回传的session信息
	PKGMETA::CONNSESSION stSession;
	TdrError::ErrorType iRet = stSession.unpack( pRecvPkg, uRecvTotalLen, &uSessLen );
	if( iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR("Unpack AccLogin pkg failed!");
		return;
	}

	this->DealLogicSvrPkg(stSession, uSessLen, pstTdrBuf);

	return;
}

void CConnUDP::DealLogicSvrPkg(PKGMETA::CONNSESSION& rstSession, size_t uSessLen, MyTdrBuf* pstTdrBuf)
{
    // 有可能只有session (SESSION_STOP)，没有pkg, 此时 iPkgLen==0
    char* pRecvPkg = pstTdrBuf->m_szTdrBuf;
    size_t uRecvTotalLen = (int)pstTdrBuf->m_uPackLen;
    int iPkgLen = uRecvTotalLen - uSessLen;
    char* pszScPkg = pRecvPkg + uSessLen;

	SClient* pstClient = m_oClientPool.GetDataByHnd(rstSession.m_iConnHnd);
	if( NULL == pstClient )
	{
		if( rstSession.m_chCmd != PKGMETA::CONNSESSION_CMD_STOP )
		{
			rstSession.m_chCmd = PKGMETA::CONNSESSION_CMD_STOP;
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
		this->CloseClient(pstClient, CONNSESSION_REASON_INVALID_CLIENT);
		return;
	}

	// 已是延迟退出状态, 不再处理多余的bus pkg
	if (IS_CLIENT_IN_STATE(pstClient, CLIENT_STATE_LOGOUT))
	{
		return;
	}

	// LogicServer通知对某Client主动断线,会将本次pkg发送给client再断开连接. 注意不一定有pkg
	if( PKGMETA::CONNSESSION_CMD_STOP == rstSession.m_chCmd )
	{
		SET_CLIENT_IN_STATE( pstClient, CLIENT_STATE_LOGOUT );
		pstClient->m_bExitByLogicSvr = true;
		LOGRUN("Disconnect from LogicServer. Player[%s] IP[%s]",pstClient->m_szAccountName, INET_HTOA(pstClient->m_dwIP) );
	}

	// iPkgLen检查
	// 有可能只有session (SESSION_STOP)，没有pkg, 此时 iPkgLen==0
    if (iPkgLen < (int)sizeof(PKGMETA::SCPKGHEAD) || iPkgLen > (int)sizeof(PKGMETA::SCPKG))
    {
        return;
    }

	if (SCPKG_MSGID(pszScPkg) == PKGMETA::SC_MSG_FIGHT_LOGIN_RSP)
	{
		SET_CLIENT_IN_STATE(pstClient, CLIENT_STATE_INGAME);
	}

	if (CSPKG_PROTOTYPE(pszScPkg) == PROTO_TYPE_UDP_R)
	{
		this->SendToClient(pstClient, pszScPkg);
	}
	else // PROTO_TYPE_UDP_N
	{
		this->SendToClientWithoutAck(pstClient, pszScPkg);
	}
}


/*
发送一个完整的pkg给client
返回:
< 0: error
*/
int CConnUDP::SendToClient(SClient* pstClient, char* pszScPkg)
{
	assert(pstClient && pszScPkg);

	UDPClientBuffIndex* pClientBuffIndex = (UDPClientBuffIndex*)pstClient->m_poUDPBuffIndex;
	if (!pClientBuffIndex)
	{
		LOGERR("pClientBuffIndex is null");
		return -1;
	}

	// 申请POOL缓存并将引用添加到发送包
	int iPkgLen = (int)SCPKG_LEN(pszScPkg);
	UDPBlock* pBlock = m_oUDPBlockPool.GetBlock(iPkgLen);
	if (!pBlock)
	{
		LOGERR("get pBlock failed");
		return -1;
	}

	// 初始化Block
	pBlock->m_pEpoll = &m_oEpoll;
	pBlock->m_pConn = this;
	pBlock->m_pClient = pstClient;
	pBlock->m_dwTimerId = 0;

	// set number
	((PKGMETA::SCPKG*)(pszScPkg))->m_stHead.m_dwSeqNum = tdr_hton32(pstClient->m_dwCurSendSeq);
	pstClient->m_dwCurSendSeq++;

	// copy data
	memcpy(pBlock->m_pszBuff, pszScPkg, iPkgLen);

	//发送包
	int iRet = pstClient->m_oSocketUDP.Send(pBlock->m_pszBuff, iPkgLen, pstClient->m_dwIP, pstClient->m_wPort);
	if (iRet < 0)
	{
		LOGERR( "Send msg to Player[%s] IP[%s] Err[%d : %s]",
			pstClient->m_szAccountName, INET_HTOA(pstClient->m_dwIP), errno, strerror(errno));
		this->CloseClient(pstClient, CONNSESSION_REASON_NETWORK_FAIL);
        m_oUDPBlockPool.ReleaseBlock(pBlock);
		return -1;
	}
	else if( iRet != iPkgLen )
	{
		LOGERR( "Sended pkg len<%d> is not equal to actual len<%d>!", iRet, iPkgLen );
		this->CloseClient(pstClient, CONNSESSION_REASON_NETWORK_FAIL);
        m_oUDPBlockPool.ReleaseBlock(pBlock);
		return -1;
	}
	else
	{
		//为已发送的包添加定时器
		if (SCPKG_MSGID(pBlock->m_pszBuff) != PKGMETA::COM_MSG_UDP_ACK_NTF)
		{
			UDPTimer* pTimer = GET_GAMEOBJECT( UDPTimer, GAMEOBJ_UDPTIMER );
			if (!pTimer)
			{
				LOGERR("pTimer is null");
                m_oUDPBlockPool.ReleaseBlock(pBlock);
				return -1;
			}

			pBlock->m_dwTimerId = pTimer->GetObjID();

            pTimer->AttachParam(pBlock, m_pstConfig->m_stUdpConfig.m_iMaxResendInterval);
            struct timeval tFirstFireTime = *(CGameTime::Instance().GetCurrTime());
            pBlock->m_tBlockTimeStamp = tFirstFireTime;
            TvAddMs(&tFirstFireTime, pstClient->m_wRTO);
            pBlock->m_bIsResend = false;
            pTimer->SetTimerAttr( tFirstFireTime, m_pstConfig->m_stUdpConfig.m_iMaxResendTimes, pstClient->m_wRTO );
			m_oUDPTimerMgr.AddTimer((GameTimer*)pTimer);

            std::pair<std::map<uint32_t, UDPBlock*>::iterator, bool> pair =
                pClientBuffIndex->m_oSendedMap.insert(std::pair<uint32_t, UDPBlock*>(SCPKG_SEQNUM(pszScPkg), pBlock));

        	if (!pair.second)
        	{
        		LOGERR("SendToClient map insert failed");
        		m_oUDPBlockPool.ReleaseBlock(pBlock);
                m_oUDPTimerMgr.DelTimer(pTimer->GetObjID());
                RELEASE_GAMEOBJECT(pTimer);
        		CloseClient(pstClient, CONNSESSION_REASON_WR_BLOCKED);
        		return -2;
        	}

            LOGRUN("SendToClient and AddTimer client sid-%u, seq-%u, msgid-%u, pkglen-%d, timerId-%d,  resendInterval-%d",
                    pstClient->m_dwSessionId, SCPKG_SEQNUM(pszScPkg), SCPKG_MSGID(pszScPkg), iRet, pTimer->GetObjID(), pstClient->m_wRTO);
		}
	}
	return 0;
}

int CConnUDP::SendToClientWithoutAck(SClient* pstClient, char* pszScPkg)
{
	assert(pstClient && pszScPkg);

	int iPkgLen = (int)SCPKG_LEN(pszScPkg);

	//发送包
	int iRet = pstClient->m_oSocketUDP.Send(pszScPkg, iPkgLen, pstClient->m_dwIP, pstClient->m_wPort);
	if (iRet < 0)
	{
		LOGERR( "Send msg to Player[%s] IP[%s] Err[%d : %s]",
			pstClient->m_szAccountName, INET_HTOA(pstClient->m_dwIP), errno, strerror(errno));
		this->CloseClient(pstClient, CONNSESSION_REASON_NETWORK_FAIL);
		return -1;
	}else if( iRet != iPkgLen )
	{
		LOGERR( "Sended pkg len<%d> is not equal to actual len<%d>!", iRet, iPkgLen );
		this->CloseClient(pstClient, CONNSESSION_REASON_NETWORK_FAIL);
		return -2;
	}

	return 0;
}

int CConnUDP::SendAckToClient(SClient* pstClient)
{
	assert(pstClient);

	UDPClientBuffIndex* pClientBuffIndex = (UDPClientBuffIndex*)pstClient->m_poUDPBuffIndex;
	if (!pClientBuffIndex)
	{
		LOGERR("pClientBuffIndex is null");
		return -1;
	}

	// 申请POOL缓存并将引用添加到发送包
	int iSize = pClientBuffIndex->m_oAckList.size();
	int iCnt = iSize > (int)(m_pstConfig->m_stUdpConfig.m_dwUdpAckRspNum) ? (int)(m_pstConfig->m_stUdpConfig.m_dwUdpAckRspNum) : iSize;
	m_stScPkg.m_stHead.m_wMsgId = PKGMETA::COM_MSG_UDP_ACK_NTF;
	m_stScPkg.m_stBody.m_stUdpAckNtf.m_chCount = 0;
    m_stScPkg.m_stBody.m_stUdpAckNtf.m_wCurRecvedSeq = (uint16_t)(pstClient->m_dwCurRecvSeq-1);

	int i = 0;
	list<uint32_t>::iterator iter = pClientBuffIndex->m_oAckList.begin();
	for (; i < iCnt && iter != pClientBuffIndex->m_oAckList.end(); i++)
	{
		// 一场战斗UDP包不会超过2个字节数, 节约Ack包长
		if ( (uint16_t)*iter > m_stScPkg.m_stBody.m_stUdpAckNtf.m_wCurRecvedSeq)
		{
		    m_stScPkg.m_stBody.m_stUdpAckNtf.m_SeqNum[m_stScPkg.m_stBody.m_stUdpAckNtf.m_chCount++] = (uint16_t)*iter;
		    LOGRUN("send ack, sid-%d, seq-%d", pstClient->m_dwSessionId, *iter);
		}

		iter = pClientBuffIndex->m_oAckList.erase(iter);
	}

    LOGRUN("send ack, sid-%d, curseq-%d", pstClient->m_dwSessionId, (pstClient->m_dwCurRecvSeq-1));

	m_stScAckBuf.Reset();
	TdrError::ErrorType iErrNo = m_stScPkg.pack(m_stScAckBuf.m_szTdrBuf, m_stScAckBuf.m_uSize, &(m_stScAckBuf.m_uPackLen));
	if (iErrNo != TdrError::TDR_NO_ERROR)
	{
		LOGERR("ack pack failed");
		return -1;
	}

	int iRet = pstClient->m_oSocketUDP.Send(m_stScAckBuf.m_szTdrBuf, m_stScAckBuf.m_uPackLen, pstClient->m_dwIP, pstClient->m_wPort);
	if (iRet < 0)
	{
		LOGERR("ack send failed");
		return -2;
	}

	return 0;
}

int CConnUDP::OnAccountLogin( SClient* pstClient )
{
	PKGMETA::CSPKG stCsPkg;

	// 解登录包
	TdrError::ErrorType iRet = stCsPkg.unpack( pstClient->m_szCltPkg, pstClient->m_iCltPkgLen );
	if( iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR("Unpack FightLogin pkg failed!");
		//this->_CloseClient( pstClient, 0, false );
		return -1;
	}

	PKGMETA::CS_PKG_FIGHT_LOGIN_REQ* pstFightLogin = &(stCsPkg.m_stBody.m_stFightLoginReq);
	StrCpy( pstClient->m_szAccountName, pstFightLogin->m_szAccountName, sizeof(pstClient->m_szAccountName) );

	if( !this->SendToLogicSvr( pstClient, pstClient->m_szCltPkg, PKGMETA::CONNSESSION_CMD_START ) )
	{
		LOGERR("Send fight login msg to LogicServer failed!. Player[%s] IP[%s]",
			pstClient->m_szAccountName, INET_HTOA(pstClient->m_dwIP) );
		this->CloseClient( pstClient, 0, false );
		return -1;
	}

	// UDP 登陆就直接INGAME状态
	SET_CLIENT_IN_STATE( pstClient, CLIENT_STATE_INGAME );

	struct timeval* pstCurTime = CGameTime::Instance().GetCurrTime();
	pstClient->m_lActiveTime = pstCurTime->tv_sec;
	return -1;
}


// 目前不处理重连
int CConnUDP::OnReconnect(SClient* pstClient)
{
	return -1;
}


void CConnUDP::Update(bool bIdle)
{
	static struct timeval tvUptStartTime = { 0 };
	struct timeval tvCurTime = { 0 };

	if( m_oClientPool.GetUsedNum() == 0  )
	{
		return;
	}

	tvCurTime = *(CGameTime::Instance().GetCurrTime());

	if( MsPass( &tvCurTime, &tvUptStartTime) < CONN_CLIENT_CHK_FREQ )
	{
		return;
	}

	int iCheckNum = bIdle ? 50 : 10;

	if( m_oClientPoolIter.IsEnd() )
	{
		m_oClientPoolIter.Begin();
	}

	SClient* pstClient = NULL;
	for( int i = 0; i < iCheckNum && !m_oClientPoolIter.IsEnd(); i++, m_oClientPoolIter.Next() )
	{
		pstClient = m_oClientPoolIter.CurrItem();
		if( NULL == pstClient )
		{
			assert( false );
			break;
		}

		// 检查缓存的ACK是否超时
		UDPClientBuffIndex* poBuffIndex = (UDPClientBuffIndex*)pstClient->m_poUDPBuffIndex;
		if (poBuffIndex->m_ullTimestampMs != 0)
		{
			if ((CGameTime::Instance().GetCurrTimeMs() - poBuffIndex->m_ullTimestampMs) >= m_pstConfig->m_stUdpConfig.m_dwUdpAckRspMs)
			{
				this->SendAckToClient(pstClient);

                // 清除收包时间戳
                poBuffIndex->m_ullTimestampMs = 0;
			}
		}

		if (pstClient->m_bExitByLogicSvr)
		{
			LOGRUN("close by logic svr, account<%s>, ", pstClient->m_szAccountName);
			this->CloseClient(pstClient, CONNSESSION_REASON_SELF_CLOSE, false);
			continue;
		}

		// 心跳检测
		time_t lCurrSec = CGameTime::Instance().GetCurrSecond();
		if( m_pstConfig->m_stUdpConfig.m_wMaxIdle > 0 )
		{
			if( lCurrSec - pstClient->m_lActiveTime >= m_pstConfig->m_stUdpConfig.m_wMaxIdle )
			{
				LOGERR("reach deadline, account<%s>, lCurrSec-%lu, pstClient->m_lActiveTime-%lu",
					pstClient->m_szAccountName, lCurrSec, pstClient->m_lActiveTime);
				this->CloseClient( pstClient, CONNSESSION_REASON_IDLE_CLOSE );
				continue;
			}
		}


        //检查缓存的乱序包是否超时
        map<uint32_t, UDPBlock*>::iterator iterBlock = poBuffIndex->m_oRecvMap.begin();
        for (; iterBlock != poBuffIndex->m_oRecvMap.end(); iterBlock++)
        {
            if(MsPass(&tvCurTime, &iterBlock->second->m_tBlockTimeStamp) > (unsigned long)m_pstConfig->m_stUdpConfig.m_iUdpRecvTimeoutMs)
            {
                LOGRUN("close by logic svr, account<%s>, ", pstClient->m_szAccountName);
			    this->CloseClient(pstClient, CONNSESSION_REASON_RECIEVE_TIMEOUT);
			    break;
            }
        }
	}

	if( m_oClientPoolIter.IsEnd( ) )
	{
		tvUptStartTime = tvCurTime;
	}
}

void CConnUDP::ReleaseTimer(GameTimer* pTimer)
{
	if (!pTimer)
	{
		LOGWARN("pTimer is NULL");
		return;
	}

	UDPBlock* pBlock = ((UDPTimer*)pTimer)->m_pBlock;
	assert(pBlock != NULL);

	CConnUDP* pConn = pBlock->m_pConn;
	SClient* pClient = pBlock->m_pClient;
	assert(pConn != NULL && pClient != NULL);

	// 释放Block
	unsigned int dwTimerId = pTimer->GetObjID();
	int iMaxFireCount = pTimer->GetMaxFireCount();
	int iFireCount = pTimer->GetFireCount();
	LOGRUN("ReleaseTimer and Block, client sid-%u, pkg seq-%u, timerid-%u, firecount-%d", pClient->m_dwSessionId, SCPKG_SEQNUM(pBlock->m_pszBuff), dwTimerId, iFireCount);
	pConn->m_oUDPBlockPool.ReleaseBlock(pBlock);

	// 释放Timer
	RELEASE_GAMEOBJECT(pTimer);

	if (iFireCount >= iMaxFireCount)
	{
		// 重传失败，关闭连接
		LOGERR("Resend failed, close client sid - %u.", pClient->m_dwSessionId);
		pConn->CloseClient(pClient, CONNSESSION_REASON_RESEND_TIMEOUT);
	}

	return;
}

void CConnUDP::RTOEstimator(UDPBlock* pBlock, uint16_t wRTT)
{
    assert(pBlock != NULL);
    if (pBlock->m_bIsResend)
    {
        return;
    }
    SClient* pClient = pBlock->m_pClient;
    if (NULL==pClient)
    {
        return;
    }

    if (pClient->m_wRTT == 0)
    {
        pClient->m_wRTT = wRTT;
    }
    else
    {
        pClient->m_wRTT = pClient->m_wRTT - (pClient->m_wRTT >> 3) + (wRTT >> 3);
    }

    // D = a*D+(1-a)|RTT-M|, a=1/4

    int16_t wDev = wRTT - pClient->m_wRTT;
    if (wDev < 0)
    {
        wDev = -wDev;
    }
    pClient->m_wDev = pClient->m_wDev -(pClient->m_wDev >> 2) + (wDev >> 2);
    pClient->m_wRTO = pClient->m_wRTT + (pClient->m_wDev << 2);

    if (pClient->m_wRTO < m_pstConfig->m_stUdpConfig.m_iMinResendInterval)
    {
        pClient->m_wRTO = m_pstConfig->m_stUdpConfig.m_iMinResendInterval;
    }
    if (pClient->m_wRTO > m_pstConfig->m_stUdpConfig.m_iMaxResendInterval)
    {
        pClient->m_wRTO = m_pstConfig->m_stUdpConfig.m_iMaxResendInterval;
    }
    return;
}

void CConnUDP::ChgSendBufferRTO(UDPClientBuffIndex* pClientBuffIndex, uint16_t wNewRTO)
{
    map<uint32_t, UDPBlock*>::iterator iterBlock = pClientBuffIndex->m_oSendedMap.begin();
    int iCount = 0;
    for(; iterBlock!= pClientBuffIndex->m_oSendedMap.end(); iterBlock++)
    {
        if (iterBlock->second == NULL)
        {
            continue;
        }
        if (iterBlock->second->m_bIsResend)
        {
            continue;
        }
        struct timeval tFireTime = iterBlock->second->m_tBlockTimeStamp;
        TvAddMs(&tFireTime, wNewRTO);
        m_oUDPTimerMgr.ModTimerFireTime(iterBlock->second->m_dwTimerId, tFireTime);
        if (iterBlock->second->m_pClient!=NULL && iterBlock->second->m_pszBuff!=NULL)
        {
            LOGRUN("mod timer firetime client sid-%u, seq-%u, msgid-%u, timerId-%d, newRTO-%d",
                    iterBlock->second->m_pClient->m_dwSessionId, SCPKG_SEQNUM(iterBlock->second->m_pszBuff),
                    SCPKG_MSGID(iterBlock->second->m_pszBuff), iterBlock->second->m_dwTimerId, wNewRTO);
        }
        iCount++;
    }

    LOGRUN("mod %d timers' firetime, mapsize-%d", iCount, (int)pClientBuffIndex->m_oSendedMap.size());
    return;
}
void CConnUDP::QuicklyResend(SClient* pstClient, uint16_t wPeerCurRecvSeq)
{
    if (wPeerCurRecvSeq == pstClient->m_wPeerCurRecvSeq)
    {
        pstClient->m_bRepeatSeqCount++;
    }
    else
    {
        pstClient->m_wPeerCurRecvSeq = wPeerCurRecvSeq;
        pstClient->m_bRepeatSeqCount = 0;
        return;
    }

    if (pstClient->m_bRepeatSeqCount >= m_pstConfig->m_stUdpConfig.m_iQuickResendTriggerTimes)
    {
        pstClient->m_bRepeatSeqCount = 0;
        UDPClientBuffIndex* pClientBuffIndex = (UDPClientBuffIndex*)pstClient->m_poUDPBuffIndex;
        map<uint32_t, UDPBlock*>::iterator iterBlock = pClientBuffIndex->m_oSendedMap.find(wPeerCurRecvSeq+1);
        if (iterBlock != pClientBuffIndex->m_oSendedMap.end())
        {
            UDPBlock* pBlock = iterBlock->second;
            if (NULL==pBlock)
            {
                LOGERR("pBlock is null");
                return;
            }
            struct timeval tFireTime = *(CGameTime::Instance().GetCurrTime());
            TvAddMs(&tFireTime, pstClient->m_wRTO);
            m_oUDPTimerMgr.ModTimerFireTime(pBlock->m_dwTimerId, tFireTime);

            int iRet = pstClient->m_oSocketUDP.Send(pBlock->m_pszBuff, SCPKG_LEN(pBlock->m_pszBuff), pstClient->m_dwIP, pstClient->m_wPort);

            LOGRUN("Quicly resend client sid-%u, seq-%u, msid-%d, RTO-%d, Send iRet = %d",
                pstClient->m_dwSessionId, wPeerCurRecvSeq+1, SCPKG_MSGID(pBlock->m_pszBuff), pstClient->m_wRTO, iRet);
        }
        else
        {
            LOGERR("Quicly resend fail, client sid-%u, seq-%u, not found",
                pstClient->m_dwSessionId, wPeerCurRecvSeq+1);
        }
    }
    return;
}

