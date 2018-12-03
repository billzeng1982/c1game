#include "GameDataMgr.h"

bool CGameDataMgr::Init()
{
    if (!INIT_RES_MGR(m_oResGuildLevelMgr)) return false;
    if (!INIT_RES_MGR(m_oResGuildFightParamMgr)) return false;
    if (!INIT_RES_MGR(m_oResGuildFightMapMgr)) return false;
    if (!INIT_RES_MGR(m_oResGuildFightStrongPointMgr)) return false;
    if (!INIT_RES_MGR(m_oResPriMailMgr)) return false;
    if( !INIT_RES_MGR(m_oResBasicMgr) )			return false;
    if( !INIT_RES_MGR(m_oResGuildBossInfoMgr) )			return false;
    if( !INIT_RES_MGR(m_oResGeneralMgr) )			return false;
	if( !INIT_RES_MGR(m_oResGuildSocietyMgr_t) )			return false;
	if( !INIT_RES_MGR(m_oResGuildSocietyInfoMgr_t) )		return false;
    if (!INIT_RES_MGR(m_oResFightLevelGeneralInfoMgr_t))		return false;
    if (!INIT_RES_MGR(m_oResRewardShowMgr_t))		return false;
    if (!INIT_RES_MGR(m_oResGuildVitalityRankRewardMgr_t))		return false;
    if (!INIT_RES_MGR(m_oResGFightScoreRankRewardMgr_t))		return false;
    return true;
}

