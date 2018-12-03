#include "LogMacros.h"
#include "GuildFightArena.h"
#include "../Guild/GuildMgr.h"
#include "../../gamedata/GameDataMgr.h"
#include "../../framework/GameObjectPool.h"
#include "GuildSvrMsgLayer.h"
#include "GameTime.h"
#include "GuildFightMgr.h"

using namespace PKGMETA;

int PlayerIdCmp(const void *pstFirst, const void *pstSecond)
{
    uint64_t ullFirst = *((uint64_t*)pstFirst);
    uint64_t ullSecond = *((uint64_t*)pstSecond);

    if (ullSecond > ullFirst)
    {
        return 1;
    }
    else if (ullFirst > ullSecond)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}


bool GuildFightArena::Init(uint64_t GuildList[], int iGuildCnt, uint8_t bAgainstId)
{
    if (!_InitBasePara(bAgainstId))
    {
        return false;
    }

    if (!_InitGuild(GuildList, iGuildCnt))
    {
        return false;
    }

    if (!_InitMap())
    {
        return false;
    }

    if (!_InitPlayer())
    {
        return false;
    }

    return true;
}

bool GuildFightArena::_InitBasePara(uint8_t bAgainstId)
{
    m_bAgainstId = bAgainstId;
    m_iState = GUILD_FIGHT_ARENA_STATE_PREPARE;
    m_bWinCamp = GUILD_FIGHT_PLAYER_GROUP_NONE;
    m_ullWinGuild = 0;
    m_ullTimeMs = CGameTime::Instance().GetCurrTimeMs();

    ResGuildFightParamMgr_t& rstResGuildFightParaMgr = CGameDataMgr::Instance().GetResGuildFightParamMgr();
    RESGUILDFIGHTPARAM *pFightPara = rstResGuildFightParaMgr.Find(GUILD_FIGHT_PARAM_END_CONDITION);
	if (NULL == pFightPara)
	{
		LOGERR_r("pFightPara is NULL");
		return false;
	}
    m_dwWinScore = (uint32_t)pFightPara->m_paramList[0];

    pFightPara = rstResGuildFightParaMgr.Find(9);
    if (NULL == pFightPara)
	{
		LOGERR_r("pFightPara is NULL");
		return false;
	}
    m_wGainScorePoint1 = (uint16_t)pFightPara->m_paramList[0];
    m_wGainScorePoint2 = (uint16_t)pFightPara->m_paramList[2];
    m_wGainScoreRate1 = (uint16_t)pFightPara->m_paramList[1];
    m_wGainScoreRate2 = (uint16_t)pFightPara->m_paramList[3];

    pFightPara = rstResGuildFightParaMgr.Find(GUILD_FIGHT_PARAM_COOL_TIME);
    if (NULL == pFightPara)
    {
        LOGERR_r("pFightPara is NULL");
        return false;
    }
    m_ullJoinCoolTimeMs = (uint64_t)pFightPara->m_paramList[1] * 1000;
    m_ullLeastPassTimeMs = (uint64_t)pFightPara->m_paramList[2] * 1000;

    return true;
}


bool GuildFightArena::_InitMap()
{
    ResGuildFightMapMgr_t& rstResGuildFightMapMgr = CGameDataMgr::Instance().GetResGuildFightMapMgr();
    int iNum = rstResGuildFightMapMgr.GetResNum();
    if (iNum > MAX_NUM_GUILD_FIGHT_MAP_POINT)
    {
        LOGERR("GuildFightMap Init failed, Point ResNum is larger than MaxNum");
        return false;
    }

    //根据数据档配置生成地图信息
    m_mapId2Point.clear();
    for (int i=0; i<iNum; i++)
    {
        RESGUILDFIGHTMAP* pResGuildFightMap = rstResGuildFightMapMgr.GetResByPos(i);
        if (pResGuildFightMap==NULL)
        {
            LOGERR("pResGuildFightMap is NULL");
            return false;
        }

        GuildFightPoint* poGuildFightPoint = GET_GAMEOBJECT(GuildFightPoint, GAMEOBJ_GUILD_FIGHT_POINT);
        if (poGuildFightPoint == NULL)
        {
            LOGERR("pstGuildFightPoint is NULL");
            return false;
        }
        if (!poGuildFightPoint->Init(this, pResGuildFightMap, m_ullTimeMs))
        {
            LOGERR("pstGuildFightPoint init failed");
            return false;
        }

        m_mapId2Point.insert(MapId2Point_t::value_type(pResGuildFightMap->m_dwId, poGuildFightPoint));
    }
    return true;
}


bool GuildFightArena::_InitGuild(uint64_t GuildList[], int iGuildCnt)
{
    if (iGuildCnt != MAX_NUM_ONE_GUILD_FIGHT_JOIN_GUILD)
    {
        LOGERR_r("iGuildCnt(%d) is error", iGuildCnt);
        return false;
    }

    RESGUILDFIGHTPARAM *pFightPara = CGameDataMgr::Instance().GetResGuildFightParamMgr().Find(9);
	if (NULL == pFightPara)
	{
		LOGERR_r("pFightPara is NULL");
		return false;
	}

    for (int i=0; i<iGuildCnt; i++)
    {
        //军团基本信息初始化
        m_GuildInfoList[i].m_ullGuildId = GuildList[i];
        m_GuildInfoList[i].m_dwGuildScore = 0;
        m_GuildInfoList[i].m_bCamp = i+1;
        m_GuildInfoList[i].m_bOwnPoint = 0;
        Guild* poGuild = GuildMgr::Instance().GetGuild(m_GuildInfoList[i].m_ullGuildId);
        if (poGuild == NULL)
        {
            LOGERR("poGuild is NULL");
            return false;
        }
        memcpy(m_GuildInfoList[i].m_szGuildName, poGuild->GetGuildName(), MAX_NAME_LENGTH);

        //战场中军团统计信息初始化
        m_GuildStatisList[i].m_bCamp = i+1;
        m_GuildStatisList[i].m_dwCapturePointNum = 0;
        m_GuildStatisList[i].m_wKillNum = 0;
        m_GuildStatisList[i].m_dwSiegeDamage = 0;
    }
    return true;
}

bool GuildFightArena::_InitPlayer()
{
    m_mapId2Player.clear();
    bzero(m_GuildPlayerList, sizeof(m_GuildPlayerList));
    return true;
}

void GuildFightArena::Clear()
{
    //清空玩家
    m_mapId2PlayerIter = m_mapId2Player.begin();
    for (; m_mapId2PlayerIter!=m_mapId2Player.end(); m_mapId2PlayerIter++)
    {
        m_mapId2PlayerIter->second->Clear();
        RELEASE_GAMEOBJECT(m_mapId2PlayerIter->second);
    }
    m_mapId2Player.clear();

    //清空据点
    m_mapId2PointIter = m_mapId2Point.begin();
    for (; m_mapId2PointIter!=m_mapId2Point.end(); m_mapId2PointIter++)
    {
        m_mapId2PointIter->second->Clear();
        RELEASE_GAMEOBJECT(m_mapId2PointIter->second);
    }
    m_mapId2Point.clear();
}


void GuildFightArena::Start()
{
    m_iState = GUILD_FIGHT_ARENA_STATE_START;
    m_ullStartTimeMs = m_ullTimeMs;
}

void GuildFightArena::Update(int iDeltaTime)
{
    m_ullTimeMs += iDeltaTime;

    //玩家Update
    m_mapId2PlayerIter = m_mapId2Player.begin();
    for (; m_mapId2PlayerIter!=m_mapId2Player.end(); m_mapId2PlayerIter++)
    {
        m_mapId2PlayerIter->second->Update(iDeltaTime);
    }

    //据点Update
    m_mapId2PointIter = m_mapId2Point.begin();
    for (; m_mapId2PointIter!=m_mapId2Point.end(); m_mapId2PointIter++)
    {
        m_mapId2PointIter->second->Update(iDeltaTime);
    }

    //每一帧内，如果此帧内有状态变化，则将此帧所有的状态变化整合起来发给客户端
    if (m_stSynMsg.m_stGainScoreInfo.m_bCount != 0 ||
      m_stSynMsg.m_stPointDamageInfo.m_bCount != 0 ||
      m_stSynMsg.m_stPointCampChgInfo.m_bCount != 0 ||
      m_stSynMsg.m_stPlayerInfo.m_bJoinerCnt != 0)
    {
        Broadcast();
    }

    //是否有一方达到了胜利条件,有的话结算战斗
    if (m_GuildInfoList[0].m_dwGuildScore >= m_dwWinScore ||
      m_GuildInfoList[1].m_dwGuildScore >= m_dwWinScore )
    {
        this->Settle();
    }
}


void GuildFightArena::Broadcast()
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_FIGHT_MSG_BROADCAST;
    SS_PKG_GUILD_FIGHT_MSG_BROADCAST& rstMsgBroadCast = m_stSsPkg.m_stBody.m_stGuildFightMsgBroadCast;

    //接收者列表
    int iIndex = 0;
    rstMsgBroadCast.m_bReceiverCnt = 0;
    for (int i=0; i<MAX_NUM_ONE_GUILD_FIGHT_JOIN_GUILD; i++)
    {
        rstMsgBroadCast.m_bReceiverCnt += m_GuildPlayerList[i].m_bPlayerCnt;

        for (int j=0; j<m_GuildPlayerList[i].m_bPlayerCnt; j++)
        {
            rstMsgBroadCast.m_ReceiverList[iIndex++] = m_GuildPlayerList[i].m_PlayerList[j];
        }
    }

    memcpy(&rstMsgBroadCast.m_stSynMsg, &m_stSynMsg, sizeof(rstMsgBroadCast.m_stSynMsg));
    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);

    //将m_stSynMsg清零,以便下一帧使用
    m_stSynMsg.m_stGainScoreInfo.m_bCount = 0;
    m_stSynMsg.m_stPointDamageInfo.m_bCount = 0;
    m_stSynMsg.m_stPointCampChgInfo.m_bCount = 0;
    m_stSynMsg.m_stPlayerInfo.m_bJoinerCnt = 0;
}


int GuildFightArena::Join(SS_PKG_GUILD_FIGHT_ARENA_JOIN_REQ& rstSsPkgReq, DT_GUILD_FIGHT_ARENA_INFO& rstArenaInfo, uint64_t& ullTimeStamp)
{
    DT_GUILD_FIGHT_PLAYER_INFO& rstPlayerInfo = rstSsPkgReq.m_stJoinerInfo;

    Guild* poGuild = GuildMgr::Instance().GetGuild(rstPlayerInfo.m_ullGuildId);
    if (poGuild == NULL)
    {
        return ERR_NOT_FOUND;
    }
    DT_ONE_GUILD_MEMBER* pstMemInfo = poGuild->FindMember(rstPlayerInfo.m_ullUin);
    if (pstMemInfo == NULL)
    {
        return ERR_NOT_FOUND;
    }

    //新加入公会后，需要经过一段时间才能加入战场
    if ((m_ullTimeMs - pstMemInfo->m_ullJoinGuildTimeStap) < m_ullLeastPassTimeMs)
    {
        ullTimeStamp = pstMemInfo->m_ullJoinGuildTimeStap + m_ullLeastPassTimeMs;
        return ERR_JUST_JOIN_GUILD;
    }

    //判断阵营
    uint8_t bCamp = 0;
    for (int i=0; i<MAX_NUM_ONE_GUILD_FIGHT_JOIN_GUILD; i++)
    {
        if (rstPlayerInfo.m_ullGuildId == m_GuildInfoList[i].m_ullGuildId)
        {
            bCamp = m_GuildInfoList[i].m_bCamp;
            break;
        }
    }
    if (bCamp == 0)
    {
        return ERR_DEFAULT;
    }

    //初始化玩家
    FightPlayer* poFightPlayer = GetPlayer(rstPlayerInfo.m_ullUin);
    if (poFightPlayer == NULL)
    {
        poFightPlayer= GET_GAMEOBJECT(FightPlayer, GAMEOBJ_GUILD_FIGHT_PLAYER);
        if (poFightPlayer == NULL)
        {
            return ERR_SYS;
        }

        if (!poFightPlayer->Init(this, rstPlayerInfo, bCamp, m_ullTimeMs, rstSsPkgReq.m_wHeadIcon, rstSsPkgReq.m_wHeadFrame, rstSsPkgReq.m_wHeadTitile))
        {
            return ERR_SYS;
        }
        m_mapId2Player.insert(MapId2Player_t::value_type(rstPlayerInfo.m_ullUin, poFightPlayer));

        //加入PlayerList
        size_t nmemb = (size_t)m_GuildPlayerList[bCamp -1].m_bPlayerCnt;
        if (nmemb >= MAX_NUM_MEMBER)
        {
            LOGERR("m_GuildPlayerList PlayerCnt(%d) is larger than MAX_NUM_MEMBER", m_GuildPlayerList[bCamp -1].m_bPlayerCnt);
            return ERR_SYS;
        }
        MyBInsert(&rstPlayerInfo.m_ullUin, m_GuildPlayerList[bCamp -1].m_PlayerList, &nmemb, sizeof(uint64_t), 1, PlayerIdCmp);
        m_GuildPlayerList[bCamp -1].m_bPlayerCnt= nmemb;
    }
    else
    {
        poFightPlayer->m_stPlayerInfo.m_wState = GUILD_FIGHT_PLAYER_STATE_DEAD;

		poFightPlayer->m_stPlayerInfo.m_wHeadIcon = rstSsPkgReq.m_wHeadIcon;
		poFightPlayer->m_stPlayerInfo.m_wHeadFrame = rstSsPkgReq.m_wHeadFrame;
		poFightPlayer->m_stPlayerInfo.m_wHeadTitle = rstSsPkgReq.m_wHeadTitile;

        if (m_ullTimeMs >= poFightPlayer->m_stPlayerInfo.m_ullStateParam)
        {
            poFightPlayer->m_iTimeLeftRevive = 0;
        }
        else
        {
            poFightPlayer->m_iTimeLeftRevive = poFightPlayer->m_stPlayerInfo.m_ullStateParam - m_ullTimeMs;
        }

    }

    //重进战场，不需要发完整信息，直接在这里返回即可，减小发送的数据量
    if (rstSsPkgReq.m_bType == GUILD_FIGHT_JOIN_ARENA_RE_IN)
    {
        return ERR_NONE;
    }

    //地图信息
    int i = 0;
    m_mapId2PointIter = m_mapId2Point.begin();
    for (; m_mapId2PointIter!=m_mapId2Point.end(); m_mapId2PointIter++, i++)
    {
        memcpy(&rstArenaInfo.m_stArenaMapInfo.m_astPointList[i], &m_mapId2PointIter->second->m_stPointInfo, sizeof(DT_GUILD_FIGHT_ARENA_MAP_PONIT_INFO));
    }
    rstArenaInfo.m_stArenaMapInfo.m_bPointCnt = i;

    //公会信息
    memcpy(rstArenaInfo.m_astArenaGuildInfo, m_GuildInfoList, sizeof(rstArenaInfo.m_astArenaGuildInfo));

    //玩家信息
    int j = 0;
    m_mapId2PlayerIter = m_mapId2Player.begin();
    for (; m_mapId2PlayerIter!=m_mapId2Player.end(); m_mapId2PlayerIter++, j++)
    {
        memcpy(&rstArenaInfo.m_stPlayerList.m_astJoinerList[j], &m_mapId2PlayerIter->second->m_stPlayerInfo, sizeof(DT_GUILD_FIGHT_ARENA_PLAYER_INFO));
    }
    rstArenaInfo.m_stPlayerList.m_bJoinerCnt = j;

    LOGRUN_r("Player(%s) Uin(%lu) join", rstPlayerInfo.m_szName, rstPlayerInfo.m_ullUin);

    return ERR_NONE;
}

int GuildFightArena::Quit(uint64_t ullPlayerId)
{
    m_mapId2PlayerIter = m_mapId2Player.find(ullPlayerId);
    if (m_mapId2PlayerIter == m_mapId2Player.end())
    {
        return ERR_NOT_FOUND;
    }

    //将玩家从所在据点退出
    FightPlayer* poFightPlayer = m_mapId2PlayerIter->second;
    assert(poFightPlayer);
    poFightPlayer->m_poGuildFightPoint->DelPlayer(poFightPlayer);

    //将玩家状态置为退出
    poFightPlayer->m_stPlayerInfo.m_wState = GUILD_FIGHT_PLAYER_STATE_QUIT;
    poFightPlayer->m_stPlayerInfo.m_ullStateParam = m_ullTimeMs;

    //准备阶段没有退出惩罚，开战阶段退出后会有惩罚
    if (m_iState == GUILD_FIGHT_ARENA_STATE_START)
    {
        poFightPlayer->m_stPlayerInfo.m_ullStateParam += m_ullJoinCoolTimeMs;
    }

    //将玩家退出的状态加入广播消息
    this->AddStateSync(poFightPlayer->m_stPlayerInfo);

    //从PlayerList中删除
#if 0
        uint8_t bCamp = poFightPlayer->m_stPlayerInfo.m_bCamp;
        size_t nmemb = (size_t)m_GuildPlayerList[bCamp -1].m_bPlayerCnt;
        MyBDelete(&ullPlayerId, m_GuildPlayerList[bCamp -1].m_PlayerList, &nmemb, sizeof(uint64_t), PlayerIdCmp);
        m_GuildPlayerList[bCamp -1].m_bPlayerCnt = nmemb;
#endif

    LOGRUN_r("Player Uin(%lu) quit", ullPlayerId);

    return ERR_NONE;
}

int GuildFightArena::Move(uint64_t ullPlayerId, uint16_t wDstPoint)
{
    if (m_iState != GUILD_FIGHT_ARENA_STATE_START)
    {
        return ERR_DEFAULT;
    }

    FightPlayer* poFightPlayer = GetPlayer(ullPlayerId);
    if (poFightPlayer == NULL)
    {
        return ERR_NOT_FOUND;
    }

    return poFightPlayer->Move(wDstPoint);
}


uint64_t GuildFightArena::Settle()
{
    this->SettleRank();

    //先比较积分，积分高的获胜
    if (m_GuildInfoList[0].m_dwGuildScore > m_GuildInfoList[1].m_dwGuildScore)
    {
        m_ullWinGuild = m_GuildInfoList[0].m_ullGuildId;
        m_bWinCamp = GUILD_FIGHT_PLAYER_GROUP_RED;
    }
    else if (m_GuildInfoList[1].m_dwGuildScore > m_GuildInfoList[0].m_dwGuildScore)
    {
        m_ullWinGuild = m_GuildInfoList[1].m_ullGuildId;
        m_bWinCamp = GUILD_FIGHT_PLAYER_GROUP_BLUE;
    }
    //积分相同的情况下比较杀敌数
    else if (m_GuildStatisList[0].m_wKillNum > m_GuildStatisList[1].m_wKillNum)
    {
        m_ullWinGuild = m_GuildInfoList[0].m_ullGuildId;
        m_bWinCamp = GUILD_FIGHT_PLAYER_GROUP_RED;
    }
    else if (m_GuildStatisList[1].m_wKillNum > m_GuildStatisList[0].m_wKillNum)
    {
        m_ullWinGuild = m_GuildInfoList[1].m_ullGuildId;
        m_bWinCamp = GUILD_FIGHT_PLAYER_GROUP_BLUE;
    }
    //都相同则比较报名资金
	else
	{
        uint32_t dwRedFund = GuildFightMgr::Instance().GetFightApplyFund(m_GuildInfoList[0].m_ullGuildId);
        uint32_t dwBlueFund = GuildFightMgr::Instance().GetFightApplyFund(m_GuildInfoList[1].m_ullGuildId);
        if (dwRedFund > dwBlueFund)
        {
		    m_ullWinGuild = m_GuildInfoList[0].m_ullGuildId;
            m_bWinCamp = GUILD_FIGHT_PLAYER_GROUP_RED;
        }
        else if (dwRedFund > dwBlueFund)
        {
            m_ullWinGuild = m_GuildInfoList[1].m_ullGuildId;
            m_bWinCamp = GUILD_FIGHT_PLAYER_GROUP_BLUE;
        }
        else if (m_GuildInfoList[0].m_ullGuildId > m_GuildInfoList[1].m_ullGuildId)
        {
            m_ullWinGuild = m_GuildInfoList[0].m_ullGuildId;
            m_bWinCamp = GUILD_FIGHT_PLAYER_GROUP_RED;
        }
        else
        {
            m_ullWinGuild = m_GuildInfoList[1].m_ullGuildId;
            m_bWinCamp = GUILD_FIGHT_PLAYER_GROUP_BLUE;
        }
	}

    m_iState = GUILD_FIGHT_ARENA_STATE_END;

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_FIGHT_SETTLE_NTF;
    SS_PKG_GUILD_FIGHT_SETTLE_NTF& rstSettleNtf = m_stSsPkg.m_stBody.m_stGuildFightSettleNtf;
    rstSettleNtf.m_stSettleInfo.m_bWinnerCamp = m_bWinCamp;
    rstSettleNtf.m_stSettleInfo.m_ullFightTimeMs = m_ullTimeMs - m_ullStartTimeMs;

    for (int i=0; i<MAX_NUM_ONE_GUILD_FIGHT_JOIN_GUILD; i++)
    {
        rstSettleNtf.m_stSettleInfo.m_astStatisInfo[i] = m_GuildStatisList[i];
    }

    int iIndex = 0;
    rstSettleNtf.m_bReceiverCnt = 0;
    for (int i=0; i<MAX_NUM_ONE_GUILD_FIGHT_JOIN_GUILD; i++)
    {
        rstSettleNtf.m_bReceiverCnt += m_GuildPlayerList[i].m_bPlayerCnt;

        for (int j=0; j<m_GuildPlayerList[i].m_bPlayerCnt; j++)
        {
            rstSettleNtf.m_ReceiverList[iIndex++] = m_GuildPlayerList[i].m_PlayerList[j];
        }
    }

	//立即发放奖励
	GuildFightMgr::Instance().FightSettle(m_GuildInfoList, m_ullWinGuild);

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return m_ullWinGuild;
}


void GuildFightArena::SettleRank()
{
    FightPlayer* temp;
    FightPlayer* astPlayerList[MAX_NUM_MEMBER];

    for (int i = 0; i < MAX_NUM_ONE_GUILD_FIGHT_JOIN_GUILD; i++)
    {
        int iCount = 0;
        for (int j = 0; j < m_GuildPlayerList[i].m_bPlayerCnt; j++)
        {
            FightPlayer* pstPlayer = this->GetPlayer(m_GuildPlayerList[i].m_PlayerList[j]);
            if (!pstPlayer)
            {
                LOGRUN("Player(%lu) is not in arena", m_GuildPlayerList[i].m_PlayerList[j]);
                continue;
            }
            if (pstPlayer->m_stPlayerInfo.m_wScore == 0)
            {
                continue;
            }

            astPlayerList[iCount] = pstPlayer;

            for (int k = iCount - 1; k >= 0; k--)
            {
                if (astPlayerList[k+1]->m_stPlayerInfo.m_wScore > astPlayerList[k]->m_stPlayerInfo.m_wScore)
                {
                    temp = astPlayerList[k+1];
                    astPlayerList[k+1] = astPlayerList[k];
                    astPlayerList[k]= temp;
                }
                else
                {
                    break;
                }
            }

            iCount++;
        }
        this->_SettleRank(iCount, astPlayerList);
    }
}


void GuildFightArena::_SettleRank(uint8_t bCount, FightPlayer* PlayerList[])
{
   //发送活跃度奖励
   RESPRIMAIL* poResPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(22);
   if (!poResPriMail)
   {
       LOGERR_r("Can't find the mail <22>");
       return;
   }

   m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
   SS_PKG_MAIL_ADD_REQ& rstMailAddReq = m_stSsPkg.m_stBody.m_stMailAddReq;
   rstMailAddReq.m_nUinCount = 0;
   DT_MAIL_DATA& rstMailData = rstMailAddReq.m_stMailData;
   rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
   rstMailData.m_bState = MAIL_STATE_UNOPENED;
   StrCpy(rstMailData.m_szTitle, poResPriMail->m_szTitle, MAX_MAIL_TITLE_LEN);
   StrCpy(rstMailData.m_szContent, poResPriMail->m_szContent, MAX_MAIL_CONTENT_LEN);
   rstMailData.m_ullFromUin = 0;
   rstMailData.m_ullTimeStampMs = CGameTime::Instance().GetCurrSecond();

   ResGFightScoreRankRewardMgr_t& rstResRankMgr = CGameDataMgr::Instance().GetResGFightScoreRankRewardMgr();
   int iCount = rstResRankMgr.GetResNum();
   int iStartRank = 1;
   int iEndRank = bCount;
   for (int i=0; i<iCount; i++)
   {
       RESGFIGHTSCORERANKREWARD* pResReward = rstResRankMgr.GetResByPos(i);
       rstMailData.m_bAttachmentCount = pResReward->m_bCount;
       for (int k=0; k<pResReward->m_bCount; k++)
       {
           rstMailData.m_astAttachmentList[k].m_bItemType = pResReward->m_szPropsType[k];
           rstMailData.m_astAttachmentList[k].m_dwItemId = pResReward->m_propsId[k];
           rstMailData.m_astAttachmentList[k].m_iValueChg = pResReward->m_propsNum[k];
       }

       for (int j=iStartRank; j<=(int)pResReward->m_dwRank && j<=iEndRank; j++)
       {
           rstMailAddReq.m_UinList[rstMailAddReq.m_nUinCount++] = PlayerList[j-1]->m_stPlayerInfo.m_ullUin;
       }
       iStartRank = pResReward->m_dwRank + 1;

       if (rstMailAddReq.m_nUinCount >= 0)
       {
           GuildSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
           rstMailAddReq.m_nUinCount = 0;
       }
   }
}


void GuildFightArena::GainScore(uint8_t bCamp, uint16_t wPointId, uint32_t dwScore)
{
    if (bCamp < GUILD_FIGHT_PLAYER_GROUP_RED || bCamp > GUILD_FIGHT_PLAYER_GROUP_BLUE)
    {
        LOGERR("bCamp is vailid. Camp=%d, PointId=%d", bCamp, wPointId);
        return;
    }

    DT_GUILD_FIGHT_GAIN_SCORE_LIST& rstGainScoreList = m_stSynMsg.m_stGainScoreInfo;
    uint8_t bIndex = rstGainScoreList.m_bCount;
    if (bIndex >= MAX_COUNT_GUILD_FIGHT_GAIN_SCORE_LIST)
	{
        LOGERR_r("GainScore failed, bIndex is larger than MAX_COUNT_GUILD_FIGHT_GAIN_SCORE_LIST");
		return;
	}

    uint16_t wGainScoreRate = 100;
    if (m_GuildInfoList[bCamp -1].m_bOwnPoint >= m_wGainScorePoint2)
    {
        wGainScoreRate = m_wGainScoreRate2;
    }
    else if (m_GuildInfoList[bCamp -1].m_bOwnPoint >= m_wGainScorePoint1)
    {
        wGainScoreRate = m_wGainScoreRate1;
    }

    dwScore = dwScore * wGainScoreRate / 100;

    m_GuildInfoList[bCamp-1].m_dwGuildScore += dwScore;
	// 不超过最大分数
	if (m_GuildInfoList[bCamp-1].m_dwGuildScore > m_dwWinScore)
	{
		m_GuildInfoList[bCamp-1].m_dwGuildScore = m_dwWinScore;
	}

    rstGainScoreList.m_bCount++;
    rstGainScoreList.m_astScoreInfo[bIndex].m_bCamp = bCamp;
    rstGainScoreList.m_astScoreInfo[bIndex].m_wPointId = wPointId;
    rstGainScoreList.m_astScoreInfo[bIndex].m_dwGainScore = dwScore;
    rstGainScoreList.m_astScoreInfo[bIndex].m_dwCurScore = m_GuildInfoList[bCamp-1].m_dwGuildScore;
}


FightPlayer* GuildFightArena::GetPlayer(uint64_t ullPlayerId)
{
    m_mapId2PlayerIter = m_mapId2Player.find(ullPlayerId);
    if (m_mapId2PlayerIter != m_mapId2Player.end())
    {
        return m_mapId2PlayerIter->second;
    }
    return NULL;
}


GuildFightPoint* GuildFightArena::GetPoint(uint16_t wPointId)
{
    m_mapId2PointIter = m_mapId2Point.find(wPointId);
    if (m_mapId2PointIter != m_mapId2Point.end())
    {
        return m_mapId2PointIter->second;
    }
    return NULL;
}

void GuildFightArena::AddStateSync(DT_GUILD_FIGHT_ARENA_PLAYER_INFO& rstArenaPlayerInfo)
{
	uint8_t bIndex = m_stSynMsg.m_stPlayerInfo.m_bJoinerCnt;
	if (bIndex >= MAX_NUM_MEMBER_DOUBLE)
	{
        LOGERR_r("AddStateSync failed, bIndex is larger than MAX_NUM_MEMBER_DOUBLE");
		return;
	}

	m_stSynMsg.m_stPlayerInfo.m_bJoinerCnt++;
	memcpy(&m_stSynMsg.m_stPlayerInfo.m_astJoinerList[bIndex], &rstArenaPlayerInfo, sizeof(DT_GUILD_FIGHT_ARENA_PLAYER_INFO));
}


void GuildFightArena::ChgStatisInfo(uint8_t bCamp, int iType, int iValue)
{
    DT_GUILD_FIGHT_STATIS_INFO& rstStatisInfo = m_GuildStatisList[bCamp -1];
    switch (iType)
    {
    case GUILD_FIGHT_STATIS_SIEGEDAMAGE:
        rstStatisInfo.m_dwSiegeDamage += iValue;
        break;
    case GUILD_FIGHT_STATIS_KILLNUM:
        rstStatisInfo.m_wKillNum += iValue;
        break;
    case GUILD_FIGHT_STATIS_CAPTUREPOINT:
        rstStatisInfo.m_dwCapturePointNum += iValue;
        break;
    default:
        LOGERR_r("iType(%d) is error", iType);
        break;
    }
}

