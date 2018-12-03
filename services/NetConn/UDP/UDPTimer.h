#pragma once

#include "GameTimer.h"

class UDPBlock;
class SClient;
class CConnUDP;
class CEpoll;

class UDPTimer : public GameTimer
{
public:
	UDPBlock* m_pBlock;
    int m_iMaxResendTime;

public:
	UDPTimer();
	virtual ~UDPTimer();

	virtual void Clear();

	void AttachParam(UDPBlock* pBlock, int iMaxResendTime);

protected:
	void _Construct();

public:
	virtual void OnTimeout();
};

