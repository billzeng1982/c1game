#include "LogMacros.h"
#include "PlayerLogic.h"
#include "PlayerMgr.h"
#include "PlayerStateMachine.h"
#include "dwlog_svr.h"
#include "../../ZoneSvr.h"
#include "../../framework/ZoneSvrMsgLayer.h"
#include "../ZoneLog.h"
#include "../FightPVP.h"
#include "../Match.h"
#include "../Guild.h"
#include "../Mail.h"
#include "../Friend.h"
#include "../RankMgr.h"
#include "../PvpRoomMgr.h"
#include "../AsyncPvp.h"

using namespace PKGMETA;
using namespace DWLOG;


/*
 *  正常登出处理逻辑
 *      掉线时会在重连失败时调用,不是及时调用的
 **/
void PlayerLogic::PlayerLogout(Player* poPlayer, int iLogoutReason, PKGMETA::SCPKG* pstScPkg, uint8_t bIsTimeOut)
{
    if (!poPlayer)
    {
        LOGERR("player logout, poPlayer is null");
        return;
    }

    this->LogoutDo(poPlayer);

    if (poPlayer->IsCurState(PLAYER_STATE_INGAME)
        || poPlayer->IsCurState(PLAYER_STATE_RECONN_WAIT)
        || poPlayer->IsCurState(PLAYER_STATE_RECONN_LOGIN)
        || poPlayer->IsCurState(PLAYER_STATE_LOGOUT))
    {
        poPlayer->UptRoleDataToDB();
    }

    if (PLAYER_LOGOUT_BY_LOGIC == iLogoutReason)
    {
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, pstScPkg, PKGMETA::CONNSESSION_CMD_STOP);
    }

    // change state
    PlayerStateMachine::Instance().ChangeState(poPlayer, PLAYER_STATE_LOGOUT);

    // write log
	if (bIsTimeOut)
	{
		//超时退出
		//ZoneLog::Instance().WriteLogoutLog(poPlayer, METHOD_LOGOUT_TIME_OUT);
	}
	else
	{
		ZoneLog::Instance().WriteLogoutLog(poPlayer, METHOD_LOGOUT_IN_GAME);
	}

    LOGRUN("Player(%s) Uin(%lu) Logout, Reason=(%d), after logout, current player num : (%d)",
            poPlayer->GetRoleName(), poPlayer->GetUin(), iLogoutReason, PlayerMgr::Instance().GetUsedNum() -1);

    PlayerMgr::Instance().DeletePlayer(poPlayer);

    return;
}

void PlayerLogic::OnSessionStop(Player* poPlayer, char chStopReason)
{
    if (!poPlayer)
    {
        LOGERR("poPlayer is null");
        return;
    }

    LOGRUN("Player(%s) Uin(%lu) session stop, reason %d", poPlayer->GetRoleName(), poPlayer->GetUin(), (int)chStopReason);

    ZONESVRCFG& rstConfig = ZoneSvr::Instance().GetConfig();
    if (rstConfig.m_iReconnectWaitTime > 0)
    {
        if (poPlayer->IsCurState(PLAYER_STATE_INGAME))
        {
            this->OfflineDo(poPlayer);

            PlayerStateMachine::Instance().ChangeState(poPlayer, PLAYER_STATE_RECONN_WAIT);
        }
    }
    else
    {
        PlayerLogout(poPlayer, chStopReason, NULL);
    }
}

/*
 *  如果重复处理会引起数据异常,请不加在此函数中
 **/
void PlayerLogic::OfflineDo(Player* poPlayer)
{
    if (poPlayer->GetUin() == 0 || poPlayer->GetState() == PLAYER_STATE_LOGIN)
    {
        return;
    }

    // send match cancel
    Match::Instance().Cancel(&poPlayer->GetPlayerData(), poPlayer->GetPlayerData().m_bMatchState);

    //quit room
    PvpRoomMgr::Instance().QuitRoom(&poPlayer->GetPlayerData());
}

void PlayerLogic::LogoutDo(Player* poPlayer)
{
    if (poPlayer->GetUin() == 0 || poPlayer->GetState() == PLAYER_STATE_LOGIN)
    {
        return;
    }

    this->OfflineDo(poPlayer);

    PlayerData& rstPlayerData = poPlayer->GetPlayerData();
    time_t startTime = rstPlayerData.GetMiscInfo().m_ullGrowthAwardOnlineLastTime == 0
        ? rstPlayerData.GetRoleBaseInfo().m_llLastLoginTime : rstPlayerData.GetMiscInfo().m_ullGrowthAwardOnlineLastTime;

    time_t finishTime = CGameTime::Instance().GetCurrSecond();

    rstPlayerData.GetMiscInfo().m_dwGrowthAwardOnlineTime += uint32_t(finishTime - startTime);

    //下线更新战力
	RankMgr::Instance().UpdateLi(&poPlayer->GetPlayerData());

    //下线更新异步pvp阵容
    AsyncPvp::Instance().UptToAsyncSvr(&poPlayer->GetPlayerData());

    //通知FriendSvr
    Friend::Instance().UpdatePlayerInfo(&poPlayer->GetPlayerData(), 0);

    // notify mail
    Mail::Instance().SendPlayerStatToMailSvr(poPlayer->GetUin(), PLAYER_STATE_LOGOUT);

    //发送消息给GuildSvr
    Guild::Instance().RefreshMemberInfo(&poPlayer->GetPlayerData(), 0);

	//记录该玩家的信息
	ZoneLog::Instance().WriteMajestyLog(&poPlayer->GetPlayerData());
}

