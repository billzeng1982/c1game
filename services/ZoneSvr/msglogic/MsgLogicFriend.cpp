#include "LogMacros.h"
#include "MsgLogicFriend.h"
#include "common_proto.h"
#include "dwlog_def.h"
#include "../gamedata/GameDataMgr.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "../module/player/Player.h"
#include "../module/player/PlayerMgr.h"
#include "../module/Friend.h"
#include "../module/Item.h"

using namespace PKGMETA;


//CS协议

int FriendHandleReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
        Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
        if (!poPlayer)
        {
            LOGERR("poPlayer is null");
            return -1;
        }
        
        CS_PKG_FRIEND_HANDLE_REQ& rstCSPkgBodyReq = rstCsPkg.m_stBody.m_stFriendHandleReq;
        SS_PKG_FRIEND_HANDLE_REQ& rstSSPkgBodyReq = m_stSsPkg.m_stBody.m_stFriendHandleReq;

        //填写请求包
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_FRIEND_HANDLE_REQ;
        m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
        rstSSPkgBodyReq.m_ullUin = rstCSPkgBodyReq.m_ullUin;
        rstSSPkgBodyReq.m_bHandleType = rstCSPkgBodyReq.m_bHandleType;
    Friend::Instance().InitPlayerInfo(&poPlayer->GetPlayerData(), rstSSPkgBodyReq.m_stPlayerInfo);
        ZoneSvrMsgLayer::Instance().SendToFriendSvr(m_stSsPkg);
        return 0;
}


int FriendGetListReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
        Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
        if (!poPlayer)
        {
            LOGERR("poPlayer is null");
            return -1;
        }
        SS_PKG_FRIEND_GET_LIST_REQ& rstSSPkgBodyReq = m_stSsPkg.m_stBody.m_stFriendGetListReq;
        Friend::Instance().InitPlayerInfo(&poPlayer->GetPlayerData(), rstSSPkgBodyReq.m_stPlayerInfo);

        Friend::Instance().IsUpdatePlayerData(&poPlayer->GetPlayerData(), rstSSPkgBodyReq.m_ullResetTimestamp);

        //填写请求包
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_FRIEND_GET_LIST_REQ;
        m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
        ZoneSvrMsgLayer::Instance().SendToFriendSvr(m_stSsPkg);
        return 0;
}


int FriendSearchReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{
        Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
        if (!poPlayer)
        {
            LOGERR("poPlayer is null");
            return -1;
        }

        CS_PKG_FRIEND_SEARCH_REQ& rstCSPkgBodyReq = rstCsPkg.m_stBody.m_stFriendSearchReq;
        SS_PKG_FRIEND_SEARCH_REQ& rstSSPkgBodyReq = m_stSsPkg.m_stBody.m_stFriendSearchReq;

        //填写请求包
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_FRIEND_SEARCH_REQ;
        m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
        rstSSPkgBodyReq.m_bType = rstCSPkgBodyReq.m_bType;
        rstSSPkgBodyReq.m_ullUin = rstCSPkgBodyReq.m_ullUin;
        StrCpy(rstSSPkgBodyReq.m_szName , rstCSPkgBodyReq.m_szName, MAX_NAME_LENGTH);
        ZoneSvrMsgLayer::Instance().SendToFriendSvr(m_stSsPkg);
        return 0;
}

int GetPlayerBriefInfoReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION * pstSession, PKGMETA::CSPKG & rstCsPkg, void * pvPara)
{
    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (!poPlayer)
    {
        LOGERR("poPlayer is null");
        return -1;
    }
    CS_PKG_GET_PLAYER_BRIEF_INFO_REQ& rstCSPkgBodyReq = rstCsPkg.m_stBody.m_stGetPlayerBriefInfoReq;
    

    Player* poTarget = PlayerMgr::Instance().GetPlayerByUin(rstCSPkgBodyReq.m_ullUin);
    if (poTarget)
    {
        /*
            玩家在线,直接取数据
        */
        SC_PKG_GET_PLAYER_BRIEF_INFO_RSP& rstSCPkgBodyRsp = m_stScPkg.m_stBody.m_stGetPlayerBriefInfoRsp;
        Friend::Instance().InitPlayerInfo(&poTarget->GetPlayerData(), rstSCPkgBodyRsp.m_stPlayerInfo);
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GET_PLAYER_BRIEF_INFO_RSP;
        rstSCPkgBodyRsp.m_nErrNo = ERR_NONE;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    }
    else
    {
        /*
            玩家没在线,通过好友搜索功能获取玩家信息
        */
        SS_PKG_FRIEND_SEARCH_REQ& rstSSPkgBodyReq = m_stSsPkg.m_stBody.m_stFriendSearchReq;
        //填写请求包
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_FRIEND_SEARCH_REQ;
        m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
        rstSSPkgBodyReq.m_bType = FRIEND_HANDLE_TYPE_GET_PLAYER_INFO;
        rstSSPkgBodyReq.m_ullUin = rstCSPkgBodyReq.m_ullUin;
        rstSSPkgBodyReq.m_szName[0] = '\0';
        ZoneSvrMsgLayer::Instance().SendToFriendSvr(m_stSsPkg);
    }

    return 0;
}

int FriendSendApReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION * pstSession, PKGMETA::CSPKG & rstCsPkg, void * pvPara)
{
        Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
        if (!poPlayer)
        {
            LOGERR("poPlayer is null");
            return -1;
        }

        CS_PKG_FRIEND_SEND_AP_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stFriendSendApReq;
        SS_PKG_FRIEND_SEND_AP_REQ& rstSsPkgBodyReq = m_stSsPkg.m_stBody.m_stFriendSendApReq;

        //填写请求包
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_FRIEND_SEND_AP_REQ;
        m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
        rstSsPkgBodyReq.m_wCount = rstCsPkgBodyReq.m_wCount;
        memcpy(rstSsPkgBodyReq.m_ApReceiverUins, rstCsPkgBodyReq.m_ApReceiverUins, sizeof(uint64_t)*rstCsPkgBodyReq.m_wCount);
        ZoneSvrMsgLayer::Instance().SendToFriendSvr(m_stSsPkg);
        return 0;
}

int FriendGetApReq_CS::HandleClientMsg(const PKGMETA::CONNSESSION * pstSession, PKGMETA::CSPKG & rstCsPkg, void * pvPara)
{
        Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
        if (!poPlayer)
        {
            LOGERR("poPlayer is null");
            return -1;
        }

        CS_PKG_FRIEND_GET_AP_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stFriendGetApReq;
        SS_PKG_FRIEND_GET_AP_REQ& rstSsPkgBodyReq = m_stSsPkg.m_stBody.m_stFriendGetApReq;

        //填写请求包
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_FRIEND_GET_AP_REQ;
        m_stSsPkg.m_stHead.m_ullUin = poPlayer->GetUin();
        rstSsPkgBodyReq.m_wCount = rstCsPkgBodyReq.m_wCount;
        memcpy(rstSsPkgBodyReq.m_ApGiverUins, rstCsPkgBodyReq.m_ApGiverUins, sizeof(uint64_t)*rstCsPkgBodyReq.m_wCount);
        ZoneSvrMsgLayer::Instance().SendToFriendSvr(m_stSsPkg);
        return 0;
}



//SS协议

int FriendHandleRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
        Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
        if (!poPlayer)
        {
            LOGERR("poPlayer is null");
            return -1;
        }
        SS_PKG_FRIEND_HANDLE_RSP& rstSSPkgBodyRsp = rstSsPkg.m_stBody.m_stFriendHandleRsp;
        SC_PKG_FRIEND_HANDLE_RSP& rstSCPkgBodyRsp = m_stScPkg.m_stBody.m_stFriendHandleRsp;
        if (ERR_NONE == rstSSPkgBodyRsp.m_nErrNo)
        {
            Friend::Instance().UpdateAgreeInfo(&poPlayer->GetPlayerData(), rstSSPkgBodyRsp.m_bHandleType, rstSSPkgBodyRsp.m_ullUin);
        }

        //填写回复包
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FRIEND_HANDLE_RSP;
        rstSCPkgBodyRsp.m_nErrNo = rstSSPkgBodyRsp.m_nErrNo;
		rstSCPkgBodyRsp.m_bHandleType = rstSSPkgBodyRsp.m_bHandleType;
		rstSCPkgBodyRsp.m_ullUin = rstSSPkgBodyRsp.m_ullUin;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        
        return 0;
}

int FriendGetListRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
        Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
        if (!poPlayer)
        {
            LOGERR("poPlayer is null");
            return -1;
        }

        DT_ROLE_MISC_INFO& rstMiscInfo = poPlayer->GetPlayerData().GetMiscInfo();

        SS_PKG_FRIEND_GET_LIST_RSP& rstSSPkgBodyRsp = rstSsPkg.m_stBody.m_stFriendGetListRsp;
        SC_PKG_FRIEND_GET_LIST_RSP& rstSCPkgBodyRsp = m_stScPkg.m_stBody.m_stFriendGetListRsp;

        if (ERR_NONE == rstSSPkgBodyRsp.m_nErrNo)
        {
            Friend::Instance().InitAgreeInfo(&poPlayer->GetPlayerData(), rstSSPkgBodyRsp.m_stAgreeInfo);
        }

        if (rstSSPkgBodyRsp.m_ullResetTimestamp != 0)
        {
            rstMiscInfo.m_ullFriendApTimestamp = rstSSPkgBodyRsp.m_ullResetTimestamp;
            rstMiscInfo.m_wApNumFromFriend = 0;
        }

        //填写回复包
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FRIEND_GET_LIST_RSP;
        rstSCPkgBodyRsp.m_nErrNo = rstSSPkgBodyRsp.m_nErrNo;
        memcpy(&rstSCPkgBodyRsp.m_stWholeData, &rstSSPkgBodyRsp.m_stWholeData, sizeof(rstSSPkgBodyRsp.m_stWholeData));
        memcpy(&rstSCPkgBodyRsp.m_stSendApInfo, &rstSSPkgBodyRsp.m_stSendApInfo, sizeof(DT_FRIEND_SEND_AP_INFO));
        memcpy(&rstSCPkgBodyRsp.m_stRecvApInfo, &rstSSPkgBodyRsp.m_stRecvApInfo, sizeof(DT_FRIEND_RECV_AP_INFO));
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

        return 0;
}


int FriendEventNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
        Player* poPlayer = NULL;
        SS_PKG_FRIEND_EVENT_NTF& rstSSPkgBodyRsp = rstSsPkg.m_stBody.m_stFriendEventNtf;
        SC_PKG_FRIEND_EVENT_NTF& rstSCPkgBodyRsp = m_stScPkg.m_stBody.m_stFriendEventNtf;



        //填写回复包
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FRIEND_EVENT_NTF;
        for (int i = 0; i < rstSSPkgBodyRsp.m_wCount; i++)
        {
            poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSSPkgBodyRsp.m_Uins[i]);
            if (!poPlayer)
            {
                 //LOGERR("poPlayer is null");
                 //return -1;
            continue;
            }
            rstSCPkgBodyRsp.m_bMsgId = rstSSPkgBodyRsp.m_bMsgId;
            memcpy(&rstSCPkgBodyRsp.m_stPlayerInfo, &rstSSPkgBodyRsp.m_stPlayerInfo, sizeof(DT_FRIEND_PLAYER_INFO));

            ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        }

        return 0;
}

int FriendSearchRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
        Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
        if (!poPlayer)
        {
            LOGERR("poPlayer is null");
            return -1;
        }
        SS_PKG_FRIEND_SEARCH_RSP& rstSSPkgBodyRsp = rstSsPkg.m_stBody.m_stFriendSearchRsp;
        if (rstSSPkgBodyRsp.m_stPlayerInfo.m_bMajestyLevel == 0)
        {
            LOGERR("Uin<%lu>, Name<%s>, IsOnline<%d>, Li<%d>", rstSSPkgBodyRsp.m_stPlayerInfo.m_ullUin,
             rstSSPkgBodyRsp.m_stPlayerInfo.m_szName, rstSSPkgBodyRsp.m_stPlayerInfo.m_bIsOnline, rstSSPkgBodyRsp.m_stPlayerInfo.m_dwLi);
        }

    if (FRIEND_HANDLE_TYPE_GET_PLAYER_INFO == rstSSPkgBodyRsp.m_bHandleType)
    {
        //获取玩家简要信息
        SC_PKG_GET_PLAYER_BRIEF_INFO_RSP& rstSCPkgBodyRsp = m_stScPkg.m_stBody.m_stGetPlayerBriefInfoRsp;
        //填写回复包
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GET_PLAYER_BRIEF_INFO_RSP;
        rstSCPkgBodyRsp.m_nErrNo = rstSSPkgBodyRsp.m_nErrNo;
        memcpy(&rstSCPkgBodyRsp.m_stPlayerInfo, &rstSSPkgBodyRsp.m_stPlayerInfo, sizeof(DT_FRIEND_PLAYER_INFO));
        
    }
    else
    {
        SC_PKG_FRIEND_SEARCH_RSP& rstSCPkgBodyRsp = m_stScPkg.m_stBody.m_stFriendSearchRsp;

        //填写回复包
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FRIEND_SEARCH_RSP;
        rstSCPkgBodyRsp.m_nErrNo = rstSSPkgBodyRsp.m_nErrNo;
        memcpy(&rstSCPkgBodyRsp.m_stPlayerInfo, &rstSSPkgBodyRsp.m_stPlayerInfo, sizeof(DT_FRIEND_PLAYER_INFO));
        
    }
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);

        return 0;
}

int FriendSendApRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
        Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
        if (!poPlayer)
        {
            LOGERR("poPlayer is null");
            return -1;
        }

        SC_PKG_FRIEND_SEND_AP_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stFriendSendApRsp;
        SS_PKG_FRIEND_SEND_AP_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stFriendSendApRsp;
        do 
        {
            if (rstSsPkgBodyRsp.m_nErrNo != ERR_NONE)
            {
                 rstScPkgBodyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;
                 break;
            }

            memcpy(&rstScPkgBodyRsp.m_stSendApInfo, &rstSsPkgBodyRsp.m_stSendApInfo, sizeof(DT_FRIEND_SEND_AP_INFO));
            rstScPkgBodyRsp.m_nErrNo = ERR_NONE;
        } while (false);

        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FRIEND_SEND_AP_RSP;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);


        //通知在线玩家有人送体力了
        if (rstScPkgBodyRsp.m_nErrNo == ERR_NONE)
        {
            m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FRIEND_RECEIVED_AP_NTF;
            SC_PKG_FRIEND_RECEIVED_AP_NTF& rstScBodyNtf = m_stScPkg.m_stBody.m_stFriendRecvdApNtf;
            rstScBodyNtf.m_ullSenderUin = rstSsPkg.m_stHead.m_ullUin;
            for (int i=0; i<rstSsPkgBodyRsp.m_stReceiverListInfo.m_wCount; i++)
            {
                 Player* poPlayerReceiver = PlayerMgr::Instance().GetPlayerByUin(rstSsPkgBodyRsp.m_stReceiverListInfo.m_List[i]);
                 if (poPlayerReceiver)
                 {
                    ZoneSvrMsgLayer::Instance().SendToClient(poPlayerReceiver, &m_stScPkg);
                 }
            }
        }

        return 0;
}

int FriendGetApRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
        Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
        if (!poPlayer)
        {
            LOGERR("poPlayer is null");
            return -1;
        }

        DT_ROLE_MISC_INFO& rstMiscInfo = poPlayer->GetPlayerData().GetMiscInfo();

        do 
        {
            SC_PKG_FRIEND_GET_AP_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stFriendGetApRsp;
            SS_PKG_FRIEND_GET_AP_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stFriendGetApRsp;
            rstScPkgBodyRsp.m_nErrNo = ERR_NONE;
            rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

            if (rstSsPkgBodyRsp.m_nErrNo != ERR_NONE)
            {
                 rstScPkgBodyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;
                 break;
            }
            

            if (rstMiscInfo.m_wApNumFromFriend >= MAX_AP_NUM_FROM_FRIEND)
            {
                 rstScPkgBodyRsp.m_nErrNo = ERR_MAX_FRIEND_AP_TODAY;
                 break;
            }

            uint16_t mTotalAp = AP_NUM_PER_FRIEND_SEND * rstSsPkgBodyRsp.m_nCount;
            uint16_t mLeftAp = MAX_AP_NUM_FROM_FRIEND - rstMiscInfo.m_wApNumFromFriend;

            uint16_t nApNum = mTotalAp > mLeftAp ? mLeftAp : mTotalAp;
            rstMiscInfo.m_wApNumFromFriend += nApNum;
            Item::Instance().ConsumeItem(&poPlayer->GetPlayerData(), ITEM_TYPE_AP, 0, nApNum, rstScPkgBodyRsp.m_stSyncItemInfo, DWLOG::METHOD_FRIEND_GET_AP);
            
            memcpy(&rstScPkgBodyRsp.m_stRecvApInfo, &rstSsPkgBodyRsp.m_stRecvApInfo, sizeof(DT_FRIEND_RECV_AP_INFO));
            rstScPkgBodyRsp.m_nErrNo = ERR_NONE;
            
        } while (false);
        
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FRIEND_GET_AP_RSP;
        ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
        return 0;
}

int FriendNameChangeRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
        SS_PKG_FRIEND_CHANGE_NAME_RSP& rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stFriendChangeNameRsp;
        SC_PKG_FRIEND_NAME_CHANGE_NTF& rstScPkgBodyNtf = m_stScPkg.m_stBody.m_stFriendChangeNameNtf;
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_FRIEND_NAME_CHANGE_NTF;

        rstScPkgBodyNtf.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
        memcpy(rstScPkgBodyNtf.m_szRoleName, rstSsPkgBodyRsp.m_szName, MAX_NAME_LENGTH);
        for (int i=0; i<rstSsPkgBodyRsp.m_stAgreeInfo.m_wCount; i++)
        {
            Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkgBodyRsp.m_stAgreeInfo.m_List[i]);
            if (poPlayer)
            {
                 ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
            }
            
        }
        return 0;
}


