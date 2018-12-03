#pragma once
#include "define.h"
#include "object.h"

class CEpoll;
class CConnUDP;
class SClient;

class UDPBlock : public IObject
{
public:
	UDPBlock();
	virtual ~UDPBlock(){};

	virtual void Clear();
	void _Construct();

public:
	CEpoll* m_pEpoll;
	CConnUDP* m_pConn;
	SClient* m_pClient;

    struct timeval m_tBlockTimeStamp; // 发送包的时刻
    bool m_bIsResend;
	uint32_t m_dwTimerId;
	char* m_pszBuff;
	int m_iHnd;
	int m_iLen;
};

