#include "LogMacros.h"
#include "GameDataMgr.h"

bool CGameDataMgr::Init()
{
    if( !INIT_RES_MGR(m_oResBasicMgr) )			return false;
    LOGRUN("m_oResBasicMgr init success.");

    if( !INIT_RES_MGR(m_oResArmyMgr ) )			return false;
    if( !INIT_RES_MGR(m_oResArmyRestrainMgr) )	return false;
    if( !INIT_RES_MGR(m_oResGeneralMgr) )		return false;
    if( !INIT_RES_MGR(m_oResGeneralLevelCapMgr) )		return false;
    if( !INIT_RES_MGR(m_oResGeneralLevelGrowMgr) )		return false;
    if( !INIT_RES_MGR(m_oResGeneralPhaseMgr) )	return false;
    if( !INIT_RES_MGR(m_oResGeneralStarMgr) )	return false;
    if( !INIT_RES_MGR(m_oResGeneralLevelMgr) )	return false;

    LOGRUN("m_oResGeneralMgr init success.");

    if( !INIT_RES_MGR(m_oResGeneralSkillMgr) )	return false;
    if( !INIT_RES_MGR(m_oResBuffMgr) )			return false;
    if( !INIT_RES_MGR(m_oResMasterSkillMgr) )	return false;
    LOGRUN("m_oResMasterSkillMgr init success.");

    if( !INIT_RES_MGR(m_oResScoreMgr) )			return false;
    if (!INIT_RES_MGR(m_oResPVPRewardMgr))		return false;
    if( !INIT_RES_MGR(m_oResEffExpMgr) )		return false;
    if( !INIT_RES_MGR(m_oResEffRatioMgr) )		return false;
    LOGRUN("m_oResEffRatioMgr init success.");

    if( !INIT_RES_MGR(m_oResEquipMgr) )      return false;
    if( !INIT_RES_MGR(m_oResEquipsListMgr) )    return false;
    if( !INIT_RES_MGR(m_oResEquipStarMgr) )      return false;
    if (!INIT_RES_MGR(m_oResGeneralEquipStarMgr))return false;
    LOGRUN("m_oResEquipStarMgr init success.");

    if( !INIT_RES_MGR(m_oResPropsMgr) )			return false;
	if( !INIT_RES_MGR(m_oResMajestyItemMgr) )	return false;
    if( !INIT_RES_MGR(m_oResConsumeMgr) )		return false;
    if( !INIT_RES_MGR(m_oResTreasureBoxMgr) )	return false;
	if( !INIT_RES_MGR(m_oResLevelBoxMgr) )		return false;
    if (!INIT_RES_MGR(m_oResPropsToDiamondMgr))	return false;
    if (!INIT_RES_MGR(m_oResTokenToDiamondMgr))	return false;
    LOGRUN("m_oResConsumeMgr init success.");

    if( !INIT_RES_MGR(m_oResMajestyLvMgr) )		return false;
    if( !INIT_RES_MGR(m_oResMajestyFunctionMgr) )	return false;
    if( !INIT_RES_MGR(m_oResMajestyGroupCardMgr) )	return false;
    if( !INIT_RES_MGR(m_oResPurchaseMgr) )		return false;

    if( !INIT_RES_MGR(m_oResTaskMgr) )			return false;
    if( !INIT_RES_MGR(m_oResTaskBonusMgr) )		return false;
    if (!INIT_RES_MGR(m_oResTaskFinalAwardMgr))		return false;
    if( !INIT_RES_MGR(m_oResFightLevelPLMgr) )	return false;
    if( !INIT_RES_MGR(m_oResFightLevelMgr) )	return false;
    if (!INIT_RES_MGR(m_oResFightLevelGeneralInfoMgr_t))		return false;
    LOGRUN("m_oResFightLevelPLMgr init success.");

    if( !INIT_RES_MGR(m_oResLotteryDrawTypeMgr) )   return false;
    if( !INIT_RES_MGR(m_oResPondLotteryMapMgr) )    return false;
    if( !INIT_RES_MGR(m_oResLotteryBonusMapMgr) )   return false;
    if( !INIT_RES_MGR(m_oResLotteryRuleMgr) )       return false;
    if( !INIT_RES_MGR(m_oResLotteryFreeCondMgr) )   return false;
    if( !INIT_RES_MGR(m_oResDiaLotteryRuleMgr) )   return false;


    LOGRUN("m_oResLotteryFreeCondMgr init success.");

    if( !INIT_RES_MGR(m_oResTutorialBonusMgr) )     return false;
    if( !INIT_RES_MGR(m_oResAPMgr) )                return false;
    if( !INIT_RES_MGR(m_oResGeneralPhaseUpMgr) )  return false;
    if( !INIT_RES_MGR(m_oResCheatsMgr) )  return false;
    if( !INIT_RES_MGR(m_oResRankPrizeMgr) )  return false;
    if( !INIT_RES_MGR(m_oResPveChapterRewardMgr) )  return false;
    LOGRUN("m_oResPveChapterRewardMgr init success.");

    if( !INIT_RES_MGR(m_oResBlackMarketBasicMgr) )  return false;
    if( !INIT_RES_MGR(m_oResBlackMarketRandomMgr) )  return false;
    if( !INIT_RES_MGR(m_oResBlackMarketUptPriceMgr) )  return false;
    if( !INIT_RES_MGR(m_oResBlackMarketGeneralRule1Mgr) )  return false;
    if( !INIT_RES_MGR(m_oResBlackMarketGeneralRule2Mgr) )  return false;
    if( !INIT_RES_MGR(m_oResBlackMarketEquipRule1Mgr) )  return false;
    if( !INIT_RES_MGR(m_oResBlackMarketEquipRule2Mgr) )  return false;

    if( !INIT_RES_MGR(m_oResSign7dAwardMgr) )  return false;
    if( !INIT_RES_MGR(m_oResSign30dAwardMgr) )  return false;
    if( !INIT_RES_MGR(m_oResSign30dAwardListMgr) )  return false;
    LOGRUN("m_oResSign30dAwardListMgr init success.");


    if( !INIT_RES_MGR(m_oResDebugMgr) )         return false;
    if( !INIT_RES_MGR(m_oResGuildTaskMgr) )		return false;
    if( !INIT_RES_MGR(m_oResGuildRewardMgr) )	return false;
    if (!INIT_RES_MGR(m_oResGuildLevelMgr))     return false;
    if (!INIT_RES_MGR(m_oResGuildActivityMgr))  return false;
    if( !INIT_RES_MGR(m_oResGuildBossInfoMgr) )			return false;
    if( !INIT_RES_MGR(m_oResGuildBossRewardMgr) )			return false;
	if( !INIT_RES_MGR(m_oResGuildSocietyMgr_t) )			return false;
	if( !INIT_RES_MGR(m_oResGuildSocietyInfoMgr_t) )		return false;
	if( !INIT_RES_MGR(m_oResGuildSalaryMgr_t) )	return false;
	if( !INIT_RES_MGR(m_oResGuildGeneralHangMgr_t) )	return false;

    LOGRUN("m_oResGuildLevelMgr init success.");

    if( !INIT_RES_MGR(m_oResGemListMgr) )      return false;
    if( !INIT_RES_MGR(m_oResGemLimitMgr) )      return false;


    if (!INIT_RES_MGR(m_oResGrowthAwardLevelMgr))     return false;
    if (!INIT_RES_MGR(m_oResGrowthAwardOnlineMgr))     return false;
    if (!INIT_RES_MGR(m_oResLvFundMgr))     return false;

    if (!INIT_RES_MGR(m_oResGeneralFateMgr))			return false;
	if (!INIT_RES_MGR(m_oResGeneralEquipFateMgr))		return false;

    if (!INIT_RES_MGR(m_oResActivityBonusMgr))			return false;
    if (!INIT_RES_MGR(m_oResSerialNumMgr))			    return false;

    if (!INIT_RES_MGR(m_oResActivityMgr))			return false;
    if (!INIT_RES_MGR(m_oResActivityRewardMgr))	    return false;
    if (!INIT_RES_MGR(m_oResSyntheticMgr))	    return false;
    if (!INIT_RES_MGR(m_oResSyntheticPropsMgr))	    return false;
    if (!INIT_RES_MGR(m_oResAfficheMgr))	    return false;

    if (!INIT_RES_MGR(m_oResVIPMgr))	    return false;
    if (!INIT_RES_MGR(m_oResGifPkgMgr))	    return false;

    if (!INIT_RES_MGR(m_oResMallPropsMgr))	    return false;

    if (!INIT_RES_MGR(m_oResMarqueeScreenMgr))	    return false;
    if (!INIT_RES_MGR(m_oResPriMailMgr))	    return false;
    if (!INIT_RES_MGR(m_oResSign30dExtraRewardMgr))	    return false;
    LOGRUN("m_oResSign30dExtraRewardMgr init success.");


    if (!INIT_RES_MGR(m_oResLiGCardBaseMgr	))	    return false;
    if (!INIT_RES_MGR(m_oResLiGCardSkillMgr	))	    return false;
    LOGRUN("m_oResLiMasterSkillMgr init success.");

    if (!INIT_RES_MGR(m_oResWeekLeagueParaMgr))       return false;
    if (!INIT_RES_MGR(m_oResWeekLeagueRewardMgr))     return false;

    if (!INIT_RES_MGR(m_oResPVPDailyRewardMgr))			return false;
    if (!INIT_RES_MGR(m_oResPVPSeasonRewardMgr))		return false;
    if (!INIT_RES_MGR(m_oResPVPRewardInfoMgr))			return false;

    if (!INIT_RES_MGR(m_oResLiRewardMgr))               return false;
    if (!INIT_RES_MGR(m_oResPayMgr))                    return false;
    if (!INIT_RES_MGR(m_oResFirstPayBagMgr))            return false;
    if (!INIT_RES_MGR(m_oResMonthDailyBagMgr))          return false;
    if (!INIT_RES_MGR(m_oResTotalPayBagMgr))            return false;
    if (!INIT_RES_MGR(m_oResPayActMgr))                 return false;
    if (!INIT_RES_MGR(m_oResTimeCycleMgr))              return false;
    if (!INIT_RES_MGR(m_oResDiscountPropsMgr))          return false;
    if (!INIT_RES_MGR(m_oResOutletsMgr))                return false;

	if (!INIT_RES_MGR(m_oResAsyncPvpRankRewardMgr))     return false;
    if (!INIT_RES_MGR(m_oResAsyncPvpScoreRewardMgr))    return false;
	if (!INIT_RES_MGR(m_oResCoinShopMgr))               return false;

    if(!INIT_RES_MGR(m_oResGeneralPhasePropsMgr))          return false;
    if(!INIT_RES_MGR(m_oResCommonRewardMgr))               return false;

    if(!INIT_RES_MGR(m_oResDailyChallengeBasicMgr))          return false;
    if(!INIT_RES_MGR(m_oResDailyChallengeMatchMgr))          return false;
    if(!INIT_RES_MGR(m_oResDailyChallengeBattleArrayMgr))    return false;
    if(!INIT_RES_MGR(m_oResDailyChallengeGeneralPoolMgr))    return false;
    if(!INIT_RES_MGR(m_oResDailyChallengeArmyForceMgr))      return false;
    if(!INIT_RES_MGR(m_oResDailyChallengeFightRewardMgr))    return false;
    if(!INIT_RES_MGR(m_oResDailyChallengeFinialRewardMgr))   return false;
    if(!INIT_RES_MGR(m_oResDailyChallengeSkipFightMgr))      return false;
	if(!INIT_RES_MGR(m_oResDailyChallengeBuffMgr))			 return false;
	LOGRUN("m_oResDailyChallengeSkipFightMgr init success.");

	if(!INIT_RES_MGR(m_oResFameHallTeamMgr))				return false;
	if(!INIT_RES_MGR(m_oResFameHallFriendShipRuleMgr))		return false;
	if(!INIT_RES_MGR(m_oResFameHallGeneralMgr))				return false;
    if(!INIT_RES_MGR(m_oResFameHallIncreaseAttrMgr))		return false;
    if(!INIT_RES_MGR(m_oResCloneBattleFightRewardMgr))		return false;

    if(!INIT_RES_MGR(m_oResPeakArenaParaMgr))	    return false;
    if(!INIT_RES_MGR(m_oResPeakArenaScoreMgr))      return false;
    if(!INIT_RES_MGR(m_oResPeakArenaRewardMgr))     return false;
    if(!INIT_RES_MGR(m_oResPeakArenaActiveRewardMgr))     return false;
    if(!INIT_RES_MGR(m_oResPeakArenaOutputRewardMgr))     return false;
    if(!INIT_RES_MGR(m_oResPeakArenaChooseRuleMgr))       return false;
    if(!INIT_RES_MGR(m_oResMessageMgr))       return false;

    if(!INIT_RES_MGR(m_oResMineMgr))       return false;
    if(!INIT_RES_MGR(m_oResTacticsMgr))         return false;
    if(!INIT_RES_MGR(m_oResGeneralSkinMgr))         return false;
    LOGRUN("all res data init success.");
    return true;
}
