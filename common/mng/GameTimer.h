#pragma once

#include "object.h"
#include "macros.h"
#include "define.h"
#include <time.h>
#include <strings.h>

/*
	逻辑timer继承GameTImer
*/

class GameTimerMgr_PQ;

class GameTimer : public IObject
{
	friend class GameTimerMgr_PQ;
	
public:
	GameTimer()
	{
		this->Clear();
	}
	
	virtual void OnTimeout() {}
	
	virtual void Clear()
	{
		m_iMaxFireCount= 1;
		m_dwPeriodMs = 0;
		
		m_iFireCount = 0;
		bzero( &m_tvFireTime, sizeof(m_tvFireTime) );

		m_bDelFlag = false;

		IObject::Clear();
	}
	
	void SetPeroidMs( uint32_t dwPeriodMs ) { m_dwPeriodMs = dwPeriodMs; }
	uint32_t GetPeroidMs() { return m_dwPeriodMs; }

	void SetMaxFireCount( int val ) { m_iMaxFireCount = val; }
	int  GetMaxFireCount() { return m_iMaxFireCount; }
	
	void SetTimerAttr( struct timeval& rFirstFireTime, int iMaxFireCount = 1, uint32_t dwPeriodMs = 0 )
	{
		m_tvFireTime = rFirstFireTime;
		m_dwPeriodMs = dwPeriodMs;
		m_iMaxFireCount = iMaxFireCount;
	}

	int GetFireCount() { return m_iFireCount; }

	//void SetFireTime( struct timeval& tvFireTime ) { m_tvFireTime = tvFireTime; }
	struct timeval* GetFireTime() { return &m_tvFireTime; }

	bool GetDelFlag() { return m_bDelFlag; }
	void SetDelFlag(bool val) { m_bDelFlag = val; }
	
protected:
	int m_iMaxFireCount; 	// 最大发触发次数,默认为1, 为0表示无限触发
	uint32_t m_dwPeriodMs;  // 触发周期，ms

	int m_iFireCount;
	struct timeval m_tvFireTime;
	bool m_bDelFlag; // 用于延迟删除
};

