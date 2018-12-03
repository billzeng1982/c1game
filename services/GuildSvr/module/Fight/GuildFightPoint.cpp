#include "GuildFightPoint.h"
#include "GuildFightArena.h"
#include "FightPlayer.h"
#include "LogMacros.h"
#include "FightPlayerStateMachine.h"
#include "../../framework/GuildSvrMsgLayer.h"

using namespace PKGMETA;

bool GuildFightPoint::Init(GuildFightArena* poGuildFightArena, RESGUILDFIGHTMAP* pResGuildFightMap, uint64_t ullTimeMs)
{
    if (poGuildFightArena==NULL || pResGuildFightMap==NULL)
    {
        LOGERR("poGuildFightArena is NULL");
        return false;
    }

    m_poGuildFightArena = poGuildFightArena;

    m_stPointInfo.m_wId = pResGuildFightMap->m_dwId;
    m_stPointInfo.m_bType = pResGuildFightMap->m_dwStrongPointId;
    m_stPointInfo.m_bCamp = pResGuildFightMap->m_bStrongPointGroup;
    m_stPointInfo.m_bAdjoinCnt = pResGuildFightMap->m_bContiguousPointNum;
    for (int i=0; i<m_stPointInfo.m_bAdjoinCnt; i++)
    {
        m_stPointInfo.m_AdjoinList[i] = pResGuildFightMap->m_contiguousPointList[i];
        m_AdjoinMoveTimeMs[i] = pResGuildFightMap->m_moveTimeCostList[i] * 1000; //配置档中填的时间单位是秒，这里是毫秒，因此乘1000
    }

    poGuildFightArena->m_GuildInfoList[m_stPointInfo.m_bCamp -1].m_bOwnPoint += 1;

    ResGuildFightStrongPointMgr_t& rstResGuildFightPointMgr = CGameDataMgr::Instance().GetResGuildFightStrongPointMgr();
    RESGUILDFIGHTSTRONGPOINT* pResGuildFightPoint = rstResGuildFightPointMgr.Find(m_stPointInfo.m_bType);
    if (pResGuildFightPoint==NULL)
    {
        LOGERR("pResGuildFightPoint is NULL");
        return false;
    }
    m_stPointInfo.m_dwMaxHp = pResGuildFightPoint->m_dwHp;//最大Hp
    m_stPointInfo.m_iCurHp = pResGuildFightPoint->m_dwHp;//当前Hp

    m_dwGainScoreCycleMs = pResGuildFightPoint->m_dwScoreCycle * 1000; //得分周期,配置档中填的时间单位是秒，这里是毫秒，因此乘1000
    m_wScoreValue = pResGuildFightPoint->m_dwScoreNum; //得分值

    m_ullTimeMs = ullTimeMs;
    m_ullNextUptTime = m_ullTimeMs + m_dwGainScoreCycleMs;
    RESGUILDFIGHTPARAM* poResParam = CGameDataMgr::Instance().GetResGuildFightParamMgr().Find(GUILD_FIGHT_PARAM_OTHER);
    if (!poResParam)
    {
        LOGERR("RESGUILDFIGHTPARAM id<%u> is NULL", GUILD_FIGHT_PARAM_OTHER);
        return false;
    }

    m_iPreMatchTimeMs = poResParam->m_paramList[0] * 1000;

    return true;
}

void GuildFightPoint::Clear()
{
    m_listPlayerRed.clear();
    m_listPlayerBlue.clear();
    m_listMatcPair.clear();
}


void GuildFightPoint::Update(int iDeltaTime)
{
    m_ullTimeMs += iDeltaTime;

    //没有正式开战状态，不加分
    if (m_poGuildFightArena->m_iState != GUILD_FIGHT_ARENA_STATE_START)
    {
       m_ullNextUptTime = m_ullTimeMs + m_dwGainScoreCycleMs;
       return;
    }

    if (m_ullTimeMs >= m_ullNextUptTime)
    {
        m_poGuildFightArena->GainScore(m_stPointInfo.m_bCamp, m_stPointInfo.m_wId, m_wScoreValue);
        m_ullNextUptTime = m_ullTimeMs + m_dwGainScoreCycleMs;
    }

    if (!m_listMatcPair.empty() &&  (CGameTime::Instance().GetCurrTimeMs() - m_listMatcPair.front().ullStartTimeMs) > (uint64_t) m_iPreMatchTimeMs)
    {
        StartFight(m_listMatcPair.front());
        m_listMatcPair.pop_front();
    }


}


int GuildFightPoint::GetMoveTime(uint16_t wDstPoint)
{
    for(int i=0; i<MAX_NUM_GUILD_FIGHT_ADJOIN_POINT; i++)
    {
        if (wDstPoint == m_stPointInfo.m_AdjoinList[i])
        {
            return m_AdjoinMoveTimeMs[i];
        }
    }

    return ERR_NOT_FOUND;
}

//  进入据点会先判断能否匹配
void GuildFightPoint::AddPlayer(FightPlayer* poPlayer)
{
    assert(poPlayer);

    // 匹配处理,匹配成功直接返回,不进入其他状态
    if (DealMatch(poPlayer))
    {
        return;
    }

    if (poPlayer->m_stPlayerInfo.m_bCamp == GUILD_FIGHT_PLAYER_GROUP_RED)
    {
        m_listPlayerRed.push_back(poPlayer);
    }
    else if (poPlayer->m_stPlayerInfo.m_bCamp == GUILD_FIGHT_PLAYER_GROUP_BLUE)
    {
        m_listPlayerBlue.push_back(poPlayer);
    }
    else
    {
        assert(false);
    }
    if ( GUILD_FIGHT_PLAYER_STATE_GOD == poPlayer->m_stPlayerInfo.m_wState)
    {
        return;
    }

    if (poPlayer->m_stPlayerInfo.m_bCamp == m_stPointInfo.m_bCamp)
    {
        FightPlayerStateMachine::Instance().ChangeState(poPlayer, GUILD_FIGHT_PLAYER_STATE_IDLE);
    }
    else
    {
        FightPlayerStateMachine::Instance().ChangeState(poPlayer, GUILD_FIGHT_PLAYER_STATE_ATTACK);
    }
}

void GuildFightPoint::DelPlayer(FightPlayer* poPlayer)
{
    assert(poPlayer);
    if (poPlayer->m_stPlayerInfo.m_bCamp == GUILD_FIGHT_PLAYER_GROUP_RED)
    {
        m_listPlayerRed.remove(poPlayer);
    }
    else if (poPlayer->m_stPlayerInfo.m_bCamp == GUILD_FIGHT_PLAYER_GROUP_BLUE)
    {
        m_listPlayerBlue.remove(poPlayer);
    }
    else
    {
        LOGERR("poPlayer camp is vailid");
    }
}

void GuildFightPoint::Damage(FightPlayer* poPlayer)
{
    if (poPlayer->m_stPlayerInfo.m_bCamp == m_stPointInfo.m_bCamp)
    {
        LOGERR_r("Point(%d) is same camp with Player(%lu)", m_stPointInfo.m_wId, poPlayer->m_stPlayerInfo.m_ullUin);
        return;
    }

    //扣血
    m_stPointInfo.m_iCurHp -= (int)poPlayer->m_wDamageValue;

    //统计攻城伤害
    m_poGuildFightArena->ChgStatisInfo(poPlayer->m_stPlayerInfo.m_bCamp,
                                        GUILD_FIGHT_STATIS_SIEGEDAMAGE, poPlayer->m_wDamageValue);

    //统计玩家攻城数据并广播
    poPlayer->DamagePoint(poPlayer->m_wDamageValue);

    //将据点伤害的消息加入广播消息
    DT_GUILD_FIGHT_POINT_DAMAGE_LIST& rstPointDamageList = m_poGuildFightArena->m_stSynMsg.m_stPointDamageInfo;
    uint8_t bIndex = rstPointDamageList.m_bCount++;
    rstPointDamageList.m_astDamageList[bIndex].m_iDamage = poPlayer->m_wDamageValue;
    rstPointDamageList.m_astDamageList[bIndex].m_iCurHp = m_stPointInfo.m_iCurHp;
    rstPointDamageList.m_astDamageList[bIndex].m_wPointId = m_stPointInfo.m_wId;
    rstPointDamageList.m_astDamageList[bIndex].m_ullPlayerId = poPlayer->m_stPlayerInfo.m_ullUin;

    //如果血量降为0,则据点阵营发生变化,不重置加分周期
    if (m_stPointInfo.m_iCurHp <= 0)
    {
        m_stPointInfo.m_iCurHp = m_stPointInfo.m_dwMaxHp;
        DT_GUILD_FIGHT_ARENA_GUILD_INFO* pstGuildInfo = m_poGuildFightArena->m_GuildInfoList;

        pstGuildInfo[m_stPointInfo.m_bCamp - 1].m_bOwnPoint -= 1;
        pstGuildInfo[2 - m_stPointInfo.m_bCamp].m_bOwnPoint += 1;

        m_stPointInfo.m_bCamp = GUILD_FIGHT_PLAYER_GROUP_NEUTRAL - m_stPointInfo.m_bCamp;
        //m_ullNextUptTime = m_ullTimeMs + m_dwGainScoreCycleMs;

        //统计据点占领
        m_poGuildFightArena->ChgStatisInfo(m_stPointInfo.m_bCamp, GUILD_FIGHT_STATIS_CAPTUREPOINT, 1);

        //将据点阵营变更的消息加入广播消息
        DT_GUILD_FIGHT_POINT_CAMP_CHG_LIST& rstCampChgList = m_poGuildFightArena->m_stSynMsg.m_stPointCampChgInfo;
        uint8_t bIndex = rstCampChgList.m_bCount++;
        rstCampChgList.m_astChgList[bIndex].m_wPointId = m_stPointInfo.m_wId;
        rstCampChgList.m_astChgList[bIndex].m_bCamp = m_stPointInfo.m_bCamp;
        rstCampChgList.m_astChgList[bIndex].m_dwMaxHp = m_stPointInfo.m_dwMaxHp;
        rstCampChgList.m_astChgList[bIndex].m_iCurHp = m_stPointInfo.m_iCurHp;
    }

    return;
}

//进入据点时匹配
bool GuildFightPoint::DealMatch(FightPlayer* poPlayer)
{

    if ( GUILD_FIGHT_PLAYER_STATE_GOD == poPlayer->m_stPlayerInfo.m_wState)
    {//自己是无敌状态不处理
        return false;
    }

    bool bIsMatched = false;
    ListFightPlayer_t::iterator iterPlayer;
    MatchPair oTmp;
    if (poPlayer->m_stPlayerInfo.m_bCamp == GUILD_FIGHT_PLAYER_GROUP_RED)
    {
        iterPlayer = m_listPlayerBlue.begin();
        if (iterPlayer != m_listPlayerBlue.end() && (*iterPlayer)->m_stPlayerInfo.m_wState != GUILD_FIGHT_PLAYER_STATE_GOD)
        {
            oTmp.ullRed = poPlayer->m_stPlayerInfo.m_ullUin;
            oTmp.ullBlue = (*iterPlayer)->m_stPlayerInfo.m_ullUin;
            bIsMatched = true;
        }
    }
    else
    {
        iterPlayer = m_listPlayerRed.begin();
        if (iterPlayer != m_listPlayerRed.end() && (*iterPlayer)->m_stPlayerInfo.m_wState != GUILD_FIGHT_PLAYER_STATE_GOD)
        {
            oTmp.ullRed = (*iterPlayer)->m_stPlayerInfo.m_ullUin;
            oTmp.ullBlue = poPlayer->m_stPlayerInfo.m_ullUin;
            bIsMatched = true;
        }
    }

    // 撮合成功
    if (bIsMatched)
    {
        oTmp.ullStartTimeMs = CGameTime::Instance().GetCurrTimeMs();

        m_listMatcPair.push_back(oTmp);
        FightPlayerStateMachine::Instance().ChangeState(poPlayer, GUILD_FIGHT_PLAYER_STATE_PRE_MATCH);
        FightPlayerStateMachine::Instance().ChangeState((*iterPlayer), GUILD_FIGHT_PLAYER_STATE_PRE_MATCH);

        //将撮合成功的玩家从据点删除
        this->DelPlayer(*iterPlayer);

        return true;
    }
    else
    {
        return false;
    }
}

void GuildFightPoint::StartFight(MatchPair& stMatchPair)
{
    FightPlayer* poPlayer1 = m_poGuildFightArena->GetPlayer(stMatchPair.ullBlue);
    FightPlayer* poPlayer2 = m_poGuildFightArena->GetPlayer(stMatchPair.ullRed );
    if (poPlayer1 == NULL || poPlayer2 == NULL)
    {
        LOGERR_r("start fight error, not find the player! the startime <%lu> Blue<%lu> Red<%lu>",
            stMatchPair.ullStartTimeMs, stMatchPair.ullBlue, stMatchPair.ullRed );
        return;
    }


    // 创建副本，进行军团PVP
    SSPKG stSsPkg = {0};
    stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_FIGHT_PVP_MATCH_REQ;
    SS_PKG_GUILD_FIGHT_PVP_MATCH_REQ& rstSsPkgReq = stSsPkg.m_stBody.m_stGuildFightPvpMatchReq;

    rstSsPkgReq.m_stPlayerInfo1.m_ullUin = poPlayer1->m_stPlayerInfo.m_ullUin;
    rstSsPkgReq.m_stPlayerInfo1.m_ullGuildId = poPlayer1->m_ullGuildId;
    rstSsPkgReq.m_stPlayerInfo2.m_ullUin = poPlayer2->m_stPlayerInfo.m_ullUin;
    rstSsPkgReq.m_stPlayerInfo2.m_ullGuildId = poPlayer2->m_ullGuildId;

    //将撮合成功的玩家状态改为匹配中
    poPlayer1->m_stPlayerInfo.m_ullStateParam = poPlayer2->m_stPlayerInfo.m_ullUin;
    FightPlayerStateMachine::Instance().ChangeState(poPlayer1, GUILD_FIGHT_PLAYER_STATE_MATCH);

    poPlayer2->m_stPlayerInfo.m_ullStateParam = poPlayer1->m_stPlayerInfo.m_ullUin;
    FightPlayerStateMachine::Instance().ChangeState(poPlayer2, GUILD_FIGHT_PLAYER_STATE_MATCH);



    GuildSvrMsgLayer::Instance().SendToZoneSvr(stSsPkg);

    LOGRUN_r("start fight_ok the startime <%lu> Blue<%lu> Red<%lu>",
        stMatchPair.ullStartTimeMs, stMatchPair.ullBlue, stMatchPair.ullRed );

}

//  从无敌状态退出时,再匹配
bool GuildFightPoint::GodExitMatch(FightPlayer* poPlayer)
{
    bool bIsMatched = false;
    ListFightPlayer_t::iterator iterPlayer;
    MatchPair oTmp;
    if (poPlayer->m_stPlayerInfo.m_bCamp == GUILD_FIGHT_PLAYER_GROUP_RED)
    {
        iterPlayer = m_listPlayerBlue.begin();
        if (iterPlayer != m_listPlayerBlue.end() && (*iterPlayer)->m_stPlayerInfo.m_wState != GUILD_FIGHT_PLAYER_STATE_GOD)
        {
            oTmp.ullRed = poPlayer->m_stPlayerInfo.m_ullUin;
            oTmp.ullBlue = (*iterPlayer)->m_stPlayerInfo.m_ullUin;
            bIsMatched = true;
        }
    }
    else
    {
        iterPlayer = m_listPlayerRed.begin();
        if (iterPlayer != m_listPlayerRed.end() && (*iterPlayer)->m_stPlayerInfo.m_wState != GUILD_FIGHT_PLAYER_STATE_GOD)
        {
            oTmp.ullRed = (*iterPlayer)->m_stPlayerInfo.m_ullUin;
            oTmp.ullBlue = poPlayer->m_stPlayerInfo.m_ullUin;
            bIsMatched = true;
        }
    }

    // 撮合成功
    if (bIsMatched)
    {
        oTmp.ullStartTimeMs = CGameTime::Instance().GetCurrTimeMs();

        m_listMatcPair.push_back(oTmp);
        FightPlayerStateMachine::Instance().ChangeState(poPlayer, GUILD_FIGHT_PLAYER_STATE_PRE_MATCH);
        FightPlayerStateMachine::Instance().ChangeState((*iterPlayer), GUILD_FIGHT_PLAYER_STATE_PRE_MATCH);

        //将撮合成功的玩家从据点删除
        this->DelPlayer(*iterPlayer);
        this->DelPlayer(poPlayer);
        return true;
    }
    else
    {
        return false;
    }
}
