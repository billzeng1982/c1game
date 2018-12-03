#include "Message.h"
#include "ss_proto.h"
#include "dwlog_def.h"
#include "Task.h"
#include "Friend.h"
#include "Mail.h"
#include "Gm/Gm.h"
#include "GeneralCard.h"
#include "Mail.h"
#include "Marquee.h"
#include "Consume.h"
#include "Item.h"

using namespace PKGMETA;


bool Message::Init()
{
    RESBASIC* poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(MESSAGE_WORLD_CD_TIME);
    assert(poResBasic);
    m_dwWordCD = (uint32_t) poResBasic->m_para[0];
	m_dwWordLvLimit = (uint32_t)poResBasic->m_para[1];
    return true;
}

int Message::Send(PlayerData* pstData, CS_PKG_MESSAGE_SEND_REQ& rstCsPkgBodyReq)
{
    int iRet = ERR_NONE;


    if (pstData->GetRoleBaseInfo().m_llBlackRoomTime > CGameTime::Instance().GetCurrSecond())
    {
        LOGERR("the role is in the back room");
        return ERR_BACK_ROOM;
    }
#ifdef _DEBUG
	if (1 == ZoneSvr::Instance().GetConfig().m_iGmDebugSwitch)
	{
		GmMgr::Instance().GmCmdTest(pstData, rstCsPkgBodyReq.m_stRecord.m_szRecord);
	}
    //GeneralCard::Instance().GetTeamLi(pstData);
    //Mail::TestSendPubMail();
#endif


    DT_MESSAGE_ONE_RECORD_INFO& rstRecord = rstCsPkgBodyReq.m_stRecord;
    rstRecord.m_szRecord[MAX_MESSAGE_RECORD_LEN - 1] = '\0';    //保证字符串复制没问题
    rstRecord.m_dwRecordType = 0;    //客户端是不能设置为1的,以后有其他扩展,再检查
    switch (rstRecord.m_bChannel)
    {
    case PKGMETA::MESSAGE_CHANNEL_WORLD:
        iRet = _SendWorldMessage(pstData, rstRecord);
        break;
    case PKGMETA::MESSAGE_CHANNEL_PRIVATE:
        iRet = _SendPrivateMessage(pstData, rstRecord);
        break;
    case PKGMETA::MESSAGE_CHANNEL_GUILD:
        iRet = _SendGuildMessage(pstData, rstRecord);
        break;
	case PKGMETA::MESSAGE_CHANNEL_HORN:
		Marquee::Instance().SaveMarqueeForHorn(pstData, rstRecord.m_szRecord);

		//rstRecord.m_bChannel = MESSAGE_CHANNEL_WORLD;
		iRet = _SendWorldMessage(pstData, rstRecord, false, true);
		break;
    default:
        LOGERR("Channel error, player cheat?");
        iRet = ERR_SYS;
        break;
    }
    return iRet;
}

int Message::GetBox(PlayerData* pstData, CS_PKG_MESSAGE_GET_BOX_REQ& rstPkgBodyReq)
{
    //填写请求包
    m_stSsPkg.m_stHead.m_ullUin = pstData->m_pOwner->GetUin();
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MESSAGE_GET_BOX_REQ;
    m_stSsPkg.m_stBody.m_stMessageGetBoxReq.m_ullUin = pstData->m_ullUin;
    m_stSsPkg.m_stBody.m_stMessageGetBoxReq.m_ullGuild = pstData->GetGuildInfo().m_ullGuildId;
    ZoneSvrMsgLayer::Instance().SendToMessageSvr(m_stSsPkg);


    
    return ERR_NONE;
}

int Message::DelPrivateBox(uint64_t ullPlayerUin)
{
    //填写请求包
    m_stSsPkg.m_stHead.m_ullUin = ullPlayerUin;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MESSAGE_DEL_BOX_REQ;
    m_stSsPkg.m_stBody.m_stMessageGetBoxReq.m_ullUin = ullPlayerUin;
    m_stSsPkg.m_stBody.m_stMessageDelBoxReq.m_bChannel = MESSAGE_CHANNEL_PRIVATE;
    ZoneSvrMsgLayer::Instance().SendToMessageSvr(m_stSsPkg);
    return ERR_NONE;
}


int Message::_SendPrivateMessage(PlayerData* pstData, DT_MESSAGE_ONE_RECORD_INFO& rstRecord)
{
    uint64_t ullFriendUin = rstRecord.m_ullReceiverUin;
    if (!Friend::Instance().IsFriend(pstData, ullFriendUin))
    {
        return ERR_SYS;
    }

    Player* poFriend = PlayerMgr::Instance().GetPlayerByUin(ullFriendUin);
    InitOneRecord(pstData, MESSAGE_CHANNEL_PRIVATE, rstRecord);
    //玩家在线立即发给玩家
    if (NULL != poFriend)
    {
        m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MESSAGE_CAST_NTF;
        memcpy(&m_stScPkg.m_stBody.m_stMessageCastNtf.m_stRecord, &rstRecord, sizeof(DT_MESSAGE_ONE_RECORD_INFO));
        ZoneSvrMsgLayer::Instance().SendToClient(poFriend, &m_stScPkg);
    }
    else
    {
		//玩家不在线,发给MessageSvr存储
        //填写请求包
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MESSAGE_SEND_REQ;
        memcpy(&m_stSsPkg.m_stBody.m_stMessageSendReq.m_stRecord, &rstRecord, sizeof(DT_MESSAGE_ONE_RECORD_INFO));
        ZoneSvrMsgLayer::Instance().SendToMessageSvr(m_stSsPkg);
        //LOGERR("OUT: _SendPrivateMessage, player not online!");
    }
     return ERR_NONE;
}

int Message::_SendGuildMessage(PlayerData* pstData, DT_MESSAGE_ONE_RECORD_INFO& rstRecord)
{
    InitOneRecord(pstData, MESSAGE_CHANNEL_GUILD, rstRecord);
   
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_MESSAGE_SEND_REQ;
    m_stSsPkg.m_stHead.m_ullUin = rstRecord.m_ullSenderUin;
    uint64_t ullGuildId = pstData->GetGuildInfo().m_ullGuildId;
    if (0 == ullGuildId)
    {
        LOGERR("Uin<%lu> GuildId is 0 ", pstData->m_ullUin);
        return ERR_NONE;
    }
    rstRecord.m_ullReceiverUin = ullGuildId;
    SS_PKG_GUILD_MESSAGE_SEND_REQ& rstGuildMessageSendReq = m_stSsPkg.m_stBody.m_stGuildMessageSendReq;
    rstGuildMessageSendReq.m_ullGuildId = ullGuildId;
    memcpy(&rstGuildMessageSendReq.m_stRecord, &rstRecord, sizeof(DT_MESSAGE_ONE_RECORD_INFO));
    m_stScPkg.m_stBody.m_stMessageCastNtf.m_stRecord.m_szRecord[MAX_MESSAGE_RECORD_LEN - 1] = '\0';
    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);

    // 任务记数修改
    Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_GUILD_OTHER, 1/*value*/, 2);

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MESSAGE_SEND_REQ;
    memcpy(&m_stSsPkg.m_stBody.m_stMessageSendReq.m_stRecord, &rstRecord, sizeof(DT_MESSAGE_ONE_RECORD_INFO));

    ZoneSvrMsgLayer::Instance().SendToMessageSvr(m_stSsPkg);
    return ERR_NONE;
}

int Message::InitOneRecord(PlayerData* pstData, uint8_t bChannel, DT_MESSAGE_ONE_RECORD_INFO& rstRecord)
{
    int iRet = ERR_NONE;
    StrCpy(rstRecord.m_szSenderName, pstData->GetMajestyInfo().m_szName, MAX_NAME_LENGTH);
	DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
	rstRecord.m_wHeadIconId = rstMajestyInfo.m_wIconId;
	rstRecord.m_wHeadFrameId = rstMajestyInfo.m_wFrameId;
	rstRecord.m_wHeadTitleId = rstMajestyInfo.m_wTitleId;
    rstRecord.m_bChannel = bChannel;
    rstRecord.m_ullSenderUin = pstData->m_ullUin;
    rstRecord.m_ullTimeStampSec = CGameTime::Instance().GetCurrTimeMs();  //@TODO 时间戳
    rstRecord.m_bVipLv = rstMajestyInfo.m_bVipLv;
    return iRet;
}

int Message::_SendWorldMessage(PlayerData* pstData, DT_MESSAGE_ONE_RECORD_INFO& rstRecord, bool bIsAutoSend, bool bIsHorn)
{
	int iRet = 0;
    if (!bIsAutoSend && (iRet = IsSendWorldMessageOk(pstData)) != 0)
    {
        return iRet;
    }
	if (!bIsHorn)
	{
		InitOneRecord(pstData, MESSAGE_CHANNEL_WORLD, rstRecord);
	}
	else
	{
		InitOneRecord(pstData, MESSAGE_CHANNEL_HORN, rstRecord);
	}
	
    if (m_MessageWorld.m_wCount < MAX_MESSAGE_WORLD_CACHE_NUM)
    {
        m_MessageWorld.m_astAllRecord[m_MessageWorld.m_wCount++] = rstRecord;
    }
    else
    {//满了顶掉最老的
        m_MessageWorld.m_astAllRecord[m_MessageWorld.m_wLastPos] = rstRecord;
        m_MessageWorld.m_wLastPos = (m_MessageWorld.m_wLastPos + 1) % MAX_MESSAGE_WORLD_CACHE_NUM;
    }
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MESSAGE_CAST_NTF;
    memcpy(&m_stScPkg.m_stBody.m_stMessageCastNtf.m_stRecord, &rstRecord, sizeof(DT_MESSAGE_ONE_RECORD_INFO));
    ZoneSvrMsgLayer::Instance().BroadcastToClient(&m_stScPkg);
    return 0;
}


int Message::_SendSysMessage(DT_MESSAGE_ONE_RECORD_INFO& rstRecord)
{
    //InitOneRecord(pstData, MESSAGE_CHANNEL_SYS, rstRecord);
    rstRecord.m_bChannel = MESSAGE_CHANNEL_SYS;
    rstRecord.m_ullTimeStampSec = CGameTime::Instance().GetCurrTimeMs();
    if (m_MessageSys.m_wCount   < MAX_MESSAGE_SYS_CACHE_NUM)
    {
        m_MessageSys.m_astAllRecord[m_MessageSys.m_wCount++] = rstRecord;
    }
    else
    {//满了顶掉最老的
        m_MessageSys.m_astAllRecord[m_MessageSys.m_wLastPos] = rstRecord;
        m_MessageSys.m_wLastPos = (m_MessageSys.m_wLastPos + 1) % MAX_MESSAGE_SYS_CACHE_NUM;
    }
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MESSAGE_CAST_NTF;
    memcpy(&m_stScPkg.m_stBody.m_stMessageCastNtf.m_stRecord, &rstRecord, sizeof(DT_MESSAGE_ONE_RECORD_INFO));
    ZoneSvrMsgLayer::Instance().BroadcastToClient(&m_stScPkg);
    return 0;
}

int Message::MessageSendCheck(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstSyncItemInfo, uint8_t bChannel)
{
	int iRet = ERR_NONE;

	do 
	{
		if (bChannel != MESSAGE_CHANNEL_HORN)
		{
			break;
		}

		ResBasicMgr_t& rstResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
		RESBASIC* pDiamondConsume = rstResBasicMgr.Find(DIAMOND_SEND_HORN_MESSAGE);
		if (pDiamondConsume == NULL)
		{
			LOGERR("pDiamondConsume is NULL");
			iRet = ERR_SYS;
			break;
		}
		
		uint32_t dwConsumeDia = pDiamondConsume->m_para[0];
		if (!Consume::Instance().IsEnough(pstData, ITEM_TYPE_DIAMOND, dwConsumeDia))
		{
			iRet = ERR_NOT_ENOUGH_DIAMOND;
			break;
		}
		
		Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -dwConsumeDia, rstSyncItemInfo, DWLOG::METHOD_MESSAGE_HORN);

	} while (false);
	

	return iRet;
	
}

int Message::IsSendWorldMessageOk(PlayerData* pstData)
{
	if (pstData->GetLv() < m_dwWordLvLimit)
	{
		return ERR_MAJESTY_UN_SATISFY;
	}
	
    uint64_t ullCurTimeMs = CGameTime::Instance().GetCurrTimeMs();
    if (pstData->GetMajestyInfo().m_ullLastSendWorldTime == 0 || 
        pstData->GetMajestyInfo().m_ullLastSendWorldTime + m_dwWordCD < ullCurTimeMs)
    {
        pstData->GetMajestyInfo().m_ullLastSendWorldTime = ullCurTimeMs;
        return 0;
    }
    
    LOGERR("Uin<%lu> sending world message is in CD.", pstData->m_ullUin);
    return ERR_MESSAGE_SEND_WORLD_IN_COLD;
}


void Message::AutoSendWorldMessage(PlayerData* pstData, uint32_t dwMsgId, const char* pszFmt, ...)
{
    RESMESSAGE* poMessage = CGameDataMgr::Instance().GetResMessageMgr().Find(dwMsgId);
    if (!poMessage || poMessage->m_bSwitch != 1)
    {
        return;
    }
    bzero(&m_stRecord, sizeof(m_stRecord));
    va_list	ap;
    va_start(ap, pszFmt);
    vsnprintf(m_stRecord.m_szRecord, sizeof(m_stRecord.m_szRecord), pszFmt, ap);
    va_end(ap);
    m_stRecord.m_dwRecordType = dwMsgId;
    _SendWorldMessage(pstData, m_stRecord, true);
}

void Message::AutoSendSysMessage(uint32_t dwMsgId, const char* pszFmt, ...)
{
    RESMESSAGE* poMessage = CGameDataMgr::Instance().GetResMessageMgr().Find(dwMsgId);
    if (!poMessage || poMessage->m_bSwitch != 1)
    {
        return;
    }
    bzero(&m_stRecord, sizeof(m_stRecord));
    va_list	ap;
    va_start(ap, pszFmt);
    vsnprintf(m_stRecord.m_szRecord, sizeof(m_stRecord.m_szRecord), pszFmt, ap);
    va_end(ap);
    m_stRecord.m_dwRecordType = dwMsgId;
    _SendSysMessage(m_stRecord);
}

