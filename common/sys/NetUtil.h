#ifndef _NET_UTIL_H_
#define _NET_UTIL_H_

/*
	udp使用 sendto和recvfrom，不要和tcp的send, recv混淆
*/

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "define.h"

#define SOCK_WAIT_INFINITE -1

#define INET_NTOA( nIP ) inet_ntoa( *(struct in_addr *)&(nIP) )

// ip地址是host序时
char* INET_HTOA( uint32_t hIP );


/* 
  返回-3为对端断开连接
*/
int TcpRecv( int iSock, char* pszBuff, int iLen );
int TcpRecvAll( int iSock, char*pszBuff, int iLen, int iTimeout /*ms*/ );

int TcpSend( int iSock, char* pszBuff, int iLen );
int TcpSendAll( int iSock, char* pszBuff, int iLen, int iTimeout /*ms*/ );

int TcpAccept( int iListenSock, struct sockaddr* pSockAddr, socklen_t* pSockLen );

int CreateListenSock( char* pszIP, uint16_t wPort, int iBackLog );

enum SELECT_FD_MASK
{
	SELECT_FD_READ = 0x01,
	SELECT_FD_WRITE = 0x02
};

int SelectWait( int fd, int iMask, int iTimeout );

struct UDPInitParam
{
	char szIP[32];
	uint16_t wPort;
	int iRecvTimeOutSec;
	int iSendTimeOutSec;
	int iRecvBufSizeByte;
	int iSendBufSizeByte;
};

class CSocketUDP
{
public:
	CSocketUDP();
	virtual ~CSocketUDP();

	int Init(UDPInitParam& rstParam);
	void SetSendFlag(int iFlag);
	void SetRecvFlag(int iFlag);
	int Send(const char* pszSendBuf, int iSendBufLen, uint32_t dwIpAddr, uint16_t wPort);
	int Recv(char* pszRecvBuf, int iRecvBufSize);
	void Close();

private:
	void _Clear();

public:
	int m_SockFd;
	struct sockaddr_in m_stCliAddr_Recv; // peer端addr
	struct sockaddr_in m_stSvrAddr;
	
private:
	socklen_t m_iCliAddrLen;
	int m_iSendFlag;
	int m_iRecvFlag;
};
#endif

