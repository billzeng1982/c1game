/*
     TODO: transaction id必须序列化，消除隐藏bug
*/

#include <stdio.h>
#include <assert.h>
#include "og_comm.h"
#include "GameTime.h"
#include "TransactionFrame.h"
#include <string.h>
#include "LogMacros.h"

#define UPT_TIME_OUT_TRANC_NUM 100

CTransactionMgr::CTransactionMgr()
{
    m_pstTransactionPool = NULL;
    //INIT_LIST_HEAD(&m_stTimeoutList);
    m_dwRunRef = 0;
    m_poTransFrame = NULL;
    m_pTransTimeoutQ = NULL;
    m_dwTransIDSeq = 0;
}


CTransactionMgr::~CTransactionMgr()
{
    this->Fini();
}


bool CTransactionMgr::Init( unsigned int dwMaxTransaction, TransactionFrame* poTransFrame )
{
    if( 0 == dwMaxTransaction || !poTransFrame )
    {
        assert( false );
        return false;
    }

    int iRet = tmempool_new( &m_pstTransactionPool, dwMaxTransaction, sizeof( TRANSACTION_NODE ) );
    if( iRet )
    {
        LOGERR("create transaction pool failed!" );
        return false;
    }

    m_pTransTimeoutQ = new IndexedPriorityQ< TRANSACTION_NODE*, TTransactionID >( dwMaxTransaction, new Comparer<TRANSACTION_NODE*>(), new GetItemKey<TRANSACTION_NODE*, TTransactionID> );
    if( !m_pTransTimeoutQ )
    {
        LOGERR( "create transaction timeout queue failed!" );
        return false;
    }

    m_poTransFrame = poTransFrame;

    return true;
}


void CTransactionMgr::Fini()
{
    if( m_pstTransactionPool )
    {
        while ( !m_pTransTimeoutQ->Empty() )
        {
            TRANSACTION_NODE* pstTransNode = m_pTransTimeoutQ->Top();
            m_pTransTimeoutQ->Pop();
            this->_ReleaseTransaction(pstTransNode);
        }

        m_pTransTimeoutQ->Clear();
        delete m_pTransTimeoutQ;
        m_pTransTimeoutQ = NULL;

        tmempool_destroy(&m_pstTransactionPool);
        m_pstTransactionPool = NULL;
    }

    m_oTransIDMap.clear();
}


int CTransactionMgr::Update()
{
    while( !m_oDelayedDelArr.Empty() )
    {
        TRANSACTION_NODE* pstTransNode = m_oDelayedDelArr.Back();
        this->_ReleaseTransaction(pstTransNode);
        m_oDelayedDelArr.PopBack();
    }

    ++m_dwRunRef;
    if( !m_pstTransactionPool || m_dwRunRef < 2 ) // 每两帧计算下
    {
        return 0;
    }

    m_dwRunRef = 0;
    int iCount = 0;

    //检查事务是否超时
    while( !m_pTransTimeoutQ->Empty() && iCount < UPT_TIMEOUT_Q_PER_LOOP )
    {
        TRANSACTION_NODE* pstTransNode = m_pTransTimeoutQ->Top();
        if( TvBefore( &pstTransNode->tTimeout, CGameTime::Instance().GetCurrTime() ) )
        {
            // timeout
            ++iCount;
            pstTransNode->poTransaction->OnFinished( TRANC_ERR_TIMEOUT );

            m_pTransTimeoutQ->Pop();
            this->_ReleaseTransaction(pstTransNode);
        }
        else
        {
            break;
        }
    }

    return iCount;
}


int CTransactionMgr::AddTransaction(Transaction* poTransaction)
{
    assert( poTransaction );
    TTransactionID dwTransactionID = this->_AddTransactionWithNoExecute(poTransaction);
    if( 0 == dwTransactionID )
    {
        //根据_AddTransactionWithNoExecute()的函数体内容，dwTransactionID为0表示invalid id.
        LOGERR("add transactionfailed!");
        return -1;
    }

	// 设置所有action的token
    poTransaction->SetActionsToken();

    int iRet = this->ExecuteTransaction(poTransaction);
    if( 0 != iRet )
    {
        LOGERR("ExecuteTransaction failed with ret(%d)", iRet);
        return -2;
    }

    return 0;
}


TTransactionID CTransactionMgr::_AddTransactionWithNoExecute(Transaction* poTransaction)
{
    assert( poTransaction );

 /*   int iRet = this->_CheckTransaction( poTransaction );
    if(iRet < 0 )
    {
        printf("add transaction failed -- check transaction obj, ret=%d\n",iRet);
        return 0;
    }*/

    //为新事务分配内存池节点
    int iNodeIdx = tmempool_alloc( m_pstTransactionPool );
    if( iNodeIdx < 0 )
    {
        printf("add transaction failed -- transaction pool is full\n");
        return 0; // invalid id
    }

    TRANSACTION_NODE* pstNode = (TRANSACTION_NODE*)tmempool_get(m_pstTransactionPool, iNodeIdx);
    if( !pstNode )
    {
        assert( false );
        return 0;
    }

    //初始化内存池节点
    m_dwTransIDSeq++;
    if( 0 == m_dwTransIDSeq ) m_dwTransIDSeq = 1;
    TTransactionID dwTransactionID = m_dwTransIDSeq;
    poTransaction->SetTransactionID(dwTransactionID);
    pstNode->dwTransactionID = dwTransactionID;
    pstNode->tAddTime = *(CGameTime::Instance().GetCurrTime());
    pstNode->tTimeout = *(TvAddMs( &pstNode->tAddTime, poTransaction->GetTimeout()));
    pstNode->poTransaction = poTransaction;

    m_pTransTimeoutQ->Push( pstNode );

    this->_Add2TransIDMap( pstNode );

    return dwTransactionID;
}


int CTransactionMgr::ExecuteTransaction(IN Transaction* poTransaction)
{
    if(NULL == poTransaction)
    {
        return -1;
    }

    TTransactionID dwTransactionID = poTransaction->GetTransactionID();

    TRANSACTION_NODE* pstNode = this->_FindTransNode(dwTransactionID);
    if( !pstNode )
    {
        //事务未找到
        printf("Transaction does Not exist, TransactionID=%u.\n", dwTransactionID);
        return -2;
    }

    //执行事务的第一个动作
    this->_ExecuteTransactionInternal(pstNode);

    return 0;
}

/*
    异步Action完成唤醒事务
    需要区分 action是普通action, 还是composite action
*/
int CTransactionMgr::WakeTransaction(TActionToken ullToken, const void* pData, unsigned int dwDataLen)
{
    //if( NULL == pData || 0 == dwDataLen )
    //{
    //    return -1;
    //}

    TTransactionID dwTransactionID = 0;
    TActionIndex iCompoActionID = 0;
    TActionIndex iActionID = 0;
    TransactionHelper::ParseActionToken(ullToken, dwTransactionID, iCompoActionID, iActionID);

    printf("Token wake transaction. transactionid=%u, actionidx=%u, token=%lu\n",
           dwTransactionID, iActionID, ullToken);

    TRANSACTION_NODE* pstNode = this->_FindTransNode(dwTransactionID);
    if( !pstNode )
    {
        //事务未找到
        LOGERR("wake transaction failed because transaction does Not exist. transactionid=%u, actionidx=%u, token=%lu, datalen=%u.\n",
            dwTransactionID, iActionID, ullToken, dwDataLen );
        return -2;
    }

    Transaction* poTransaction = pstNode->poTransaction;

    IAction* poAction = NULL;
    if( iCompoActionID > 0 )
    {
        // 处理组合action的子action
        poAction = poTransaction->GetAction( ID2Index(iCompoActionID) );
        if( !poAction )
        {
            m_oDelayedDelArr.PushBack(pstNode);
            return -3;
        }

        CompositeAction* poCompoAction = dynamic_cast<CompositeAction*>(poAction);
        if( !poCompoAction )
        {
            assert(false);
            m_oDelayedDelArr.PushBack(pstNode);
            return -3;
        }

        IAction* poSubAction = poCompoAction->GetAction( ID2Index(iActionID) );
        if( poSubAction && poSubAction->FiniFlag() == 0 )
        {
            //事务动作处理异步返回数据
            poSubAction->OnAsyncRspMsg( ID2Index(iActionID), pData, dwDataLen );
        }

    }
    else
    {
        poAction = poTransaction->GetAction( ID2Index(iActionID) );
        if( !poAction )
        {
            LOGERR("wake transaction failed --"
                " action is not existed, transactionid=%u, actionidx=%u, datalen=%u"
                , dwTransactionID, ID2Index(iActionID), dwDataLen );
            m_oDelayedDelArr.PushBack(pstNode);
            return -3;
        }

        if( poAction->FiniFlag() == 0 )
        {
            //事务动作处理异步返回数据
            poAction->OnAsyncRspMsg( ID2Index(iActionID), pData, dwDataLen );
        }
    }

    if( poAction->FiniFlag() == 1 )
    {
        poAction->OnFinished();

        poTransaction->OnActionFinished(poAction->GetIndex());

        //检查事务是否结束
        if( poTransaction->IsFinished())
        {
            LOGERR("release transaction"
                " -- transaction is finished, transactionid=%u, actionidx=%u"
                , dwTransactionID, ID2Index(iActionID));

            poTransaction->OnFinished(TRANC_ERR_SUCCESS);
            m_oDelayedDelArr.PushBack(pstNode);
        }
        else
        {
            poTransaction->NextStep();
            //执行下一个动作
            this->_ExecuteTransactionInternal(pstNode);
        }
    }
    else if( poAction->FiniFlag() < 0 )
    {
        //error ocurrs
        poTransaction->OnFinished(TRANC_ERR_ACTION_ERR);
        m_oDelayedDelArr.PushBack(pstNode);
    }

    return 0;
}


void CTransactionMgr::_ReleaseTransaction(TRANSACTION_NODE* pstNode)
{
    Transaction* poTransaction = pstNode->poTransaction;

    if( !poTransaction )
    {
        return;
    }

    m_pTransTimeoutQ->Erase( pstNode->dwTransactionID );
    this->_DelFromTransIDMap( pstNode->dwTransactionID );

    m_poTransFrame->OnReleaseTransaction(poTransaction);

    memset(pstNode, 0, sizeof(TRANSACTION_NODE));
    tmempool_free_byptr(m_pstTransactionPool, pstNode);
}


void CTransactionMgr::_ExecuteTransactionInternal(TRANSACTION_NODE* pstNode)
{
    assert(pstNode);
    Transaction* poTransaction = pstNode->poTransaction;
    TTransactionID dwTransactionID = poTransaction->GetTransactionID();
    IAction* poAction = NULL;

    //TActionIndex dwCurIdx = 0;
    //TActionIndex dwStartIdx = pstNode->dwNextActionIdx;

    // 检查事物是否结束
    if (poTransaction->IsFinished())
    {
        printf("release transaction -- transaction is finished, transactionid=%u, actionidx=%u\n", dwTransactionID, poTransaction->CurStep());
        poTransaction->OnFinished(TRANC_ERR_SUCCESS);
        m_oDelayedDelArr.PushBack(pstNode);
        return;
    }

    do
    {
        //取得下一个要执行的事务动作
        //dwCurIdx = pstNode->dwNextActionIdx;
        poAction = poTransaction->GetAction(poTransaction->CurStep());

        if(!poAction)
        {
            //无下一个事务动作则认为事务结束
            poTransaction->OnFinished(TRANC_ERR_SUCCESS);
            m_oDelayedDelArr.PushBack(pstNode);
            break;
        }

        //执行事务动作
        int iRet = poAction->Execute(poTransaction);
        if( iRet == 0 )
        {
            printf("execute transaction and hang up, transactionid=%u, action id=%u, token=%lu, name=%s, step=%u\n",
                 dwTransactionID, poAction->GetIndex()+1, poAction->GetToken(), poAction->GetActionName(), poTransaction->CurStep());
            //需要等待执行结果, 异步!
            //printf("execute transaction and hang up, "
            //    "nActionToken=%"PRIu64", transactionid=%u, actionidx=%u, poAction=%p, step=%u\n",
            //    nActionToken, dwTransactionID, dwCurIdx, poAction, dwStep);

            break;
        }
        else if( iRet == 1 )
        {
            //无需等待执行结果
            poAction->SetFiniFlag(1);
            poAction->OnFinished();
            poTransaction->OnActionFinished(poTransaction->CurStep());

            printf("execute transaction and go to next step, transactionid=%u, action id=%u, token=%lu, name=%s, step=%u\n",
                 dwTransactionID, poAction->GetIndex()+1, poAction->GetToken(), poAction->GetActionName(), poTransaction->CurStep());

            poTransaction->NextStep();

            if( poTransaction->IsFinished() )
            {
                printf("release transaction"
                    " -- transaction is finished, transactionid=%u, actionidx=%u\n"
                    , dwTransactionID, poTransaction->CurStep());

                poTransaction->OnFinished(TRANC_ERR_SUCCESS);
                m_oDelayedDelArr.PushBack(pstNode);
                break;
            }
        }else
        {
            //error occurs
            LOGERR("Action execute error!");
            poTransaction->OnFinished( TRANC_ERR_ACTION_ERR );
            m_oDelayedDelArr.PushBack(pstNode);
            break;
        }
    } while ( true );
}


void CTransactionMgr::_DelFromTransIDMap( uint32_t dwTransID )
{
    m_oTransIDIter = m_oTransIDMap.find(dwTransID);
	if (m_oTransIDIter != m_oTransIDMap.end())
    {
		m_oTransIDMap.erase(m_oTransIDIter);
	}
}

void CTransactionMgr::_Add2TransIDMap( TRANSACTION_NODE* pstTransNode )
{
    assert(pstTransNode);
	m_oTransIDMap.insert( TransIDMap_t::value_type( pstTransNode->dwTransactionID, pstTransNode) );
}

TRANSACTION_NODE* CTransactionMgr::_FindTransNode( uint32_t dwTransID )
{
    m_oTransIDIter = m_oTransIDMap.find(dwTransID);
	if (m_oTransIDIter != m_oTransIDMap.end())
    {
		return m_oTransIDIter->second;
	}
	return NULL;
}

