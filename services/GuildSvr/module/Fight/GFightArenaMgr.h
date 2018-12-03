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
    //����id��ս���Ķ�Ӧ��ϵ,��Ҽ���ս����Э�鴫�����Լ��Ĺ���id,Ҫ�ܸ��ݹ���id�ҵ���Ӧ��ս��
	MapGuildId2Arena_t m_mapId2Arena;
	MapGuildId2Arena_t::iterator m_mapId2ArenaIter;

    //ս��list
    ListArena_t m_listArena;
    ListArena_t::iterator m_listArenaIter;

    //��ɾ��ս��list
    ListArena_t m_listToDelArena;
    ListArena_t::iterator m_listToDelArenaIter;

    uint64_t m_ullUptTime;
};

