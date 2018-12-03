#pragma once

#include "singleton.h"
#include "Player.h"
#include "cs_proto.h"
#include "common_proto.h"

class PlayerLogic: public TSingleton<PlayerLogic>
{
public:
	PlayerLogic() {}
	~PlayerLogic() {}

    //  正常登出处理, 掉线的时候不会立即调用
	void PlayerLogout(Player* poPlayer, int iLogoutReason, PKGMETA::SCPKG* pstScPkg = NULL, uint8_t bIsTimeOut = 0);

    //  掉线立即处理
	void OnSessionStop(Player* poPlayer, char chStopReason);

    //断开连接后处理逻辑
    void OfflineDo(Player* poPlayer);

    //登出后处理逻辑
    void LogoutDo(Player* poPlayer);
};

