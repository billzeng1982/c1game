#pragma once

#include "common_proto.h"
#include <map>
#include <list>
using namespace std;
using namespace PKGMETA;

class GuildPvpRoom
{
private:
    typedef map<uint64_t, DT_PVP_ROOM_INFO*> MapId2Room_t;
    const int static TIME_OUT_SECOND = 600; 
public:
    GuildPvpRoom();
    ~GuildPvpRoom();

    bool Init(uint64_t ullGuildId);
    void Clear();

    void GetGuildRoomInfo(DT_GUILD_ROOM_INFO& rstGuildRoom);

    int UpdateRoom(DT_PVP_ROOM_INFO& rstRoomInfo);
    int DelRoom(DT_PVP_ROOM_INFO& rstRoomInfo);
    void AddToWaitDelList(DT_PVP_ROOM_INFO& rstRoomInfo);
    void Update();
private:
    uint64_t m_ullGuildId;
    MapId2Room_t m_mapId2Room;
    MapId2Room_t::iterator m_mapId2RoomIter;
    list<pair<uint64_t, uint64_t> > m_WaitToDelList;    //定时删除列表
};