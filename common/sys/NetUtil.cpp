#include "NetUtil.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include "pal/pal.h"
#include "pal/ttime.h"
#include <time.h>

/* 
    返回-3为对端断开连接
*/
int TcpRecv( int iSock, char* pszBuff, int iLen )
{
    int iRet     = 0;

    if( NULL == pszBuff )
    {
        return -1;
    }    

    if( iLen <= 0 )
    {
        return 0;
    }

RECV_AGAIN:
    iRet = recv( iSock, pszBuff, iLen, 0 ); 
    if( iRet < 0 )
    {
        if( EINTR == errno )
        {
            goto RECV_AGAIN;
        }

        if( EAGAIN == errno || EWOULDBLOCK == errno )
        {
            return 0;
        }

        // error ocurrs
        return -2;
    }
    else if( 0 == iRet )
    {
        // 对端断开链接
        return -3;
    }
    else
    {
        return iRet;
    }
}


int TcpRecvAll( int iSock, char*pszBuff, int iLen, int iTimeout /*ms*/ )
{
    assert( pszBuff && iLen > 0 );
    
    int iTotal = 0;
    int iRet = 0;
    int iTime = 0;

    while( iTotal < iLen )
    {
        if( ( SOCK_WAIT_INFINITE != iTimeout) && ( iTime > iTimeout ) )
        {
            return iTotal;
        }
    
        iRet = SelectWait( iSock, SELECT_FD_READ, iTimeout );
        if( iRet < 0 )
        {
            if( EINTR == errno )
            {
                return iTotal;
            }
            
            return -1;
        }
        if( 0 == iRet )
        {
            // time out
            return iTotal;
        }

        // read fd ready
        iRet = recv( iSock, pszBuff + iTotal, iLen - iTotal, 0 ); 
        if( iRet < 0 )
        {
            if( EAGAIN == errno || EWOULDBLOCK == errno )
            {
                usleep( 100000 );
                iTime += 100000/1000;
                continue;
            }else
            {
                return -1;
            }
        }

        if( 0 == iRet )
        {
            return ( 0 == iTotal ?  -1 : iTotal );
        }

        iTotal += iRet;
    }

    return iTotal;
}


int TcpSend( int iSock, char* pszBuff, int iLen )
{
    int iRet     = 0;

    if( NULL == pszBuff )
    {
        return -1;
    }

    if( iLen <= 0 )
    {
        return 0;
    }

SEND_AGAIN:
    iRet = send( iSock, pszBuff, iLen, 0);
    if( iRet < 0 )
    {
        if( EINTR == errno )
        {
            goto SEND_AGAIN;
        }

        if( EAGAIN == errno || EWOULDBLOCK == errno )
        {
            return 0;
        }

        // error occurs
        return -2;
    }
    else
    {
        return iRet;
    }
}


int TcpSendAll( int iSock, char* pszBuff, int iLen, int iTimeout /*ms*/ )
{
    assert( pszBuff && iLen > 0 );

    if( 0 == iTimeout )
    {
        // do not wait
        return TcpSend( iSock, pszBuff, iLen );
    }

    int iTotal = 0;
    int iRet = 0;
    int iTime = 0;
    
    while( iTotal < iLen )
    {
        if( ( SOCK_WAIT_INFINITE != iTimeout) && ( iTime > iTimeout ) )
        {
            return iTotal;
        }

        iRet = SelectWait( iSock, SELECT_FD_WRITE, iTimeout );
        if( iRet < 0 )
        {
            return -1;
        }
        if( 0 == iRet && SOCK_WAIT_INFINITE != iTimeout  )
        {
            // time out
            return iTotal;
        }

        // write fd ready
        iRet = send( iSock, pszBuff+iTotal, iLen - iTotal, 0 );
        if( iRet < 0 )
        {
            if( EAGAIN == errno || EWOULDBLOCK == errno )
            {
                usleep(20000);
                iTime += 20000 /1000;
                continue;
            }

            return -2;
        }
        
        if( 0 == iRet )
        {
            return ( 0 == iTotal ? -3 : iTotal );
        }

        iTotal += iRet;
    }

    return iTotal;
}


int TcpAccept( int iListenSock, struct sockaddr* pSockAddr, socklen_t* pSockLen  )
{
    int iSock = 0;

    if( NULL == pSockAddr|| NULL == pSockLen )
    {
        assert( false );
        return -1;
    }

ACCEPT_AGAIN:
    iSock = accept( iListenSock, pSockAddr, pSockLen );
    if( iSock < 0 )
    {
        if( EINTR == errno )
        {
            goto ACCEPT_AGAIN;
        }

        if( EAGAIN == errno ||  EWOULDBLOCK == errno )
        {
			return 0; 
        }

        // error ocurrs
        return -2;
    }

    return iSock;
}


char* INET_HTOA( uint32_t hIP )
{
    static __thread char buffer[18];

    unsigned char *bytes = (unsigned char *) &hIP;
    snprintf (buffer, sizeof (buffer), "%u.%u.%u.%u",
          bytes[3], bytes[2], bytes[1], bytes[0]);

  return buffer;
}


int CreateListenSock( char* pszIP, uint16_t wPort, int iBackLog )
{
    char szUri[ 256 ];
    bzero( szUri, sizeof(szUri) );

    if( iBackLog < 0 )
    {
        assert( false );
        iBackLog = 5;
    }
    
    snprintf( szUri, sizeof(szUri)-1, "tcp://%s:%u?reuse=1", pszIP, wPort); 
    int iSock = tnet_listen( szUri, iBackLog );
    if( iSock < 0 )
    {
        return -1;
    }

    if( tnet_set_nonblock( iSock, 1 ) < 0 )
    {
        printf("Set nonblock failed! uri=[%s]", szUri);
        tnet_close(  iSock );
        return -2;
    }

    return iSock;
}


int SelectWait( int fd, int iMask, int iTimeout )
{
    struct timeval tv;
	struct timeval* ptv;
    int iRet = 0;

    if( SOCK_WAIT_INFINITE == iTimeout )
    {
        ptv = NULL;
    }else
    {
        MS_TO_TV( tv, iTimeout );
        ptv = &tv;
    }

    if( fd != 0 )
    {
        fd_set  fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        iRet = select( fd+1, ( iMask&SELECT_FD_READ ? &fds : NULL ), ( iMask&SELECT_FD_WRITE ? &fds : NULL ),
                       NULL, ptv);
    }
    else
    {
        iRet = select( 0, NULL, NULL, NULL, ptv );
    }

    return iRet;
}

CSocketUDP::CSocketUDP()
{
	this->_Clear();
}

void CSocketUDP::_Clear()
{
    m_SockFd = 0;
    m_iCliAddrLen = 0;
    bzero(&m_stCliAddr_Recv, sizeof(m_stCliAddr_Recv));
    bzero(&m_stSvrAddr, sizeof(m_stSvrAddr));
    m_iSendFlag = 0;
    m_iRecvFlag = 0;
}

CSocketUDP::~CSocketUDP()
{
    if( m_SockFd > 0 )
    {
        close( m_SockFd );
    }
}

int CSocketUDP::Init(UDPInitParam& rstParam)
{
    char szUri[128];
    bzero(szUri, sizeof(szUri));   
    snprintf(szUri, sizeof(szUri)-1, "udp://%s:%u?reuse=1", rstParam.szIP, rstParam.wPort); 

	m_SockFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_SockFd < 0)
	{
		printf("socket error\n");
		return -3;
	}

	if (inet_pton(AF_INET, rstParam.szIP, &m_stSvrAddr.sin_addr) < 0)
	{
		printf("inet_pton error for %s\n", rstParam.szIP);
		return -4;
	}

	memset(&m_stSvrAddr, 0, sizeof(m_stSvrAddr));
	m_stSvrAddr.sin_family = AF_INET;
    m_stSvrAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_stSvrAddr.sin_port = htons(rstParam.wPort);

    SetSendFlag(0);	
	SetRecvFlag(0);

    //设置为非阻塞    
    if (fcntl(m_SockFd, F_SETFL, fcntl(m_SockFd, F_GETFD, 0)|O_NONBLOCK) == -1)
    {
        printf("nonblock failed");
        return -1;
    } 

    //绑定端口
    if (bind(m_SockFd, (struct sockaddr *)&m_stSvrAddr, sizeof(m_stSvrAddr)) < 0)
    {
        printf("bind failed");
        return -1;
    }

    // 设置缓存
    int iRet = setsockopt(m_SockFd, SOL_SOCKET, SO_RCVBUF, (const char*)&rstParam.iRecvBufSizeByte, sizeof(int));
    if (iRet<0)
    {
        printf("set recv buff size failed");
        return -3;
    }
    
    iRet = setsockopt(m_SockFd, SOL_SOCKET, SO_SNDBUF, (const char*)&rstParam.iSendBufSizeByte, sizeof(int));
    if (iRet<0)
    {
        printf("set send buff size failed");
        return -4;
    }
    
	return m_SockFd;
}

void CSocketUDP::SetSendFlag(int iFlag)
{
	m_iSendFlag = iFlag;
}

void CSocketUDP::SetRecvFlag(int iFlag)
{
	m_iRecvFlag = iFlag;
}

int CSocketUDP::Send(const char* pszSendBuf, int iSendBufLen, uint32_t dwIpAddr, uint16_t wPort)
{
    struct sockaddr_in stCliAddr;
    stCliAddr.sin_family = AF_INET;
    stCliAddr.sin_addr.s_addr = htonl(dwIpAddr);
	stCliAddr.sin_port = htons(wPort);
    
	int iRet = sendto(m_SockFd, pszSendBuf, iSendBufLen, m_iSendFlag, (struct sockaddr*)&stCliAddr, sizeof(stCliAddr));
	if (iRet < 0) 
	{
		printf("%s\n", "send data error");
	}
	return iRet;
}

int CSocketUDP::Recv(char* pszRecvBuf, int iRecvBufSize)
{
    m_iCliAddrLen = sizeof(struct sockaddr_in);
	int iRet = recvfrom(m_SockFd, pszRecvBuf, iRecvBufSize, m_iRecvFlag, (struct sockaddr*)&m_stCliAddr_Recv, &m_iCliAddrLen);
	if (iRet < 0) 
	{
		printf("%s\n", "recv data error");
	}

	return iRet;
}

void CSocketUDP::Close()
{
    if( m_SockFd > 0 )
    {
        close( m_SockFd );
    }

    this->_Clear();
}



