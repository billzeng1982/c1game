#pragma once

#include "../../../common/tresload/TGameDataMgr.h"
#include "../../../common/tresload/ov_res_keywords.h"
#include "../../../common/tresload/ov_res_public.h"
#include "../../../common/tresload/ov_res_server.h"
#include "../../../common/utils/singleton.h"
#include "../../../common/macros.h"


/*
    目前先启动时全部加载所有gamedata,后期内存优化时按需动态加载，卸载
*/


using namespace std;

typedef TGameDataMgr<RESBASIC>				ResBasicMgr_t;
typedef TGameDataMgr<RESMESSAGE>			ResMessageMgr_t;
typedef TGameDataMgr<RESARMY>				ResArmyMgr_t;
typedef TGameDataMgr<RESARMYRESTRAIN>		ResArmyRestrainMgr_t; // 兵种相克
typedef TGameDataMgr<RESGENERAL>			ResGeneralMgr_t;
typedef TGameDataMgr<RESGENERALPHASE>	  	ResGeneralPhaseMgr_t;
typedef TGameDataMgr<RESGENERALSTAR>	  	ResGeneralStarMgr_t;
typedef TGameDataMgr<RESGENERALEQUIPSTAR>   ResGeneralEquipStarMgr_t;
typedef TGameDataMgr<RESGENERALLEVEL>     	ResGeneralLevelMgr_t;
typedef TGameDataMgr<RESGENERALLEVELGROW>   ResGeneralLevelGrowMgr_t;
typedef TGameDataMgr<RESGENERALSKILL>		ResGeneralSkillMgr_t;
typedef TGameDataMgr<RESBUFF>				ResBuffMgr_t;
typedef TGameDataMgr<RESSCORE>				ResScoreMgr_t;
typedef TGameDataMgr<RESEFFICIENCYEXP>		ResEffExpMgr_t;
typedef TGameDataMgr<RESEFFICIENCYRATIO>	ResEffRatioMgr_t;
typedef TGameDataMgr<RESPVPREWARD>			ResPVPRewardMgr_t;

typedef TGameDataMgr<RESEQUIP>				ResEquipMgr_t;
typedef TGameDataMgr<RESEQUIPSTAR>			ResEquipStarMgr_t;
typedef TGameDataMgr<RESEQUIPSLIST>			ResEquipsListMgr_t;

typedef TGameDataMgr<RESPROPS>				ResPropsMgr_t;
typedef TGameDataMgr<RESCONSUME>			ResConsumeMgr_t;
typedef TGameDataMgr<RESTREASUREBOX>        ResTreasureBoxMgr_t;
typedef TGameDataMgr<RESLEVELBOX>			ResLevelBoxMgr_t;
typedef TGameDataMgr<RESMAJESTYLV>			ResMajestyLvMgr_t;
typedef TGameDataMgr<RESMAJESTYFUNCTION>	ResMajestyFunctionMgr_t;
typedef TGameDataMgr<RESPURCHASE>			ResPurchaseMgr_t;
typedef TGameDataMgr<RESMASTERSKILL>		ResMasterSkillMgr_t;
typedef TGameDataMgr<RESTASK>				ResTaskMgr_t;
typedef TGameDataMgr<RESTASKFINALAWARD>		ResTaskFinalAwardMgr_t;
typedef TGameDataMgr<RESTASKBONUS>			ResTaskBonusMgr_t;
typedef TGameDataMgr<RESLOTTERYDRAWTYPE>	ResLotteryDrawTypeMgr_t;
typedef TGameDataMgr<RESPONDLOTTERYMAP>		ResPondLotteryMapMgr_t;
typedef TGameDataMgr<RESLOTTERYBONUSMAP>	ResLotteryBonusMapMgr_t;
typedef TGameDataMgr<RESLOTTERYRULE>		ResLotteryRuleMgr_t;
typedef TGameDataMgr<RESLOTTERYFREECOND>	ResLotteryFreeCondMgr_t;
typedef TGameDataMgr<RESDIALOTTERYRULE>	    ResDiaLotteryRuleMgr_t;
typedef TGameDataMgr<RESTUTORIALBONUS>		ResTutorialBonusMgr_t;
typedef TGameDataMgr<RESPROPSTODIAMOND>     ResPropsToDiamondMgr_t;
typedef TGameDataMgr<RESTOKENTODIAMOND>     ResTokenToDiamondMgr_t;

// PVE
typedef TGameDataMgr<RESFIGHTLEVELPL>		    ResFightLevelPLMgr_t;
typedef TGameDataMgr<RESFIGHTLEVEL>			    ResFightLevelMgr_t;
typedef TGameDataMgr<RESAP>					    ResAPMgr_t;
typedef TGameDataMgr<RESGENERALPHASEUP>		    ResGeneralPhaseUpMgr_t;
typedef TGameDataMgr<RESGENERALLEVELCAP>		ResGeneralLevelCapMgr_t;
typedef TGameDataMgr<RESCHEATS>		    	    ResCheatsMgr_t;
typedef TGameDataMgr<RESRANKPRIZE>    		    ResRankPrizeMgr_t;
typedef TGameDataMgr<RESPVECHAPTERREWARD>		ResPveChapterRewardMgr_t;

typedef TGameDataMgr<RESBMARKETBASIC>   	ResBlackMarketBasicMgr_t;
typedef TGameDataMgr<RESBMARKETRANDOM>		ResBlackMarketRandomMgr_t;
typedef TGameDataMgr<RESBMARKETUPTPRICE>	ResBlackMarketUptPriceMgr_t;

typedef TGameDataMgr<RESBMARKETGENERALRULE1>	ResBlackMarketGeneralRule1Mgr_t;
typedef TGameDataMgr<RESBMARKETGENERALRULE2>	ResBlackMarketGeneralRule2Mgr_t;
typedef TGameDataMgr<RESBMARKETEQUIPRULE1>		ResBlackMarketEquipRule1Mgr_t;
typedef TGameDataMgr<RESBMARKETEQUIPRULE2>		ResBlackMarketEquipRule2Mgr_t;

typedef TGameDataMgr<RESSIGN7DAWARD>		ResSign7dAwardMgr_t;
typedef TGameDataMgr<RESSIGN30DAWARD>		ResSign30dAwardMgr_t;
typedef TGameDataMgr<RESSIGN30DAWARDLIST>		ResSign30dAwardListMgr_t;
typedef TGameDataMgr<RESDEBUGCOND>		        ResDebugMgr_t;

typedef TGameDataMgr<RESGUILDTASK>		            ResGuildTaskMgr_t;
typedef TGameDataMgr<RESGUILDREWARD>		        ResGuildRewardMgr_t;
typedef TGameDataMgr<RESGUILDLEVEL>		            ResGuildLevelMgr_t;
typedef TGameDataMgr<RESGUILDACTIVITY>				ResGuildActivityMgr_t;
typedef TGameDataMgr<RESGUILDBOSSINFO>				ResGuildBossInfoMgr_t;
typedef TGameDataMgr<RESGUILDBOSSREWARD>			ResGuildBossRewardMgr_t;
typedef TGameDataMgr<RESGUILDSOCIETY>				ResGuildSocietyMgr_t;
typedef TGameDataMgr<RESGUILDSOCIETYINFO>			ResGuildSocietyInfoMgr_t;
typedef TGameDataMgr<RESGUILDSALARY>				ResGuildSalaryMgr_t;
typedef TGameDataMgr<RESGUILDGENERALHANG>			ResGuildGeneralHangMgr_t;

typedef TGameDataMgr<RESGEMLIST>		            ResGemListMgr_t;
typedef TGameDataMgr<RESGEMLIMIT>		            ResGemLimitMgr_t;
typedef TGameDataMgr<RESCOMMONREWARD>		        ResCommonRewardMgr_t;
typedef TGameDataMgr<RESFIGHTLEVELGENERAL>          ResFightLevelGeneralMgr_t;

//成长奖励系统
typedef TGameDataMgr<RESGROWTHAWARDLEVEL>		    ResGrowthAwardLevelMgr_t;
typedef TGameDataMgr<RESGROWTHAWARDONLINE>		    ResGrowthAwardOnlineMgr_t;
typedef TGameDataMgr<RESLVFUND>		    ResLvFundMgr_t;

typedef TGameDataMgr<RESGENERALFATE>				ResGeneralFateMgr_t;
typedef TGameDataMgr<RESGENERALEQUIPFATE>			ResGeneralEquipFateMgr_t;

typedef TGameDataMgr<RESACTIVITYBONUS>				ResActivityBonusMgr_t;
typedef TGameDataMgr<RESSERIALNUM>					ResSerialNumMgr_t;

typedef TGameDataMgr<RESACTIVITY>					ResActivityMgr_t;
typedef TGameDataMgr<RESACTIVITYREWARD>				ResActivityRewardMgr_t;
typedef TGameDataMgr<RESSYNTHETIC>				    ResSyntheticMgr_t;
typedef TGameDataMgr<RESSYNTHETICPROPS>				ResSyntheticPropsMgr_t;
typedef TGameDataMgr<RESAFFICHE>				    ResAfficheMgr_t;

typedef TGameDataMgr<RESVIP>				        ResVIPMgr_t;
typedef TGameDataMgr<RESGIFTPACKAGE>				ResGifPkgMgr_t;
typedef TGameDataMgr<RESMALLPROPS>				    ResMallPropsMgr_t;

typedef TGameDataMgr<RESMARQUEESCREEN>              ResMarqueeScreenMgr_t;
typedef TGameDataMgr<RESPRIMAIL>                    ResPriMailMgr_t;
typedef TGameDataMgr<RESSIGN30DEXTRAREWARD>         ResSign30dExtraRewardMgr_t;

typedef TGameDataMgr<RESLIGCARDBASE>                ResLiGCardBaseMgr_t;
typedef TGameDataMgr<RESLIGCARDSKILL>               ResLiGCardSkillMgr_t;

typedef TGameDataMgr<RESWEEKLEAGUEPARA>             ResWeekLeagueParaMgr_t;
typedef TGameDataMgr<RESWEEKLEAGUEREWARD>           ResWeekLeagueRewardMgr_t;

typedef TGameDataMgr<RESPVPDAILYREWARD>				ResPVPDailyRewardMgr_t;
typedef TGameDataMgr<RESPVPSEASONREWARD>			ResPVPSeasonRewardMgr_t;
typedef TGameDataMgr<RESPVPREWARDINFO>				ResPVPRewardInfoMgr_t;

typedef TGameDataMgr<RESMAJESTYGROUPCARD>			ResMajestyGroupCardMgr_t;

typedef TGameDataMgr<RESPAY>                        ResPayMgr_t;
typedef TGameDataMgr<RESDISCOUNTPROPS>              ResDiscountPropsMgr_t;
typedef TGameDataMgr<RESACTIVITYFORCEFORCEREWARD>      ResLiRewardMgr_t;


typedef TGameDataMgr<RESMAJESTYITEM>                ResMajestyItemMgr_t;
typedef TGameDataMgr<RESFIRSTPAYBAG>                ResFirstPayBagMgr_t;
typedef TGameDataMgr<RESMONTHDAILYBAG>              ResMonthDailyBagMgr_t;
typedef TGameDataMgr<RESTOTALPAYBAG>                ResTotalPayBagMgr_t;
typedef TGameDataMgr<RESPAYACT>                     ResPayActMgr_t;
typedef TGameDataMgr<RESTIMECYCLE>                  ResTimeCycleMgr_t;
typedef TGameDataMgr<RESOUTLETS>                    ResOutletsMgr_t;

typedef TGameDataMgr<RESCOINSHOP>                   ResCoinShopMgr_t;

typedef TGameDataMgr<RESASYNCPVPHIGHESTRANKREWARD>        ResAsyncPvpRankRewardMgr_t;
typedef TGameDataMgr<RESASYNCPVPSCOREREWARD>      		ResAsyncPvpScoreRewardMgr_t;

typedef TGameDataMgr<RESGENERALPHASEPROPS>		ResGeneralPhasePropsMgr_t;

//每日挑战赛
typedef TGameDataMgr<RESDAILYCHALLENGEBASIC>	        ResDailyChallengeBasicMgr_t;
typedef TGameDataMgr<RESDAILYCHALLENGEMATCH>	        ResDailyChallengeMatchMgr_t;
typedef TGameDataMgr<RESDAILYCHALLENGEBATTLEARRAY>		ResDailyChallengeBattleArrayMgr_t;
typedef TGameDataMgr<RESDAILYCHALLENGEGENERALPOOL>		ResDailyChallengeGeneralPoolMgr_t;
typedef TGameDataMgr<RESDAILYCHALLENGEARMYFORCE>		ResDailyChallengeArmyForceMgr_t;
typedef TGameDataMgr<RESDAILYCHALLENGEFIGHTREWARD>		ResDailyChallengeFightRewardMgr_t;
typedef TGameDataMgr<RESDAILYCHALLENGEFINIALREWARD>		ResDailyChallengeFinialRewardMgr_t;
typedef TGameDataMgr<RESDAILYCHALLENGESKIPFIGHT>		ResDailyChallengeSkipFightMgr_t;
typedef TGameDataMgr<RESDAILYCHALLENGEBUFF>				ResDailyChallengeBuffMgr_t;

typedef TGameDataMgr<RESFAMEHALLTEAM>					ResFameHallTeamMgr_t;
typedef TGameDataMgr<RESFAMEHALLFRIENDSHIPRULE>			ResFameHallFriendShipRuleMgr_t;
typedef TGameDataMgr<RESFAMEHALLGENERAL>                ResFameHallGeneralMgr_t;
typedef TGameDataMgr<RESFAMEHALLINCREASEATTR>			ResFameHallIncreaseAttrMgr_t;
typedef TGameDataMgr<RESCLONEBATTLEFIGHTREWARD>			ResCloneBattleFightRewardMgr_t;

typedef TGameDataMgr<RESPEAKARENAPARA>					ResPeakArenaParaMgr_t;
typedef TGameDataMgr<RESPEAKARENAREWARD>				ResPeakArenaRewardMgr_t;
typedef TGameDataMgr<RESPEAKARENASCORE>					ResPeakArenaScoreMgr_t;
typedef TGameDataMgr<RESPEAKARENAACTIVEREWARD>			ResPeakArenaActiveRewardMgr_t;
typedef TGameDataMgr<RESPEAKARENAOUTPUTREWARD>			ResPeakArenaOutputRewardMgr_t;
typedef TGameDataMgr<RESPEAKARENACHOOSERULE>			ResPeakArenaChooseRuleMgr_t;

typedef TGameDataMgr<RESTACTICS>                        ResTacticsMgr_t;
typedef TGameDataMgr<RESMINE>			                ResMineMgr_t;

typedef TGameDataMgr<RESGENERALSKIN>			        ResGeneralSkinMgr_t;


class CGameDataMgr : public TSingleton<CGameDataMgr>
{
public:
    CGameDataMgr(){}
    ~CGameDataMgr(){}

    bool Init();

private:
    DECL_INIT_RES_MGR(m_oResBasicMgr,                   "gamedata/ResData/public/basic/ResBasic.bytes");
    DECL_INIT_RES_MGR(m_oResMessageMgr,                   "gamedata/ResData/public/basic/ResMessage.bytes");
    DECL_INIT_RES_MGR( m_oResArmyMgr,					"gamedata/ResData/public/troops/ResArmy.bytes" );
    DECL_INIT_RES_MGR( m_oResArmyRestrainMgr,			"gamedata/ResData/public/troops/ResArmyRestrain.bytes" );
    DECL_INIT_RES_MGR( m_oResGeneralMgr,				"gamedata/ResData/public/troops/ResGeneral.bytes" );
    DECL_INIT_RES_MGR( m_oResGeneralStarMgr,	        "gamedata/ResData/public/troops/ResGeneralStar.bytes" );
    DECL_INIT_RES_MGR( m_oResGeneralPhaseMgr,	        "gamedata/ResData/public/troops/ResGeneralPhase.bytes" );
    DECL_INIT_RES_MGR( m_oResGeneralLevelMgr,           "gamedata/ResData/public/troops/ResGeneralLevel.bytes" );
    DECL_INIT_RES_MGR( m_oResGeneralLevelGrowMgr,       "gamedata/ResData/public/troops/ResGeneralLevelGrow.bytes" );
    DECL_INIT_RES_MGR( m_oResGeneralLevelCapMgr,        "gamedata/ResData/public/troops/ResGeneralLevelCap.bytes" );
    DECL_INIT_RES_MGR( m_oResGeneralSkillMgr,			"gamedata/ResData/public/skill/ResGeneralSkill.bytes" );
    DECL_INIT_RES_MGR( m_oResBuffMgr,					"gamedata/ResData/public/skill/ResBuff.bytes" );
    DECL_INIT_RES_MGR( m_oResMasterSkillMgr,			"gamedata/ResData/public/skill/ResMasterSkill.bytes" );
    DECL_INIT_RES_MGR( m_oResScoreMgr,					"gamedata/ResData/public/match/ResScore.bytes" );
    DECL_INIT_RES_MGR( m_oResEffExpMgr,				    "gamedata/ResData/public/match/ResEfficiencyExp.bytes" );
    DECL_INIT_RES_MGR( m_oResEffRatioMgr,				"gamedata/ResData/public/match/ResEfficiencyRatio.bytes" );
    DECL_INIT_RES_MGR( m_oResPVPRewardMgr,				"gamedata/ResData/public/match/ResPVPReward.bytes");

    DECL_INIT_RES_MGR( m_oResEquipMgr,					"gamedata/ResData/public/props/ResEquip.bytes" );
    DECL_INIT_RES_MGR( m_oResEquipsListMgr,				"gamedata/ResData/public/props/ResEquipsList.bytes" );
    DECL_INIT_RES_MGR( m_oResEquipStarMgr,				"gamedata/ResData/public/props/ResEquipStar.bytes" );
    DECL_INIT_RES_MGR(m_oResGeneralEquipStarMgr, "gamedata/ResData/public/props/ResGeneralEquipStar.bytes");

    DECL_INIT_RES_MGR( m_oResPropsMgr,					"gamedata/ResData/public/props/ResProps.bytes" );
    DECL_INIT_RES_MGR( m_oResConsumeMgr,				"gamedata/ResData/public/props/ResConsume.bytes" );
    DECL_INIT_RES_MGR( m_oResTreasureBoxMgr,			"gamedata/ResData/public/props/ResTreasureBox.bytes" );
	DECL_INIT_RES_MGR( m_oResLevelBoxMgr,				"gamedata/ResData/public/props/ResLevelBox.bytes");
	DECL_INIT_RES_MGR( m_oResMajestyItemMgr,			"gamedata/ResData/public/props/ResMajestyItem.bytes" );
    DECL_INIT_RES_MGR( m_oResMajestyLvMgr,				"gamedata/ResData/public/majesty/ResMajestyLv.bytes" );
    DECL_INIT_RES_MGR( m_oResMajestyFunctionMgr,		"gamedata/ResData/public/majesty/ResMajestyFunction.bytes" );
    DECL_INIT_RES_MGR( m_oResMajestyGroupCardMgr,		"gamedata/ResData/public/majesty/ResMajestyGroupCard.bytes");
    DECL_INIT_RES_MGR( m_oResPurchaseMgr,				"gamedata/ResData/public/majesty/ResPurchase.bytes" );
    DECL_INIT_RES_MGR( m_oResSyntheticPropsMgr,			"gamedata/ResData/public/props/ResSyntheticProps.bytes" );
    DECL_INIT_RES_MGR( m_oResSyntheticMgr,				"gamedata/ResData/public/props/ResSynthetic.bytes" );
    DECL_INIT_RES_MGR( m_oResPropsToDiamondMgr,         "gamedata/ResData/public/props/ResPropsToDiamond.bytes");
    DECL_INIT_RES_MGR( m_oResTokenToDiamondMgr,         "gamedata/ResData/public/props/ResTokenToDiamond.bytes");

    DECL_INIT_RES_MGR( m_oResTaskMgr,					"gamedata/ResData/public/task/ResTask.bytes" );
    DECL_INIT_RES_MGR( m_oResTaskFinalAwardMgr,			"gamedata/ResData/public/task/ResTaskFinalAward.bytes" );
    DECL_INIT_RES_MGR( m_oResTaskBonusMgr,				"gamedata/ResData/public/task/ResTaskBonus.bytes" );

    DECL_INIT_RES_MGR( m_oResFightLevelPLMgr,			"gamedata/ResData/public/fightlevel/ResFightLevelPL.bytes" );
    DECL_INIT_RES_MGR( m_oResFightLevelMgr,			    "gamedata/ResData/public/fightlevel/ResFightLevel.bytes" );

    DECL_INIT_RES_MGR( m_oResTutorialBonusMgr,			"gamedata/ResData/public/tutorial/ResTutorialBonus.bytes" );
    DECL_INIT_RES_MGR( m_oResAPMgr,					    "gamedata/ResData/public/majesty/ResAP.bytes" );

    DECL_INIT_RES_MGR( m_oResGeneralPhaseUpMgr,	        "gamedata/ResData/public/troops/ResGeneralPhaseUp.bytes" );
    DECL_INIT_RES_MGR( m_oResGeneralFateMgr,			"gamedata/ResData/public/troops/ResGeneralfate.bytes" );
	DECL_INIT_RES_MGR( m_oResGeneralEquipFateMgr,		"gamedata/ResData/public/troops/ResGeneralEquipFate.bytes");
    DECL_INIT_RES_MGR( m_oResCheatsMgr,	     		    "gamedata/ResData/public/skill/ResCheats.bytes" );

    DECL_INIT_RES_MGR( m_oResRankPrizeMgr,	 			"gamedata/ResData/public/rank/ResRankPrize.bytes" );
    DECL_INIT_RES_MGR( m_oResPveChapterRewardMgr,	 	"gamedata/ResData/public/fightlevel/ResPveChapterReward.bytes" );
    DECL_INIT_RES_MGR( m_oResPondLotteryMapMgr,		    "gamedata/ResData/server/lottery/ResPondLotteryMap.bytes" );

    DECL_INIT_RES_MGR( m_oResLotteryDrawTypeMgr,		"gamedata/ResData/server/lottery/ResLotteryDrawMap.bytes");
    DECL_INIT_RES_MGR( m_oResLotteryBonusMapMgr,		    "gamedata/ResData/server/lottery/ResLotteryBonusMap.bytes" );
    DECL_INIT_RES_MGR( m_oResLotteryRuleMgr,			    "gamedata/ResData/server/lottery/ResLotteryRule.bytes" );
    DECL_INIT_RES_MGR( m_oResLotteryFreeCondMgr,		    "gamedata/ResData/server/lottery/ResLotteryFreeCond.bytes" );
    DECL_INIT_RES_MGR( m_oResDiaLotteryRuleMgr,		    "gamedata/ResData/server/lottery/ResDiaLotteryRule.bytes" );

    DECL_INIT_RES_MGR( m_oResBlackMarketBasicMgr,	 	    "gamedata/ResData/server/blackmarket/ResBMarketBasic.bytes" );
    DECL_INIT_RES_MGR( m_oResBlackMarketRandomMgr,	 	    "gamedata/ResData/server/blackmarket/ResBMarketRandom.bytes" );
    DECL_INIT_RES_MGR( m_oResBlackMarketUptPriceMgr,	    "gamedata/ResData/server/blackmarket/ResBMarketUptPrice.bytes" );
    DECL_INIT_RES_MGR( m_oResBlackMarketGeneralRule1Mgr,	"gamedata/ResData/server/blackmarket/ResBMarketGeneralRule1.bytes" );
    DECL_INIT_RES_MGR( m_oResBlackMarketGeneralRule2Mgr,	"gamedata/ResData/server/blackmarket/ResBMarketGeneralRule2.bytes" );
    DECL_INIT_RES_MGR( m_oResBlackMarketEquipRule1Mgr,		"gamedata/ResData/server/blackmarket/ResBMarketEquipRule1.bytes" );
    DECL_INIT_RES_MGR( m_oResBlackMarketEquipRule2Mgr,		"gamedata/ResData/server/blackmarket/ResBMarketEquipRule2.bytes" );

    DECL_INIT_RES_MGR( m_oResSign7dAwardMgr,		        "gamedata/ResData/public/activity/ResSign7dAward.bytes" );
    DECL_INIT_RES_MGR( m_oResSign30dAwardMgr,		        "gamedata/ResData/public/activity/ResSign30dAward.bytes" );
    DECL_INIT_RES_MGR( m_oResSign30dAwardListMgr,		     "gamedata/ResData/public/activity/ResSign30dAwardList.bytes" );

    DECL_INIT_RES_MGR( m_oResDebugMgr,		                 "gamedata/ResData/server/debug/ResDebugCond.bytes" );

    DECL_INIT_RES_MGR( m_oResGuildTaskMgr,		             "gamedata/ResData/public/guild/ResGuildTask.bytes" );
    DECL_INIT_RES_MGR( m_oResGuildRewardMgr,		         "gamedata/ResData/public/guild/ResGuildReward.bytes" );
    DECL_INIT_RES_MGR( m_oResGuildLevelMgr,	                "gamedata/ResData/public/guild/ResGuildLevel.bytes" );
    DECL_INIT_RES_MGR( m_oResGuildActivityMgr,				"gamedata/ResData/public/guild/ResGuildActivity.bytes");
    DECL_INIT_RES_MGR( m_oResGuildBossInfoMgr,			    "gamedata/ResData/public/guild/ResGuildBossInfo.bytes" );
    DECL_INIT_RES_MGR( m_oResGuildBossRewardMgr,			    "gamedata/ResData/public/guild/ResGuildBossReward.bytes" );
	DECL_INIT_RES_MGR( m_oResGuildSocietyMgr_t,			    "gamedata/ResData/public/guild/ResGuildSociety.bytes" );
	DECL_INIT_RES_MGR( m_oResGuildSocietyInfoMgr_t,			"gamedata/ResData/public/guild/ResGuildSocietyInfo.bytes" );
	DECL_INIT_RES_MGR( m_oResGuildSalaryMgr_t,				"gamedata/ResData/public/guild/ResGuildSalary.bytes");
	DECL_INIT_RES_MGR( m_oResGuildGeneralHangMgr_t,				"gamedata/ResData/public/guild/ResGuildGeneralHang.bytes");

    DECL_INIT_RES_MGR( m_oResGemListMgr,                      "gamedata/ResData/public/props/ResGemList.bytes" );
    DECL_INIT_RES_MGR( m_oResGemLimitMgr,                   "gamedata/ResData/public/props/ResGemLimit.bytes" );

    DECL_INIT_RES_MGR( m_oResGrowthAwardLevelMgr,	         "gamedata/ResData/public/activity/ResGrowthAwardLevel.bytes" );
    DECL_INIT_RES_MGR(m_oResGrowthAwardOnlineMgr, "gamedata/ResData/public/activity/ResGrowthAwardOnline.bytes");
    DECL_INIT_RES_MGR(m_oResLvFundMgr, "gamedata/ResData/public/activity/ResLvFund.bytes");

    DECL_INIT_RES_MGR( m_oResActivityBonusMgr,				"gamedata/ResData/server/serialnum/ResActivityBonus.bytes" );
    DECL_INIT_RES_MGR( m_oResSerialNumMgr,					"gamedata/ResData/server/serialnum/ResSerialNum.bytes" );

    DECL_INIT_RES_MGR( m_oResActivityMgr,					"gamedata/ResData/public/activity/ResActivity.bytes" );
    DECL_INIT_RES_MGR( m_oResActivityRewardMgr,				"gamedata/ResData/public/activity/ResActivityReward.bytes" );
    DECL_INIT_RES_MGR( m_oResAfficheMgr,				"gamedata/ResData/server/affiche/ResAffiche.bytes" );

    DECL_INIT_RES_MGR( m_oResVIPMgr,				"gamedata/ResData/public/activity/ResVIP.bytes" );
    DECL_INIT_RES_MGR( m_oResGifPkgMgr,				"gamedata/ResData/public/props/ResGiftPackage.bytes" );
    DECL_INIT_RES_MGR( m_oResMallPropsMgr,				"gamedata/ResData/public/props/ResMallProps.bytes" );

    DECL_INIT_RES_MGR( m_oResMarqueeScreenMgr,				"gamedata/ResData/public/marquee/ResMarqueeScreen.bytes" );
    DECL_INIT_RES_MGR( m_oResPriMailMgr,				"gamedata/ResData/server/mail/ResPriMail.bytes" );
    DECL_INIT_RES_MGR( m_oResSign30dExtraRewardMgr,				"gamedata/ResData/public/activity/ResSign30dExtraReward.bytes" );

    DECL_INIT_RES_MGR( m_oResLiGCardBaseMgr,				"gamedata/ResData/public/troops/ResLiGCardBase.bytes" );
    DECL_INIT_RES_MGR( m_oResLiGCardSkillMgr,				"gamedata/ResData/public/troops/ResLiGCardSkill.bytes" );


    DECL_INIT_RES_MGR( m_oResWeekLeagueParaMgr,				"gamedata/ResData/public/rank/ResWeekLeaguePara.bytes" );
    DECL_INIT_RES_MGR( m_oResWeekLeagueRewardMgr,			"gamedata/ResData/public/rank/ResWeekLeagueReward.bytes" );

    DECL_INIT_RES_MGR( m_oResPVPDailyRewardMgr,				"gamedata/ResData/public/match/ResPVPDailyReward.bytes");
    DECL_INIT_RES_MGR( m_oResPVPSeasonRewardMgr,			"gamedata/ResData/public/match/ResPVPSeasonReward.bytes");
    DECL_INIT_RES_MGR( m_oResPVPRewardInfoMgr,				"gamedata/ResData/public/match/ResPVPRewardInfo.bytes");

    DECL_INIT_RES_MGR( m_oResLiRewardMgr,                   "gamedata/ResData/public/activity/ResActivityForceForceReward.bytes");
    DECL_INIT_RES_MGR( m_oResPayMgr,                        "gamedata/ResData/public/pay/ResPay.bytes");
    DECL_INIT_RES_MGR( m_oResFirstPayBagMgr,                 "gamedata/ResData/public/pay/ResFirstPayBag.bytes");
    DECL_INIT_RES_MGR( m_oResMonthDailyBagMgr,                 "gamedata/ResData/public/pay/ResMonthDailyBag.bytes");
    DECL_INIT_RES_MGR( m_oResTotalPayBagMgr,                 "gamedata/ResData/public/pay/ResTotalPayBag.bytes");
    DECL_INIT_RES_MGR( m_oResPayActMgr,                 "gamedata/ResData/public/pay/ResPayAct.bytes");
    DECL_INIT_RES_MGR( m_oResTimeCycleMgr,                 "gamedata/ResData/public/basic/ResTimeCycle.bytes");
    DECL_INIT_RES_MGR( m_oResDiscountPropsMgr,             "gamedata/ResData/public/pay/ResDiscountProps.bytes");
    DECL_INIT_RES_MGR(m_oResOutletsMgr, "gamedata/ResData/public/pay/ResOutlets.bytes");

	DECL_INIT_RES_MGR( m_oResCoinShopMgr,                  "gamedata/ResData/server/blackmarket/ResCoinShop.bytes");

	DECL_INIT_RES_MGR( m_oResAsyncPvpRankRewardMgr,        "gamedata/ResData/public/asyncpvp/ResAsyncPVPHighestRankReward.bytes");
    DECL_INIT_RES_MGR( m_oResAsyncPvpScoreRewardMgr,       "gamedata/ResData/public/asyncpvp/ResAsyncPVPScoreReward.bytes");

	DECL_INIT_RES_MGR( m_oResGeneralPhasePropsMgr, 	"gamedata/ResData/public/troops/ResGeneralPhaseProps.bytes" );
    DECL_INIT_RES_MGR( m_oResCommonRewardMgr, 	    "gamedata/ResData/public/props/ResCommonReward.bytes" );

    DECL_INIT_RES_MGR( m_oResDailyChallengeBasicMgr, 	"gamedata/ResData/public/dailychallenge/ResDailyChallengeBasic.bytes" );
    DECL_INIT_RES_MGR( m_oResDailyChallengeMatchMgr, 	"gamedata/ResData/server/dailychallenge/ResDailyChallengeMatch.bytes" );
    DECL_INIT_RES_MGR( m_oResDailyChallengeBattleArrayMgr,  "gamedata/ResData/server/dailychallenge/ResDailyChallengeBattleArray.bytes" );
    DECL_INIT_RES_MGR( m_oResDailyChallengeGeneralPoolMgr,  "gamedata/ResData/server/dailychallenge/ResDailyChallengeGeneralPool.bytes" );
    DECL_INIT_RES_MGR( m_oResDailyChallengeArmyForceMgr,    "gamedata/ResData/server/dailychallenge/ResDailyChallengeArmyForce.bytes" );
    DECL_INIT_RES_MGR( m_oResDailyChallengeFightRewardMgr,  "gamedata/ResData/public/dailychallenge/ResDailyChallengeFightReward.bytes" );
    DECL_INIT_RES_MGR( m_oResDailyChallengeFinialRewardMgr,  "gamedata/ResData/public/dailychallenge/ResDailyChallengeFinialReward.bytes" );
    DECL_INIT_RES_MGR( m_oResDailyChallengeSkipFightMgr,    "gamedata/ResData/public/dailychallenge/ResDailyChallengeSkipFight.bytes" );
	DECL_INIT_RES_MGR( m_oResDailyChallengeBuffMgr,    "gamedata/ResData/public/dailychallenge/ResDailyChallengeBuff.bytes" );

	DECL_INIT_RES_MGR( m_oResFameHallTeamMgr,    "gamedata/ResData/public/famehall/ResFameHallTeam.bytes" );
	DECL_INIT_RES_MGR( m_oResFameHallFriendShipRuleMgr,    "gamedata/ResData/public/famehall/ResFameHallFriendShipRule.bytes" );
	DECL_INIT_RES_MGR( m_oResFameHallGeneralMgr,    "gamedata/ResData/public/famehall/ResFameHallGeneral.bytes" );
	DECL_INIT_RES_MGR( m_oResFameHallIncreaseAttrMgr,    "gamedata/ResData/public/famehall/ResFameHallIncreaseAttr.bytes" );
    DECL_INIT_RES_MGR(m_oResCloneBattleFightRewardMgr, "gamedata/ResData/server/clonebattle/ResCloneBattleFightReward.bytes");

	DECL_INIT_RES_MGR(m_oResPeakArenaParaMgr, "gamedata/ResData/public/match/ResPeakArenaPara.bytes");
	DECL_INIT_RES_MGR(m_oResPeakArenaScoreMgr, "gamedata/ResData/public/match/ResPeakArenaScore.bytes");
	DECL_INIT_RES_MGR(m_oResPeakArenaRewardMgr, "gamedata/ResData/public/match/ResPeakArenaReward.bytes");
	DECL_INIT_RES_MGR(m_oResPeakArenaActiveRewardMgr, "gamedata/ResData/public/match/ResPeakArenaActiveReward.bytes");
	DECL_INIT_RES_MGR(m_oResPeakArenaOutputRewardMgr, "gamedata/ResData/public/match/ResPeakArenaOutputReward.bytes");
	DECL_INIT_RES_MGR(m_oResPeakArenaChooseRuleMgr, "gamedata/ResData/public/match/ResPeakArenaChooseRule.bytes");
    DECL_INIT_RES_MGR(m_oResFightLevelGeneralInfoMgr_t, "gamedata/ResData/public/fightlevel/ResFightLevelGeneral.bytes");

    DECL_INIT_RES_MGR(m_oResTacticsMgr, "gamedata/ResData/public/tactics/ResTactics.bytes");
    DECL_INIT_RES_MGR(m_oResMineMgr, "gamedata/ResData/public/mine/ResMine.bytes");
	DECL_INIT_RES_MGR(m_oResGeneralSkinMgr, "gamedata/ResData/public/troops/ResGeneralSkin.bytes");

private:
    DECL_GETTER_REF( ResBasicMgr_t,				    m_oResBasicMgr,					ResBasicMgr );
    DECL_GETTER_REF( ResMessageMgr_t,               m_oResMessageMgr,               ResMessageMgr);
    DECL_GETTER_REF( ResArmyMgr_t,					m_oResArmyMgr,					ResArmyMgr );
    DECL_GETTER_REF( ResArmyRestrainMgr_t,			m_oResArmyRestrainMgr,  		ResArmyRestrainMgr );
    DECL_GETTER_REF( ResGeneralMgr_t,				m_oResGeneralMgr,				ResGeneralMgr );

    DECL_GETTER_REF( ResGeneralLevelCapMgr_t,	m_oResGeneralLevelCapMgr,		ResGeneralLevelCapMgr );
    DECL_GETTER_REF( ResGeneralPhaseMgr_t,		m_oResGeneralPhaseMgr,	ResGeneralPhaseMgr );
    DECL_GETTER_REF( ResGeneralStarMgr_t,		m_oResGeneralStarMgr,	ResGeneralStarMgr );
    DECL_GETTER_REF( ResGeneralLevelMgr_t,		m_oResGeneralLevelMgr,	ResGeneralLevelMgr );
    DECL_GETTER_REF( ResGeneralLevelGrowMgr_t,  m_oResGeneralLevelGrowMgr,  ResGeneralLevelGrowMgr);

    DECL_GETTER_REF( ResGeneralSkillMgr_t,			m_oResGeneralSkillMgr,			ResGeneralSkillMgr );
    DECL_GETTER_REF( ResBuffMgr_t,					m_oResBuffMgr,					ResBuffMgr );
    DECL_GETTER_REF( ResMasterSkillMgr_t,			m_oResMasterSkillMgr,			ResMasterSkillMgr );
    DECL_GETTER_REF( ResScoreMgr_t,				    m_oResScoreMgr,					ResScoreMgr );
    DECL_GETTER_REF( ResEffExpMgr_t,				m_oResEffExpMgr,				ResEffExpMgr );
    DECL_GETTER_REF( ResEffRatioMgr_t,				m_oResEffRatioMgr,				ResEffRatioMgr );
    DECL_GETTER_REF( ResPVPRewardMgr_t,				m_oResPVPRewardMgr,				ResPVPRewardMgr);

    DECL_GETTER_REF( ResEquipMgr_t,				m_oResEquipMgr,						ResEquipMgr );
    DECL_GETTER_REF( ResEquipsListMgr_t,	    m_oResEquipsListMgr,				ResEquipsListMgr );
    DECL_GETTER_REF( ResEquipStarMgr_t,	        m_oResEquipStarMgr,					ResEquipStarMgr );
    DECL_GETTER_REF( ResGeneralEquipStarMgr_t,   m_oResGeneralEquipStarMgr,          ResGeneralEquipStarMgr);

    DECL_GETTER_REF( ResPropsMgr_t,				    m_oResPropsMgr,					ResPropsMgr );
    DECL_GETTER_REF( ResPropsToDiamondMgr_t,        m_oResPropsToDiamondMgr,        ResPropsToDiamondMgr);
    DECL_GETTER_REF( ResTokenToDiamondMgr_t,        m_oResTokenToDiamondMgr,        ResTokenToDiamondMgr);
    DECL_GETTER_REF( ResConsumeMgr_t,				m_oResConsumeMgr,				ResConsumeMgr );
    DECL_GETTER_REF( ResTreasureBoxMgr_t,   		m_oResTreasureBoxMgr,   		ResTreasureBoxMgr );
	DECL_GETTER_REF( ResLevelBoxMgr_t,				m_oResLevelBoxMgr,				ResLevelBoxMgr );
	DECL_GETTER_REF( ResMajestyItemMgr_t,   		m_oResMajestyItemMgr,   		ResMajestyItemMgr );
    DECL_GETTER_REF( ResMajestyLvMgr_t,			    m_oResMajestyLvMgr,				ResMajestyLvMgr );
    DECL_GETTER_REF( ResMajestyFunctionMgr_t,		m_oResMajestyFunctionMgr,		ResMajestyFunctionMgr );
    DECL_GETTER_REF( ResMajestyGroupCardMgr_t,		m_oResMajestyGroupCardMgr,		ResMajestyGroupCardMgr );
    DECL_GETTER_REF( ResPurchaseMgr_t,			    m_oResPurchaseMgr,				ResPurchaseMgr );
    DECL_GETTER_REF( ResTaskMgr_t,					m_oResTaskMgr,					ResTaskMgr );
    DECL_GETTER_REF( ResTaskFinalAwardMgr_t,		m_oResTaskFinalAwardMgr,		ResTaskFinalAwardMgr );
    DECL_GETTER_REF( ResTaskBonusMgr_t,			m_oResTaskBonusMgr,				ResTaskBonusMgr );
    DECL_GETTER_REF( ResFightLevelPLMgr_t,			m_oResFightLevelPLMgr,			ResFightLevelPLMgr );
    DECL_GETTER_REF( ResFightLevelMgr_t,			m_oResFightLevelMgr,			ResFightLevelMgr );
    DECL_GETTER_REF( ResLotteryDrawTypeMgr_t,		m_oResLotteryDrawTypeMgr,		ResLotteryDrawTypeMgr );
    DECL_GETTER_REF( ResPondLotteryMapMgr_t,		m_oResPondLotteryMapMgr,		ResPondLotteryMapMgr );
    DECL_GETTER_REF( ResLotteryBonusMapMgr_t,		m_oResLotteryBonusMapMgr,		ResLotteryBonusMapMgr );
    DECL_GETTER_REF( ResLotteryRuleMgr_t,			m_oResLotteryRuleMgr,			ResLotteryRuleMgr );
    DECL_GETTER_REF( ResLotteryFreeCondMgr_t,		m_oResLotteryFreeCondMgr,		ResLotteryFreeCondMgr );
    DECL_GETTER_REF( ResDiaLotteryRuleMgr_t,		m_oResDiaLotteryRuleMgr,		ResDiaLotteryRuleMgr );
    DECL_GETTER_REF( ResTutorialBonusMgr_t,		    m_oResTutorialBonusMgr,			ResTutorialBonusMgr );
    DECL_GETTER_REF( ResAPMgr_t,					m_oResAPMgr,					ResAPMgr );
    DECL_GETTER_REF( ResGeneralPhaseUpMgr_t,	  	m_oResGeneralPhaseUpMgr,	    ResGeneralPhaseUpMgr );
    DECL_GETTER_REF( ResCheatsMgr_t,	  			m_oResCheatsMgr,	            ResCheatsMgr );
    DECL_GETTER_REF( ResRankPrizeMgr_t,	  		m_oResRankPrizeMgr,	        	ResRankPrizeMgr );
    DECL_GETTER_REF( ResPveChapterRewardMgr_t,	  	m_oResPveChapterRewardMgr,	        ResPveChapterRewardMgr );
    DECL_GETTER_REF( ResBlackMarketBasicMgr_t,	  	m_oResBlackMarketBasicMgr,	    ResBlackMarketBasicMgr );
    DECL_GETTER_REF( ResBlackMarketRandomMgr_t,	m_oResBlackMarketRandomMgr,	    ResBlackMarketRandomMgr );
    DECL_GETTER_REF( ResBlackMarketUptPriceMgr_t,	m_oResBlackMarketUptPriceMgr,	ResBlackMarketUptPriceMgr );
    DECL_GETTER_REF( ResBlackMarketGeneralRule1Mgr_t,	m_oResBlackMarketGeneralRule1Mgr,	ResBlackMarketGeneralRule1Mgr );
    DECL_GETTER_REF( ResBlackMarketGeneralRule2Mgr_t,	m_oResBlackMarketGeneralRule2Mgr,	ResBlackMarketGeneralRule2Mgr );
    DECL_GETTER_REF( ResBlackMarketEquipRule1Mgr_t,	m_oResBlackMarketEquipRule1Mgr,				ResBlackMarketEquipRule1Mgr );
    DECL_GETTER_REF( ResBlackMarketEquipRule2Mgr_t,	m_oResBlackMarketEquipRule2Mgr,				ResBlackMarketEquipRule2Mgr );
    DECL_GETTER_REF( ResSign7dAwardMgr_t,	        m_oResSign7dAwardMgr,						ResSign7dAwardMgr );
    DECL_GETTER_REF( ResSign30dAwardMgr_t,	        m_oResSign30dAwardMgr,						ResSign30dAwardMgr );
    DECL_GETTER_REF( ResSign30dAwardListMgr_t,	        m_oResSign30dAwardListMgr,		        ResSign30dAwardListMgr );
    DECL_GETTER_REF( ResDebugMgr_t,	                m_oResDebugMgr,								ResDebugMgr );
    DECL_GETTER_REF( ResGuildTaskMgr_t,	            m_oResGuildTaskMgr,							ResGuildTaskMgr );
    DECL_GETTER_REF( ResGuildRewardMgr_t,	        m_oResGuildRewardMgr,						ResGuildRewardMgr );
    DECL_GETTER_REF( ResGuildActivityMgr_t,         m_oResGuildActivityMgr,                     ResGuildActivityMgr );
    DECL_GETTER_REF( ResGuildBossInfoMgr_t,			m_oResGuildBossInfoMgr,				ResGuildBossInfoMgr );
    DECL_GETTER_REF( ResGuildBossRewardMgr_t,			m_oResGuildBossRewardMgr,				ResGuildBossRewardMgr );
	DECL_GETTER_REF( ResGuildSocietyMgr_t,			m_oResGuildSocietyMgr_t,					ResGuildSocietyMgr);
	DECL_GETTER_REF( ResGuildSocietyInfoMgr_t,		m_oResGuildSocietyInfoMgr_t,				ResGuildSocietyInfoMgr);
	DECL_GETTER_REF( ResGuildSalaryMgr_t,			m_oResGuildSalaryMgr_t,						ResGuildSalaryMgr);
	DECL_GETTER_REF( ResGuildGeneralHangMgr_t,		m_oResGuildGeneralHangMgr_t,				ResGuildGeneralHangMgr);

    DECL_GETTER_REF( ResGemListMgr_t,	        m_oResGemListMgr,								ResGemListMgr );
    DECL_GETTER_REF( ResGemLimitMgr_t,	          m_oResGemLimitMgr,							ResGemLimitMgr );
    DECL_GETTER_REF( ResGuildLevelMgr_t,	      m_oResGuildLevelMgr,							ResGuildLevelMgr );
    DECL_GETTER_REF( ResGrowthAwardLevelMgr_t,	      m_oResGrowthAwardLevelMgr,	            ResGrowthAwardLevelMgr );
    DECL_GETTER_REF(ResGrowthAwardOnlineMgr_t, m_oResGrowthAwardOnlineMgr, ResGrowthAwardOnlineMgr);
    DECL_GETTER_REF(ResLvFundMgr_t, m_oResLvFundMgr, ResLvFundMgr);
    DECL_GETTER_REF( ResGeneralFateMgr_t,	      m_oResGeneralFateMgr,							ResGeneralFateMgr );
	DECL_GETTER_REF( ResGeneralEquipFateMgr_t,		m_oResGeneralEquipFateMgr,					ResGeneralEquipFateMgr);
    DECL_GETTER_REF( ResActivityBonusMgr_t,	  m_oResActivityBonusMgr,						ResActivityBonusMgr );
    DECL_GETTER_REF( ResSerialNumMgr_t,	      m_oResSerialNumMgr,							ResSerialNumMgr );

    DECL_GETTER_REF( ResActivityMgr_t,			m_oResActivityMgr,							ResActivityMgr );
    DECL_GETTER_REF( ResActivityRewardMgr_t,	m_oResActivityRewardMgr,					ResActivityRewardMgr );
    DECL_GETTER_REF( ResSyntheticPropsMgr_t,	m_oResSyntheticPropsMgr,					ResSyntheticPropsMgr );
    DECL_GETTER_REF( ResSyntheticMgr_t,	m_oResSyntheticMgr,					ResSyntheticMgr );
    DECL_GETTER_REF( ResAfficheMgr_t,	m_oResAfficheMgr,					ResAfficheMgr );

    DECL_GETTER_REF( ResVIPMgr_t,	    m_oResVIPMgr,					    ResVIPMgr );
    DECL_GETTER_REF( ResGifPkgMgr_t,	m_oResGifPkgMgr,					ResGifPkgMgr );
    DECL_GETTER_REF( ResMallPropsMgr_t,	m_oResMallPropsMgr,					ResMallPropsMgr );

    DECL_GETTER_REF( ResMarqueeScreenMgr_t,	m_oResMarqueeScreenMgr,					ResMarqueeScreenMgr );
    DECL_GETTER_REF( ResPriMailMgr_t,	m_oResPriMailMgr,					ResPriMailMgr );
    DECL_GETTER_REF( ResSign30dExtraRewardMgr_t,	m_oResSign30dExtraRewardMgr,					ResSign30dExtraRewardMgr );
    DECL_GETTER_REF(ResFightLevelGeneralMgr_t, m_oResFightLevelGeneralInfoMgr_t, ResFightLevelGeneralMgr);

    DECL_GETTER_REF( ResLiGCardBaseMgr_t,	m_oResLiGCardBaseMgr,						ResLiGCardBaseMgr );
    DECL_GETTER_REF( ResLiGCardSkillMgr_t,	m_oResLiGCardSkillMgr,						ResLiGCardSkillMgr );


    DECL_GETTER_REF( ResWeekLeagueParaMgr_t,    m_oResWeekLeagueParaMgr,                ResWeekLeagueParaMgr );
    DECL_GETTER_REF( ResWeekLeagueRewardMgr_t,   m_oResWeekLeagueRewardMgr,             ResWeekLeagueRewardMgr );

    DECL_GETTER_REF( ResPVPDailyRewardMgr_t,    m_oResPVPDailyRewardMgr,	    ResPVPDailyRewardMgr);
    DECL_GETTER_REF( ResPVPSeasonRewardMgr_t,	m_oResPVPSeasonRewardMgr,		ResPVPSeasonRewardMgr);
    DECL_GETTER_REF( ResPVPRewardInfoMgr_t,		m_oResPVPRewardInfoMgr,			ResPVPRewardInfoMgr);

    DECL_GETTER_REF( ResLiRewardMgr_t,          m_oResLiRewardMgr,              ResLiRewardMgr);
    DECL_GETTER_REF( ResPayMgr_t,		        m_oResPayMgr,			        ResPayMgr);
    DECL_GETTER_REF( ResFirstPayBagMgr_t,		m_oResFirstPayBagMgr,			ResFirstPayBagMgr);
    DECL_GETTER_REF( ResMonthDailyBagMgr_t,		m_oResMonthDailyBagMgr,			ResMonthDailyBagMgr);
    DECL_GETTER_REF( ResTotalPayBagMgr_t,		m_oResTotalPayBagMgr,			ResTotalPayBagMgr);
    DECL_GETTER_REF( ResPayActMgr_t,		    m_oResPayActMgr,			    ResPayActMgr);
    DECL_GETTER_REF( ResTimeCycleMgr_t,		    m_oResTimeCycleMgr,			    ResTimeCycleMgr);
	DECL_GETTER_REF( ResCoinShopMgr_t,		    m_oResCoinShopMgr,			    ResCoinShopMgr);
    DECL_GETTER_REF( ResDiscountPropsMgr_t,     m_oResDiscountPropsMgr,         ResDiscountPropsMgr);
    DECL_GETTER_REF(ResOutletsMgr_t,      m_oResOutletsMgr, ResOutletsMgr);

	DECL_GETTER_REF( ResAsyncPvpRankRewardMgr_t,    m_oResAsyncPvpRankRewardMgr,    ResAsyncPvpRankRewardMgr);
    DECL_GETTER_REF( ResAsyncPvpScoreRewardMgr_t,	m_oResAsyncPvpScoreRewardMgr,   ResAsyncPvpScoreRewardMgr);

    DECL_GETTER_REF( ResGeneralPhasePropsMgr_t, m_oResGeneralPhasePropsMgr, ResGeneralPhasePropsMgr );
    DECL_GETTER_REF( ResCommonRewardMgr_t, m_oResCommonRewardMgr, ResCommonRewardMgr );

    DECL_GETTER_REF( ResDailyChallengeBasicMgr_t, m_oResDailyChallengeBasicMgr, ResDailyChallengeBasicMgr );
    DECL_GETTER_REF( ResDailyChallengeMatchMgr_t, m_oResDailyChallengeMatchMgr, ResDailyChallengeMatchMgr );
    DECL_GETTER_REF( ResDailyChallengeBattleArrayMgr_t, m_oResDailyChallengeBattleArrayMgr, ResDailyChallengeBattleArrayMgr );
    DECL_GETTER_REF( ResDailyChallengeGeneralPoolMgr_t, m_oResDailyChallengeGeneralPoolMgr, ResDailyChallengeGeneralPoolMgr );
    DECL_GETTER_REF( ResDailyChallengeArmyForceMgr_t, m_oResDailyChallengeArmyForceMgr, ResDailyChallengeArmyForceMgr );
    DECL_GETTER_REF( ResDailyChallengeFightRewardMgr_t, m_oResDailyChallengeFightRewardMgr, ResDailyChallengeFightRewardMgr );
    DECL_GETTER_REF( ResDailyChallengeFinialRewardMgr_t, m_oResDailyChallengeFinialRewardMgr, ResDailyChallengeFinialRewardMgr );
    DECL_GETTER_REF( ResDailyChallengeSkipFightMgr_t, m_oResDailyChallengeSkipFightMgr, ResDailyChallengeSkipFightMgr );
	DECL_GETTER_REF( ResDailyChallengeBuffMgr_t, m_oResDailyChallengeBuffMgr, ResDailyChallengeBuffMgr );

	DECL_GETTER_REF( ResFameHallTeamMgr_t, m_oResFameHallTeamMgr, ResFameHallTeamMgr );
	DECL_GETTER_REF( ResFameHallFriendShipRuleMgr_t, m_oResFameHallFriendShipRuleMgr, ResFameHallFriendShipRuleMgr);
	DECL_GETTER_REF( ResFameHallGeneralMgr_t, m_oResFameHallGeneralMgr, ResFameHallGeneralMgr );
    DECL_GETTER_REF(ResFameHallIncreaseAttrMgr_t, m_oResFameHallIncreaseAttrMgr, ResFameHallIncreaseAttrMgr);
    DECL_GETTER_REF(ResCloneBattleFightRewardMgr_t, m_oResCloneBattleFightRewardMgr, ResCloneBattleFightRewardMgr);

	DECL_GETTER_REF(ResPeakArenaParaMgr_t, m_oResPeakArenaParaMgr, ResPeakArenaParaMgr);
	DECL_GETTER_REF(ResPeakArenaScoreMgr_t, m_oResPeakArenaScoreMgr, ResPeakArenaScoreMgr);
	DECL_GETTER_REF(ResPeakArenaRewardMgr_t, m_oResPeakArenaRewardMgr, ResPeakArenaRewardMgr);
	DECL_GETTER_REF(ResPeakArenaActiveRewardMgr_t, m_oResPeakArenaActiveRewardMgr, ResPeakArenaActiveRewardMgr);
	DECL_GETTER_REF(ResPeakArenaOutputRewardMgr_t, m_oResPeakArenaOutputRewardMgr, ResPeakArenaOutputRewardMgr);
	DECL_GETTER_REF(ResPeakArenaChooseRuleMgr_t, m_oResPeakArenaChooseRuleMgr, ResPeakArenaChooseRuleMgr);

    DECL_GETTER_REF(ResMineMgr_t, m_oResMineMgr, ResMineMgr);
    DECL_GETTER_REF(ResTacticsMgr_t, m_oResTacticsMgr, ResTacticsMgr);
    DECL_GETTER_REF(ResGeneralSkinMgr_t, m_oResGeneralSkinMgr, ResGeneralSkinMgr);
};
