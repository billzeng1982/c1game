#include "LogMacros.h"
#include "FightPlayer.h"
#include "strutil.h"
#include "GuildFightArena.h"
#include "GuildFightPoint.h"
#include "FightPlayerStateMachine.h"
#include "GameDataMgr.h"

using namespace PKGMETA;

bool FightPlayer::Init(GuildFightArena* poGuildFightArena, DT_GUILD_FIGHT_PLAYER_INFO& rstPlayerInfo, uint8_t bCamp, uint64_t ullTimeMs,
					   uint16_t wHeadIcon, uint16_t wHeadFrame, uint16_t wHeadTitle)
{
    assert(poGuildFightArena);
    m_poGuildFightArena = poGuildFightArena;

    m_ullTimeMs = ullTimeMs;
    m_iFightPvpResult = GUILD_FIGHT_PVP_NONE;

    bzero(&m_stPlayerInfo, sizeof(m_stPlayerInfo));
    m_ullGuildId = rstPlayerInfo.m_ullGuildId;
    m_stPlayerInfo.m_ullUin = rstPlayerInfo.m_ullUin;
    memcpy(m_stPlayerInfo.m_szName, rstPlayerInfo.m_szName, MAX_NAME_LENGTH);
    m_stPlayerInfo.m_wHeadIcon = wHeadIcon;
	m_stPlayerInfo.m_wHeadFrame = wHeadFrame;
	m_stPlayerInfo.m_wHeadTitle = wHeadTitle;
    m_stPlayerInfo.m_bCamp = bCamp;
    if (bCamp == GUILD_FIGHT_PLAYER_GROUP_RED)
    {
        m_wRevivePoint = GUILD_FIGHT_STRAT_POINT_RED;
        m_stPlayerInfo.m_wPointId = m_wRevivePoint;
    }
    else if(bCamp == GUILD_FIGHT_PLAYER_GROUP_BLUE)
    {
        m_wRevivePoint = GUILD_FIGHT_STRAT_POINT_BLUE;
        m_stPlayerInfo.m_wPointId = m_wRevivePoint;
    }
    else
    {
        LOGERR("bCamp is vailid");
        return false;
    }
    m_stPlayerInfo.m_wState = GUILD_FIGHT_PLAYER_STATE_NONE;

    RESGUILDFIGHTPARAM *pDamagePara = CGameDataMgr::Instance().GetResGuildFightParamMgr().Find((uint32_t)GUILD_FIGHT_PARAM_PLAYER_DAMAGE);
    if(NULL == pDamagePara)
    {
        LOGERR_r("pPhaseTime is NULL");
        return false;
    }
    m_dwDamageCycleMs = (uint32_t)(pDamagePara->m_paramList[2]) * 1000;  //伤害周期,配置档中填的时间单位是秒,这里是毫秒,因此乘1000
    m_wDamageValue = (uint16_t)(pDamagePara->m_paramList[3]);

    RESGUILDFIGHTPARAM *pCoolTimePara = CGameDataMgr::Instance().GetResGuildFightParamMgr().Find(GUILD_FIGHT_PARAM_COOL_TIME);
    if(NULL == pCoolTimePara)
    {
        LOGERR_r("pCoolTimePara is NULL");
        return false;
    }
    m_dwReviveCoolTimeMs = (uint32_t)(pCoolTimePara->m_paramList[0]) * 1000;  //复活冷却时间,配置档中填的时间单位是秒,这里是毫秒,因此乘1000

    RESGUILDFIGHTPARAM *pOtherPara = CGameDataMgr::Instance().GetResGuildFightParamMgr().Find(GUILD_FIGHT_PARAM_OTHER);
    if(NULL == pOtherPara)
    {
        LOGERR_r("RESGUILDFIGHTPARAM is id<%u> NULL", GUILD_FIGHT_PARAM_OTHER);
        return false;
    }
    m_wMatchWinScore = (uint16_t)pOtherPara->m_paramList[2]; //匹配中胜利时获得的积分
    m_iIniGodLeftTimeMs = (int)pOtherPara->m_paramList[1] * 1000;   //无敌时间

    pOtherPara = CGameDataMgr::Instance().GetResGuildFightParamMgr().Find(10);
    if(NULL == pOtherPara)
    {
        LOGERR_r("RESGUILDFIGHTPARAM is id<%u> NULL", GUILD_FIGHT_PARAM_OTHER);
        return false;
    }
    m_wWinPersonScore = (uint16_t)pOtherPara->m_paramList[1];
    m_wDamageScore = (uint16_t)pOtherPara->m_paramList[0];

    m_poGuildFightPoint = m_poGuildFightArena->GetPoint(m_stPlayerInfo.m_wPointId);
    assert(m_poGuildFightPoint);
    m_poGuildFightPoint->AddPlayer(this);

    m_iTimeLeftMove = 0;
    m_iTimeLeftDamage = 0;
    m_iTimeLeftRevive = 0;

    return true;
}

void FightPlayer::Clear()
{

}

int FightPlayer::Move(uint16_t wDstPoint)
{
    //开战准备阶段不能移动
    if (m_poGuildFightArena->m_iState != GUILD_FIGHT_ARENA_STATE_START)
    {
        return ERR_IN_PREPARE_PHASE;
    }

    //只有处于空闲/无敌和攻击据点状态时才能移动
    if (m_stPlayerInfo.m_wState != GUILD_FIGHT_PLAYER_STATE_IDLE &&
      m_stPlayerInfo.m_wState != GUILD_FIGHT_PLAYER_STATE_ATTACK &&
      m_stPlayerInfo.m_wState != GUILD_FIGHT_PLAYER_STATE_GOD)
    {
        return ERR_DEFAULT;
    }

    //不能移动到对方复活点
    if (wDstPoint == GUILD_FIGHT_STRAT_POINT_RED ||wDstPoint == GUILD_FIGHT_STRAT_POINT_BLUE)
    {
        if (wDstPoint != m_wRevivePoint)
        {
            return ERR_CANT_MOVETO_REVIVE_POINT;
        }
    }

    //获取当前据点到目的据点的移动时间
    int iRet = m_poGuildFightPoint->GetMoveTime(wDstPoint);
    if (iRet < 0)
    {
        return iRet;
    }

    //下次移动的目的地和时间
    m_wDstPointId = wDstPoint;
    m_iTimeLeftMove = iRet;

    //当前变为移动状态
    FightPlayerStateMachine::Instance().ChangeState(this, GUILD_FIGHT_PLAYER_STATE_MOVE);

    return iRet;
}


void FightPlayer::MatchSettle(uint64_t ullWinnerId)
{
    if (m_stPlayerInfo.m_wState != GUILD_FIGHT_PLAYER_STATE_MATCH)
    {
        LOGERR_r("Player(%s) Uin(%lu) state(%d) is not in match state, ignore", m_stPlayerInfo.m_szName, m_stPlayerInfo.m_ullUin, m_stPlayerInfo.m_wState);
        return;
    }

    //匹配中胜利
    if (m_stPlayerInfo.m_ullUin == ullWinnerId)
    {
        m_iFightPvpResult = GUILD_FIGHT_PVP_WIN;
        m_stPlayerInfo.m_wKillNum++;
        m_stPlayerInfo.m_wScore += m_wWinPersonScore;
        m_poGuildFightArena->GainScore(m_stPlayerInfo.m_bCamp, 0, m_wMatchWinScore);
        m_poGuildFightArena->ChgStatisInfo(m_stPlayerInfo.m_bCamp, GUILD_FIGHT_STATIS_KILLNUM, 1);
    }
    //匹配中失败
    else
    {
        m_iFightPvpResult = GUILD_FIGHT_PVP_LOSE;
    }

    FightPlayerStateMachine::Instance().ChangeState(this, GUILD_FIGHT_PLAYER_STATE_MATCH_END);

    return;

}


void FightPlayer::DamagePoint(uint16_t wDamageValue)
{
    //统计玩家攻城数据并广播
    m_stPlayerInfo.m_dwSiegeDamage += wDamageValue;
    m_stPlayerInfo.m_wScore += m_wDamageScore;
    m_poGuildFightArena->AddStateSync(m_stPlayerInfo);
}


void FightPlayer::Revive()
{
    //复活在出生点
    m_stPlayerInfo.m_wPointId = m_wRevivePoint;

    //加入据点
    m_poGuildFightPoint = m_poGuildFightArena->GetPoint(m_stPlayerInfo.m_wPointId);
    assert(m_poGuildFightPoint);
    m_poGuildFightPoint->AddPlayer(this);
}


void FightPlayer::Update(int iDeltaTime)
{
    m_ullTimeMs += iDeltaTime;

    if (m_poGuildFightArena->m_iState != GUILD_FIGHT_ARENA_STATE_START)
    {
       return;
    }

    FightPlayerStateMachine::Instance().Update(this, iDeltaTime);
}

int FightPlayer::GetState()
{
    return m_stPlayerInfo.m_wState;
}

void FightPlayer::SetState(int iNewState)
{
    m_stPlayerInfo.m_wState = iNewState;
}


