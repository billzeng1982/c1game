#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

/*
	封装tcp client
	注意socket为同步模式
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

class CTcpClient
{
public:
	CTcpClient( int iTimeout = TCP_DEFAULT_TIMEOUT );
	~CTcpClient();

public:
	const static int TCP_DEFAULT_TIMEOUT = 5000; // ms, tcp client默认超时时间

public:
	const char* GetErrMsg() { return m_szErrMsg; }
	
	int Connect( const char* pszIPStr, int iPort );
	void Close();

	/*
		Send iLen bytes, 返回实际发送数量
	*/
	int Sendn( char* pszBuff, int iLen );

	/*
		Recv iLen bytes, 返回实际读到的数量
	*/
	int Recvn( char* pszBuff, int iLen );

	// 返回: 0 - 成功, <0 - 错误
	int SendAndRecv( char* pszSndBuf, int iSndLen, char* pszRcvBuf, int iRcvLen );

private:
	int _OpenSocket();

private:

	char 	m_szErrMsg[256];
	int 	m_iSock;
	struct sockaddr_in m_stSockAddr;
	int 	m_iTimeout;
};

#endif

