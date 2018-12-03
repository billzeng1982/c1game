#pragma once

/*
	注意 manager 不管对象的分配释放
*/

#include <map>
#include <vector>
#include "singleton.h"
#include "define.h"
#include "functors.h"
#include "Dungeon.h"

class DungeonMgr : public TSingleton<DungeonMgr>
{
private:
	typedef std::map<uint64_t, Dungeon*> MapId2Dungeon_t;

public:
	bool Init();

	Dungeon* New();
	void Delete(Dungeon* poDungeon);

	Dungeon* GetById(uint32_t dwId);

	void AddToIdMap( Dungeon* poDungeon );
	void DelFromIdMap( Dungeon* poDungeon );
	int GetLoad() { return (int)m_mapId2Dungeon.size(); }

    size_t GetWaitDealBufferLen() { return m_uWaitDealBufferLen; }

private:
    void _InitWaitDealBufferLen();

private:
	MapId2Dungeon_t m_mapId2Dungeon;

    size_t m_uWaitDealBufferLen;
};

