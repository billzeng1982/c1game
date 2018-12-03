#include "FightStatistics.h"
#include "workdir.h"
#include "LogMacros.h"
#include <stdio.h>

DwLog::DwLog()
{
    m_pstLogCtx = tlog_init_from_file("tlogconf.xml");
    m_pstPVPCat = tlog_get_category(m_pstLogCtx, "PVPLog");
    m_pstPVECat = tlog_get_category(m_pstLogCtx, "PVELog");
}

DwLog::~DwLog()
{
    if(m_pstLogCtx != NULL)
    {
        tlog_fini_ctx(&m_pstLogCtx);
    }
}

void DwLog::WriteDwLog(const CS_PKG_DW_LOG & rstCSPkgBody)
{
    switch(rstCSPkgBody.m_wType)
    {
        case CLT_FIGHT_STATS:
            WriteCltFightStats(rstCSPkgBody);
            break;
        case CLT_FIGHT_STATS_PVE:
            WriteCltFightStatsPVE(rstCSPkgBody);
            break;
        default:
            break;
    }
    return;
}

void DwLog::WriteDwLogPVE(const CltFightStatsPVE& stFightStatsPVE)
{
    if(NULL == m_pstPVECat)
    {
       LOGERR("m_pstCat is null");
       return;
    }

    char StarEvalIDList[DWLOG::MAX_PVE_STAR_COND * 10];
    sprintf(StarEvalIDList, "[%d|%d|%d]", stFightStatsPVE.m_StarEvalIDList[0], stFightStatsPVE.m_StarEvalIDList[1], stFightStatsPVE.m_StarEvalIDList[2]);
    char FallPropsList[DWLOG::MAX_PVE_FALL * 10];
    sprintf(FallPropsList, "[%d|%d|%d]", stFightStatsPVE.m_FallPropsList[0], stFightStatsPVE.m_FallPropsList[1], stFightStatsPVE.m_FallPropsList[2]);
    char FallPropsNumList[DWLOG::MAX_PVE_FALL * 10];
    sprintf(FallPropsNumList, "[%d|%d|%d]", stFightStatsPVE.m_szFallPropsNumList[0], stFightStatsPVE.m_szFallPropsNumList[1], stFightStatsPVE.m_szFallPropsNumList[2]);

    tlog_error(m_pstPVECat, 0, 0, "[CltFightStatsPVE]:%lu,%s,%u,%u,%d,%d,%d,%d,%s,%s,%d,%d,%s,%d,%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%u,%u",
            stFightStatsPVE.m_ullPlayerID, stFightStatsPVE.m_szPlayerName, stFightStatsPVE.m_dwPlayerLevel,
            stFightStatsPVE.m_dwFLevelID, stFightStatsPVE.m_chFLevelType, stFightStatsPVE.m_chFLevelHardType,
            stFightStatsPVE.m_bIsPass, stFightStatsPVE.m_chPassReason, stFightStatsPVE.m_szTimeStart,
            stFightStatsPVE.m_szTimeFinish, stFightStatsPVE.m_bStarEvalResult, stFightStatsPVE.m_bStarEvalIDCount,
            StarEvalIDList, stFightStatsPVE.m_chFallPropsCount, FallPropsList,
            FallPropsNumList, stFightStatsPVE.m_bKillTroopNum, stFightStatsPVE.m_bKillTowerNum,
            stFightStatsPVE.m_bKillBarrierNum, stFightStatsPVE.m_bDeadTroopNum, stFightStatsPVE.m_bGSkillUseNum,
            stFightStatsPVE.m_bMSkillUseNum, stFightStatsPVE.m_bCityHpSelf, stFightStatsPVE.m_bCityHpEnemy,
            stFightStatsPVE.m_dwDamageOut, stFightStatsPVE.m_dwDamageIn);
}

void DwLog::WriteCltFightStatsPVE(const CS_PKG_DW_LOG& rstCSPkgBody)
{
    CltFightStatsPVE stFightStatsPVE;
    TdrError::ErrorType iRet = stFightStatsPVE.unpack((char*)rstCSPkgBody.m_szBinLog, rstCSPkgBody.m_wLength);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
       LOGERR("Unpack CltFightStatsPVE pkg failed!");
       return;
    }

    WriteDwLogPVE(stFightStatsPVE);
}

void DwLog::WriteCltFightStats(const CS_PKG_DW_LOG & rstCSPkgBody)
{
    CltFightStatsArray stFightStatsArray;
    TdrError::ErrorType iRet = stFightStatsArray.unpack((char*)rstCSPkgBody.m_szBinLog, rstCSPkgBody.m_wLength);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
       LOGERR("Unpack CltFightStats pkg failed!");
       return;
    }

    if(NULL == m_pstPVPCat)
    {
       LOGERR("m_pstCat is null");
       return;
    }

    for(int i=0; i<stFightStatsArray.m_bCltFightStatsCNT; i++)
    {
        CltFightStats & stCltFightStats = stFightStatsArray.m_astCltFightStatsList[i];
        tlog_error(m_pstPVPCat, 0, 0, "[CltFightStats]:%s,%d,%s,%d,%d,%d,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
               stCltFightStats.m_szDateTime,
               stCltFightStats.m_iDungeonID, stCltFightStats.m_szPlayerName, stCltFightStats.m_iMasterSkillID,
			   stCltFightStats.m_iMasterSkillUsedTimes, stCltFightStats.m_iGeneralID, stCltFightStats.m_szGeneralName,
               stCltFightStats.m_iRepelTimes, stCltFightStats.m_iDeadWithdrawTimes, stCltFightStats.m_iActiveWithdrawTimes,
               stCltFightStats.m_iCastSkillTimes, stCltFightStats.m_iSkillHitNum,  stCltFightStats.m_iTotalHealIn,
               stCltFightStats.m_iSkillHealIn, stCltFightStats.m_iBuffHealIn, stCltFightStats.m_iTotalHealOut,
               stCltFightStats.m_iSkillHealOut, stCltFightStats.m_iBuffHealOut, stCltFightStats.m_iTotalDamageIn,
               stCltFightStats.m_iNomarlAtkDamageIn, stCltFightStats.m_iSpearAtkDamageIn, stCltFightStats.m_iShootAtkDamageIn,
               stCltFightStats.m_iRushAtkDamageIn, stCltFightStats.m_iFaceAtkDamageIn, stCltFightStats.m_iSkillAtkDamageIn,
               stCltFightStats.m_iBuffDamageIn, stCltFightStats.m_iAtkCityDamageIn, stCltFightStats.m_iAmbushAtkDamageIn,
               stCltFightStats.m_iTotalDamageOut, stCltFightStats.m_iNomarlAtkDamageOut, stCltFightStats.m_iSpearAtkDamageOut,
               stCltFightStats.m_iShootAtkDamageOut, stCltFightStats.m_iRushAtkDamageOut, stCltFightStats.m_iFaceAtkDamageOut,
               stCltFightStats.m_iSkillAtkDamageOut, stCltFightStats.m_iBuffDamageOut, stCltFightStats.m_iAtkCityDamageOut,
               stCltFightStats.m_iAmbushAtkDamageOut, stCltFightStats.m_iRecoveryInCity, stCltFightStats.m_szWinner,
			   stCltFightStats.m_iGeneralStar, stCltFightStats.m_iGeneralMaxHP, stCltFightStats.m_iGeneralStrength,
			   stCltFightStats.m_iGeneralStrengthDef, stCltFightStats.m_iGeneralWit, stCltFightStats.m_iGeneralWitDef,
			   stCltFightStats.m_iBaseDamageAtkNormal, stCltFightStats.m_iBaseDamageAtkCity, stCltFightStats.m_dwArmyBaseValue,
			   stCltFightStats.m_bIsAutoBattle, stCltFightStats.m_iMaxFramePerSecond, stCltFightStats.m_iMinFramePerSecond, stCltFightStats.m_iAverageFramePerSecond);
    }

    return;
}

/*void DWLOG::ArrayToString(char* Dest, char* Src, int conut, int size)
{
    for (int i=0; i<count; i++)
    {
        sprintf(Dest, "%s%d", Dest, );
    }
}*/