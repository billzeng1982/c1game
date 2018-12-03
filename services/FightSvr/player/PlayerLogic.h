#pragma once

#include "singleton.h"
#include "cs_proto.h"
#include "Player.h"

class PlayerLogic : public TSingleton<PlayerLogic>
{
public:
	PlayerLogic() {}
	virtual ~PlayerLogic() {}

	void PlayerLogout( Player* poPlayer, int iLogoutReason, PKGMETA::SCPKG* pstScPkg = NULL );
	void OnSessionStop( Player* poPlayer, char chStopReason );

private:
};

// Player callback
void UpdatePlayer( IObject* pObj, int iDeltaTime );

