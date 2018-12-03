#include "MsgLogicMessage.h"
#include "LogMacros.h"
#include "ss_proto.h"
#include "cs_proto.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "../module/player/PlayerMgr.h"
#include "../module/Message.h"


//***********
//  CS协议  
//***********

//发送聊天消息
int MessageSendReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{


    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }
    
    //  处理逻辑    
    CS_PKG_MESSAGE_SEND_REQ& rstCsPkgBodyReq = rstCsPkg.m_stBody.m_stMessageSendReq;
    SC_PKG_MESSAGE_SEND_RSP& rstScPkgBodyReq = m_stScPkg.m_stBody.m_stMessageSendRsp;
    //rstCsPkgBodyReq.m_stRecord.m_ullSenderUin = poPlayer->GetUin();
	rstScPkgBodyReq.m_stSyncItemInfo.m_bSyncItemCount = 0;
	
	int iRet = ERR_NONE;
	do 
	{
		iRet = Message::Instance().MessageSendCheck(&poPlayer->GetPlayerData(), rstScPkgBodyReq.m_stSyncItemInfo, rstCsPkgBodyReq.m_stRecord.m_bChannel);
		if (iRet != ERR_NONE)
		{
			break;
		}

		iRet = Message::Instance().Send(&poPlayer->GetPlayerData(),rstCsPkgBodyReq);
	} while (false);

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MESSAGE_SEND_RSP;
    rstScPkgBodyReq.m_nErrNo = iRet;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);



    return 0;
}

int MessageGetBoxReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{



    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    Message::Instance().GetBox(&poPlayer->GetPlayerData(),rstCsPkg.m_stBody.m_stMessageGetBoxReq);



    return 0;
}

//删除
int MessageDelBoxReq_CS::HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara )
{



    Player* poPlayer = PlayerMgr::Instance().GetPlayerBySessionId(pstSession->m_dwSessionId);
    if (NULL == poPlayer)
    {
        LOGERR("player not exist.");
        return -1;
    }

    Message::Instance().DelPrivateBox(poPlayer->GetUin());



    return 0;
}



//***********
//  SS协议  
//***********


//给发言玩家回复
int MessageSendRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_GUILD_MESSAGE_SEND_RSP & rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stGuildMessageSendRsp;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer is null");
        return -1;
    }
    //将GuildSvr的结果发给Clinet
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MESSAGE_SEND_RSP;
    SC_PKG_MESSAGE_SEND_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stMessageSendRsp;
    rstScPkgBodyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}

//给发言玩家回复
int MessageGetBoxRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_MESSAGE_GET_BOX_RSP & rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stMessageGetBoxRsp;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer is null");
        return -1;
    }
    //填写回复包
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MESSAGE_GET_BOX_RSP;
    SC_PKG_MESSAGE_GET_BOX_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stMessageGetBoxRsp;
    rstScPkgBodyRsp.m_nErrNo = ERR_NONE;
    memcpy(&rstScPkgBodyRsp.m_stGuildBox, &rstSsPkgBodyRsp.m_stGuildBox, sizeof(rstSsPkgBodyRsp.m_stGuildBox));
    memcpy(&rstScPkgBodyRsp.m_stPrivateBox, &rstSsPkgBodyRsp.m_stPrivateBox, sizeof(rstSsPkgBodyRsp.m_stPrivateBox));
    Message::Instance().GetSysBox(rstScPkgBodyRsp.m_stSysBox);
    Message::Instance().GetWorldBox(rstScPkgBodyRsp.m_stWorldBox);
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);



    return 0;
}


//给删除留言箱玩家回复
int MessageDelBoxRsp_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{



    SS_PKG_MESSAGE_DEL_BOX_RSP & rstSsPkgBodyRsp = rstSsPkg.m_stBody.m_stMessageDelBoxRsp;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsPkg.m_stHead.m_ullUin);
    if (!poPlayer)
    {
        LOGERR("poPlayer is null");
        return -1;
    }
    //填写回复包
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MESSAGE_DEL_BOX_RSP;
    SC_PKG_MESSAGE_DEL_BOX_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stMessageDelBoxRsp;
    rstScPkgBodyRsp.m_nErrNo = rstSsPkgBodyRsp.m_nErrNo;
    ZoneSvrMsgLayer::Instance().SendToClient(poPlayer, &m_stScPkg);
    return 0;
}


int MessageAutoSendNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL*/)
{
    Player* poPlayer = NULL;
    SS_PKG_MESSAGE_AUTO_SEND_NTF & rstSsNtf = rstSsPkg.m_stBody.m_stMessageAutoSendNtf;
    for (uint8_t i = 0; i < rstSsNtf.m_bCount; i++)
    {
        if (rstSsNtf.m_astRecords[i].m_bChannel == MESSAGE_CHANNEL_SYS)
        {
            Message::Instance().AutoSendSysMessage(rstSsNtf.m_astRecords[i].m_dwRecordType, rstSsNtf.m_astRecords[i].m_szRecord);
        }
        else if (rstSsNtf.m_astRecords[i].m_bChannel == MESSAGE_CHANNEL_WORLD)
        {
            poPlayer = PlayerMgr::Instance().GetPlayerByUin(rstSsNtf.m_astRecords[i].m_ullSenderUin);
            if (!poPlayer)
            {
                continue;;
            }
            Message::Instance().AutoSendWorldMessage(&poPlayer->GetPlayerData(), rstSsNtf.m_astRecords[i].m_dwRecordType, rstSsNtf.m_astRecords[i].m_szRecord);
        }
    }
    return 0;
}

