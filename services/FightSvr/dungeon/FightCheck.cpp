#include "FightCheck.h"
#include "LogMacros.h"
#include "GameTime.h"
#include "oi_misc.h"
#include "FightSvrMsgLayer.h"

using namespace PKGMETA;

int IdCmp(const void *pstFirst, const void *pstSecond)
{
    DT_ONE_LAST_ATTACK_TIME_INFO* pstItemFirst = (DT_ONE_LAST_ATTACK_TIME_INFO*)pstFirst;
    DT_ONE_LAST_ATTACK_TIME_INFO* pstItemSecond = (DT_ONE_LAST_ATTACK_TIME_INFO*)pstSecond;

    int iResult = (int)pstItemFirst->m_bId - (int)pstItemSecond->m_bId;
    return iResult;
}

bool FightCheck::CheckCityWallAttFreq(FightObj* poSource, FightObj* poTarget, int16_t iValueChgType, DT_LAST_ATTACK_TIME_INFO& rstLastAttackTimeInfo)
{
    if(iValueChgType != VALUE_CHG_TYPE::CHG_HP_ATK_CITYGATE
        && iValueChgType != VALUE_CHG_TYPE::CHG_HP_ATK_CITYWALL)
    {
        return true;
    }

    if (poSource->m_chType != FIGHTOBJ_TROOP
        && poTarget->m_chType != FIGHTOBJ_WALL)
    {
        return true;
    }

    uint64_t ullTime = CGameTime::Instance().GetCurrTimeMs();
    DT_ONE_LAST_ATTACK_TIME_INFO stOneInfo;
    stOneInfo.m_bId = poSource->m_bId;
    stOneInfo.m_ullTimeStamp = ullTime;
    stOneInfo.m_bCount = 1;

    int iEqual = 0;
    int iIndex =MyBSearch(&stOneInfo, rstLastAttackTimeInfo.m_astDataList, rstLastAttackTimeInfo.m_bCount, sizeof(stOneInfo), &iEqual, IdCmp);

    if(!iEqual)
    {
        size_t nmemb = (size_t)rstLastAttackTimeInfo.m_bCount;
        if (nmemb >= MAX_ALL_TROOP_NUM_PVP)
        {
            LOGERR("rstLastAttackTimeInfo.m_astDataList reaches the max.");
            return true;
        }
        if (MyBInsert(&stOneInfo, rstLastAttackTimeInfo.m_astDataList, &nmemb, sizeof(stOneInfo), 1, IdCmp))
        {
            rstLastAttackTimeInfo.m_bCount = (uint16_t)nmemb;
        }

        return true;
    }

    DT_ONE_LAST_ATTACK_TIME_INFO& rstOneInfo = rstLastAttackTimeInfo.m_astDataList[iIndex];
    uint64_t ullDeta = ullTime - rstOneInfo.m_ullTimeStamp;
    rstOneInfo.m_ullTimeStamp = ullTime;
    LOGRUN("sourceId<%d>, ullDeta<%lu> rstOneInfo.m_bCount<%d>", poSource->m_bId, ullDeta, rstOneInfo.m_bCount);

    // 连续攻击间隔过短判定作弊
    if(ullDeta <= m_ullMinDetaTime)
    {
        rstOneInfo.m_bCount++;
    }
    else
    {
        rstOneInfo.m_bCount = 0;
    }
    if (rstOneInfo.m_bCount >= MaxOutNum)
    {
        return false;
    }

    return true;
}

int FightCheck::SendKickPlayerNtf(uint64_t ullUin, int iZoneSvrProcId)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_FIGHT_PLAYER_CHEAT_NTF;

    SS_PKG_FIGHT_PLAYER_CHEAT_NTF& rstNtf = m_stSsPkg.m_stBody.m_stFightPlayerCheatNtf;
    rstNtf.m_ullUin = ullUin;

    m_stSsPkg.m_stHead.m_ullReservId = (uint64_t)iZoneSvrProcId;
    FightSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);

    return 0;
}

void FightCheck::SetMinDetaTime(float value)
{
    m_ullMinDetaTime = (uint64_t)(value * MinDetaTimeRatio / 100);
    //LOGRUN("the MinDetaTime<%lu>", m_ullMinDetaTime);
}

