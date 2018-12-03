#ifndef _TRANSACTION_FRAME_H_
#define _TRANSACTION_FRAME_H_

#include "TransactionMgr.h"
#include "define.h"
#include "CompositeAction.h"
#include "TransactionErrno.h"
#include "TransactionHelper.h"


class TransactionFrame
{
 	typedef void(*fReleaseSimpleAction_t)(IAction* poAction); // 释放简单action, reset and release
	typedef void(*fReleaseTransactionSelf_t)(Transaction* poTrans); // 释放事物本身, reset and release

public:
	TransactionFrame();
	virtual ~TransactionFrame();

	virtual bool Init( uint32_t dwMaxTransaction, uint32_t dwMaxCompositeAction, 
		fReleaseSimpleAction_t fReleaseSimpleAction, fReleaseTransactionSelf_t fReleaseTransactionSelf );
	void Fini();

	CTransactionMgr& GetTransactionMgr() { return m_oTransactionMgr; }

	// 分配组合动作
	CompositeAction* CreateCompositeAction();

	// 通知回收事务对象及相关资源, 由CTransactionMgr调用
	void OnReleaseTransaction(Transaction* poTransaction);

	void Update() { m_oTransactionMgr.Update(); }

	void ScheduleTransaction( Transaction* poTransaction, uint32_t dwTimeout = 0 );

	/*  异步action完成，由外部逻辑调用, 用于唤醒transaction
		pvData为异步取得的数据, 可能为空
	*/
	void AsyncActionDone( TActionToken ullToken, void* pvData, uint32_t dwDataLen );
	
protected:
	
	// 释放组合动作
	void _ReleaseCompositeAction( CompositeAction* pCompoAction);

private:
	CTransactionMgr m_oTransactionMgr;
	LPTMEMPOOL     	m_pstActionPool;	// CompositeAction

	fReleaseSimpleAction_t 		m_fReleaseSimpleAction;
	fReleaseTransactionSelf_t	m_fReleaseTransactionSelf;
};

#endif

