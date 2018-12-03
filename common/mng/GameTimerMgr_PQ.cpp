#include "GameTimerMgr_PQ.h"
#include "../sys/GameTime.h"
#include <assert.h>


GameTimerMgr_PQ::~GameTimerMgr_PQ()
{
    std::list<GameTimer*>::iterator it;
    for( it = m_oReleasedList.begin(); it != m_oReleasedList.end(); it++ )
    {
        m_fOnReleaseTimer(*it);
    }

    m_oReleasedList.clear();
}


bool GameTimerMgr_PQ::Init( int iInitTimerNum, OnReleaseTimerCb_t fOnReleaseTimer )
{
    assert( iInitTimerNum > 0 );
	m_pPriorityQ = new IndexedPriorityQ<GameTimer*, uint32_t>( iInitTimerNum,
                                                               new Comparer<GameTimer*>(),
                                                               new GetItemKey<GameTimer*, uint32_t>() );
    if( !m_pPriorityQ )
    {
        return false;
    }

	m_fOnReleaseTimer = fOnReleaseTimer;

    return true;
}

void GameTimerMgr_PQ::Update()
{
    int iCounter = 0;
    struct timeval* pCurTime =  CGameTime::Instance().GetCurrTime();
    GameTimer* pTimer = NULL;

    // 清理released list
    while( !m_oReleasedList.empty() )
    {
        pTimer = m_oReleasedList.front();
        if(pTimer->GetObjID() > 0 )
            m_fOnReleaseTimer(pTimer);
        m_oReleasedList.pop_front();
    }

    while( !m_pPriorityQ->Empty() && iCounter < MAX_DEAL_NUM_PER_FRAME )
    {
        pTimer = m_pPriorityQ->Top();
        if( pTimer->GetObjID() == 0 )
        {
            assert(false);
            m_pPriorityQ->Pop();
            continue;
        }

        if( CmpTimeVal( pCurTime, pTimer->GetFireTime() ) >= 0 )
        {
            // 触发定时器, 注意 OnTimeout()逻辑可能调用DelTimer()
            pTimer->OnTimeout( );  // DelTime set flag true
            if( pTimer->GetDelFlag() )
            {
                continue;
            }

            ++pTimer->m_iFireCount;

            //是否指定了下次触发时间
            if( 0 == pTimer->m_iMaxFireCount || pTimer->m_iFireCount < pTimer->m_iMaxFireCount )
            {
                assert( pTimer->m_dwPeriodMs > 0 );

                TvAddMs( &pTimer->m_tvFireTime, pTimer->m_dwPeriodMs );
                m_pPriorityQ->Update(0);
            }
            else
            {
                // 定时器逻辑finish, 弹出定时器
                m_pPriorityQ->Pop();
                if( pTimer->GetObjID() > 0 )
				{
				    m_fOnReleaseTimer(pTimer);
                }
            }

            iCounter++;
        }
        else
        {
            break;
        }
    }
}

// pTimer需要在外面设置好
void GameTimerMgr_PQ::AddTimer( GameTimer* pTimer )
{
    if( !pTimer )
    {
        assert( false );
        return;
    }

    m_pPriorityQ->Push( pTimer );
}

GameTimer* GameTimerMgr_PQ::FindTimer( uint32_t dwTimeID )
{
    int iPos = m_pPriorityQ->Find(dwTimeID);
    if( iPos < 0 )
        return NULL;
    return (*m_pPriorityQ)[iPos];
}

// 外部函数调用, 注意延迟回收
void GameTimerMgr_PQ::DelTimer( uint32_t dwTimeID )
{
    GameTimer* pTimer = this->FindTimer( dwTimeID );
    if( !pTimer )
    {
        return;
    }

    m_pPriorityQ->Erase( dwTimeID );
    if( pTimer->GetObjID() > 0 )
    {
        this->_Push2ReleasedList( pTimer );
    }
}

void GameTimerMgr_PQ::_Push2ReleasedList( GameTimer* pTimer)
{
    pTimer->SetDelFlag(true);
    m_oReleasedList.push_back( pTimer );
}

void GameTimerMgr_PQ::ModTimerPeriod( uint32_t dwTimeID, uint32_t dwPeroidMs )
{
    GameTimer* pTimer = this->FindTimer( dwTimeID );
    if( !pTimer )
    {
        return;
    }

    pTimer->m_dwPeriodMs = dwPeroidMs;
}

void GameTimerMgr_PQ::ModTimerFireTime( uint32_t dwTimeID, struct timeval& tvFireTime )
{
    int iPos = m_pPriorityQ->Find(dwTimeID);
    if( iPos < 0 )
    {
        return;
    }

    (*m_pPriorityQ)[iPos]->m_tvFireTime = tvFireTime;
    m_pPriorityQ->Update(iPos);

    return;
}


void GameTimerMgr_PQ::ModTimerMaxFireCount( uint32_t dwTimeID, int iCount )
{
    GameTimer* pTimer = this->FindTimer( dwTimeID );
    if( !pTimer )
    {
        return;
    }

    pTimer->m_iMaxFireCount = iCount;
}


