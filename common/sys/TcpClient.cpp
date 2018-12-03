#include <string.h>
#include <errno.h>
#include <assert.h>
#include "TcpClient.h"
#include "NetUtil.h"
#include <stdio.h>
#include <unistd.h>

CTcpClient::CTcpClient( int iTimeout ) : m_iTimeout( iTimeout )
{
    bzero( m_szErrMsg, sizeof( m_szErrMsg ) );
    m_iSock = 0;
    bzero( &m_stSockAddr, sizeof(m_stSockAddr) );
}

CTcpClient::~CTcpClient()
{
    this->Close();
}

void CTcpClient::Close()
{
    if( m_iSock != 0 )
    {
        close( m_iSock );
        m_iSock = 0;
    }
}


int CTcpClient::Connect( const char* pszIPStr, int iPort )
{
    assert( pszIPStr && iPort > 0 );

    if( this->_OpenSocket() != 0 )
    {
        return -1;
    }

    m_stSockAddr.sin_family = AF_INET;
	m_stSockAddr.sin_port   = htons( iPort );
	m_stSockAddr.sin_addr.s_addr = inet_addr( pszIPStr );

    if( connect( m_iSock, (struct sockaddr *)&m_stSockAddr, sizeof(m_stSockAddr)) < 0 )
    {
        snprintf(m_szErrMsg, sizeof(m_szErrMsg), "%s", strerror(errno));
		return -1;
    }

    return 0;
}


int CTcpClient::_OpenSocket()
{
    if( m_iSock != 0 )
    {
        // already opened
        return 0;
    }

    m_iSock = socket( PF_INET, SOCK_STREAM, 0 );
    if( m_iSock < 0 )
    {
        snprintf( m_szErrMsg, sizeof(m_szErrMsg), "%s", strerror(errno) );
        return -1;
    }

    return 0;
}


/*
	Send iLen bytes, 返回实际发送数量
*/
int CTcpClient::Sendn( char* pszBuff, int iLen )
{
    return ::TcpSendAll( m_iSock, pszBuff, iLen, m_iTimeout );
}


/*
	Recv iLen bytes, 返回实际读到的数量
*/
int CTcpClient::Recvn( char* pszBuff, int iLen )
{
    return ::TcpRecvAll( m_iSock, pszBuff, iLen, m_iTimeout );
}


// 返回: 0 - 成功, <0 - 错误
int CTcpClient::SendAndRecv( char* pszSndBuf, int iSndLen, char* pszRcvBuf, int iRcvLen )
{
    assert( pszSndBuf && pszRcvBuf && iSndLen > 0 && iRcvLen > 0 );

    if( this->Sendn( pszSndBuf, iSndLen ) <= 0 ) 
    {
        return -1;
    }

    if( this->Recvn( pszRcvBuf,iRcvLen ) <= 0 )
    {
        return - 2;
    }

    return 0;
}

