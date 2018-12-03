#include "FriendTransFrame.h"
#include "GameObjectPool.h"

void ReleaseSimpleAction(IAction* poAction)
{
	if (NULL == poAction)
	{
		return;
	}
	GameObjectPool::Instance().Release((IObject*) poAction);
}

void ReleaseTransactionSelf(Transaction* poTrans)
{
	if (NULL == poTrans)
	{
		return;
	}
	GameObjectPool::Instance().Release((IObject*) poTrans);
}


bool FriendTransFrame::Init(uint32_t dwMaxTransaction, uint32_t dwMaxCompositeAction)
{
	return m_oFrame.Init(dwMaxTransaction, dwMaxCompositeAction, ReleaseSimpleAction, ReleaseTransactionSelf);
}
