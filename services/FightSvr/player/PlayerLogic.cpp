#include "PlayerLogic.h"
#include "LogMacros.h"
#include "ObjectUpdatorMgr.h"
#include "../dungeon/Dungeon.h"
#include "../dungeon/DungeonMgr.h"
#include "../framework/GameObjectPool.h"
#include "../framework/FightSvrMsgLayer.h"
#include "PlayerMgr.h"

void PlayerLogic::PlayerLogout( Player* poPlayer, int iLogoutReason, PKGMETA::SCPKG* pstScPkg )
{
    if(!poPlayer)
    {
        assert( false );
        return;
    }

    uint32_t dwDungeonId = 0;
    if (poPlayer->getDungeon())
    {
        dwDungeonId = poPlayer->getDungeon()->m_dwDungeonId;
    }

    LOGRUN("Player(%lu) name(%s) logout, in dungeon(%u)", poPlayer->m_ullUin, poPlayer->m_szName, dwDungeonId);

	PlayerMgr::Instance().Delete(poPlayer);

}

void PlayerLogic::OnSessionStop( Player* poPlayer, char chStopReason )
{
    if(!poPlayer)
    {
        return;
    }

    // chStopReasonÔÝ²»´¦Àí
    this->PlayerLogout(poPlayer, PLAYER_LOGOUT_BY_CONN);
}

void UpdatePlayer( IObject* pObj, int iDeltaTime )
{
    assert(pObj->GetObjType() == GAMEOBJ_PLAYER);
    Player* poPlayer = dynamic_cast<Player*>(pObj);

    // some logic here ...
    poPlayer->Update(iDeltaTime);
}

