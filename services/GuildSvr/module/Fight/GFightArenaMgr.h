#pragma once

#include "GuildFightArena.h"
#include "singleton.h"
#include "common_proto.h"
#include <map>
#include <list>

using namespace PKGMETA;
using namespace std;

typedef map<uint64_t, GuildFightArena*> MapGuildId2Arena_t;
typedef list<GuildFightArena*> ListArena_t;


class GFightArenaMgr : public TSingleton<GFightArenaMgr>
{
public:
	bool Init();
    void Clear();

	GuildFightArena* New();

	void Delete(GuildFightArena* poArena);

    void AddArena(GuildFightArena* poArena);

    void Update();

    GuildFightArena* GetArenaByGuildId(uint64_t ullGuildId);

public:
    //公会id和战场的对应关系,玩家加入战场的协议传的是自己的公会id,要能根据公会id找到对应的战场
	MapGuildId2Arena_t m_mapId2Arena;
	MapGuildId2Arena_t::iterator m_mapId2ArenaIter;

    //战场list
    ListArena_t m_listArena;
    ListArena_t::iterator m_listArenaIter;

    //待删除战场list
    ListArena_t m_listToDelArena;
    ListArena_t::iterator m_listToDelArenaIter;

    uint64_t m_ullUptTime;
};

