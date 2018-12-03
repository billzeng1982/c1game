#pragma once

#include "common_proto.h"
#include <list>

using namespace std;
using namespace PKGMETA;

class GuildReplay
{
private:
    typedef list<DT_REPLAY_INFO*> ListReplay_t;

public:
    GuildReplay();
    ~GuildReplay();

    bool InitFromDB(DT_GUILD_REPLAY_BLOB& rstBlob, uint64_t ullGuildId, uint16_t wVersion);
    bool PackGuildReplayInfo(DT_GUILD_REPLAY_BLOB& rstBlob, uint16_t wVersion);
    void Clear();

    int Add(DT_REPLAY_INFO* rstOneReplay);

private:
    void _RemoveReplay(uint32_t dwRaceNumber);

private:
    int m_iCount;
    uint64_t m_ullGuildId;
    ListReplay_t m_listReplay;
    ListReplay_t::iterator m_listReplayIter;
};

