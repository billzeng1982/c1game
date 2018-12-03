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

typedef TGameDataMgr<RESBASIC> ResBasicMgr_t;
typedef TGameDataMgr<RESDAILYCHALLENGERANKREWARD> ResDailyChallengeRankRewardMgr_t;
typedef TGameDataMgr<RESPRIMAIL> ResPriMailMgr_t;
typedef TGameDataMgr<RESACTIVITYFORCERANKREWARD> ResLiRankRewardMgr_t;
typedef TGameDataMgr<RESPEAKARENARANKREWARD> ResPeakArenaRankRewardMgr_t;


class CGameDataMgr : public TSingleton<CGameDataMgr>
{
public:
	CGameDataMgr(){}
	~CGameDataMgr(){}

	bool Init();

private:
	DECL_INIT_RES_MGR( m_oResBasicMgr,	"gamedata/ResData/public/basic/ResBasic.bytes" );
    DECL_INIT_RES_MGR( m_oResDailyChallengeRankRewardMgr,  "gamedata/ResData/public/dailychallenge/ResDailyChallengeRankReward.bytes" );
    DECL_INIT_RES_MGR( m_oResPriMailMgr,      "gamedata/ResData/server/mail/ResPriMail.bytes" );
    DECL_INIT_RES_MGR( m_oResLiRankRewardMgr,      "gamedata/ResData/public/activity/ResActivityForceRankReward.bytes" );
	DECL_INIT_RES_MGR( m_oResPeakArenaRankRewardMgr,	   "gamedata/ResData/public/match/ResPeakArenaRankReward.bytes" );

private:
	DECL_GETTER_REF( ResBasicMgr_t,	    m_oResBasicMgr,		ResBasicMgr );
    DECL_GETTER_REF( ResDailyChallengeRankRewardMgr_t, m_oResDailyChallengeRankRewardMgr, ResDailyChallengeRankRewardMgr );
    DECL_GETTER_REF( ResPriMailMgr_t,	m_oResPriMailMgr,	 ResPriMailMgr );
    DECL_GETTER_REF( ResLiRankRewardMgr_t,	m_oResLiRankRewardMgr,	 ResLiRankRewardMgr );
    DECL_GETTER_REF( ResPeakArenaRankRewardMgr_t,	m_oResPeakArenaRankRewardMgr,	 ResPeakArenaRankRewardMgr );
};


