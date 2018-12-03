#pragma once

/*
	用于server-server之间通信, 不带逻辑, 仅封装数据收发
	注意不是可靠udp哈，主要用做server之间定期发送状态信息

	UdpRelay分为主动方和被动方
	定义：
	主动方：相当于udp client，用于发送数据
	被动方：相当于udp server, 用于接收数据(单独线程)

	主动方放在主进程里，直接用udp发送
	被动方放到从线程里，收到的数据放到无锁队列，主进程逻辑每次update去取数据，并作逻辑处理
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

	// 传入的dwIpAddr是本机序
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
	// 线程逻辑处理, 返回处理请求包个数
	virtual int _ThreadAppProc();
	// 返回 1: 成功处理包, 0: 包不完整,未处理, < 0 出错
	int _OnReadable();
	
private:
	int 	m_iSockFd;
	struct sockaddr_in m_stAddr;
	CEpoll 	m_oEpoll;
	char 	m_szRecvBuf[ RECV_BUF_SIZE ];
};

