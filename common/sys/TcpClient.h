#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_

/*
	��װtcp client
	ע��socketΪͬ��ģʽ
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
	const static int TCP_DEFAULT_TIMEOUT = 5000; // ms, tcp clientĬ�ϳ�ʱʱ��

public:
	const char* GetErrMsg() { return m_szErrMsg; }
	
	int Connect( const char* pszIPStr, int iPort );
	void Close();

	/*
		Send iLen bytes, ����ʵ�ʷ�������
	*/
	int Sendn( char* pszBuff, int iLen );

	/*
		Recv iLen bytes, ����ʵ�ʶ���������
	*/
	int Recvn( char* pszBuff, int iLen );

	// ����: 0 - �ɹ�, <0 - ����
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

