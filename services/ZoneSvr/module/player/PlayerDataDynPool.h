#pragma once

#include "DynMempool.h"
#include "common_proto.h"

using namespace PKGMETA;

/*
	player data 动态内存池管理，防止过多的内存浪费
*/

class PlayerDataDynPool : public TSingleton<PlayerDataDynPool>
{
	const static int EQUIL_POOL_INIT_NUM = 2048;
	const static int EQUIL_POOL_DELTA_NUM = 1024;

public:
	PlayerDataDynPool() {}
	~PlayerDataDynPool() {}

	bool Init();

	DynMempool<DT_ITEM_EQUIP>& EquipPool() { return m_oEquipPool; }

private:
	DynMempool<DT_ITEM_EQUIP> m_oEquipPool;
};

