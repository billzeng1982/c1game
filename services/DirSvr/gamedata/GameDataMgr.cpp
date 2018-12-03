#include "LogMacros.h"
#include "GameDataMgr.h"

bool CGameDataMgr::Init()
{
    if( !INIT_RES_MGR(m_oResServerMgr) )         return false;
    LOGRUN("m_oResBasicMgr init success.");
    
    LOGRUN("all res data init success.");
    return true;
}

bool CGameDataMgr::Reload()
{
    if( !m_oResServerMgr.Reload("gamedata/ResData/server/server/ResServer.bytes") )         return false;
    LOGRUN("m_oResBasicMgr reload success.");
    
    LOGRUN("all res data reload success.");
    return true;
}
