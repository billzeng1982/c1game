#include "GFightArenaMgr.h"
#include "GameTime.h"

using namespace PKGMETA;

bool GFightArenaMgr::Init()
{
    this->Clear();
    return true;
}

void GFightArenaMgr::Clear()
{
    //ToDelList的clear
    for(m_listToDelArenaIter=m_listToDelArena.begin();m_listToDelArenaIter!=m_listToDelArena.end();m_listToDelArenaIter++)
    {
        GuildFightArena* poArena= *m_listToDelArenaIter;
        poArena->Clear();
        m_listArena.remove(poArena);
        delete poArena;
    }
    m_listToDelArena.clear();

    //List的clear
    m_listArenaIter = m_listArena.begin();
    for (; m_listArenaIter!= m_listArena.end(); m_listArenaIter++)
    {
        GuildFightArena* poArena = *m_listArenaIter;
        this->Delete(poArena);
    }
    m_listArena.clear();

    //map的clear
    m_mapId2Arena.clear();

    m_ullUptTime = CGameTime::Instance().GetCurrTimeMs();
}

void GFightArenaMgr::Update()
{
    if (m_listToDelArena.size() > 0)
    {
        for(m_listToDelArenaIter=m_listToDelArena.begin();m_listToDelArenaIter!=m_listToDelArena.end();m_listToDelArenaIter++)
        {
            GuildFightArena* poArena= *m_listToDelArenaIter;
            //将对阵双方Guild从m_mapId2Arena中删除
            for (int i=0; i<MAX_NUM_ONE_GUILD_FIGHT_JOIN_GUILD; i++)
            {
                m_mapId2ArenaIter = m_mapId2Arena.find(poArena->m_GuildInfoList[i].m_ullGuildId);
                if (m_mapId2ArenaIter != m_mapId2Arena.end())
                {
                    m_mapId2Arena.erase(m_mapId2ArenaIter);
                }
            }
            poArena->Clear();
            m_listArena.remove(poArena);
            delete poArena;
        }
        m_listToDelArena.clear();
    }
}


GuildFightArena* GFightArenaMgr::New()
{
    GuildFightArena* poArena = new GuildFightArena();
    return poArena;
}


void GFightArenaMgr::Delete(GuildFightArena* poArena)
{
    if (poArena==NULL)
    {
        return;
    }

    m_listToDelArena.push_back(poArena);

    return;
}


void GFightArenaMgr::AddArena(GuildFightArena* poArena)
{
    //将公会Id与战场的对应关系加入map
    m_mapId2Arena.insert(MapGuildId2Arena_t::value_type(poArena->m_GuildInfoList[0].m_ullGuildId, poArena));
    m_mapId2Arena.insert(MapGuildId2Arena_t::value_type(poArena->m_GuildInfoList[1].m_ullGuildId, poArena));

    //将战场加入List
    m_listArena.push_back(poArena);
}

GuildFightArena* GFightArenaMgr::GetArenaByGuildId(uint64_t ullGuildId)
{
     m_mapId2ArenaIter = m_mapId2Arena.find(ullGuildId);
     if (m_mapId2ArenaIter == m_mapId2Arena.end())
     {
        return NULL;
     }
     return m_mapId2ArenaIter->second;
}

