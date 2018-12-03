#include "FightPVP.h"
#include "LogMacros.h"
#include "../gamedata/GameDataMgr.h"
#include "Consume.h"
#include "Props.h"
#include"Lottery.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "Item.h"
#include "ZoneLog.h"
#include "dwlog_svr.h"
#include "Message.h"

using namespace PKGMETA;
using namespace DWLOG;

#define RECV_REWARD_INTREVAL    (24 * 3600)

Fight3V3::Fight3V3()
{
#if 0
    m_bStreakTimes = 0;
    m_bStreakScore = 0;
    m_oMapScore2Id.clear();
#endif
}

Fight3V3::~Fight3V3()
{

}

bool Fight3V3::Init()
{
#if 0
    LOGRUN("init match 3v3 info");
    LOGRUN("---------------------------------------------------");

    if (!_InitChampionScore())
    {
        return false;
    }

    if (!_InitLvIndex())
    {
        return false;
    }

    LOGRUN("m_dwChampionScore = %u", m_dwChampionScore);
    LOGRUN("m_bStreakTimes = %u", m_bStreakTimes);
    LOGRUN("m_bStreakScore = %u", m_bStreakScore);
    LOGRUN("---------------------------------------------------");
#endif
    return true;
}

bool Fight3V3::_InitChampionScore()
{
#if 0
    ResScoreMgr_t& rstResMgr = CGameDataMgr::Instance().GetResScoreMgr();

    // 获取王者积分
    int iNum = rstResMgr.GetResNum();
    RESSCORE *pResScore = rstResMgr.GetResByPos(iNum - 1);
    if (!pResScore)
    {
        LOGERR("pResScore is null.");
        return false;
    }

    m_dwChampionScore = pResScore->m_dwScore;

    ResBasicMgr_t& rResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
    RESBASIC* poBasic = rResBasicMgr.Find(BASIC_3V3_STREAK_TIMES);
    if (!poBasic)
    {
        LOGERR("poBasic is null");
        return false;
    }
    m_bStreakTimes = (uint8_t)poBasic->m_para[0];

    poBasic = rResBasicMgr.Find(BASIC_3V3_STREAK_SCORE);
    if (!poBasic)
    {
        LOGERR("poBasic is null");
        return false;
    }
    m_bStreakScore = (uint8_t)poBasic->m_para[0];
#endif
    return true;
}

bool Fight3V3::_InitLvIndex()
{
#if 0
    ResScoreMgr_t& rstResMgr = CGameDataMgr::Instance().GetResScoreMgr();

    int iNum = rstResMgr.GetResNum();
    iNum--; // 王者不计算进来
    for (int i=0; i<iNum; i++)
    {
        RESSCORE *pResScore = rstResMgr.GetResByPos(i);
        if (!pResScore)
        {
            LOGERR("pResScore is null.");
            return false;
        }

        RESSCORE *pResScoreNext = rstResMgr.GetResByPos(i+1);
        if (!pResScoreNext)
        {
            LOGERR("pResScoreNext is null.");
            return false;
        }

        uint32_t dwScoreRange = pResScoreNext->m_dwScore - pResScore->m_dwScore; // 邻近等级之间的分数范围
        for (uint32_t s=0; s<dwScoreRange; s++)
        {
            m_oMapScore2Id.insert(MapScore2LvId_t::value_type(pResScore->m_dwScore + s, pResScore->m_dwId));
        }
    }
#endif
    return true;
}

void Fight3V3::HandleELOSettle(PlayerData* pData, DT_FIGHT_PLAYER_INFO* pstOppoInfo, uint8_t bWinGroup)
{
#if 0
    this->_HandleScore(pData, bWinGroup);
    this->_HandleEffiency(pData, pstOppoInfo, bWinGroup);
#endif
    return;
}

void Fight3V3::_HandleScore(PlayerData* pData, uint8_t bWinGroup)
{
#if 0
    DT_ROLE_ELO_INFO& rstInfo = pData->GetELOInfo();

    uint32_t & dwScore = rstInfo.m_stPvp3v3Info.m_stBaseInfo.m_dwScore;
    uint8_t & bStreakTimes = rstInfo.m_stPvp3v3Info.m_stBaseInfo.m_bStreakTimes;
    uint8_t & bELOLvId = rstInfo.m_stPvp3v3Info.m_stBaseInfo.m_bELOLvId;
	uint8_t & bELOLvIdHighest = rstInfo.m_stPvp3v3Info.m_stBaseInfo.m_bBELOLvIdHighest;
    rstInfo.m_stPvp3v3Info.m_stBaseInfo.m_wFightCount++;
    //胜利
    if (pData->m_oSelfInfo.m_chGroup == (int8_t)bWinGroup)
    {
        rstInfo.m_stPvp3v3Info.m_stBaseInfo.m_wWinCount++;
        bStreakTimes++;
        if (dwScore < m_dwChampionScore)
        {
            MapScore2LvId_t::iterator iter = m_oMapScore2Id.find(dwScore);
            if (iter == m_oMapScore2Id.end())
            {
                LOGERR("iter is null");
                return;
            }

            ResScoreMgr_t& rstResMgr = CGameDataMgr::Instance().GetResScoreMgr();
            RESSCORE *pResScore = rstResMgr.Find(iter->second);
            if (!pResScore)
            {
                LOGERR("pResScore is null");
                return;
            }

            // 检查是否有连胜标志
            if ((bStreakTimes >= m_bStreakTimes) && (pResScore->m_dwIsBonus == 1))
            {
                dwScore += m_bStreakScore;
                if (dwScore > m_dwChampionScore)
                {
                    dwScore = m_dwChampionScore;
                }
            }
            else
            {
                dwScore++;
            }

            // 计算排名等级
            // 新的积分对应的等级
            iter = m_oMapScore2Id.find(dwScore);
            if (iter == m_oMapScore2Id.end())
            {
                LOGERR("iter is null");
                return;
            }

            // 新的等级是否在临界值上
            pResScore = rstResMgr.Find(iter->second);
            if((uint32_t)(bELOLvId + 1) == iter->second)
            {
                if (pResScore->m_dwScore != dwScore)
                {
                    // 不在临界值及更新
                    bELOLvId = iter->second;
                }
            }
            else if((uint32_t)(bELOLvId + 2) == iter->second)
            {
                // 不在临界值及更新
                bELOLvId += 1;
            }
			//更新历史最高段位
			bELOLvIdHighest = bELOLvIdHighest<bELOLvId ? bELOLvId : bELOLvIdHighest;
        }
    }
    //失败
    else
    {
        // 清除连胜
        bStreakTimes = 0;

        // 检查当前等级是否减分
        MapScore2LvId_t::iterator iter = m_oMapScore2Id.find(dwScore);
        if (iter == m_oMapScore2Id.end())
        {
            LOGERR("iter is null");
            return;
        }

        ResScoreMgr_t& rstResMgr = CGameDataMgr::Instance().GetResScoreMgr();
        RESSCORE *pResScore = rstResMgr.Find(iter->second);
        if (!pResScore)
        {
            LOGERR("pScore is null");
            return;
        }

        if (pResScore->m_bIsMinus == 1 /*当前等级是否可以降级，减分*/)
        {
            dwScore--;

            // 计算排名等级, 新的积分对应的等级
            iter = m_oMapScore2Id.find(dwScore);
            if (iter == m_oMapScore2Id.end())
            {
                LOGERR("iter is null");
                return;
            }

            // 新的等级是否在临界值上
            pResScore = rstResMgr.Find(iter->second);
            if (pResScore->m_dwScore != dwScore)
            {
                // 不在临界值及更新
                bELOLvId = iter->second;
            }
        }
    }
#endif
}

void Fight3V3::_HandleEffiency(PlayerData* pData, DT_FIGHT_PLAYER_INFO* pstOppoInfo, uint8_t bWinGroup)
{
#if 0
    DT_ROLE_ELO_INFO& rstInfo = pData->GetELOInfo();
    uint32_t dwK = 5;
    float fRealRatio = 0.0f;
    float fExpRatio = 0;

    uint32_t & dwEfficiency = rstInfo.m_stPvp3v3Info.m_stBaseInfo.m_dwEfficiency;

    if (pData->m_oSelfInfo.m_chGroup == (int8_t)bWinGroup)
    {
        // 胜利
        fRealRatio = 1.0f;
    }
    else if (PLAYER_GROUP_NONE == bWinGroup)
    {
        // 平局
        fRealRatio = 0.5f;
    }
    else
    {
        // 失败
        fRealRatio = 0.0f;
    }

    // 查表获取期望胜率
    ResEffExpMgr_t& rstResEffExpMgr = CGameDataMgr::Instance().GetResEffExpMgr();
    if (dwEfficiency > pstOppoInfo->m_dwEfficiency)
    {
        // stronger
        uint32_t dwEffMinus = (dwEfficiency - pstOppoInfo->m_dwEfficiency);
        dwEffMinus = (dwEffMinus / 10) * 10;

        RESEFFICIENCYEXP* pstEffExp = rstResEffExpMgr.Find(dwEffMinus);
        if (!pstEffExp)
        {
            LOGERR("pstEffExp is null.");
            return;
        }

        fExpRatio = pstEffExp->m_fStrongerRatio;
    }
    else
    {
        // weaker
        uint32_t dwEffMinus = (pstOppoInfo->m_dwEfficiency -dwEfficiency);
        dwEffMinus = (dwEffMinus / 10) * 10;

        RESEFFICIENCYEXP* pstResExp = rstResEffExpMgr.Find(dwEffMinus);
        if (!pstResExp)
        {
            LOGERR("pstResExp is null.");
            return;
        }

        fExpRatio = pstResExp->m_fWeakerRatio;
    }

    // K系数由战斗力差决定
    ResEffRatioMgr_t& rstResEffRatioMgr = CGameDataMgr::Instance().GetResEffRatioMgr();
    int iNum = rstResEffRatioMgr.GetResNum();
    RESEFFICIENCYRATIO* pstResRatio = NULL;
    for (int i=iNum-1; i>=0; i--)
    {
        pstResRatio = rstResEffRatioMgr.GetResByPos(i);
        if (!pstResRatio)
        {
            LOGERR("pstResRatio is null");
            return;
        }

        if (dwEfficiency > pstResRatio->m_dwEffLowRange)
        {
            dwK = pstResRatio->m_dwKRatio;
            break;
        }
    }
    dwEfficiency += (uint32_t)(dwK * (fRealRatio - fExpRatio));
    LOGRUN("3v3 eff=%u, k=%u, real_ratio=%f, exp_ratio=%f", dwEfficiency, dwK, fRealRatio, fExpRatio);
#endif
}

Fight6V6::Fight6V6()
{
    m_bStreakTimes = 0;
    m_bStreakScore = 0;
    m_oMapScore2Id.clear();
}

Fight6V6::~Fight6V6()
{

}

bool Fight6V6::Init()
{
    LOGRUN("init match 6v6 info");
    LOGRUN("---------------------------------------------------");
    if (!_InitChampionScore())
    {
        return false;
    }

    if (!_InitLvIndex())
    {
        return false;
    }

    _InitTopList();

	RESBASIC* poBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(PVP_DAILY_REWARD_RESET_TIME);
	if (NULL == poBasic)
	{
		LOGERR("FATAL ERROR:poBasic is null");
		m_iUpdateTime = 0;
	}
	else
	{
		m_iUpdateTime = (int) poBasic->m_para[0];
	}

    LOGRUN("m_dwChampionScore = %u", m_dwChampionScore);
    LOGRUN("m_bStreakTimes = %u", m_bStreakTimes);
    LOGRUN("m_bStreakScore = %u", m_bStreakScore);
    LOGRUN("---------------------------------------------------");

    return true;
}

bool Fight6V6::_InitChampionScore()
{
    ResScoreMgr_t& rstResMgr = CGameDataMgr::Instance().GetResScoreMgr();

    // 获取王者积分
    int iNum = rstResMgr.GetResNum();
    RESSCORE *pResScore = rstResMgr.GetResByPos(iNum - 1);
    if (!pResScore)
    {
        LOGERR("pResScore is null.");
        return false;
    }

    m_dwChampionScore = pResScore->m_dwScore;

    ResBasicMgr_t& rResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
    RESBASIC* poBasic = rResBasicMgr.Find(BASIC_6V6_STREAK_TIMES);
    if (!poBasic)
    {
        LOGERR("poBasic is null");
        return false;
    }
    m_bStreakTimes = (uint8_t)poBasic->m_para[0];

    poBasic = rResBasicMgr.Find(BASIC_6V6_STREAK_SCORE);
    if (!poBasic)
    {
        LOGERR("poBasic is null");
        return false;
    }
    m_bStreakScore = (uint8_t)poBasic->m_para[0];

    return true;
}

bool Fight6V6::_InitLvIndex()
{
    ResScoreMgr_t& rstResMgr = CGameDataMgr::Instance().GetResScoreMgr();

    int iNum = rstResMgr.GetResNum();
    iNum--;
    for (int i=0; i<iNum; i++)
    {
        RESSCORE *pResScore = rstResMgr.GetResByPos(i);
        if (!pResScore)
        {
            LOGERR("pResScore is null.");
            return false;
        }

        RESSCORE *pResScoreNext = rstResMgr.GetResByPos(i+1);
        if (!pResScoreNext)
        {
            LOGERR("pResScoreNext is null.");
            return false;
        }

        uint32_t dwScoreRange = pResScoreNext->m_dwScore - pResScore->m_dwScore; // 邻近等级之间的分数范围
        for (uint32_t s=0; s<dwScoreRange; s++)
        {
            m_oMapScore2Id.insert(MapScore2LvId_t::value_type(pResScore->m_dwScore + s, pResScore->m_dwId));
        }
    }
    // 最后一个段位 处理
    RESSCORE *pResScorFin = rstResMgr.GetResByPos(iNum);
    if (!pResScorFin)
    {
        LOGERR("pResScorFin is null.");
        return false;
    }
    m_oMapScore2Id.insert(MapScore2LvId_t::value_type(pResScorFin->m_dwScore, pResScorFin->m_dwId));
    return true;
}

void Fight6V6::_InitTopList()
{
	m_bTopListCount = 0;
	bzero(&m_TopList, sizeof(DT_RANK_INFO)*MAX_RANK_TOP_NUM);

	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_6V6_RANK_GET_TOPLIST_REQ;
	m_stSsPkg.m_stBody.m_stRank6V6GetTopReq.m_dwPad = MATCH_TYPE_6V6;

	ZoneSvrMsgLayer::Instance().SendToRankSvr(m_stSsPkg);
}

void Fight6V6::HandleELOSettle(PlayerData* pData, uint8_t bWinGroup)
{
    this->_HandleScore(pData, bWinGroup);
    //this->_HandleEffiency(pData, pstOppoInfo, bWinGroup);
    return;
}

void Fight6V6::_HandleScore(PlayerData* pData, uint8_t bResult)
{
    DT_ROLE_ELO_INFO& rstInfo = pData->GetELOInfo();

    uint32_t & dwScore = rstInfo.m_stPvp6v6Info.m_stBaseInfo.m_dwScore;
    uint8_t & bWinningStreakTimes = rstInfo.m_stPvp6v6Info.m_stBaseInfo.m_bStreakTimes;
    uint8_t & bELOLvId = rstInfo.m_stPvp6v6Info.m_stBaseInfo.m_bELOLvId;
    uint8_t bOldELOLvId = bELOLvId;
	uint8_t & bELOLvIdHighest = rstInfo.m_stPvp6v6Info.m_stBaseInfo.m_bBELOLvIdHighest;

    //胜利
    if (bResult == 1)
    {
        rstInfo.m_stPvp6v6Info.m_stBaseInfo.m_wWinCount++;
        bWinningStreakTimes++;
        if (dwScore < m_dwChampionScore)
        {
            MapScore2LvId_t::iterator iter = m_oMapScore2Id.find(dwScore);
            if (iter == m_oMapScore2Id.end())
            {
                LOGERR("Uin<%lu> iter is null", pData->m_ullUin);
                return;
            }

            ResScoreMgr_t& rstResMgr = CGameDataMgr::Instance().GetResScoreMgr();
            RESSCORE *pResScore = rstResMgr.Find(iter->second);
            if (!pResScore)
            {
                LOGERR("Uin<%lu> pResScore is null", pData->m_ullUin);
                return;
            }
            // 检查是否有满足连胜条件
            if ((bWinningStreakTimes >= m_bStreakTimes) && (pResScore->m_dwIsBonus == 1))
            {
                dwScore += m_bStreakScore;
            }
            else
            {
                dwScore++;
            }
            // 计算排名等级
            // 新的积分对应的等级
            iter = m_oMapScore2Id.find(MIN(dwScore, m_dwChampionScore));
            if (iter == m_oMapScore2Id.end())
            {
                LOGERR("Uin<%lu> iter is null", pData->m_ullUin);
                return;
            }
            bELOLvId = iter->second;

			//更新历史最高段位
			bELOLvIdHighest = bELOLvIdHighest<bELOLvId ? bELOLvId : bELOLvIdHighest;
            if (bELOLvId > bOldELOLvId) //有升级段位
            {
                if (bELOLvId == 15) //黄金
                {
                    Message::Instance().AutoSendSysMessage(1801, "Name=%s", pData->GetRoleName());
                    Message::Instance().AutoSendWorldMessage(pData, 1805);
                }
                else if (bELOLvId == 10)    //铂金
                {
                    Message::Instance().AutoSendSysMessage( 1802, "Name=%s", pData->GetRoleName());
                    Message::Instance().AutoSendWorldMessage(pData, 1806);
                }
                else if (bELOLvId == 5)     //钻石
                {
                    Message::Instance().AutoSendSysMessage(1803, "Name=%s", pData->GetRoleName());
                    Message::Instance().AutoSendWorldMessage(pData, 1807);
                }
                else if (bELOLvId == 0)     //最强王者
                {
                    Message::Instance().AutoSendSysMessage( 1804, "Name=%s", pData->GetRoleName());
                    Message::Instance().AutoSendWorldMessage(pData, 1808);
                }
            }
        }
        else
        {
            dwScore++;
        }
    }
    //失败
    else
    {
        // 清除连胜
        bWinningStreakTimes = 0;
		rstInfo.m_stPvp6v6Info.m_stBaseInfo.m_wLoseCount++;

        // 检查当前等级是否减分
        MapScore2LvId_t::iterator iter = m_oMapScore2Id.find(MIN(m_dwChampionScore, dwScore));
        if (iter == m_oMapScore2Id.end())
        {
            LOGRUN("Uin<%lu> not need minus score", pData->m_ullUin);
            return;
        }

        ResScoreMgr_t& rstResMgr = CGameDataMgr::Instance().GetResScoreMgr();
        RESSCORE *pResScore = rstResMgr.Find(iter->second);
        if (!pResScore)
        {
            LOGERR("Uin<%lu> pScore<%u> is null", pData->m_ullUin, iter->second);
            return;
        }
        if (pResScore->m_bIsMinus == 1)
        {
            dwScore = MAX(dwScore - 1 , 0);
            // 计算排名等级, 新的积分对应的等级
            MapScore2LvId_t::iterator Newiter = m_oMapScore2Id.find(MIN(m_dwChampionScore ,dwScore));
            if (Newiter == m_oMapScore2Id.end())
            {
                LOGERR("Uin<%lu> iter is null, MaxELOLvIdScore<%u> CureScore<%u>", pData->m_ullUin,m_dwChampionScore, dwScore );
                return;
            }
            bELOLvId = Newiter->second;
        }
    }
}

void Fight6V6::_HandleEffiency(PlayerData* pData, DT_FIGHT_PLAYER_INFO* pstOppoInfo, uint8_t bWinGroup)
{
#if 0
    DT_ROLE_ELO_INFO& rstInfo = pData->GetELOInfo();
    uint32_t dwK = 5;
    float fRealRatio = 0.0f;
    float fExpRatio = 0;

    uint32_t & dwEfficiency = rstInfo.m_stPvp6v6Info.m_stBaseInfo.m_dwEfficiency;

    if (pData->m_oSelfInfo.m_chGroup == (int8_t)bWinGroup)
    {
        // 胜利
        fRealRatio = 1.0f;
    }
    else if (PLAYER_GROUP_NONE == bWinGroup)
    {
        // 平局
        fRealRatio = 0.5f;
    }
    else
    {
        // 失败
        fRealRatio = 0.0f;
    }

    // 查表获取期望胜率
    ResEffExpMgr_t& rstResEffExpMgr = CGameDataMgr::Instance().GetResEffExpMgr();
    if (dwEfficiency > pstOppoInfo->m_dwEfficiency)
    {
        // stronger
        uint32_t dwEffMinus = (dwEfficiency - pstOppoInfo->m_dwEfficiency);
        dwEffMinus = (dwEffMinus / 10) * 10;

        RESEFFICIENCYEXP* pstEffExp = rstResEffExpMgr.Find(dwEffMinus);
        if (!pstEffExp)
        {
            LOGERR("pstEffExp is null.");
            return;
        }

        fExpRatio = pstEffExp->m_fStrongerRatio;
    }
    else
    {
        // weaker
        uint32_t dwEffMinus = (pstOppoInfo->m_dwEfficiency -dwEfficiency);
        dwEffMinus = (dwEffMinus / 10) * 10;

        RESEFFICIENCYEXP* pstResExp = rstResEffExpMgr.Find(dwEffMinus);
        if (!pstResExp)
        {
            LOGERR("pstResExp is null.");
            return;
        }

        fExpRatio = pstResExp->m_fWeakerRatio;
    }

    // K系数由战斗力差决定
    ResEffRatioMgr_t& rstResEffRatioMgr = CGameDataMgr::Instance().GetResEffRatioMgr();
    int iNum = rstResEffRatioMgr.GetResNum();
    RESEFFICIENCYRATIO* pstResRatio = NULL;
    for (int i=iNum-1; i>=0; i--)
    {
        pstResRatio = rstResEffRatioMgr.GetResByPos(i);
        if (!pstResRatio)
        {
            LOGERR("pstResRatio is null");
            return;
        }

        if (dwEfficiency > pstResRatio->m_dwEffLowRange)
        {
            dwK = pstResRatio->m_dwKRatio;
            break;
        }
    }
    dwEfficiency += (uint32_t)(dwK * (fRealRatio - fExpRatio));
    LOGRUN("6v6 eff=%u, k=%u, real_ratio=%f, exp_ratio=%f", dwEfficiency, dwK, fRealRatio, fExpRatio);
#endif
}

void Fight6V6::_AddOneHistory(DT_ROLE_PVP_HISTORY_INFO* pHistoryInfo, DT_FIGHT_PLAYER_INFO* pstMyInfo,  uint8_t bWinGroup)
{
    pHistoryInfo->m_bGeneralCount = pstMyInfo->m_bTroopNum;
    for (int i=0; i<pHistoryInfo->m_bGeneralCount && i<MAX_TROOP_NUM_PVP; i++)
    {
        pHistoryInfo->m_astGeneralList[i].m_dwGeneralID = pstMyInfo->m_astTroopList[i].m_stGeneralInfo.m_dwId;
        pHistoryInfo->m_astGeneralList[i].m_bGrade = pstMyInfo->m_astTroopList[i].m_stGeneralInfo.m_bPhase;
    }

    pHistoryInfo->m_dwMSkillID = pstMyInfo->m_dwMasterSkillId;

    if (pstMyInfo->m_chGroup == (int8_t)bWinGroup)
    {
        pHistoryInfo->m_bResult = 1;
    }
    else
    {
        pHistoryInfo->m_bResult = 0;
    }
}

void Fight6V6::AddFightHistory(PlayerData* pData, DT_FIGHT_PLAYER_INFO* pstMyInfo, uint8_t bWinGroup, uint64_t ulTimeStamp)
{
    DT_ROLE_PVP_6V6_INFO & rstInfo = pData->GetELOInfo().m_stPvp6v6Info;
    if (rstInfo.m_bHistoryCount < MAX_HISTORY_NUM)
    {
        _AddOneHistory(&rstInfo.m_astHistoryList[rstInfo.m_bHistoryCount], pstMyInfo, bWinGroup);
        rstInfo.m_astHistoryList[rstInfo.m_bHistoryCount].m_ullDate = ulTimeStamp;
        rstInfo.m_bHistoryCount++;
    }
    else
    {
        for (int i=0; i<MAX_HISTORY_NUM-1; i++)
        {
            rstInfo.m_astHistoryList[i] = rstInfo.m_astHistoryList[i+1];
        }
        _AddOneHistory(&rstInfo.m_astHistoryList[MAX_HISTORY_NUM-1], pstMyInfo, bWinGroup);
        rstInfo.m_astHistoryList[MAX_HISTORY_NUM-1].m_ullDate = ulTimeStamp;
    }

    return;
}

void Fight6V6::UpdateTopList(uint8_t bTopListCount, DT_RANK_INFO* pstTopList)
{
#if 0
    if (pstTopList == NULL)
    {
        return;
    }

    if (bTopListCount > MAX_RANK_TOP_NUM)
    {
        bTopListCount = MAX_RANK_TOP_NUM;
    }

    m_bTopListCount = bTopListCount;
    memcpy(&m_TopList, pstTopList, bTopListCount*sizeof(DT_RANK_INFO));
#endif
}

void Fight6V6::GetTopList(DT_RANK_INFO* pstTopList, uint8_t* pbTopListCount)
{
#if 0
    if (pstTopList == NULL || pbTopListCount == NULL)
    {
        return;
    }

    memcpy(pstTopList, &m_TopList, m_bTopListCount*sizeof(DT_RANK_INFO));

    *pbTopListCount = m_bTopListCount;
#endif
}

int Fight6V6::UpdateServer()
{
    // PVP每日重置时间
    uint64_t ullUpdateTime = 0;
    if (CGameTime::Instance().IsNeedUpdateByHour(m_ullUpdateLastTime, m_iUpdateTime, ullUpdateTime))
    {
        m_ullUpdateLastTime = ullUpdateTime;
    }

    return 0;
}

// 初始化玩家更新
void Fight6V6::InitPlayerData(PlayerData* pstData)
{
	DT_ROLE_ELO_INFO& rstEloInfo = pstData->GetELOInfo();

	if (rstEloInfo.m_stPvp6v6Info.m_stDailyInfo.m_ullResetLastTime < m_ullUpdateLastTime)
	{
		rstEloInfo.m_stPvp6v6Info.m_stDailyInfo.m_ullResetLastTime = m_ullUpdateLastTime;

		// 每天PVP重置
		rstEloInfo.m_stPvp6v6Info.m_stDailyInfo.m_wWinCount = 0;
		rstEloInfo.m_stPvp6v6Info.m_stDailyInfo.m_dwRewardProgress = 0;
	}

	return;
}

// 在线玩家更新
void Fight6V6::UpdatePlayerData(PlayerData* pstData)
{
	DT_ROLE_ELO_INFO& rstEloInfo = pstData->GetELOInfo();

    if (rstEloInfo.m_stPvp6v6Info.m_stDailyInfo.m_ullResetLastTime < m_ullUpdateLastTime)
    {
		rstEloInfo.m_stPvp6v6Info.m_stDailyInfo.m_ullResetLastTime = m_ullUpdateLastTime;

		// 每天PVP重置
		rstEloInfo.m_stPvp6v6Info.m_stDailyInfo.m_wWinCount = 0;
		rstEloInfo.m_stPvp6v6Info.m_stDailyInfo.m_dwRewardProgress = 0;
    }

   return;
}

// PVP每天累积奖励领奖
int Fight6V6::HandlePVPDailyReward(PlayerData* pstData, uint32_t dwProgress, SC_PKG_COMMON_REWARD_RSP& rstScPkgBodyRsp)
{
	DT_ROLE_ELO_INFO& rstEloInfo = pstData->GetELOInfo();

	rstScPkgBodyRsp.m_bOtherParaCnt = 0;

	int iErrNo = ERR_NONE;
	do
	{
		if (rstEloInfo.m_stPvp6v6Info.m_stDailyInfo.m_wWinCount < dwProgress)
		{
			// 未达到胜场要求
			iErrNo = ERR_NOT_SATISFY_COND;
			LOGRUN("HandlePVPDailyReward ERR_NOT_SATISFY_COND, dwProgress = %u", dwProgress);
			break;
		}

		if (rstEloInfo.m_stPvp6v6Info.m_stDailyInfo.m_dwRewardProgress >= dwProgress)
		{
			// 已领奖
			iErrNo = ERR_NOT_SATISFY_COND;
			LOGRUN("HandlePVPDailyReward ERR_ALREADY_GET, dwProgress = %u, dwRewardProgress = %u",
				dwProgress, rstEloInfo.m_stPvp6v6Info.m_stDailyInfo.m_dwRewardProgress);
			break;
		}

		uint32_t dwRewardInfoId = 0;
		RESPVPDAILYREWARD* pstResPVPDailyReward = CGameDataMgr::Instance().GetResPVPDailyRewardMgr().Find(rstEloInfo.m_stPvp6v6Info.m_stBaseInfo.m_bELOLvId);
		if (pstResPVPDailyReward == NULL)
		{
			LOGERR("pstResPVPDailyReward is null, ResId = %u", (uint32_t)rstEloInfo.m_stPvp6v6Info.m_stBaseInfo.m_bELOLvId);
			iErrNo = ERR_SYS;
			break;
		}

		for (int i=0; i<RES_MAX_PVP_DAILY_REWARD_LEVEL; i++)
		{
			if (pstResPVPDailyReward->m_winCnt[i] == dwProgress)
			{
				dwRewardInfoId = pstResPVPDailyReward->m_rewardInfoId[i];
				break;
			}
		}

		RESPVPREWARDINFO* pstResPVPRewardInfo = CGameDataMgr::Instance().GetResPVPRewardInfoMgr().Find(dwRewardInfoId);
		if (pstResPVPRewardInfo == NULL)
		{
			LOGERR("pstResPVPRewardInfo is null, ResId = %u", dwRewardInfoId);
			iErrNo = ERR_SYS;
			break;
		}

		for (int i=0; i<RES_MAX_PVP_REWARD_COUNT; i++)
		{
			if (pstResPVPRewardInfo->m_szRewardType[i] == 0)
			{
				break;
			}

			//注明来源
			Item::Instance().RewardItem(pstData, pstResPVPRewardInfo->m_szRewardType[i], pstResPVPRewardInfo->m_rewardId[i], pstResPVPRewardInfo->m_rewardNum[i],
				rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_PVP_DAILY_REWARD);
		}

	} while (false);

	if (iErrNo != ERR_NONE)
	{
		rstScPkgBodyRsp.m_dwDrawProgress = rstEloInfo.m_stPvp6v6Info.m_stDailyInfo.m_dwRewardProgress;

		// 返回当前胜场记录
		rstScPkgBodyRsp.m_bOtherParaCnt = 1;
		rstScPkgBodyRsp.m_OtherPara[0] = rstEloInfo.m_stPvp6v6Info.m_stDailyInfo.m_wWinCount;

		return iErrNo;
	}

	rstScPkgBodyRsp.m_dwDrawProgress = dwProgress;

	// 更新领奖记录
	rstEloInfo.m_stPvp6v6Info.m_stDailyInfo.m_dwRewardProgress = dwProgress;

	ZoneLog::Instance().WriteAwardLog(pstData, METHOD_PVP_DAILY_REWARD, rstScPkgBodyRsp.m_stSyncItemInfo);

	return iErrNo;
}

// PVP赛季段位领奖
int Fight6V6::HandlePVPSeasonReward(PlayerData* pstData, uint32_t dwProgress, SC_PKG_COMMON_REWARD_RSP& rstScPkgBodyRsp)
{
	DT_ROLE_ELO_INFO& rstEloInfo = pstData->GetELOInfo();

	rstScPkgBodyRsp.m_bOtherParaCnt = 0;

	int iErrNo = ERR_NONE;

	do
	{
		if (rstEloInfo.m_stPvp6v6Info.m_stBaseInfo.m_bELOLvId < dwProgress)
		{
			iErrNo = ERR_NOT_SATISFY_COND;
			LOGRUN("HandlePVPSeasonReward ERR_NOT_SATISFY_COND, dwProgress = %u", dwProgress);
			break;
		}

		if (rstEloInfo.m_stPvp6v6Info.m_stSeasonInfo.m_dwRewardProgress >= dwProgress)
		{
			// 已领奖
			iErrNo = ERR_NOT_SATISFY_COND;
			LOGRUN("HandlePVPSeasonReward ERR_ALREADY_GET, dwProgress = %u, dwRewardProgress = %u",
				dwProgress, rstEloInfo.m_stPvp6v6Info.m_stSeasonInfo.m_dwRewardProgress);
			break;
		}

		RESPVPSEASONREWARD* pstResPVPSeasonReward = CGameDataMgr::Instance().GetResPVPSeasonRewardMgr().Find(dwProgress);
		if (pstResPVPSeasonReward == NULL)
		{
			LOGERR("pstResPVPSeasonReward is null, ResId = %u", dwProgress);
			iErrNo = ERR_SYS;
			break;
		}

		RESPVPREWARDINFO* pstResPVPRewardInfo = CGameDataMgr::Instance().GetResPVPRewardInfoMgr().Find(pstResPVPSeasonReward->m_dwRewardInfoId);
		if (pstResPVPRewardInfo == NULL)
		{
			LOGERR("pstResPVPRewardInfo is null, ResId = %u", pstResPVPSeasonReward->m_dwRewardInfoId);
			iErrNo = ERR_SYS;
			break;
		}

		for (int i=0; i<RES_MAX_PVP_REWARD_COUNT; i++)
		{
			if (pstResPVPRewardInfo->m_szRewardType[i] == 0)
			{
				break;
			}

			//注明来源
			Item::Instance().RewardItem(pstData, pstResPVPRewardInfo->m_szRewardType[i], pstResPVPRewardInfo->m_rewardId[i], pstResPVPRewardInfo->m_rewardNum[i],
				 rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_PVP_SEASON_REWARD);
		}
	} while (false);

	if (iErrNo != ERR_NONE)
	{
		rstScPkgBodyRsp.m_dwDrawProgress = rstEloInfo.m_stPvp6v6Info.m_stSeasonInfo.m_dwRewardProgress;

		// 返回当前段位
		rstScPkgBodyRsp.m_bOtherParaCnt = 1;
		rstScPkgBodyRsp.m_OtherPara[0] = rstEloInfo.m_stPvp6v6Info.m_stBaseInfo.m_bELOLvId;

		return iErrNo;
	}

	rstScPkgBodyRsp.m_dwDrawProgress = dwProgress;

	// 更新领奖记录
	rstEloInfo.m_stPvp6v6Info.m_stSeasonInfo.m_dwRewardProgress = dwProgress;

	ZoneLog::Instance().WriteAwardLog(pstData, METHOD_PVP_SEASON_REWARD, rstScPkgBodyRsp.m_stSyncItemInfo);

	return iErrNo;
}

int Fight6V6::SendRankInfoToRankSvr(PlayerData* poPlayerData, SS_PKG_FIGHT_SETTLE_NTF& rstSettleNtf)
{
    if (!poPlayerData)
    {
        return -1;
    }

    DT_ROLE_PVP_BASE_INFO & rstRole6v6PvpInfo = poPlayerData->GetELOInfo().m_stPvp6v6Info.m_stBaseInfo;
	DT_ROLE_MAJESTY_INFO& rstMajestyInfo = poPlayerData->GetMajestyInfo();

    // check condition
    if (rstRole6v6PvpInfo.m_dwScore < m_bStreakScore)
    {
        // 没有达到
        return -1;
    }

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_6V6_RANK_UPDATE_REQ;
    SS_PKG_6V6_RANK_UPDATE_REQ & rstSsPkgBodyReq = m_stSsPkg.m_stBody.m_stRank6V6UpdateReq;
    rstSsPkgBodyReq.m_bMatchType = rstSettleNtf.m_stMyInfo.m_bMatchType;
    rstSsPkgBodyReq.m_chGroup = rstSettleNtf.m_chGroup;
    rstSsPkgBodyReq.m_chReason = rstSettleNtf.m_chReason;
    rstSsPkgBodyReq.m_ullTimeStamp = rstSettleNtf.m_ullTimeStamp;

    rstSsPkgBodyReq.m_bIsWin = rstSettleNtf.m_stMyInfo.m_chGroup == rstSettleNtf.m_chGroup ? 1 : 0;

    rstSsPkgBodyReq.m_stReqInfo.m_dwELOId = rstRole6v6PvpInfo.m_bELOLvId;
    rstSsPkgBodyReq.m_stReqInfo.m_dwScore = rstRole6v6PvpInfo.m_dwScore;
    rstSsPkgBodyReq.m_stReqInfo.m_dwEffiency = rstMajestyInfo.m_dwHighestLi;
    rstSsPkgBodyReq.m_stReqInfo.m_ullTimeStampMs = CGameTime::Instance().GetCurrTimeMs();
    StrCpy(rstSsPkgBodyReq.m_stReqInfo.m_szAccountName, poPlayerData->GetRoleBaseInfo().m_szRoleName, PKGMETA::MAX_NAME_LENGTH);
    rstSsPkgBodyReq.m_stReqInfo.m_ullUin = rstSettleNtf.m_stMyInfo.m_ullUin;
    rstSsPkgBodyReq.m_stReqInfo.m_bGeneralCount = rstSettleNtf.m_stMyInfo.m_bTroopNum;
    for (int i=0; i<rstSsPkgBodyReq.m_stReqInfo.m_bGeneralCount && i<MAX_TROOP_NUM_PVP; i++)
    {
        rstSsPkgBodyReq.m_stReqInfo.m_astGeneralList[i].m_dwGeneralID = rstSettleNtf.m_stMyInfo.m_astTroopList[i].m_stGeneralInfo.m_dwId;
        rstSsPkgBodyReq.m_stReqInfo.m_astGeneralList[i].m_bGrade = rstSettleNtf.m_stMyInfo.m_astTroopList[i].m_stGeneralInfo.m_bPhase;
    }
	rstSsPkgBodyReq.m_stReqInfo.m_bVipLv = rstMajestyInfo.m_bVipLv;

    ZoneSvrMsgLayer::Instance().SendToRankSvr(m_stSsPkg);

    return 0;
}
