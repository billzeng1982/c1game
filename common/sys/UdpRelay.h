#pragma once

/*
	����server-server֮��ͨ��, �����߼�, ����װ�����շ�
	ע�ⲻ�ǿɿ�udp������Ҫ����server֮�䶨�ڷ���״̬��Ϣ

	UdpRelay��Ϊ�������ͱ�����
	���壺
	���������൱��udp client�����ڷ�������
	���������൱��udp server, ���ڽ�������(�����߳�)

	�����������������ֱ����udp����
	�������ŵ����߳���յ������ݷŵ��������У��������߼�ÿ��updateȥȡ���ݣ������߼�����
*/

#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "ThreadFrame.h"
#include "CEpoll.h"

class UdpRelayClt
{
public:
	UdpRelayClt()
	{
		m_iSockFd = 0;
	}

	~UdpRelayClt()
	{
		this->Close();
	}
	
	bool Init();

	// �����dwIpAddr�Ǳ�����
	int Send( const char* pBuf, int iBufLen, uint32_t dwIpAddr, uint16_t wPort );
	int Send( const char* pBuf, int iBufLen, char* pszIpAddr, uint16_t wPort );

	void Close();

private:
	int m_iSockFd;
};


class UdpRelaySvr : public CThreadFrame
{
public:
	static const int RECV_BUF_SIZE = 2048;
	static const int DEFAULT_THREAD_Q_SIZE = 1024*256;

public:
	UdpRelaySvr()
	{
		m_iSockFd = 0;
		bzero(&m_stAddr, sizeof(m_stAddr));
	}

	~UdpRelaySvr()
	{
		this->Close();
	}

public:
	bool Init( char* pIpAddr, uint16_t wPort, uint32_t dwThreadQSzie, key_t* piShmKey = NULL );
	void Close();

protected:
	// �߳��߼�����, ���ش������������
	virtual int _ThreadAppProc();
	// ���� 1: �ɹ������, 0: ��������,δ����, < 0 ����
	int _OnReadable();
	
private:
	int 	m_iSockFd;
	struct sockaddr_in m_stAddr;
	CEpoll 	m_oEpoll;
	char 	m_szRecvBuf[ RECV_BUF_SIZE ];
};

