#pragma once

#include "common_proto.h"
#include <map>

using namespace std;
using namespace PKGMETA;


class GuildApply
{
public:
    GuildApply();
    virtual ~GuildApply(){};

    bool InitFromDB(DT_GUILD_APPLY_BLOB& rstBlob, uint64_t ullGuildId, int iMaxNum, uint16_t wVersion);
    bool PackGuildApplyInfo(DT_GUILD_APPLY_BLOB& rstBlob, uint16_t wVersion);
    void Clear();

    int Add(DT_ONE_GUILD_MEMBER& rstOneApply);
    int Del(uint64_t ullUin);
    DT_ONE_GUILD_MEMBER* Find(uint64_t ullUin);

    uint64_t GetUptTimeStamp() { return m_ullUptTimestamp; }
    void SetUptTimeStamp(uint64_t ullTimestamp) { m_ullUptTimestamp = ullTimestamp; }

private:
    int m_iCount;
    int m_iMaxNum;
    uint64_t m_ullGuildId;
    uint64_t m_ullUptTimestamp;
    map<uint64_t, DT_ONE_GUILD_MEMBER*> m_oUinToApplyMap;
    map<uint64_t, DT_ONE_GUILD_MEMBER*>::iterator m_oUinToApplyMapIter;
};