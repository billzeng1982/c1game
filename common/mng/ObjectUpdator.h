#pragma once

#include "object.h"
#include "list.h"
#include <sys/time.h>

#define DFT_IDLE_UPT_COUNT 30
#define DFT_BUSY_UPT_COUNT 10
#define DFT_UPDATE_FREQ	   300 // ms

typedef void (*UpdateCallback)( IObject* pObj, int iDeltaTime /* ms */ );

class ObjectUpdator
{
public:
	ObjectUpdator();
	~ObjectUpdator();

	void Update( bool bIdle );
	void Schedule( IObject* pObj );
	void UnSchedule( IObject* pObj );
	void BindCallback( UpdateCallback cb ) { m_fUpdateCb = cb; } 
	unsigned long GetMsPassInterval();

public:
	int m_iIdleUptCount;
	int m_iBusyUptCount;
	int m_iUptFreq;
	unsigned short m_wObjType;

private:
	void _IterBegin() {  m_pCurIter = m_stUpdateList.next; }
	bool _IsIterEnd() { return ( m_pCurIter == &m_stUpdateList ); }
	void _IterNext() { m_pCurIter = m_pCurIter->next; }
	IObject* _IterCurr() { return list_entry( m_pCurIter, IObject, m_stUptNode ); }

private:
	UpdateCallback m_fUpdateCb;

	int m_iObjNum;
	struct list_head m_stUpdateList;
	struct list_head* m_pCurIter;

	struct timeval m_tvUptStartTime;
};

