#include "AsyncPvpTransFrame.h"
#include "GameObjectPool.h"

using namespace PKGMETA;

void ReleaseSimpleAction(IAction* poAction)
{
    poAction->Reset();
    RELEASE_GAMEOBJECT(poAction);
}

void ReleaseTransaction(Transaction* poTrans)
{
    poTrans->Reset();
    RELEASE_GAMEOBJECT(poTrans);
}

bool AsyncPvpTransFrame::Init(uint32_t dwMaxTransaction, uint32_t dwMaxCompositeAction)
{
    return m_oFrame.Init(dwMaxTransaction, dwMaxCompositeAction, ReleaseSimpleAction, ReleaseTransaction);
}

