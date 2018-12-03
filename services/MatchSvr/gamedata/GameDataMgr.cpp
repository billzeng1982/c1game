#include "GameDataMgr.h"

bool CGameDataMgr::Init()
{
    if( !INIT_RES_MGR(m_oResBasicMgr) )		return false;
    if( !INIT_RES_MGR(m_oResScoreMgr) )		return false;
    if (!INIT_RES_MGR(m_oResMatchMgr))     return false;
    return true;
}

