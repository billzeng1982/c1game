#pragma once
#include "common_proto.h"

using namespace PKGMETA;

class Player
{
public:
    Player();
    virtual ~Player();

    void Clear();

    bool InitFromDB(DT_GUILD_PLAYER_DATA& rstPlayerData);
    bool PackPlayerData(DT_GUILD_PLAYER_DATA& rstPlayerData, uint16_t wVersion);

    int AddApply(uint64_t ullGuildId);
    int DelApply(uint64_t ullGuildId);
    int FindApply(uint64_t ullGuildId);

    void ClearApply();

    uint64_t GetPlayerId() { return m_stBaseInfo.m_ullUin; }
    uint64_t GetGuildId() { return m_stBaseInfo.m_ullGuildId; }
    void SetGuildId(uint64_t ullGuildId);

    DT_GUILD_PLAYER_BASE_INFO& GetGuildBaseInfo() { return m_stBaseInfo; }

private:
    DT_GUILD_PLAYER_BASE_INFO 		m_stBaseInfo;
    DT_GUILD_PLAYER_APPLY_INFO 	    m_stApplyInfo;
};

