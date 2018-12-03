#include "PvpRoomMgr.h"
#include "ZoneSvrMsgLayer.h"
#include "ss_proto.h"
#include "PlayerMgr.h"
#include "Match.h"
#include "ZoneLog.h"
#include "dwlog_def.h"
#include "Majesty.h"
#include "GloryItemsMgr.h"

using namespace PKGMETA;
using namespace DWLOG;

PvpRoomMgr::PvpRoomMgr()
{
    m_ullRoomNo = 10000;
    m_mapId2Room.clear();
}

PvpRoomMgr::~PvpRoomMgr()
{

}

int PvpRoomMgr::CreateRoom(PlayerData* pstData, CS_PKG_PVP_CREATE_ROOM_REQ& rstReq, SC_PKG_PVP_CREATE_ROOM_RSP&rstRsp)
{
    //TODO:检查是否可以创建房间
    if (pstData->m_ullRoomNo != 0)
    {
        return ERR_DEFAULT;
    }

    DT_PVP_ROOM_INFO* pstRoomInfo = _NewRoom();
    if (NULL == pstRoomInfo)
    {
        return ERR_SYS;
    }

    pstRoomInfo->m_ullRoomNo = _GenRoomNo();
    pstRoomInfo->m_bRoomType = rstReq.m_bRoomType;
    pstRoomInfo->m_ullReserveId = pstData->GetGuildInfo().m_ullGuildId;
    pstRoomInfo->m_bPlayerCount = 1;
    this->_InitRoomPlayer(pstData, pstRoomInfo->m_astPlayerList[0]);

    m_mapId2Room.insert(MapId2Room_t::value_type(pstRoomInfo->m_ullRoomNo, pstRoomInfo));

    pstData->m_ullRoomNo = pstRoomInfo->m_ullRoomNo;
    rstRsp.m_stRoom = *pstRoomInfo;

    //如果是公会约战，需要将房间信息同步到GuildSvr
    if (pstRoomInfo->m_bRoomType == PVP_ROOM_GUILD)
    {
		//记录军团约战Log
		ZoneLog::Instance().WriteGuildLog(pstData, METHOD_GUILD_OP_PVP_CREATE_ROOM, pstData->GetGuildInfo().m_ullGuildId, NULL, 0, 0);
        this->_SendRoomInfoToGuildSvr(*pstRoomInfo);
    }

    return ERR_NONE;
}

int PvpRoomMgr::QuitRoom(PlayerData* pstData, CS_PKG_PVP_QUIT_ROOM_REQ& rstReq, SC_PKG_PVP_QUIT_ROOM_RSP&rstRsp)
{
    pstData->m_ullRoomNo = 0;

    m_mapId2RoomIter = m_mapId2Room.find(rstReq.m_ullRoomNo);
    if (m_mapId2RoomIter == m_mapId2Room.end())
    {
        //TODO:不存在的房间
        return ERR_NOT_FOUND;
    }

    DT_PVP_ROOM_INFO* pstRoomInfo = m_mapId2RoomIter->second;
    int iRet = _DelRoomPlayer(*pstRoomInfo, pstData->GetRoleBaseInfo().m_ullUin);
    if (iRet != ERR_NONE)
    {
        //处于战斗状态，不能重置RoomNo
        if (iRet == ERR_NOT_SATISFY_COND)
        {
            pstData->m_ullRoomNo = pstRoomInfo->m_ullRoomNo;
        }
        return iRet;
    }

    //如果是公会约战，需要将房间信息同步到GuildSvr
    if (pstRoomInfo->m_bRoomType == PVP_ROOM_GUILD)
    {
        this->_SendRoomInfoToGuildSvr(*pstRoomInfo);
    }

    if (pstRoomInfo->m_bPlayerCount == 0)
    {
        //房间人数为0时，自动删除房间
        this->_DelRoom(*pstRoomInfo);
    }
    /*
    else
    {
        //将房间信息同步给房间内的另一个人
        this->_SendRoomUptNtf(*pstRoomInfo, pstRoomInfo->m_astPlayerList[0].m_ullPlayerId);
    }*/

    return ERR_NONE;
}

int PvpRoomMgr::JoinRoom(PlayerData* pstData, CS_PKG_PVP_JOIN_ROOM_REQ& rstReq, SC_PKG_PVP_JOIN_ROOM_RSP&rstRsp)
{
    if (pstData->m_ullRoomNo != 0)
    {
        return ERR_DEFAULT;
    }

    m_mapId2RoomIter = m_mapId2Room.find(rstReq.m_ullRoomNo);
    if (m_mapId2RoomIter == m_mapId2Room.end())
    {
        return ERR_NOT_FOUND;
    }
    DT_PVP_ROOM_INFO* pstRoomInfo = m_mapId2RoomIter->second;

    if (pstRoomInfo->m_bPlayerCount >= MAX_FIGHT_PLAYER_NUM)
    {
        //TODO:房间已满错误
        return ERR_ROOM_FULL;
    }

    this->_InitRoomPlayer(pstData, pstRoomInfo->m_astPlayerList[pstRoomInfo->m_bPlayerCount ++]);
    pstData->m_ullRoomNo = pstRoomInfo->m_ullRoomNo;
    rstRsp.m_stRoom = *pstRoomInfo;

    /*
    //将房间信息同步给房间内的另一个人
    this->_SendRoomUptNtf(*pstRoomInfo, pstRoomInfo->m_astPlayerList[0].m_ullPlayerId);
    */

    //如果是公会约战，需要将房间信息同步到GuildSvr
    if (pstRoomInfo->m_bRoomType == PVP_ROOM_GUILD)
    {
		//记录军团约战Log
		ZoneLog::Instance().WriteGuildLog(pstData, METHOD_GUILD_OP_PVP_JOIN_ROOM, pstData->GetGuildInfo().m_ullGuildId, NULL, 0, 0);
        this->_SendRoomInfoToGuildSvr(*pstRoomInfo);
    }

    int iRet = ERR_NONE;
    if (pstRoomInfo->m_bPlayerCount >= MAX_FIGHT_PLAYER_NUM)
    {
        iRet = this->_CreateDungeon(*pstRoomInfo);
        if (iRet != ERR_NONE)
        {
            LOGERR("PvpRoom Create Dungeon failed, RoomNo=(%lu), Player1=(%s), Player2=(%s)",
                    pstRoomInfo->m_ullRoomNo, pstRoomInfo->m_astPlayerList[0].m_szPlayerName, pstRoomInfo->m_astPlayerList[1].m_szPlayerName);
            this->DestroyRoom(pstRoomInfo->m_ullRoomNo);
        }
        else
        {
            pstRoomInfo->m_astPlayerList[0].m_bState = ROOM_PLAYER_FIGHT;
            pstRoomInfo->m_astPlayerList[1].m_bState = ROOM_PLAYER_FIGHT;
			_AddPlayerFightTimes(pstRoomInfo);
        }
    }

    return ERR_NONE;
}

int PvpRoomMgr::InviteToRoom(PlayerData* pstData, CS_PKG_PVP_INVITE_TO_ROOM_REQ& rstReq, SC_PKG_PVP_INVITE_TO_ROOM_RSP&rstRsp)
{
    //被邀请者是否在线
    Player* poInvitee = PlayerMgr::Instance().GetPlayerByUin(rstReq.m_ullInviteeId);
    if (poInvitee == NULL)
    {
        return ERR_NOT_FOUND;
    }

    //给被邀请者发送消息
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PVP_INVITE_TO_ROOM_NTF;
    SC_PKG_PVP_INVITE_TO_ROOM_NTF& rstNtf = m_stScPkg.m_stBody.m_stPvpInviteToRoomNtf;
    rstNtf.m_ullRoomNo = rstReq.m_ullRoomNo;
    memcpy(rstNtf.m_szInviterName, pstData->GetRoleBaseInfo().m_szRoleName, PKGMETA::MAX_NAME_LENGTH);
    ZoneSvrMsgLayer::Instance().SendToClient(poInvitee, &m_stScPkg);

    return ERR_NONE;
}


int PvpRoomMgr::ChgState(PlayerData* pstData, CS_PKG_PVP_ROOM_CHG_STATE_REQ& rstReq, SC_PKG_PVP_ROOM_CHG_STATE_RSP& rstRsp)
{
    m_mapId2RoomIter = m_mapId2Room.find(rstReq.m_ullRoomNo);
    if (m_mapId2RoomIter == m_mapId2Room.end())
    {
        return ERR_NOT_FOUND;
    }
    DT_PVP_ROOM_INFO* pstRoomInfo = m_mapId2RoomIter->second;

    int iIndex = -1;
    for (uint8_t i=0; i<pstRoomInfo->m_bPlayerCount; i++)
    {
        if (pstRoomInfo->m_astPlayerList[i].m_ullPlayerId == pstData->GetRoleBaseInfo().m_ullUin)
        {
            iIndex = (int)i;
        }
    }
    if (iIndex == -1)
    {
        return ERR_NOT_FOUND;
    }

    pstRoomInfo->m_astPlayerList[iIndex].m_bState = rstReq.m_bState;

    if (pstRoomInfo->m_bPlayerCount >= MAX_FIGHT_PLAYER_NUM)
    {
        //将房间信息同步给房间内的另一个人
        this->_SendRoomUptNtf(*pstRoomInfo,
        pstRoomInfo->m_astPlayerList[(MAX_FIGHT_PLAYER_NUM- iIndex) % MAX_FIGHT_PLAYER_NUM].m_ullPlayerId);
    }

    //当双方都准备好后，直接开战
    if (pstRoomInfo->m_bPlayerCount == MAX_FIGHT_PLAYER_NUM &&
      pstRoomInfo->m_astPlayerList[0].m_bState == ROOM_PLAYER_READY &&
      pstRoomInfo->m_astPlayerList[1].m_bState == ROOM_PLAYER_READY)
    {
        int iRet = _CreateDungeon(*pstRoomInfo);
        if (iRet == ERR_NONE)
        {
            pstRoomInfo->m_astPlayerList[0].m_bState = ROOM_PLAYER_FIGHT;
            pstRoomInfo->m_astPlayerList[1].m_bState = ROOM_PLAYER_FIGHT;
        }
    }

    return ERR_NONE;
}


void PvpRoomMgr::DestroyRoom(uint64_t ullRoomNo)
{
    m_mapId2RoomIter = m_mapId2Room.find(ullRoomNo);
    if (m_mapId2RoomIter != m_mapId2Room.end())
    {
        DT_PVP_ROOM_INFO* pstRoomInfo = m_mapId2RoomIter->second;
        if (pstRoomInfo->m_bRoomType == PVP_ROOM_GUILD)
        {
            //删除房间时要将玩家的RoomNo清零
            for (int i=0; i<pstRoomInfo->m_bPlayerCount; i++)
            {
                Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(pstRoomInfo->m_astPlayerList[i].m_ullPlayerId);
                if (poPlayer)
                {
                    poPlayer->GetPlayerData().m_ullRoomNo = 0;
                }
            }
            pstRoomInfo->m_bPlayerCount = 0;
            this->_SendRoomInfoToGuildSvr(*pstRoomInfo);
        }

        delete(pstRoomInfo);
        m_mapId2Room.erase(m_mapId2RoomIter);
    }
}


void PvpRoomMgr::QuitRoom(PlayerData* pstData)
{
    if (pstData->m_ullRoomNo == 0)
    {
        return;
    }

    CS_PKG_PVP_QUIT_ROOM_REQ rstReq;
    SC_PKG_PVP_QUIT_ROOM_RSP rstRsp;
    rstReq.m_ullRoomNo = pstData->m_ullRoomNo;

    this->QuitRoom(pstData, rstReq, rstRsp);
}

uint64_t PvpRoomMgr::_GenRoomNo()
{
    return m_ullRoomNo++;
}

DT_PVP_ROOM_INFO* PvpRoomMgr::_NewRoom()
{
    return new DT_PVP_ROOM_INFO();
}

void PvpRoomMgr::_DelRoom(DT_PVP_ROOM_INFO& pstRoomInfo)
{
    m_mapId2RoomIter = m_mapId2Room.find(pstRoomInfo.m_ullRoomNo);
    if (m_mapId2RoomIter != m_mapId2Room.end())
    {
        delete(m_mapId2RoomIter->second);
        m_mapId2Room.erase(m_mapId2RoomIter);
    }
}

int PvpRoomMgr::_DelRoomPlayer(DT_PVP_ROOM_INFO& pstRoomInfo, uint64_t ullUin)
{
    for (uint8_t i=0; i<pstRoomInfo.m_bPlayerCount; i++)
    {
        if (pstRoomInfo.m_astPlayerList[i].m_ullPlayerId == ullUin)
        {
            if (pstRoomInfo.m_astPlayerList[i].m_bState == ROOM_PLAYER_FIGHT)
            {
                return ERR_NOT_SATISFY_COND;
            }
            pstRoomInfo.m_astPlayerList[i] = pstRoomInfo.m_astPlayerList[--pstRoomInfo.m_bPlayerCount];
            return ERR_NONE;
        }
    }
    return ERR_NOT_FOUND;
}

void PvpRoomMgr::_InitRoomPlayer(PlayerData* pstData, DT_PVP_ROOM_PLAYER_INFO& rstRoomPlayer)
{
    rstRoomPlayer.m_bState = ROOM_PLAYER_IDLE;
    rstRoomPlayer.m_ullPlayerId = pstData->GetRoleBaseInfo().m_ullUin;
    memcpy(rstRoomPlayer.m_szPlayerName, pstData->GetRoleBaseInfo().m_szRoleName, PKGMETA::MAX_NAME_LENGTH);
}

void PvpRoomMgr::_SendRoomInfoToGuildSvr(DT_PVP_ROOM_INFO& rstRoomInfo)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_REFRESH_GUILD_PVP_ROOM_NTF;
    SS_PKG_REFRESH_GUILD_PVP_ROOM_NTF& rstNtf = m_stSsPkg.m_stBody.m_stGuildRoomRefreshNtf;
    rstNtf.m_stRoomInfo = rstRoomInfo;
    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
}

int PvpRoomMgr::_CreateDungeon(DT_PVP_ROOM_INFO& rstRoomInfo)
{
    Player* poPlayer1 = PlayerMgr::Instance().GetPlayerByUin(rstRoomInfo.m_astPlayerList[0].m_ullPlayerId);
    Player* poPlayer2 = PlayerMgr::Instance().GetPlayerByUin(rstRoomInfo.m_astPlayerList[1].m_ullPlayerId);

    // 初始化匹配玩家数据
    int iRet = ERR_NONE;
    do
    {
        if (poPlayer1 == NULL || poPlayer2 == NULL)
        {
            iRet = ERR_NOT_SATISFY_COND;
            break;
        }

        iRet = Match::Instance().InitFightPlayerInfo(&poPlayer1->GetPlayerData(), 0);
        if (iRet != ERR_NONE)
        {
            break;
        }

        iRet = Match::Instance().InitFightPlayerInfo(&poPlayer2->GetPlayerData(), 0);
        if (iRet != ERR_NONE)
        {
            break;
        }

        // 创建副本
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_DUNGEON_CREATE_REQ;
        m_stSsPkg.m_stHead.m_ullReservId = 0;
        SS_PKG_DUNGEON_CREATE_REQ& rstSsPkgBodyReq = m_stSsPkg.m_stBody.m_stDungeonCreateReq;
        rstSsPkgBodyReq.m_stDungeonInfo.m_bMatchType = MATCH_TYPE_GUILD_PVP;
        rstSsPkgBodyReq.m_stDungeonInfo.m_bFakeType = MATCH_FAKE_NONE;
        rstSsPkgBodyReq.m_stDungeonInfo.m_bFightPlayerNum = 2;
        rstSsPkgBodyReq.m_stDungeonInfo.m_astFightPlayerList[0] = poPlayer1->GetPlayerData().m_oSelfInfo;
        rstSsPkgBodyReq.m_stDungeonInfo.m_astFightPlayerList[0].m_chGroup = PLAYER_GROUP_DOWN;
        rstSsPkgBodyReq.m_stDungeonInfo.m_astFightPlayerList[1] = poPlayer2->GetPlayerData().m_oSelfInfo;
        rstSsPkgBodyReq.m_stDungeonInfo.m_astFightPlayerList[1].m_chGroup = PLAYER_GROUP_UP;

        ZoneSvrMsgLayer::Instance().SendToFightSvr(m_stSsPkg);
    }while(false);

    return iRet;
}

void PvpRoomMgr::_SendRoomUptNtf(DT_PVP_ROOM_INFO& rstRoomInfo, uint64_t ullUin)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(ullUin);
    if (poPlayer == NULL)
    {
        LOGERR("poPlayer is not exsit, Uin=(%lu)", ullUin);
        return;
    }

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PVP_ROOM_UPDATE_NTF;
    SC_PKG_PVP_ROOM_UPDATE_NTF& rstNtf = m_stScPkg.m_stBody.m_stPvpRoomInfoUptNtf;
    rstNtf.m_stRoom = rstRoomInfo;

    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
}

void PvpRoomMgr::_AddPlayerFightTimes(DT_PVP_ROOM_INFO* pstRoomInfo)
{
	for (int i = 0; i < pstRoomInfo->m_bPlayerCount; i++)
	{
		uint64_t ullUin = pstRoomInfo->m_astPlayerList[i].m_ullPlayerId;
		Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(ullUin);
		if (poPlayer)
		{
			DT_ROLE_MISC_INFO& rstMiscInfo = poPlayer->GetPlayerData().GetMiscInfo();
			rstMiscInfo.m_wGuildFightCnt++;
			//主公称号任务获得
			GloryItemsMgr::Instance().AddMajestyItems(&poPlayer->GetPlayerData(), MAJESTY_ITEM_ACCESS_GUILD_PVP, rstMiscInfo.m_wGuildFightCnt);
		}
	}
}
