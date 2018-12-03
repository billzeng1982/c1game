#include "GameDataMgr.h"

bool CGameDataMgr::Init()
{
    if (!INIT_RES_MGR(m_oResSerialMgr)) return false;
    return true;
}

