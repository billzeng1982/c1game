#include "DungeonMgr.h"
#include "LogMacros.h"

bool DungeonMgr::Init()
{
	m_mapId2Dungeon.clear();
    _InitWaitDealBufferLen();
	return true;
}

Dungeon* DungeonMgr::New()
{
	Dungeon* poDungeon = Dungeon::Get();
	/* 创建具有唯一ID的副本 */
	// 暂时Dungeon ID借用GameObject的ID
	if (NULL != poDungeon)
	{
		poDungeon->m_dwDungeonId = poDungeon->GetObjID();
		this->AddToIdMap(poDungeon);
	}

	LOGRUN("new dungeon Id-%u", poDungeon->m_dwDungeonId);
	return poDungeon;
}

void DungeonMgr::Delete(Dungeon* poDungeon)
{
	if ( NULL == poDungeon )
	{
		return;
	}

	this->DelFromIdMap(poDungeon);

	Dungeon::Release(poDungeon);
}

Dungeon* DungeonMgr::GetById(uint32_t dwId)
{
	MapId2Dungeon_t::iterator it = m_mapId2Dungeon.find(dwId);
	if ( it != m_mapId2Dungeon.end() )
	{
		return it->second;
	}

	return NULL;
}

void DungeonMgr::AddToIdMap( Dungeon* poDungeon )
{
    if( NULL == poDungeon || 0 == poDungeon->m_dwDungeonId )
    {
        return;
    }

    m_mapId2Dungeon.insert( MapId2Dungeon_t::value_type( poDungeon->m_dwDungeonId, poDungeon ) );
}

void DungeonMgr::DelFromIdMap( Dungeon* poDungeon )
{
    if( NULL == poDungeon || 0 == poDungeon->m_dwDungeonId )
    {
        return;
    }

    MapId2Dungeon_t::iterator it = m_mapId2Dungeon.find( poDungeon->m_dwDungeonId );
    if( it != m_mapId2Dungeon.end() )
    {
        m_mapId2Dungeon.erase(it);
    }
    return;
}

void DungeonMgr::_InitWaitDealBufferLen()
{
    m_uWaitDealBufferLen = sizeof(PKGMETA::CSPKGHEAD)*2 + sizeof(CS_PKG_FIGHT_HP_NTF)*2 + 1;
    size_t uTmp = sizeof(PKGMETA::CSPKGHEAD)*2 + sizeof(CS_PKG_FIGHT_RETREAT_NTF)*2 + 1;
    m_uWaitDealBufferLen = m_uWaitDealBufferLen < uTmp ? uTmp : m_uWaitDealBufferLen;
}

