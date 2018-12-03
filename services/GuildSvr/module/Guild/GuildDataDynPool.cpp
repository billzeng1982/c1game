#include "GuildDataDynPool.h"
#include "LogMacros.h"

bool GuildDataDynPool::Init()
{
    if (!m_oGuildMemberPool.Init(MEMBER_POOL_INIT_NUM, MEMBER_POOL_DELTA_NUM))
    {
        LOGERR_r("Init GuildMemberPool failed");
        return false;
    }

    if (!m_oGuildApplyPool.Init(APPLY_POOL_INIT_NUM, APPLY_POOL_DELTA_NUM))
    {
        LOGERR_r("Init GuildApplyPool failed");
        return false;
    }

    if (!m_oGuildReplayPool.Init(REPLAY_POOL_INIT_NUM, REPLAY_POOL_DELTA_NUM))
    {
        LOGERR_r("Init GuildApplyPool failed");
        return false;
    }

    if (!m_oGuildRoomPool.Init(ROOM_POOL_INIT_NUM, ROOM_POOL_DELTA_NUM))
    {
        LOGERR_r("Init GuildApplyPool failed");
        return false;
    }

    return true;
}
