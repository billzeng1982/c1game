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

typedef TGameDataMgr<RESGUILDLEVEL>				    ResGuildLevelMgr_t;
typedef TGameDataMgr<RESGUILDFIGHTPARAM>		    ResGuildFightParamMgr_t;
typedef TGameDataMgr<RESGUILDFIGHTMAP>		        ResGuildFightMapMgr_t;
typedef TGameDataMgr<RESGUILDFIGHTSTRONGPOINT>		ResGuildFightStrongPointMgr_t;
typedef TGameDataMgr<RESPRIMAIL>				    ResPriMailMgr_t;
typedef TGameDataMgr<RESBASIC>						ResBasicMgr_t;
typedef TGameDataMgr<RESGUILDBOSSINFO>				ResGuildBossInfoMgr_t;
typedef TGameDataMgr<RESGENERAL>		            ResGeneralMgr_t;
typedef TGameDataMgr<RESGUILDSOCIETY>				ResGuildSocietyMgr_t;
typedef TGameDataMgr<RESGUILDSOCIETYINFO>			ResGuildSocietyInfoMgr_t;
typedef TGameDataMgr<RESFIGHTLEVELGENERAL>          ResFightLevelGeneralMgr_t;
typedef TGameDataMgr<RESREWARDSHOW>                 ResRewardShowMgr_t;
typedef TGameDataMgr<RESGUILDVITALITYRANKREWARD>    ResGuildVitalityRankRewardMgr_t;
typedef TGameDataMgr<RESGFIGHTSCORERANKREWARD>      ResGFightScoreRankRewardMgr_t;


class CGameDataMgr : public TSingleton<CGameDataMgr>
{
public:
    CGameDataMgr(){}
    ~CGameDataMgr(){}

    bool Init();

private:
    DECL_INIT_RES_MGR( m_oResGuildLevelMgr,	                "gamedata/ResData/public/guild/ResGuildLevel.bytes" );
    DECL_INIT_RES_MGR( m_oResGuildFightParamMgr,	        "gamedata/ResData/public/guild/ResGuildFightParam.bytes" );
    DECL_INIT_RES_MGR( m_oResGuildFightMapMgr,              "gamedata/ResData/public/guild/ResGuildFightMap.bytes" );
    DECL_INIT_RES_MGR( m_oResGuildFightStrongPointMgr,      "gamedata/ResData/public/guild/ResGuildFightStrongPoint.bytes" );
    DECL_INIT_RES_MGR( m_oResPriMailMgr,                    "gamedata/ResData/server/mail/ResPriMail.bytes" );
    DECL_INIT_RES_MGR( m_oResBasicMgr,					    "gamedata/ResData/public/basic/ResBasic.bytes" );
    DECL_INIT_RES_MGR( m_oResGuildBossInfoMgr,			    "gamedata/ResData/public/guild/ResGuildBossInfo.bytes" );
    DECL_INIT_RES_MGR( m_oResGeneralMgr,		            "gamedata/ResData/public/troops/ResGeneral.bytes" );
	DECL_INIT_RES_MGR( m_oResGuildSocietyMgr_t,			    "gamedata/ResData/public/guild/ResGuildSociety.bytes" );
	DECL_INIT_RES_MGR( m_oResGuildSocietyInfoMgr_t,			"gamedata/ResData/public/guild/ResGuildSocietyInfo.bytes" );
    DECL_INIT_RES_MGR( m_oResFightLevelGeneralInfoMgr_t,    "gamedata/ResData/public/fightlevel/ResFightLevelGeneral.bytes");
    DECL_INIT_RES_MGR( m_oResRewardShowMgr_t,               "gamedata/ResData/public/props/ResRewardShow.bytes" );
	DECL_INIT_RES_MGR( m_oResGuildVitalityRankRewardMgr_t,	"gamedata/ResData/public/guild/ResGuildVitalityRankReward.bytes");
	DECL_INIT_RES_MGR( m_oResGFightScoreRankRewardMgr_t,	"gamedata/ResData/public/guild/ResGFightScoreRankReward.bytes" );

private:
    DECL_GETTER_REF( ResGuildLevelMgr_t,	        m_oResGuildLevelMgr,	            ResGuildLevelMgr );
    DECL_GETTER_REF( ResGuildFightParamMgr_t,	    m_oResGuildFightParamMgr,	        ResGuildFightParamMgr );
    DECL_GETTER_REF( ResGuildFightMapMgr_t,	        m_oResGuildFightMapMgr,	            ResGuildFightMapMgr );
    DECL_GETTER_REF( ResGuildFightStrongPointMgr_t,	m_oResGuildFightStrongPointMgr,	    ResGuildFightStrongPointMgr );
    DECL_GETTER_REF( ResPriMailMgr_t,	            m_oResPriMailMgr,	                ResPriMailMgr );
    DECL_GETTER_REF( ResBasicMgr_t,					m_oResBasicMgr,						ResBasicMgr );
    DECL_GETTER_REF( ResGuildBossInfoMgr_t,			m_oResGuildBossInfoMgr,				ResGuildBossInfoMgr );
    DECL_GETTER_REF( ResGeneralMgr_t,			    m_oResGeneralMgr,		            ResGeneralMgr );
	DECL_GETTER_REF( ResGuildSocietyMgr_t,			m_oResGuildSocietyMgr_t,			ResGuildSocietyMgr);
	DECL_GETTER_REF( ResGuildSocietyInfoMgr_t,		m_oResGuildSocietyInfoMgr_t,		ResGuildSocietyInfoMgr);
    DECL_GETTER_REF( ResFightLevelGeneralMgr_t,     m_oResFightLevelGeneralInfoMgr_t,   ResFightLevelGeneralMgr);
    DECL_GETTER_REF( ResRewardShowMgr_t,            m_oResRewardShowMgr_t,              ResRewardShowMgr);
    DECL_GETTER_REF( ResGuildVitalityRankRewardMgr_t, m_oResGuildVitalityRankRewardMgr_t, ResGuildVitalityRankRewardMgr);
    DECL_GETTER_REF( ResGFightScoreRankRewardMgr_t,   m_oResGFightScoreRankRewardMgr_t,   ResGFightScoreRankRewardMgr);
};
