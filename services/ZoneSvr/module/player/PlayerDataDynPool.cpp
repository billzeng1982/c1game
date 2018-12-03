#include "PlayerDataDynPool.h"

bool PlayerDataDynPool::Init()
{
    if( !m_oEquipPool.Init( EQUIL_POOL_INIT_NUM, EQUIL_POOL_DELTA_NUM ))
    {
        return false;
    }

    return true;
}

