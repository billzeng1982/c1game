#include <assert.h>
#include "GameTime.h"
#include "LogMacros.h"
#include "GameTime.h"
#include "FakeRandom.h"
#include "Player.h"
#include "PlayerState.h"
#include "PlayerStateMachine.h"
#include "PlayerMgr.h"
#include "PlayerLogic.h"

void PlayerState::ChangeState(Player* poPlayer, int iNewState)
{
	if (!poPlayer)
    {
		assert(false);
		return;
	}

    int iOldSate = poPlayer->GetState();

	LOGRUN("Player(%s) Uin(%lu) Account(%s) change state from %d to %d",
             poPlayer->GetRoleName(), poPlayer->GetUin(), poPlayer->GetAccountName(), iOldSate, iNewState);

    if (iOldSate == iNewState)
    {
        return;
    }

	// 当前状态退出
	this->Exit(poPlayer);

	// 设置新状态
	poPlayer->SetState(iNewState);
	PlayerState* poNewState = PlayerStateMachine::Instance().GetState(iNewState);

	// 进入新状态
	if (poNewState)
    {
		poNewState->Enter(poPlayer);
	}
}

void PlayerState_Login::Enter(Player* poPlayer)
{

}

void PlayerState_Login::Update(Player* poPlayer, int iDeltaTime)
{
    ZONESVRCFG& rzConfig = ZoneSvr::Instance().GetConfig();
    if (CGameTime::Instance().GetCurrSecond() >= (poPlayer->GetPlayerData().GetRoleBaseInfo().m_llLastLoginTime + rzConfig.m_iLoginTimeOut))
    {
        PlayerLogic::Instance().PlayerLogout(poPlayer, PLAYER_LOGOUT_BY_LOGIC, NULL, 1/*超时登出*/);
    }
}

void PlayerState_Login::Exit(Player* poPlayer)
{

}

void PlayerState_InGame::Enter(Player* poPlayer)
{

}

void PlayerState_InGame::Update(Player* poPlayer, int iDeltaTime)
{
    ZONESVRCFG& rstConfig = ZoneSvr::Instance().GetConfig();

    // update to db
	if (CGameTime::Instance().GetCurrSecond() -(int)poPlayer->GetLastUptDbTime() >= rstConfig.m_iPlayerUptDBFreq)
    {
        poPlayer->SetLastUptDbTime(CGameTime::Instance().GetCurrSecond());
		poPlayer->UptRoleDataToDB();
	}
}

void PlayerState_InGame::Exit(Player* poPlayer)
{

}

void PlayerState_Logout::Enter(Player* poPlayer)
{

}

void PlayerState_Logout::Exit(Player* poPlayer)
{

}

void PlayerState_ReconnWait::Enter(Player* poPlayer)
{
	if (!poPlayer)
	{
        LOGERR("poPlayer is null");
        return;
    }

    poPlayer->SetEntryReconnTime(CGameTime::Instance().GetCurrSecond());
}

void PlayerState_ReconnWait::Update(Player* poPlayer, int iDeltaTime)
{
    if (!poPlayer)
	{
        LOGERR("poPlayer is null");
        return;
    }

    ZONESVRCFG& rstConfig = ZoneSvr::Instance().GetConfig();

    // check reconn wait time
    int iTimeDiff = (int)(CGameTime::Instance().GetCurrSecond() - poPlayer->GetEntryReconnTime());
    if (iTimeDiff >= rstConfig.m_iReconnectWaitTime)
    {
        PlayerLogic::Instance().PlayerLogout(poPlayer, PLAYER_LOGOUT_BY_LOGIC);
        return;
    }

    // update to db
	if (CGameTime::Instance().GetCurrSecond() - (int)poPlayer->GetLastUptDbTime() >= rstConfig.m_iPlayerUptDBFreq)
    {
        poPlayer->SetLastUptDbTime(CGameTime::Instance().GetCurrSecond());
		poPlayer->UptRoleDataToDB();
	}
}

void PlayerState_ReconnWait::Exit(Player* poPlayer)
{
	if (!poPlayer)
	{
        LOGERR("poPlayer is null");
        return;
    }

    poPlayer->SetEntryReconnTime(0);
}

void PlayerState_ReconnLogin::Enter(Player* poPlayer)
{
    return;
}

void PlayerState_ReconnLogin::Update(Player* poPlayer, int iDeltaTime)
{
    ZONESVRCFG& rzConfig = ZoneSvr::Instance().GetConfig();
    if (CGameTime::Instance().GetCurrSecond() >= (poPlayer->GetPlayerData().GetRoleBaseInfo().m_llLastLoginTime + rzConfig.m_iLoginTimeOut))
    {
        PlayerLogic::Instance().PlayerLogout(poPlayer, PLAYER_LOGOUT_BY_LOGIC, NULL);
    }
}

void PlayerState_ReconnLogin::Exit(Player* poPlayer)
{
    return;
}



