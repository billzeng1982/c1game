#include "UdpRelay.h"
#include "LogMacros.h"
#include "CommBusLayer.h"
#include "strutil.h"

bool UdpRelayClt::Init()
{
    m_iSockFd = socket(AF_INET, SOCK_DGRAM, 0);
	if(m_iSockFd < 0)
	{
		LOGERR("open socket error\n");
		return false;
	}

    //设置为非阻塞
    if( fcntl(m_iSockFd, F_SETFL, fcntl(m_iSockFd, F_GETFD, 0)|O_NONBLOCK) == -1 )
    {
        LOGERR("set nonblock failed");
        this->Close();
        return false;
    } 

    return true;
}

// 传入的dwIpAddr是本机序
int UdpRelayClt::Send( const char* pBuf, int iBufLen, uint32_t dwIpAddr, uint16_t wPort )
{
    struct sockaddr_in stAddr;
    stAddr.sin_family = AF_INET;
    stAddr.sin_addr.s_addr = htonl(dwIpAddr);
	stAddr.sin_port = htons(wPort);
    
	return sendto(m_iSockFd, pBuf, iBufLen, 0, (struct sockaddr*)&stAddr, sizeof(stAddr));
}

int UdpRelayClt::Send( const char* pBuf, int iBufLen, char* pszIpAddr, uint16_t wPort )
{
    if( StrUtil::IsStringNull(pszIpAddr))
    {
        assert( false );
        return -1;
    }

    uint32_t dwIpAddr = 0;
    if ( inet_pton(AF_INET, pszIpAddr, &dwIpAddr) < 0 )
    {
        LOGERR("Convert ip addr <%s> failed!", pszIpAddr);
        return -1;
    }

    return this->Send( pBuf, iBufLen, ntohl(dwIpAddr), wPort );
}

void UdpRelayClt::Close()
{
    if( m_iSockFd !=0 )
    {
        close(m_iSockFd);
        m_iSockFd = 0;
    }
}


bool UdpRelaySvr::Init( char* pIpAddr, uint16_t wPort, uint32_t dwThreadQSzie, key_t* piShmKey )
{
    m_iSockFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_iSockFd < 0)
	{
		LOGERR("open socket error\n");
		return false;
	}

    bool bRet = false;
    do
    {
        //设置为非阻塞
        if( fcntl(m_iSockFd, F_SETFL, fcntl(m_iSockFd, F_GETFD, 0)|O_NONBLOCK) == -1 )
        {
            LOGERR("set nonblock failed");
            break;
        }

        if( inet_pton(AF_INET, pIpAddr, &m_stAddr.sin_addr ) < 0)
    	{
    		LOGERR("inet_pton error for %s\n", pIpAddr);
    		break;
    	}

    	m_stAddr.sin_family = AF_INET;
        m_stAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    	m_stAddr.sin_port = htons(wPort);

        if( bind(m_iSockFd, (struct sockaddr *)&m_stAddr, sizeof(m_stAddr) ) < 0)
        {
            LOGERR("bind failed");
            break;
        }

        if( !m_oEpoll.Create( 1 + 1) )
    	{
    		LOGERR("%s", m_oEpoll.GetErrMsg() );
    		break;
    	}

        if( !m_oEpoll.EpollAdd( m_iSockFd, NULL ) )
        {
            LOGERR("epoll add udp sock failed!");
            break;
        }

        // 注意最后创建thread
        if( !this->InitThread(0, dwThreadQSzie, THREAD_QUEUE_MODE_RSP,  NULL, piShmKey) )
        {
            LOGERR("Start thread failed!");
            break;
        }

        bRet = true;
    }while(0);

    if( !bRet )
    {
        this->Close();
    }

    return bRet;
}


void UdpRelaySvr::Close()
{
    if( m_iSockFd !=0 )
    {
        close(m_iSockFd);
        m_iSockFd = 0;
    }

    this->FiniThread();
}


// 线程逻辑处理, 返回处理请求包个数
int UdpRelaySvr::_ThreadAppProc()
{
	if(m_oEpoll.Wait( 10 ) <= 0)
	{
		return 0;
	}

    if( m_oEpoll.IsEvReadable(0) )
    {
        // recv ss pkg
        return this->_OnReadable();
    }
    else
    {
        return 0;
    }
}


// 返回 1: 成功处理包, 0: 包不完整,不处理, < 0 出错
int UdpRelaySvr::_OnReadable()
{
    struct sockaddr_in m_stSrcAddr;
    socklen_t iSrcAddrLen = sizeof(struct sockaddr_in);
	int iRet = recvfrom(m_iSockFd, m_szRecvBuf, sizeof(m_szRecvBuf), 0, (struct sockaddr*)&m_stSrcAddr, &iSrcAddrLen);
	if(iRet < 0) 
	{
		LOGERR("recv data error");
        return -1;
	}

    if( 0 == iRet )
    {
        // 收到字节为0的包
        return 0;
    }

	if( iRet < (int)sizeof(PKGMETA::SSPKGHEAD) )
	{
		LOGERR("Recv pkg len <%d> is less than header length!", iRet);
		return -1;
	}

    // 放入 RspQueue
    if( m_oRspQueue.WriteMsg( m_szRecvBuf, iRet) != THREAD_Q_SUCESS )
    {
        LOGERR("Push pkg failed! MsgID <%d>", SSPKG_MSGID(m_szRecvBuf));
        return -1;
    }

    return 1;
}

