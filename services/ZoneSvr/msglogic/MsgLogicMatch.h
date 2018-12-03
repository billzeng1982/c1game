#pragma once

#include "MsgBase.h"
#include "../module/player/Player.h"

class MatchStart_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class MatchCancel_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
};

class DungeonCreateRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class MatchCancelRsp_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

class FightSettleNtf_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
private:
    int _HandleFight6v6SettleNtf(PKGMETA::SSPKG& rstSsPkg, PKGMETA::SCPKG& rstScPkg, PlayerData* pstData);
    int _HandleGFightSettleNtf(PKGMETA::SSPKG& rstSsPkg, PKGMETA::SCPKG& rstScPkg, PlayerData* pstData);
    int _HandleWeekLeagueSettleNtf(PKGMETA::SSPKG& rstSsPkg, PKGMETA::SCPKG& rstScPkg, PlayerData* pstData);
    int _HandleLeisureSettleNtf(PKGMETA::SSPKG& rstSsPkg, PKGMETA::SCPKG& rstScPkg, PlayerData* pstData);
    int _HandleRoomFightSettleNtf(PKGMETA::SSPKG& rstSsPkg, PKGMETA::SCPKG& rstScPkg, PlayerData* pstData);
    //¼«ÏÞÌôÕ½
    int _HandleDailyChallengeSettleNtf(PKGMETA::SSPKG& rstSsPkg, PKGMETA::SCPKG& rstScPkg, PlayerData* pstData);
    int _HandlePeakArenaSettleNtf(PKGMETA::SSPKG& rstSsPkg, PKGMETA::SCPKG& rstScPkg, PlayerData* pstData);

	void _StatGeneralInfo(PlayerData* pstData, SS_PKG_FIGHT_SETTLE_NTF& rstSsPkgBodyRsp);
	uint32_t _GenRaceNumber(PKGMETA::SS_PKG_FIGHT_SETTLE_NTF& rstNtf);
    int _HandlePvpTask(PlayerData& roPlayerData, SS_PKG_FIGHT_SETTLE_NTF& rstSsPkgBodyRsp, uint32_t dwPvpType);
};

class PvpFakeSettle_CS : public IMsgBase
{
public:
    virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL );
    FightSettleNtf_SS m_oSettleNtfHandle;
};
