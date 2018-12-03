#include "TransactionFrame.h"
#include "CompositeAction.h"
#include "LogMacros.h"

TransactionFrame::TransactionFrame()
{
    m_pstActionPool = NULL;
    m_fReleaseSimpleAction = NULL;
    m_fReleaseTransactionSelf = NULL;
}

TransactionFrame::~TransactionFrame()
{
    this->Fini();
}


bool TransactionFrame::Init( uint32_t dwMaxTransaction, uint32_t dwMaxCompositeAction,
		fReleaseSimpleAction_t fReleaseSimpleAction, fReleaseTransactionSelf_t fReleaseTransactionSelf )
{
    if( NULL == fReleaseSimpleAction || NULL == fReleaseTransactionSelf )
    {
        assert( false );
        return false;
    }

    m_fReleaseSimpleAction      = fReleaseSimpleAction;
    m_fReleaseTransactionSelf   = fReleaseTransactionSelf;

    if( !m_oTransactionMgr.Init(dwMaxTransaction, this) )
    {
        return false;
    }

    int iRet = tmempool_new(&m_pstActionPool, dwMaxCompositeAction, sizeof(CompositeAction));
    if( iRet )
    {
        printf("create composite action pool failed\n");
        return false;
    }

    return true;
}


void TransactionFrame::Fini()
{
    m_oTransactionMgr.Fini();

    if(m_pstActionPool)
    {
        tmempool_destroy(&m_pstActionPool);
        m_pstActionPool = NULL;
    }
}


void TransactionFrame::OnReleaseTransaction(Transaction* poTransaction)
{
    int iActionCount = poTransaction->GetActionCount();
    for( int i = 0; i < iActionCount; i++ )
    {
        IAction* poAction = poTransaction->GetAction(i);
        assert( poAction );
        if( poAction->ActionType() == ACTION_TYPE_COMPOSITE )
        {
            this->_ReleaseCompositeAction( dynamic_cast<CompositeAction*>(poAction) );
        }else
        {
            this->m_fReleaseSimpleAction(poAction);
        }
    }

    this->m_fReleaseTransactionSelf(poTransaction);
}

// 释放组合动作
void TransactionFrame::_ReleaseCompositeAction(CompositeAction* poCompoAction)
{
    if( !poCompoAction )
    {
        return;
    }

    int iActionCount = poCompoAction->GetActionCount();
    for( int i = 0; i < iActionCount; i++ )
    {
        IAction* poAction = poCompoAction->GetAction(i);
        assert(poAction);
        this->m_fReleaseSimpleAction(poAction);
    }

    poCompoAction->Reset();
    tmempool_free_byptr( m_pstActionPool, poCompoAction );

    //释放组合动作
    this->m_fReleaseSimpleAction((IAction*)poCompoAction);
}


// 分配组合动作
CompositeAction* TransactionFrame::CreateCompositeAction()
{
    if(m_pstActionPool)
    {
        int iIdx = tmempool_alloc(m_pstActionPool);
        if(iIdx < 0)
        {
            printf("action pool is used up, allocate action failed!\n");
            return NULL;
        }

        void* pNode = (CompositeAction*)tmempool_get(m_pstActionPool, iIdx);
        if(!pNode)
        {
            printf("get action from pool failed, idx=%d!\n", iIdx);
            tmempool_free(m_pstActionPool, iIdx);
            return NULL;
        }

        CompositeAction* pCompoAction = new(pNode) CompositeAction(); // replacement new 调用一下构造函数

        return pCompoAction;
    }

    return NULL;
}


void TransactionFrame::ScheduleTransaction( Transaction* poTransaction, uint32_t dwTimeout )
{
    if( !poTransaction )
    {
        assert( false );
        return;
    }

    if( 0 == dwTimeout )
    {
        poTransaction->SetTimeout( DEFAULT_TRANC_TIME_OUT );
    }else
    {
        poTransaction->SetTimeout( dwTimeout );
    }

    if( m_oTransactionMgr.AddTransaction(poTransaction) != 0 )
    {
        // add failed!
        LOGERR("schedule transaction<%s> failed!", poTransaction->GetTransactionName());
        return;
    }
}

/*  异步action完成，由外部逻辑调用, 用于唤醒transaction
	pvData为异步取得的数据, 可能为空
*/
void TransactionFrame::AsyncActionDone( TActionToken ullToken, void* pvData, uint32_t dwDataLen )
{
    m_oTransactionMgr.WakeTransaction( ullToken,  pvData, dwDataLen );
}

