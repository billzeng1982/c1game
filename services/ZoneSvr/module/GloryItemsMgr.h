#pragma once
#include "define.h"
#include "common_proto.h"
#include "cs_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"
#include "../gamedata/GameDataMgr.h"

using namespace PKGMETA;

class GloryItemsMgr: public TSingleton<GloryItemsMgr>
{
public:
	GloryItemsMgr();
	virtual ~GloryItemsMgr(){};
private:
	SCPkg m_stScPkg;

public:
	bool Init();
	int AddMajestyItems(PlayerData* pstData, uint32_t dwApproach, uint32_t dwPara);
	int AddMajestyItems(PlayerData* pstData, uint32_t dwId);
private:
	int _AddMajestyItems(PlayerData* pstData, RESMAJESTYITEM* poResMajestyItem);
};