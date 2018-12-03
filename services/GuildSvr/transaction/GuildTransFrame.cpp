#include "GuildTransFrame.h"
#include "../framework/GameObjectPool.h"

using namespace PKGMETA;

void ReleaseSimpleAction(IAction* poAction)
{
    RELEASE_GAMEOBJECT(poAction);
}

void ReleaseTransactionSelf(Transaction* poTrans)
{
    RELEASE_GAMEOBJECT(poTrans);
}


bool GuildTransFrame::Init(uint32_t dwMaxTransaction, uint32_t dwMaxCompositeAction)
{
    return m_oFrame.Init(dwMaxTransaction, dwMaxCompositeAction, ReleaseSimpleAction, ReleaseTransactionSelf);
}


void GuildTransFrame::AddCreateGuildTrans(uint64_t ullPlayerId, const char* pszGuildName, SSPKG& rstSsPkg)
{
    GuildTransaction* poGuildTransaction = GameObjectPool::Instance().GETOBJ_GetGuildTransaction();

    GetPlayerAction* poGetPlayerAction = GameObjectPool::Instance().GETOBJ_GetPlayerAction();
    poGetPlayerAction->SetPlayerId(ullPlayerId);
    poGuildTransaction->AddAction(poGetPlayerAction);

    CreateGuildAction* poCreateGuildAction = GameObjectPool::Instance().GETOBJ_CreateGuildAction();
    poCreateGuildAction->SetGuildName(pszGuildName);
    poGuildTransaction->AddAction(poCreateGuildAction);

    poGuildTransaction->SaveReqPkg(rstSsPkg);

    this->ScheduleTransaction(poGuildTransaction);
}


void GuildTransFrame::AddGetGuildTrans(uint64_t ullPlayerId, uint64_t ullGuildId, SSPKG& rstSsPkg)
{
    GuildTransaction* poGuildTransaction = GameObjectPool::Instance().GETOBJ_GetGuildTransaction();

    if (ullPlayerId != 0)
    {
        GetPlayerAction* poGetPlayerAction = GameObjectPool::Instance().GETOBJ_GetPlayerAction();
        poGetPlayerAction->SetPlayerId(ullPlayerId);
        poGuildTransaction->AddAction(poGetPlayerAction);
    }

    GetGuildAction* poGetGuildAction = GameObjectPool::Instance().GETOBJ_GetGuildAction();
    poGetGuildAction->SetGuildId(ullGuildId);
    poGuildTransaction->AddAction(poGetGuildAction);

    poGuildTransaction->SaveReqPkg(rstSsPkg);

    this->ScheduleTransaction(poGuildTransaction);
}


void GuildTransFrame::AddRstPlayerTrans(uint64_t ullPlayerId)
{
    GuildTransaction* poGuildTransaction = GameObjectPool::Instance().GETOBJ_GetGuildTransaction();

    GetPlayerAction* poGetPlayerAction = GameObjectPool::Instance().GETOBJ_GetPlayerAction();
    poGetPlayerAction->SetPlayerId(ullPlayerId);
    poGuildTransaction->AddAction(poGetPlayerAction);

    poGuildTransaction->SetMsgId(SS_MSG_GUILD_PLAYER_RESET);

    this->ScheduleTransaction(poGuildTransaction);
}


void GuildTransFrame::AddGetGuildByNameTrans(char* pszName, SSPKG& rstSsPkg)
{
    GuildTransaction* poGuildTransaction = GameObjectPool::Instance().GETOBJ_GetGuildTransaction();

    GetGuildByNameAction* poGetGuildAction = GameObjectPool::Instance().GETOBJ_GetGuildByNameAction();
    poGetGuildAction->SetGuildName(pszName);
    poGuildTransaction->AddAction(poGetGuildAction);

    poGuildTransaction->SaveReqPkg(rstSsPkg);

    this->ScheduleTransaction(poGuildTransaction);
}

