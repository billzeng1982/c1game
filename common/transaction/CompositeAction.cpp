#include "CompositeAction.h"
#include "TransactionHelper.h"
#include "TransactionErrno.h"
#include <string.h>
#include "LogMacros.h"

CompositeAction::CompositeAction()
{
    this->Reset();
}

CompositeAction::~CompositeAction()
{
}

void CompositeAction::Reset()
{
    memset(m_aActionArray, 0, sizeof(m_aActionArray));
    m_dwTimeout = 0;
    m_iActionCount = 0;

    IAction::Reset();
}

//事务动作是否结束
int CompositeAction::FiniFlag( )
{
    for( int i = 0; i < m_iActionCount; i++ )
    {
        IAction* poAction = m_aActionArray[i];

        int iFiniFlag = poAction->FiniFlag();
        if( 0 == iFiniFlag )
        {
            return 0;
        }
        if( iFiniFlag < 0 )
        {
            // errer occurs
            m_iFiniFlag = iFiniFlag;
            return m_iFiniFlag;
        }
    }

    m_iFiniFlag = 1;
    return m_iFiniFlag;

#if 0
    if( m_dwFirstIdx != (TActionIndex)-1 )
    {
        int iRet = 0;
        for( TActionIndex i = m_dwFirstIdx; i <= m_dwLastIdx; ++i )
        {
            IAction* poAction = m_aActionArray[i - m_dwFirstIdx];
            if(poAction)
            {
                iRet = poAction->ErrCode();
                if(!iRet)
                {
                    return 0;
                }
            }
        }
    }

    return 1;
#endif
}


/*执行事务动作
  返回值：
   1=所有执行已结束，无需等待执行结果
   0=同步执行结束，需等待异步结果返回
*/
int CompositeAction::Execute( Transaction* poTrans )
{
    int iRet = 1;
    int iSubRet = 0;

    for( int i = 0; i < m_iActionCount; i++ )
    {
        IAction* poAction = m_aActionArray[i];
        if( poAction )
        {
            iSubRet = poAction->Execute( poTrans );
            if( iSubRet == 0 )
            {
                iRet = 0; // 需等待异步结果返回
            }
        }
    }

    return iRet;
}


//处理事务动作结果
void CompositeAction::OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen)
{
    if(iActionIdx < 0)
    {
        return;
    }

    if( iActionIdx < m_iActionCount && m_aActionArray[iActionIdx])
    {
        m_aActionArray[iActionIdx]->OnAsyncRspMsg(iActionIdx, pResult, dwResLen);
    }
}


int CompositeAction::AddAction(IAction* pSubAction)
{
    if(!pSubAction)
    {
        assert( false );
        return TRANC_ERR_OBJ_ISNULL;
    }

    if( m_iActionCount >= MAX_COMPOSITE_ACTIONS )
    {
        return TRANC_ERR_COMPOSITEACTION_FULL;
    }

    if( pSubAction->GetTimeout() > m_dwTimeout )
    {
        m_dwTimeout = pSubAction->GetTimeout();
    }

    TActionIndex iIndex = m_iActionCount;
    pSubAction->SetIndex( iIndex );
    m_aActionArray[m_iActionCount++] = pSubAction;

    printf("CompositeAction add action id=(%d) name=(%s) token=(%lu)\n",  iIndex, pSubAction->GetActionName(), pSubAction->GetToken());

    return iIndex;
}


IAction* CompositeAction::GetAction(TActionIndex iActionIdx)
{
    if( iActionIdx < 0 )
    {
        assert( false );
        return NULL;
    }

    if( iActionIdx > m_iActionCount )
    {
        return NULL;
    }

    return m_aActionArray[ iActionIdx ];

#if 0
    if( iActionIdx!=(TActionIndex)-1 &&
        iActionIdx >= m_dwFirstIdx &&
        iActionIdx <= m_dwLastIdx )
    {
        return m_aActionArray[iActionIdx - m_dwFirstIdx];
    }

    return NULL;
#endif
}

