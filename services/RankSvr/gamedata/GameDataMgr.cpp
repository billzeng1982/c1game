#include "GameDataMgr.h"

bool CGameDataMgr::Init()
{
	if( !INIT_RES_MGR(m_oResBasicMgr) )			return false;
    if( !INIT_RES_MGR(m_oResDailyChallengeRankRewardMgr) )      return false;
    if (!INIT_RES_MGR(m_oResPriMailMgr)) return false;
    if (!INIT_RES_MGR(m_oResLiRankRewardMgr)) return false;
    if (!INIT_RES_MGR(m_oResPeakArenaRankRewardMgr)) return false;

	return true;
}

