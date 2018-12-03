#pragma once

#include "common_proto.h"
#include "DynMempool.h"

using namespace PKGMETA;

class GuildDataDynPool : public TSingleton<GuildDataDynPool>
{
    const static int MEMBER_POOL_INIT_NUM = 1024;
    const static int MEMBER_POOL_DELTA_NUM = 256;

    const static int APPLY_POOL_INIT_NUM = 1024;
    const static int APPLY_POOL_DELTA_NUM = 256;

    const static int REPLAY_POOL_INIT_NUM = 128;
    const static int REPLAY_POOL_DELTA_NUM = 32;

    const static int ROOM_POOL_INIT_NUM = 256;
    const static int ROOM_POOL_DELTA_NUM = 128;

public:
    GuildDataDynPool() {}
    virtual ~GuildDataDynPool() {}

    bool Init();

    DynMempool<DT_ONE_GUILD_MEMBER>&     GuildMemberPool() { return m_oGuildMemberPool; }
    DynMempool<DT_ONE_GUILD_MEMBER>&     GuildApplyPool() { return m_oGuildApplyPool; }
    DynMempool<DT_REPLAY_INFO>&   GuildReplayPool() { return m_oGuildReplayPool; }
    DynMempool<DT_PVP_ROOM_INFO>&    GuildRoomPool() { return m_oGuildRoomPool; }

private:
    DynMempool<DT_ONE_GUILD_MEMBER> m_oGuildMemberPool;
    DynMempool<DT_ONE_GUILD_MEMBER> m_oGuildApplyPool;
    DynMempool<DT_REPLAY_INFO> m_oGuildReplayPool;
    DynMempool<DT_PVP_ROOM_INFO> m_oGuildRoomPool;
};

