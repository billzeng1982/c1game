#include "Tactics.h"
#include "LogMacros.h"
#include "../../framework/GameObjectPool.h"


Tactics::Tactics()
{
    this->_Construct();
}

Tactics::~Tactics()
{

}

void Tactics::Clear()
{
    _Construct();
    IObject::Clear();
}

void Tactics::_Construct()
{
    m_bTacticsType = 0;
    m_bTacticsLevel = 0;

    m_poFightPlayer = NULL;
    m_poTroop = NULL;

    m_pResTactics = NULL;
    m_pResTacticailBuffIndex = NULL;
    m_pResTacticailBuffList = NULL;
    m_pResConsume = NULL;
    m_pResGeneral = NULL;
}

Tactics* Tactics::Get()
{
    return GET_GAMEOBJECT(Tactics, GAMEOBJ_TACTICS);
}

void Tactics::Release(Tactics* pObj)
{
    RELEASE_GAMEOBJECT(pObj);
}

bool Tactics::Init(FightPlayer* poFightPlayer, Troop *poTroop)
{
    m_bTacticsType = poFightPlayer->m_stPlayerInfo.m_bTacticsType;
    m_bTacticsLevel = poFightPlayer->m_stPlayerInfo.m_bTacticsLevel;
    m_poFightPlayer = poFightPlayer;
    m_poTroop = poTroop;

    m_pResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(poTroop->m_dwGeneralId);
    if (!m_pResGeneral)
    {
        LOGERR("m_pResGeneral is NULL");
        return false;
    }
    uint8_t bArmyType = m_pResGeneral->m_bArmyType;
    

    m_pResTactics = NULL;
    ResTacticsMgr_t& rstResTacticsMgr = CGameDataMgr::Instance().GetResTacticsMgr();
    for (int i = 0; i < rstResTacticsMgr.GetResNum(); i++)
    {
        RESTACTICS* pResTactics = rstResTacticsMgr.GetResByPos(i);
        if (!pResTactics)
        {
            LOGERR("pResTactics is NULL.");
            return false;
        }
        if (pResTactics->m_bType == m_bTacticsType)
        {
            m_pResTactics = pResTactics;
            break;
        }
    }
    if (!m_pResTactics)
    {   
        LOGERR("m_pResTactics is NULL.");
        return false;
    }
    
    //È«¾Ö
    uint32_t dwBuffIndex4All = m_pResTactics->m_dwForAllBuffIndex;
    if (!_AddBuffId2Value(dwBuffIndex4All, bArmyType))
    {
        LOGERR("dwBuffIndex4All<%d>", dwBuffIndex4All);
        return false;
    }

    //Î»ÖÃ1-6
    uint8_t bPositionIndex = (m_poTroop->m_bId - 1) % MAX_TROOP_NUM;
    if ( bPositionIndex >= m_pResTactics->m_bPositionCount)
    {
        LOGERR("bPositionIndex<%d> is larger than m_pResTactics->m_bPositionCount<%d>", bPositionIndex, m_pResTactics->m_bPositionCount);
        return false;
    }
    uint32_t dwPositionId = m_pResTactics->m_positionBuffIndex[bPositionIndex];
    if (!_AddBuffId2Value(dwPositionId, bArmyType))
    {
        LOGERR("dwPositionId<%d>", dwPositionId);
        return false;
    }

    return true;
}

float Tactics::GetAddValue(uint32_t dwBuffId)
{
    for (m_BuffId2BuffValueIter = m_BuffId2BuffValue.begin(); m_BuffId2BuffValueIter != m_BuffId2BuffValue.end(); m_BuffId2BuffValueIter++)
    {
        if (m_BuffId2BuffValueIter->dwBuffId == dwBuffId)
        {
            return m_BuffId2BuffValueIter->fBuffValue;
        }
    }

    return 0;
}

bool Tactics::_AddBuffId2Value(uint32_t dwBuffIndex, uint8_t bArmyType)
{
    m_pResTacticailBuffIndex = CGameDataMgr::Instance().GetResTacticialBuffIndexMgr().Find(dwBuffIndex);
    if (!m_pResTacticailBuffIndex)
    {
        LOGERR("m_pResTacticailBuffIndex is NULL.");
        return false;
    }
    for (int i = 0; i < m_pResTacticailBuffIndex->m_bArmyTypeCount; i++)
    {
        if (bArmyType != m_pResTacticailBuffIndex->m_szArmyType[i])
        {
            continue;
        }

        uint8_t bArmyType = m_pResTacticailBuffIndex->m_szArmyType[i];
        uint32_t dwBuffListId = m_pResTacticailBuffIndex->m_buffList[i];
        m_pResTacticailBuffList = CGameDataMgr::Instance().GetResTacticialBuffListMgr().Find(dwBuffListId);
        if (!m_pResTacticailBuffList)
        {
            LOGERR("m_pResTacticailBuffIndex is NULL.");
            return false;
        }
        for (int j = 0; j < m_pResTacticailBuffList->m_bBuffCount; j++)
        {
            m_pResConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(m_pResTacticailBuffList->m_buffGrowId[i]);
            if (!m_pResConsume)
            {
                LOGERR("m_pResConsume is NULL.");
                return false;
            }

            m_oBuffId2Value.dwBuffId = m_pResTacticailBuffList->m_buffId[j];
            if (m_bTacticsLevel == 0 || m_bTacticsLevel > MAX_TACTICS_LEVEL || m_bTacticsLevel > m_pResConsume->m_dwLvCount)
            {
                LOGERR("m_bTacticsLevel<%d> m_pResConsume->m_dwLvCount<%d>", m_bTacticsLevel, m_pResConsume->m_dwLvCount);
                return false;
            }
            m_oBuffId2Value.fBuffValue = m_pResConsume->m_lvList[m_bTacticsLevel - 1] / m_pResConsume->m_dwDivideRate;
            m_BuffId2BuffValue.insert(m_BuffId2BuffValue.end(), m_oBuffId2Value);
        }
    }

    return true;
}


