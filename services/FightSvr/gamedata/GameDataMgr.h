#pragma once

#include "TGameDataMgr.h"
#include "ov_res_keywords.h"
#include "ov_res_public.h"
#include "ov_res_server.h"
#include "singleton.h"
#include "macros.h"


/*
	目前先启动时全部加载所有gamedata,后期内存优化时按需动态加载，卸载
*/

using namespace std;

typedef TGameDataMgr<RESBASIC>			ResBasicMgr_t;

typedef TGameDataMgr<RESARMY>			ResArmyMgr_t;
typedef TGameDataMgr<RESARMYRESTRAIN>	ResArmyRestrainMgr_t; // 兵种相克
typedef TGameDataMgr<RESGENERAL>		ResGeneralMgr_t;

typedef TGameDataMgr<RESGENERALPHASE>	  ResGeneralPhaseMgr_t;
typedef TGameDataMgr<RESGENERALSTAR>	  ResGeneralStarMgr_t;
typedef TGameDataMgr<RESGENERALLEVEL>     ResGeneralLevelMgr_t;
typedef TGameDataMgr<RESGENERALLEVELGROW>     ResGeneralLevelGrowMgr_t;
typedef TGameDataMgr<RESGENERALFATE>	ResGeneralFateMgr_t;
typedef TGameDataMgr<RESGENERALEQUIPFATE>	ResGeneralEquipFateMgr_t;

typedef TGameDataMgr<RESGENERALSKILL>	ResGeneralSkillMgr_t;
typedef TGameDataMgr<RESPASSIVESKILL>	ResPassiveSkillMgr_t;
typedef TGameDataMgr<RESMASTERSKILL>	ResMasterSkillMgr_t;
typedef TGameDataMgr<RESBUFF>			ResBuffMgr_t;
typedef TGameDataMgr<RESCHEATS>			ResCheatsMgr_t;

typedef TGameDataMgr<RESEQUIP>			ResEquipMgr_t;
typedef TGameDataMgr<RESEQUIPSTAR>		ResEquipStarMgr_t;
typedef TGameDataMgr<RESGENERALEQUIPSTAR>   ResGeneralEquipStarMgr_t;

typedef TGameDataMgr<RESPROPS>			ResPropsMgr_t;
typedef TGameDataMgr<RESCONSUME>		ResConsumeMgr_t;

typedef TGameDataMgr<RESMAJESTYLV>			ResMajestyLvMgr_t;
typedef TGameDataMgr<RESGEMLIST>		    ResGemListMgr_t;

typedef TGameDataMgr<RESPEAKARENAPARA>			ResPeakArenaParaMgr_t;
typedef TGameDataMgr<RESPEAKARENACHOOSERULE>	ResPeakArenaChooseRuleMgr_t;

typedef TGameDataMgr<RESTACTICS>                ResTacticsMgr_t;
typedef TGameDataMgr<RESTACTICIALBUFFINDEX>     ResTacticialBuffIndexMgr_t;
typedef TGameDataMgr<RESTACTICIALBUFFLIST>      ResTacticialBuffListMgr_t;

typedef TGameDataMgr<RESPEAKARENAGENERAL>       ResPeakArenaGeneralMgr_t;
typedef TGameDataMgr<RESGENERALSKIN>      	 	ResGeneralSkinMgr_t;


class CGameDataMgr : public TSingleton<CGameDataMgr>
{
public:
	CGameDataMgr(){}
	~CGameDataMgr(){}

	bool Init();

private:
	DECL_INIT_RES_MGR( m_oResBasicMgr,			"gamedata/ResData/public/basic/ResBasic.bytes" );

	DECL_INIT_RES_MGR( m_oResArmyMgr,			"gamedata/ResData/public/troops/ResArmy.bytes" );
	DECL_INIT_RES_MGR( m_oResArmyRestrainMgr,	"gamedata/ResData/public/troops/ResArmyRestrain.bytes" );
	DECL_INIT_RES_MGR( m_oResGeneralMgr,		"gamedata/ResData/public/troops/ResGeneral.bytes" );

    DECL_INIT_RES_MGR( m_oResGeneralStarMgr,	"gamedata/ResData/public/troops/ResGeneralStar.bytes" );
    DECL_INIT_RES_MGR( m_oResGeneralPhaseMgr,	"gamedata/ResData/public/troops/ResGeneralPhase.bytes" );
    DECL_INIT_RES_MGR( m_oResGeneralLevelMgr,   "gamedata/ResData/public/troops/ResGeneralLevel.bytes" );
    DECL_INIT_RES_MGR( m_oResGeneralLevelGrowMgr,  "gamedata/ResData/public/troops/ResGeneralLevelGrow.bytes" );
    DECL_INIT_RES_MGR( m_oResGeneralFateMgr,    "gamedata/ResData/public/troops/ResGeneralfate.bytes" );
	DECL_INIT_RES_MGR( m_oResGeneralEquipFateMgr,	"gamedata/ResData/public/troops/ResGeneralEquipFate.bytes");

	DECL_INIT_RES_MGR( m_oResGeneralSkillMgr,	"gamedata/ResData/public/skill/ResGeneralSkill.bytes" );
	DECL_INIT_RES_MGR( m_oResPassiveSkillMgr,	"gamedata/ResData/public/skill/ResPassiveSkill.bytes" );
	DECL_INIT_RES_MGR( m_oResMasterSkillMgr,	"gamedata/ResData/public/skill/ResMasterSkill.bytes" );
	DECL_INIT_RES_MGR( m_oResBuffMgr,			"gamedata/ResData/public/skill/ResBuff.bytes" );
	DECL_INIT_RES_MGR( m_oResCheatsMgr,			"gamedata/ResData/public/skill/ResCheats.bytes" );

	DECL_INIT_RES_MGR( m_oResEquipMgr,			"gamedata/ResData/public/props/ResEquip.bytes" );
    DECL_INIT_RES_MGR( m_oResEquipStarMgr,		"gamedata/ResData/public/props/ResEquipStar.bytes" );
    DECL_INIT_RES_MGR(m_oResGeneralEquipStarMgr, "gamedata/ResData/public/props/ResGeneralEquipStar.bytes");

	DECL_INIT_RES_MGR( m_oResPropsMgr,			"gamedata/ResData/public/props/ResProps.bytes" );

	DECL_INIT_RES_MGR( m_oResMajestyLvMgr,		"gamedata/ResData/public/majesty/ResMajestyLv.bytes" );
    DECL_INIT_RES_MGR( m_oResConsumeMgr,        "gamedata/ResData/public/props/ResConsume.bytes" );
    DECL_INIT_RES_MGR( m_oResGemListMgr,        "gamedata/ResData/public/props/ResGemList.bytes" );

    DECL_INIT_RES_MGR( m_oResPeakArenaParaMgr,        "gamedata/ResData/public/match/ResPeakArenaPara.bytes" );
    DECL_INIT_RES_MGR( m_oResPeakArenaChooseRuleMgr,  "gamedata/ResData/public/match/ResPeakArenaChooseRule.bytes" );

    DECL_INIT_RES_MGR(m_oResTacticsMgr, "gamedata/ResData/public/tactics/ResTactics.bytes");
    DECL_INIT_RES_MGR(m_oResTacticialBuffIndexMgr, "gamedata/ResData/public/tactics/ResTacticialBuffIndex.bytes");
    DECL_INIT_RES_MGR(m_oResTacticialBuffListMgr, "gamedata/ResData/public/tactics/ResTacticialBuffList.bytes");

	DECL_INIT_RES_MGR( m_oResPeakArenaGeneralMgr,		"gamedata/ResData/public/troops/ResPeakArenaGeneral.bytes" );
	DECL_INIT_RES_MGR( m_oResGeneralSkinMgr,			"gamedata/ResData/public/troops/ResGeneralSkin.bytes" );

private:
	DECL_GETTER_REF( ResBasicMgr_t,				m_oResBasicMgr,			ResBasicMgr );

	DECL_GETTER_REF( ResArmyMgr_t,				m_oResArmyMgr,			ResArmyMgr );
	DECL_GETTER_REF( ResArmyRestrainMgr_t,		m_oResArmyRestrainMgr,  ResArmyRestrainMgr );
	DECL_GETTER_REF( ResGeneralMgr_t,			m_oResGeneralMgr,		ResGeneralMgr );

	DECL_GETTER_REF( ResGeneralPhaseMgr_t,		m_oResGeneralPhaseMgr,	ResGeneralPhaseMgr );
	DECL_GETTER_REF( ResGeneralStarMgr_t,		m_oResGeneralStarMgr,	ResGeneralStarMgr );
    DECL_GETTER_REF( ResGeneralLevelMgr_t,		m_oResGeneralLevelMgr,	ResGeneralLevelMgr );
    DECL_GETTER_REF( ResGeneralLevelGrowMgr_t,  m_oResGeneralLevelGrowMgr,  ResGeneralLevelGrowMgr);
    DECL_GETTER_REF( ResGeneralFateMgr_t,       m_oResGeneralFateMgr,       ResGeneralFateMgr );
	DECL_GETTER_REF( ResGeneralEquipFateMgr_t,	m_oResGeneralEquipFateMgr,	ResGeneralEquipFateMgr);

	DECL_GETTER_REF( ResGeneralSkillMgr_t,		m_oResGeneralSkillMgr,	ResGeneralSkillMgr );
	DECL_GETTER_REF( ResPassiveSkillMgr_t,		m_oResPassiveSkillMgr,	ResPassiveSkillMgr );
	DECL_GETTER_REF( ResMasterSkillMgr_t,		m_oResMasterSkillMgr,	ResMasterSkillMgr );
	DECL_GETTER_REF( ResBuffMgr_t,				m_oResBuffMgr,			ResBuffMgr );
	DECL_GETTER_REF( ResCheatsMgr_t,			m_oResCheatsMgr,		ResCheatsMgr );

	DECL_GETTER_REF( ResEquipMgr_t,				m_oResEquipMgr,			ResEquipMgr );
    DECL_GETTER_REF( ResEquipStarMgr_t,			m_oResEquipStarMgr,		ResEquipStarMgr );
    DECL_GETTER_REF(ResGeneralEquipStarMgr_t, m_oResGeneralEquipStarMgr, ResGeneralEquipStarMgr);

	DECL_GETTER_REF( ResPropsMgr_t,				m_oResPropsMgr,			ResPropsMgr );

	DECL_GETTER_REF( ResMajestyLvMgr_t,	        m_oResMajestyLvMgr,		ResMajestyLvMgr );
    DECL_GETTER_REF( ResConsumeMgr_t,	        m_oResConsumeMgr,		ResConsumeMgr );
    DECL_GETTER_REF( ResGemListMgr_t,	        m_oResGemListMgr,		ResGemListMgr );

    DECL_GETTER_REF( ResPeakArenaParaMgr_t,	        m_oResPeakArenaParaMgr,		ResPeakArenaParaMgr );
    DECL_GETTER_REF( ResPeakArenaChooseRuleMgr_t,	m_oResPeakArenaChooseRuleMgr,  ResPeakArenaChooseRuleMgr );

    DECL_GETTER_REF( ResTacticsMgr_t,	            m_oResTacticsMgr,               ResTacticsMgr );
    DECL_GETTER_REF( ResTacticialBuffIndexMgr_t,	m_oResTacticialBuffIndexMgr,    ResTacticialBuffIndexMgr );
    DECL_GETTER_REF( ResTacticialBuffListMgr_t,	    m_oResTacticialBuffListMgr,     ResTacticialBuffListMgr );
    DECL_GETTER_REF( ResPeakArenaGeneralMgr_t,	    m_oResPeakArenaGeneralMgr,      ResPeakArenaGeneralMgr );
    DECL_GETTER_REF( ResGeneralSkinMgr_t,	    	m_oResGeneralSkinMgr,      		ResGeneralSkinMgr );
};


