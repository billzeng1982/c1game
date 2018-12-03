#include "GameDataMgr.h"

bool CGameDataMgr::Init()
{
	if( !INIT_RES_MGR(m_oResBasicMgr) )			return false;

	if( !INIT_RES_MGR(m_oResArmyMgr ) )			return false;
	if( !INIT_RES_MGR(m_oResArmyRestrainMgr) )	return false;
	if( !INIT_RES_MGR(m_oResGeneralMgr) )		return false;

	if( !INIT_RES_MGR(m_oResGeneralPhaseMgr) )	return false;
	if( !INIT_RES_MGR(m_oResGeneralStarMgr) )	return false;
    if( !INIT_RES_MGR(m_oResGeneralLevelMgr) )	return false;
    if( !INIT_RES_MGR(m_oResGeneralLevelGrowMgr) )  return false;
    if( !INIT_RES_MGR(m_oResGeneralFateMgr) )  return false;
	if( !INIT_RES_MGR(m_oResGeneralEquipFateMgr))	return false;

	if( !INIT_RES_MGR(m_oResGeneralSkillMgr) )	return false;
	if( !INIT_RES_MGR(m_oResPassiveSkillMgr) )	return false;
	if( !INIT_RES_MGR(m_oResMasterSkillMgr) )	return false;
	if( !INIT_RES_MGR(m_oResBuffMgr) )			return false;
	if( !INIT_RES_MGR(m_oResCheatsMgr) )		return false;

    if( !INIT_RES_MGR(m_oResEquipMgr) )			return false;
    if( !INIT_RES_MGR(m_oResEquipStarMgr) )			return false;

    if( !INIT_RES_MGR(m_oResPropsMgr) )			return false;

    if( !INIT_RES_MGR(m_oResMajestyLvMgr) )		return false;
    if( !INIT_RES_MGR(m_oResConsumeMgr) )		return false;
    if( !INIT_RES_MGR(m_oResGemListMgr) )		return false;

    if( !INIT_RES_MGR(m_oResPeakArenaParaMgr) )		return false;
    if( !INIT_RES_MGR(m_oResPeakArenaChooseRuleMgr) ) return false;

    if( !INIT_RES_MGR(m_oResTacticsMgr) ) return false;
    if( !INIT_RES_MGR(m_oResTacticialBuffIndexMgr) ) return false;
    if( !INIT_RES_MGR(m_oResTacticialBuffListMgr) ) return false;

    if( !INIT_RES_MGR(m_oResPeakArenaGeneralMgr) ) return false;
    if( !INIT_RES_MGR(m_oResGeneralSkinMgr) ) return false;

	return true;
}

