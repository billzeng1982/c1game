#include "LogMacros.h"
#include "Player.h"
#include "PlayerMgr.h"
#include "PlayerStateMachine.h"
#include "ss_proto.h"
#include "GameTime.h"
#include "../../ZoneSvr.h"
#include "../../framework/ZoneSvrMsgLayer.h"
#include "../Task.h"
#include "../Lottery.h"
#include "../Majesty.h"
#include "../AP.h"
#include "../FightPVE.h"
#include "../FightPVP.h"
#include "../Guild.h"
#include "../ActivityMgr.h"
#include "PlayerLogic.h"
#include "MsgLogicMatch.h"
#include "../Mall.h"
#include "../RankMgr.h"
#include "ZoneSvr.h"
#include "../Mail.h"
#include "../AsyncPvp.h"
#include "../Shops.h"
#include "../SkillPoint.h"
#include "../DailyChallenge.h"
#include "../Pay.h"
#include "../Mail.h"
#include "../CloneBattle.h"
#include "../PeakArena.h"
#include "../GuildGeneralHang.h"
#include "../TaskAct.h"
#include "../Mine.h"
#include "../GeneralCard.h"
#include "../Gem.h"
#include "../Task.h"
using namespace PKGMETA;

Player::Player()
{
    bzero(&m_stConnSession, sizeof(m_stConnSession));
    m_iCurState = PLAYER_STATE_NULL;
    m_szAccountName[0] = '\0';
	m_szSdkUserName[0] = '\0';
    m_szPhoneType[0] = '\0';
    m_szChannelName[0] = '\0';
    m_llEntryReconnTime = 0;
    m_dwOnlineTokenId = 0;
    m_wVersion = 1;
    m_dwPkgSeqNo = 10000;
    m_oPlayerData.Init(this);
    m_dwRecvClientPkgSeq = 0;
	m_dwLoginSvrId = 0;
    uint32_t dwQueueSize = ZoneSvr::Instance().GetConfig().m_dwPlayerBufferSize;
    m_oSendBuffer.Init(dwQueueSize, 0);
}

// when release to pool
void Player::Reset()
{
    bzero(&m_stConnSession, sizeof(m_stConnSession));
    m_iCurState = PLAYER_STATE_NULL;
    m_szAccountName[0] = '\0';
	m_szSdkUserName[0] = '\0';
    m_szPhoneType[0] = '\0';
    m_szChannelName[0] = '\0';
    m_llEntryReconnTime = 0;
    m_dwOnlineTokenId = 0;
    m_wVersion = 0;
    m_dwPkgSeqNo = 10000;
    m_dwRecvClientPkgSeq = 0;
    m_iFightState = FIGHT_STATE_NONE;
    m_oPlayerData.Clear();
    m_oSendBuffer.Clear();
	m_dwLoginSvrId = 0;
}

void Player::InitNewPlayer()
{
    m_oPlayerData.NewInit();
}

bool Player::InitFromDB(DT_ROLE_WHOLE_DATA& rstRoleWholeData)
{
    if (!m_oPlayerData.InitFromDB(rstRoleWholeData))
    {
        return false;
    }

    _UpdateAfterInitFromDB();
    return true;
}

void Player::UptRoleDataToDB()
{
    SSPKG stSsPkg;

    stSsPkg.m_stHead.m_wMsgId = SS_MSG_ROLE_UPDATE_REQ;
    stSsPkg.m_stHead.m_iDstProcId = ZoneSvr::Instance().GetConfig().m_iRoleSvrID;
    stSsPkg.m_stHead.m_ullUin = this->GetUin();

    if (!this->m_oPlayerData.PackRoleDataUpt(stSsPkg.m_stBody.m_stRoleUpdateReq))
    {
        LOGERR("update db account-%s failed", this->GetAccountName());
        return;
    }

    if (stSsPkg.m_stBody.m_stRoleUpdateReq.m_nCount > 0)
    {
        ZoneSvrMsgLayer::Instance().SendToRoleSvr(stSsPkg);
    }

    return;
}

bool Player::_UpdateAfterInitFromDB()
{
    PlayerMgr::Instance().AddToRoleNameHM(this);

    //设置登录时间
    SetLastLoginTime(CGameTime::Instance().GetCurrSecond());
    // 更新离线体力
    AP::Instance().InitPlayerData(&this->m_oPlayerData);
    TaskAct::Instance().UpdatePlayer(&this->GetPlayerData());
    //更新离线技能点
    SkillPoint::Instance().InitPlayerData(&this->m_oPlayerData);

    Fight6V6::Instance().InitPlayerData(&this->m_oPlayerData);

    // 检查是否刷新关卡次数
    DT_ROLE_PVE_INFO& rstPveInfo = this->GetPlayerData().GetPveInfo();
    if (rstPveInfo.m_ullPveUpdateTime != FightPVE::Instance().m_ullUpdateLastTime)
    {
        FightPVE::Instance().UpdatePlayerData(&this->GetPlayerData());
    }

    // 更新公会相关信息
    Guild::Instance().InitPlayerData(&this->m_oPlayerData);

	//更新代币商店数据
	Shops::Instance().InitPlayerData(&this->m_oPlayerData);

    Mall::Instance().UpdatePlayerData(&this->m_oPlayerData);

    //更新招募
    Lottery::Instance().InitPlayerData(&this->m_oPlayerData);
    Majesty::Instance().UpdatePlayerData(&this->m_oPlayerData);

    //异步PvP更新
    AsyncPvp::Instance().UpdatePlayerData(&this->m_oPlayerData);

    //每日挑战赛更新
    DailyChallenge::Instance().UpdatePlayerData(&this->m_oPlayerData);

    //充值活动更新
    Pay::Instance().UpdatePlayerData(&this->m_oPlayerData);

    //克隆战更新
    CloneBattle::Instance().InitPlayerData(this->m_oPlayerData);

    //巅峰竞技场更新
	PeakArena::Instance().AdaptActiveRewardBuff(&this->m_oPlayerData);
    PeakArena::Instance().UpdatePlayerData(&this->m_oPlayerData);

	Mine::Instance().UpdatePlayer(this->GetPlayerData());
	//更新离线练兵场经验
	GuildGeneralHang::Instance().InitPlayerData(&this->m_oPlayerData);

    GeneralCard::Instance().InitLeaderValue(&this->m_oPlayerData);

    PlayerStateMachine::Instance().ChangeState(this, PLAYER_STATE_INGAME);
	Gem::Instance().Repair(&this->m_oPlayerData);

    //BUGFIX
    Task::Instance().UnlockTask(&this->m_oPlayerData, TASK_TYPE_PASS_GIFT);

    return true;
}

void Player::OnUpdate()
{
    PlayerStateMachine::Instance().Update(this, 0/*不需要deltaTime*/);

    if (IsCurState(PLAYER_STATE_INGAME) || IsCurState(PLAYER_STATE_RECONN_WAIT))
    {
        // logic update
        Majesty::Instance().UpdatePlayerData(&this->GetPlayerData());
        AP::Instance().UpdatePlayerData(&this->GetPlayerData());
        SkillPoint::Instance().UpdatePlayerData(&this->GetPlayerData());
        FightPVE::Instance().UpdatePlayerData(&this->GetPlayerData());
        TaskAct::Instance().UpdatePlayer(&this->GetPlayerData());
        Lottery::Instance().UpdatePlayerData(&this->GetPlayerData());
        Task::Instance().UpdatePlayerData(&this->GetPlayerData());
        Fight6V6::Instance().UpdatePlayerData(&this->GetPlayerData());
        Guild::Instance().UpdatePlayerData(&this->GetPlayerData());
		Shops::Instance().UpdatePlayerData(&this->GetPlayerData());
        ActivityMgr::Instance().UpdatePlayerData(&this->GetPlayerData());
        Mall::Instance().UpdatePlayerData(&this->GetPlayerData());
        RankMgr::Instance().UpdatePlayerData(&this->GetPlayerData());
        AsyncPvp::Instance().UpdatePlayerData(&this->GetPlayerData());
        DailyChallenge::Instance().UpdatePlayerData(&this->GetPlayerData());
        Pay::Instance().UpdatePlayerData(&this->GetPlayerData());
        Mail::Instance().UpdatePlayerData(&this->GetPlayerData());
        PeakArena::Instance().UpdatePlayerData(&this->GetPlayerData());
		Mine::Instance().UpdatePlayer(this->GetPlayerData());
    }
}

bool Player::IsCurState(int iState)
{
    return iState == m_iCurState;
}

void Player::SaveSendPkg(char* pszBuffer, uint32_t dwLen)
{
    int iRet= m_oSendBuffer.WriteMsg_F(pszBuffer, dwLen);
    if (iRet != THREAD_Q_SUCESS)
    {
        //LOGERR("Player(%s) Uin(%lu) SaveSendPkg failed, MsgId=%d, iRet=%d", GetRoleName(), GetUin(), SCPKG_MSGID(pszBuffer), iRet);
    }
}

void Player::AfterRoleLogin()
{
    // 通知邮箱服务器，玩家登陆
    Mail::Instance().SendPlayerStatToMailSvr(this->GetUin(), PLAYER_STATE_LOGIN);
    RankMgr::Instance().UpdateLi(&this->m_oPlayerData);
    RankMgr::Instance().UpdateGCardCnt(&this->m_oPlayerData);
    RankMgr::Instance().UpdatePveStar(&this->m_oPlayerData);

}

