#include "GuildPvpRoom.h"
#include "GuildDataDynPool.h"

using namespace PKGMETA;

GuildPvpRoom::GuildPvpRoom()
{
}

GuildPvpRoom::~GuildPvpRoom()
{
}

bool GuildPvpRoom::Init(uint64_t ullGuildId)
{
    m_ullGuildId = ullGuildId;
    return true;
}

void GuildPvpRoom::Clear()
{
    m_mapId2RoomIter = m_mapId2Room.begin();
    for (; m_mapId2RoomIter!= m_mapId2Room.end(); m_mapId2RoomIter++)
    {
        DT_PVP_ROOM_INFO* pstRoom = m_mapId2RoomIter->second;
        GuildDataDynPool::Instance().GuildRoomPool().Release(pstRoom);
    }
    m_mapId2Room.clear();
}

int GuildPvpRoom::UpdateRoom(DT_PVP_ROOM_INFO& rstRoomInfo)
{
    m_mapId2RoomIter = m_mapId2Room.find(rstRoomInfo.m_ullRoomNo);
    if (m_mapId2RoomIter == m_mapId2Room.end())
    {
        DT_PVP_ROOM_INFO* pstRoom = GuildDataDynPool::Instance().GuildRoomPool().Get();
        if (pstRoom == NULL)
        {
            LOGERR_r("Guild(%lu) get DT_PVP_ROOM_INFO from GuildDataDynPool failed", m_ullGuildId);
            return ERR_SYS;
        }
        *pstRoom = rstRoomInfo;
        m_mapId2Room.insert(MapId2Room_t::value_type(rstRoomInfo.m_ullRoomNo, pstRoom));
        AddToWaitDelList(*pstRoom);
        return ERR_NONE;
    }
    else
    {
        DT_PVP_ROOM_INFO* pstRoom = m_mapId2RoomIter->second;
        *pstRoom = rstRoomInfo;
        AddToWaitDelList(*pstRoom);
        return ERR_NONE;
    }
}

int GuildPvpRoom::DelRoom(DT_PVP_ROOM_INFO& rstRoomInfo)
{
    m_mapId2RoomIter = m_mapId2Room.find(rstRoomInfo.m_ullRoomNo);
    if (m_mapId2RoomIter == m_mapId2Room.end())
    {
        return ERR_NOT_FOUND;
    }
    else
    {
        GuildDataDynPool::Instance().GuildRoomPool().Release(m_mapId2RoomIter->second);
        m_mapId2Room.erase(m_mapId2RoomIter);
        return ERR_NONE;
    }
}

void GuildPvpRoom::AddToWaitDelList(DT_PVP_ROOM_INFO & rstRoomInfo)
{
    if (rstRoomInfo.m_bPlayerCount >= 2)
    {
        m_WaitToDelList.push_back(make_pair(CGameTime::Instance().GetCurrSecond(), rstRoomInfo.m_ullRoomNo));
    }
}


void GuildPvpRoom::Update()
{
    if (!m_WaitToDelList.empty() && 
        (CGameTime::Instance().GetCurrSecond() - m_WaitToDelList.front().first ) > TIME_OUT_SECOND)
    {
        DT_PVP_ROOM_INFO stRoomInfo;
        stRoomInfo.m_ullRoomNo = m_WaitToDelList.front().second;
        DelRoom(stRoomInfo);
        m_WaitToDelList.pop_front();
    }
}

void GuildPvpRoom::GetGuildRoomInfo(DT_GUILD_ROOM_INFO& rstGuildRoom)
{
    rstGuildRoom.m_bRoomCount = 0;
    m_mapId2RoomIter = m_mapId2Room.begin();
    for (; m_mapId2RoomIter!= m_mapId2Room.end(); m_mapId2RoomIter++)
    {
        DT_PVP_ROOM_INFO* pstRoom = m_mapId2RoomIter->second;
        rstGuildRoom.m_astRoomList[rstGuildRoom.m_bRoomCount++] = *pstRoom;
    }
}

