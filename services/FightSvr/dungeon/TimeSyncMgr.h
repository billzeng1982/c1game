#pragma once

#include "singleton.h"
#include "GameTimerMgr_PQ.h"

class TimeSyncMgr : public TSingleton<TimeSyncMgr> 
{
public:
	TimeSyncMgr();
	virtual ~TimeSyncMgr(){}

protected:
	GameTimerMgr_PQ m_oTimerMgr;

public:
	void Update();
		
};


