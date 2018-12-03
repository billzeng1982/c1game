#ifndef _TRANSACTION_FRAME_H_
#define _TRANSACTION_FRAME_H_

#include "TransactionMgr.h"
#include "define.h"
#include "CompositeAction.h"
#include "TransactionErrno.h"
#include "TransactionHelper.h"


class TransactionFrame
{
 	typedef void(*fReleaseSimpleAction_t)(IAction* poAction); // �ͷż�action, reset and release
	typedef void(*fReleaseTransactionSelf_t)(Transaction* poTrans); // �ͷ����ﱾ��, reset and release

public:
	TransactionFrame();
	virtual ~TransactionFrame();

	virtual bool Init( uint32_t dwMaxTransaction, uint32_t dwMaxCompositeAction, 
		fReleaseSimpleAction_t fReleaseSimpleAction, fReleaseTransactionSelf_t fReleaseTransactionSelf );
	void Fini();

	CTransactionMgr& GetTransactionMgr() { return m_oTransactionMgr; }

	// ������϶���
	CompositeAction* CreateCompositeAction();

	// ֪ͨ����������������Դ, ��CTransactionMgr����
	void OnReleaseTransaction(Transaction* poTransaction);

	void Update() { m_oTransactionMgr.Update(); }

	void ScheduleTransaction( Transaction* poTransaction, uint32_t dwTimeout = 0 );

	/*  �첽action��ɣ����ⲿ�߼�����, ���ڻ���transaction
		pvDataΪ�첽ȡ�õ�����, ����Ϊ��
	*/
	void AsyncActionDone( TActionToken ullToken, void* pvData, uint32_t dwDataLen );
	
protected:
	
	// �ͷ���϶���
	void _ReleaseCompositeAction( CompositeAction* pCompoAction);

private:
	CTransactionMgr m_oTransactionMgr;
	LPTMEMPOOL     	m_pstActionPool;	// CompositeAction

	fReleaseSimpleAction_t 		m_fReleaseSimpleAction;
	fReleaseTransactionSelf_t	m_fReleaseTransactionSelf;
};

#endif

