#include "GameDataMgr.h"

bool CGameDataMgr::Init()
{
    if (!INIT_RES_MGR(m_oResFakePlayerMgr)) return false;
    if (!INIT_RES_MGR(m_oResRankRewardMgr)) return false;
    if (!INIT_RES_MGR(m_oResPriMailMgr)) return false;
    if (!INIT_RES_MGR(m_oResBasicMgr)) return false; 
    return true;
}

