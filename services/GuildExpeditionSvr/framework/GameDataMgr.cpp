#include "GameDataMgr.h"

bool CGameDataMgr::Init()
{

    if( !INIT_RES_MGR(m_oResBasicMgr) )			return false;

    return true;
}

