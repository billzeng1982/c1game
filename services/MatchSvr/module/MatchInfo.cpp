#include "MatchMgr.h"
#include "LogMacros.h"
#include "../gamedata/GameDataMgr.h"
#include "../framework/GameObjectPool.h"

using namespace std;
using namespace PKGMETA;

MatchInfo::MatchInfo()
{
    this->_Construct();
}

MatchInfo::~MatchInfo()
{
    
}

void MatchInfo::Clear()
{
    this->_Construct();
    return;
}

void MatchInfo::_Construct()
{
    bzero((void*)&m_stPlayerInfo, sizeof(m_stPlayerInfo));

    m_wWaitCount = 0;
    m_ulFakeMatchWaitTimeSec = 0;
    m_pResMatch = NULL;
    m_pTimer = NULL;
    m_bIsMatched = false;
}

