#pragma once
#include "cs_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"
#include <map>

class PvpRoomMgr : public TSingleton<PvpRoomMgr>
{
private:
    typedef std::map<uint64_t, DT_PVP_ROOM_INFO*> MapId2Room_t;

public:
    PvpRoomMgr();
    virtual ~PvpRoomMgr();
    bool Init();

    int CreateRoom(PlayerData* pstData, CS_PKG_PVP_CREATE_ROOM_REQ& rstReq, SC_PKG_PVP_CREATE_ROOM_RSP&rstRsp);
    void QuitRoom(PlayerData* pstData);
    int QuitRoom(PlayerData* pstData, CS_PKG_PVP_QUIT_ROOM_REQ& rstReq, SC_PKG_PVP_QUIT_ROOM_RSP&rstRsp);
    int JoinRoom(PlayerData* pstData, CS_PKG_PVP_JOIN_ROOM_REQ& rstReq, SC_PKG_PVP_JOIN_ROOM_RSP&rstRsp);
    int InviteToRoom(PlayerData* pstData, CS_PKG_PVP_INVITE_TO_ROOM_REQ& rstReq, SC_PKG_PVP_INVITE_TO_ROOM_RSP&rstRsp);
    int ChgState(PlayerData* pstData, CS_PKG_PVP_ROOM_CHG_STATE_REQ& rstReq, SC_PKG_PVP_ROOM_CHG_STATE_RSP& rstRsp);
    void DestroyRoom(uint64_t ullRoomNo);

private:
    //���ɷ����
    uint64_t _GenRoomNo();

    //����һ���·���
    DT_PVP_ROOM_INFO* _NewRoom();

    //����һ������
    void _DelRoom(DT_PVP_ROOM_INFO& pstRoomInfo);

    //��ʼ�����������Ϣ
    void _InitRoomPlayer(PlayerData* pstData, DT_PVP_ROOM_PLAYER_INFO& rstRoomPlayer);

    //�ڷ�����ɾ��ָ�������
    int _DelRoomPlayer(DT_PVP_ROOM_INFO& pstRoomInfo, uint64_t ullUin);

    //��������Ϣ����GuildSvr
    void _SendRoomInfoToGuildSvr(DT_PVP_ROOM_INFO& rstRoomInfo);

    //��������
    int _CreateDungeon(DT_PVP_ROOM_INFO& rstRoomInfo);

    //���ͷ�����Ϣ���֪ͨ
    void _SendRoomUptNtf(DT_PVP_ROOM_INFO& rstRoomInfo, uint64_t ullUin);

	//������Ҿ���Լս�ۼƴ���
	void _AddPlayerFightTimes(DT_PVP_ROOM_INFO* pstRoomInfo);

private:
    PKGMETA::SCPkg m_stScPkg;
    PKGMETA::SSPkg m_stSsPkg;
    uint64_t m_ullRoomNo;
    MapId2Room_t m_mapId2Room;
    MapId2Room_t::iterator m_mapId2RoomIter;
};