#pragma once

#include "define.h"
#include "common_proto.h"
#include "cs_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"

using namespace PKGMETA;

class Skin : public TSingleton<Skin>
{
public:
	Skin();
	virtual ~Skin();

public:
	int BuySkin(PlayerData* pstData, uint32_t dwSkinId, DT_SYNC_ITEM_INFO& rstSyncInfo);
	int AddSkin(PlayerData* pstData, DT_ITEM_SKIN& rstSkin);
	int ChgSkin(PlayerData* pstData, uint32_t dwGeneralId, uint32_t dwSkinId);

private:
	DT_ITEM_SKIN* FindSkin(PlayerData* pstData, uint32_t dwId);

private:
	DT_ITEM_SKIN m_stSkin;
};
