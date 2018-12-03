#ifndef _TRANSACTION_MGR_H_
#define _TRANSACTION_MGR_H_

#include <time.h>
#include "transaction.h"
#include "list.h"
#include "comm/tmempool.h"
#include "IndexedPriorityQ.h"
#include "og_comm.h"
#include "DynArray.h"
#include <map>

// 内部使用结构
struct TRANSACTION_NODE
{
    TTransactionID    dwTransactionID; // tmempool内复用
    struct timeval    tAddTime;
    struct timeval    tTimeout;
    Transaction*      poTransaction;
};

template<>
class Comparer<TRANSACTION_NODE*>
{
public:
    int Compare( const TRANSACTION_NODE* x, const TRANSACTION_NODE* y )
    {
        if( TvBefore( &x->tTimeout, &y->tTimeout))
        {
            return -1;
        }
        if( x->tTimeout.tv_sec == y->tTimeout.tv_sec &&
			x->tTimeout.tv_usec == y->tTimeout.tv_usec )
        {
            return 0;
        }
        return 1;
    }
};


template<>
class GetItemKey<TRANSACTION_NODE*, TTransactionID>
{
public:
    TTransactionID& GetKey( TRANSACTION_NODE* elem )
    {
        return elem->dwTransactionID;
    }
};


class TransactionFrame;

class CTransactionMgr
{
	const static int UPT_TIMEOUT_Q_PER_LOOP = 20;

	typedef std::map<uint32_t/*transaction id*/, TRANSACTION_NODE*> TransIDMap_t;

public:
	CTransactionMgr();
	~CTransactionMgr();

	bool Init( unsigned int dwMaxTransaction, TransactionFrame* poTransFrame );
	void Fini();
	int Update();
	int  AddTransaction(Transaction* poTransaction);

	int ExecuteTransaction(IN Transaction* poTransaction);
    int WakeTransaction(TActionToken ullToken, const void* pData, unsigned int dwDataLen);

private:
	//int  _CheckTransaction(Transaction* poTransaction);
    void _ReleaseTransaction(TRANSACTION_NODE* pstNode);
    void _ExecuteTransactionInternal(TRANSACTION_NODE* pstNode);
	TTransactionID _AddTransactionWithNoExecute(Transaction* poTransaction);

	void _DelFromTransIDMap( uint32_t dwTransID );
	void _Add2TransIDMap( TRANSACTION_NODE* pstTransNode );
	TRANSACTION_NODE* _FindTransNode( uint32_t dwTransID );

private:
	LPTMEMPOOL m_pstTransactionPool;
	//struct list_head m_stTimeoutList;
	unsigned int   m_dwRunRef;
	TransactionFrame* m_poTransFrame;

	// transaction timeout priority queue
	IndexedPriorityQ< TRANSACTION_NODE*, TTransactionID >* m_pTransTimeoutQ;
	CDynArray<TRANSACTION_NODE*> m_oDelayedDelArr; // 延迟删除

	TransIDMap_t m_oTransIDMap;
	TransIDMap_t::iterator m_oTransIDIter;
	uint32_t m_dwTransIDSeq;
};

#endif

