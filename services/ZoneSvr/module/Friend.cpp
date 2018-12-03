#include "define.h"
#include "LogMacros.h"
#include "Friend.h"
#include "Majesty.h"
#include "GeneralCard.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "GameTime.h"
#include "./player/PlayerMgr.h"
#include "ZoneLog.h"
#include "dwlog_svr.h"


using namespace PKGMETA;
using namespace DWLOG;

static int CompareUin(const void* pA, const void* pB)
{
	if (*((uint64_t*)pA) > *((uint64_t*)pB))
	{
		return 1;
	}
	else if (*((uint64_t*)pA) < *((uint64_t*)pB))
	{
		return -1;
	}
    return 0;
}

void Friend::UpdatePlayerInfo(PlayerData* pstData, uint8_t bIsOnline)
{
    if (NULL == pstData)
    {
        LOGERR("pstData is NULL");
        return;
    }

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_FRIEND_EVENT_NTF;
    m_stSsPkg.m_stHead.m_ullUin = pstData->GetRoleBaseInfo().m_ullUin;
    SS_PKG_FRIEND_EVENT_NTF& rstSSBodyNtf = m_stSsPkg.m_stBody.m_stFriendEventNtf;
    rstSSBodyNtf.m_bMsgId = FRIEND_HANDLE_TYPE_UPDATE_PLAYER_INFO;
    InitPlayerInfo(pstData, rstSSBodyNtf.m_stPlayerInfo);
    rstSSBodyNtf.m_stPlayerInfo.m_bIsOnline = bIsOnline;
    if (0 == bIsOnline)
    {   //离线时间
        rstSSBodyNtf.m_stPlayerInfo.m_ullLastOffTimestamp = CGameTime::Instance().GetCurrSecond();
    }
    if ('\0' == rstSSBodyNtf.m_stPlayerInfo.m_szName[0])
    {//玩家在创建角色 输入名字时就退出了, 导致创建角色失败
        LOGWARN("Player create role name failed! <%lu> ", pstData->GetRoleBaseInfo().m_ullUin);
        return;
    }
    ZoneSvrMsgLayer::Instance().SendToFriendSvr(m_stSsPkg);
    return ;
}



void Friend::InitPlayerInfo(IN PlayerData* pstData, OUT DT_FRIEND_PLAYER_INFO& stPlayerInfo)
{
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
    DT_ROLE_PVP_BASE_INFO & rstPvpInfo = pstData->GetELOInfo().m_stPvp6v6Info.m_stBaseInfo;
    DT_ROLE_GCARD_INFO& rstGCardInfo =  pstData->GetGCardInfo();
    DT_ROLE_ELO_INFO& rstELOInfo = pstData->GetELOInfo();

    DT_ROLE_MAJESTY_INFO& rstRoleMajestyInfo = pstData->GetMajestyInfo();
    stPlayerInfo.m_ullUin = pstData->m_pOwner->GetUin();
    stPlayerInfo.m_bIsOnline = 1;  //默认上线
    stPlayerInfo.m_wHeadIconId = rstRoleMajestyInfo.m_wIconId;
	stPlayerInfo.m_wHeadFrameId = rstRoleMajestyInfo.m_wFrameId;
	stPlayerInfo.m_wHeadTitleId = rstRoleMajestyInfo.m_wTitleId;
    stPlayerInfo.m_bELOLvId = rstPvpInfo.m_bELOLvId;
    stPlayerInfo.m_dwELOFightNum = rstPvpInfo.m_wWinCount + rstPvpInfo.m_wLoseCount;
    stPlayerInfo.m_wELOFightWinNum = rstPvpInfo.m_wWinCount;   //胜利场数
    stPlayerInfo.m_dwLeaderValue = pstData->m_dwLeaderValue;

    stPlayerInfo.m_wOwnGeneralCount = rstGCardInfo.m_iCount;
    stPlayerInfo.m_bMajestyLevel = rstMajestyInfo.m_wLevel;
    stPlayerInfo.m_bStarNum = rstELOInfo.m_stPvp6v6Info.m_stBaseInfo.m_dwScore;
    stPlayerInfo.m_ullLastOffTimestamp = 0;
    StrCpy(stPlayerInfo.m_szName, rstRoleMajestyInfo.m_szName, DWLOG::MAX_NAME_LENGTH);

	stPlayerInfo.m_bVipLv = rstMajestyInfo.m_bVipLv;

    DT_BATTLE_ARRAY_INFO* pstBattleArrayInfo = Majesty::Instance().GetBattleArrayInfo(pstData, BATTLE_ARRAY_TYPE_NORMAL);
    if (NULL == pstBattleArrayInfo)
    {
        LOGERR("Uin<%lu> Find pstBattleArrayInfo error", pstData->m_ullUin);
        return;
    }
    for (size_t i = 0; i < pstBattleArrayInfo->m_bGeneralCnt; i++)
    {
        uint32_t dwGeneralId = pstBattleArrayInfo->m_GeneralList[i];

        stPlayerInfo.m_astGCardList[i].m_dwGeneralID = dwGeneralId;
        if (0 == dwGeneralId)
        {
            continue;
        }
        //获取相应的武将卡
        DT_ITEM_GCARD* pstGeneral = GeneralCard::Instance().Find(pstData, dwGeneralId);
        if (!pstGeneral)
        {
            LOGERR("Uin<%lu> cant find the GCard<%u>", pstData->m_ullUin, dwGeneralId);
            continue;
        }
        stPlayerInfo.m_astGCardList[i].m_bGrade = pstGeneral->m_bPhase;
        stPlayerInfo.m_astGCardList[i].m_bStar = pstGeneral->m_bStar;
        stPlayerInfo.m_astGCardList[i].m_bLv = pstGeneral->m_bLevel;
        stPlayerInfo.m_astGCardList[i].m_dwSkinId = pstGeneral->m_dwSkinId;
    }
    return;
}

void Friend::InitAgreeInfo(IN PlayerData* pstData, OUT DT_FRIEND_AGREE_INFO& rstAgreeInfo)
{
    if (NULL == pstData)
    {
        LOGERR("pstData is NULL");
    }
    memcpy(&pstData->m_oFriendAgreeInfo, &rstAgreeInfo, sizeof(DT_FRIEND_AGREE_INFO));
}


void Friend::UpdateAgreeInfo(PlayerData* pstData, uint8_t bType, uint64_t ullUinReceiver)
{
    if (NULL == pstData)
    {
        LOGERR("pstData is NULL");
    }
    switch (bType)
    {
    case FRIEND_HANDLE_TYPE_AGREE:
        _AgreeFriend(pstData->m_pOwner->GetUin(), pstData->m_oFriendAgreeInfo, ullUinReceiver);
        break;
    case FRIEND_HANDLE_TYPE_DELETE:
        _DeleteFriend(pstData->m_pOwner->GetUin(), pstData->m_oFriendAgreeInfo, ullUinReceiver);
        break;
    default:
        break;
    }

    //好友流水日志
    ZoneLog::Instance().WriteFriendLog(pstData, bType, ullUinReceiver);
}

void Friend::_AgreeFriend(uint64_t ullUinSender, DT_FRIEND_AGREE_INFO& rstAgreeInfo, uint64_t ullUinReceiver)
{
    size_t nmemb = (size_t)rstAgreeInfo.m_wCount;
    if (nmemb >= MAX_FRIEND_AGREE_LIST_CNT)
    {
        LOGERR("Add AgreeInfo err! List is full. rstAgreeInfo.m_wCount<%d>", rstAgreeInfo.m_wCount);
        return;
    }
    if (!MyBInsert(&ullUinReceiver, rstAgreeInfo.m_List, &nmemb, sizeof(uint64_t), 1, CompareUin))
    {
        LOGERR("Add AgreeInfo err!");
        return;
    }
    rstAgreeInfo.m_wCount = (uint16_t)nmemb;

    //把ullUinSender加入到对方的好友列表中
    Player* poReceiverPlayer = PlayerMgr::Instance().GetPlayerByUin(ullUinReceiver);
    if (NULL == poReceiverPlayer)
    {//没在线,不处理
        return;
    }
    DT_FRIEND_AGREE_INFO& rstReceierAgreeInfo = poReceiverPlayer->GetPlayerData().m_oFriendAgreeInfo;
    nmemb = (size_t)rstReceierAgreeInfo.m_wCount;
    if (nmemb >= MAX_FRIEND_AGREE_LIST_CNT)
    {
        LOGERR("Add AgreeInfo err! List is full. rstReceierAgreeInfo.m_wCount<%d>", rstReceierAgreeInfo.m_wCount);
        return;
    }
    if (!MyBInsert(&ullUinSender, rstReceierAgreeInfo.m_List, &nmemb, sizeof(uint64_t), 1, CompareUin))
    {
        LOGERR("Add ReceierAgreeInfo  err!");
        return;
    }
    rstReceierAgreeInfo.m_wCount = (uint16_t)nmemb;
}

void Friend::_DeleteFriend(uint64_t ullUinSender, DT_FRIEND_AGREE_INFO& rstAgreeInfo, uint64_t ullUinReceiver)
{
    size_t nmemb = (size_t)rstAgreeInfo.m_wCount;
    if (!MyBDelete(&ullUinReceiver, rstAgreeInfo.m_List, &nmemb, sizeof(uint64_t), CompareUin))
    {
        LOGERR("Delete rstAgreeInfo  err!");
    }
    rstAgreeInfo.m_wCount = (uint16_t)nmemb;

    //把ullUinSender从对方的好友列表中删除
    Player* poReceiverPlayer = PlayerMgr::Instance().GetPlayerByUin(ullUinReceiver);
    if (NULL == poReceiverPlayer)
    {//没在线,不处理
        return;
    }
    DT_FRIEND_AGREE_INFO& rstReceierAgreeInfo = poReceiverPlayer->GetPlayerData().m_oFriendAgreeInfo;
    nmemb = (size_t)rstReceierAgreeInfo.m_wCount;
    if (!MyBDelete(&ullUinSender, rstReceierAgreeInfo.m_List, &nmemb, sizeof(uint64_t),  CompareUin))
    {
        LOGERR("Delete ReceierAgreeInfo  err!");
        return;
    }
    rstReceierAgreeInfo.m_wCount = (uint16_t)nmemb;
}

bool Friend::IsFriend(PlayerData* pstData, uint64_t ullMateUin)
{
    if (NULL == pstData)
    {
        LOGERR("pstData is null");
        return false;
    }
    DT_FRIEND_AGREE_INFO& rstAgreeInfo = pstData->m_oFriendAgreeInfo;
    int iEqual = 0;
    MyBSearch(&ullMateUin, rstAgreeInfo.m_List, rstAgreeInfo.m_wCount, sizeof(uint64_t), &iEqual, CompareUin);
    return iEqual;
}

//  服务器更新
void Friend::UpdateServer()
{
	// 每日活动副本更新
	uint64_t ullUpdateTime = 0;
	if (CGameTime::Instance().IsNeedUpdateByHour(m_ullDayUpdateLastTime, m_iUptTime, ullUpdateTime))
	{
		m_ullDayUpdateLastTime = ullUpdateTime;
	}
}

bool Friend::Init()
{
	//初始化 更新时间 相关
	RESBASIC* pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(COMMON_UPDATE_TIME);
	if (!pResBasic)
	{
		LOGERR("Mall init error");
		return false;
	}
	m_iUptTime = (int)(pResBasic->m_para[0]);
	m_ullDayUpdateLastTime = CGameTime::Instance().GetSecOfHourInCurrDay(m_iUptTime) * 1000;
	if (CGameTime::Instance().GetCurrHour() < m_iUptTime)
	{
		m_ullDayUpdateLastTime -= MSSECONDS_OF_DAY;
	}

	return true;
}

void Friend::IsUpdatePlayerData(PlayerData* pstData, OUT uint64_t& ullTimestamp)
{
	DT_ROLE_MISC_INFO& rstMiscInfo = pstData->GetMiscInfo();
	if ( rstMiscInfo.m_ullFriendApTimestamp < m_ullDayUpdateLastTime)
	{
		ullTimestamp = m_ullDayUpdateLastTime;
		//每日通过好友赠送获取的体力数目清零
	}
	else
	{
		ullTimestamp = 0;
	}
}

