#include <algorithm>
#include "Friend.h"
#include "strutil.h"
#include "ss_proto.h"
#include "common_proto.h"
#include "LogMacros.h"

static int CompareUin(const void* pA, const void* pB)
{
	if (*((uint64_t*)pA) > *((uint64_t*)pB))
	{
		return 1;
	}
	else if (*((uint64_t*)pA) == *((uint64_t*)pB))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

void Friend::Reset()
{
    m_ullUin = 0;
    bzero(m_szName, MAX_NAME_LENGTH);
    bzero(&m_stAgreeInfo, sizeof(DT_FRIEND_AGREE_INFO));
    bzero(&m_stApplyInfo, sizeof(DT_FRIEND_APPLY_INFO));
	bzero(&m_stSendInfo, sizeof(DT_FRIEND_SEND_AP_INFO));
	bzero(&m_stRecvInfo, sizeof(DT_FRIEND_RECV_AP_INFO));
    bzero(&m_stPlayerInfo, sizeof(m_stPlayerInfo));
    bzero(&m_stWholeDataFront, sizeof(m_stWholeDataFront));
    INIT_LIST_NODE(&m_stTimeListNode);
    INIT_LIST_NODE(&m_stDirtyListNode);
}


bool Friend::InitFromDB(DT_FRIEND_WHOLE_DATA &rstFriendWholedata)
{
    m_ullUin = rstFriendWholedata.m_stBaseInfo.m_ullUin;
    StrCpy(m_szName, rstFriendWholedata.m_stBaseInfo.m_szName, MAX_NAME_LENGTH);
    uint32_t dwVersion = rstFriendWholedata.m_stBaseInfo.m_wVersion;
    size_t ulUseSize = 0;
    //解压 好友包
    int iRet = m_stAgreeInfo.unpack((char*)rstFriendWholedata.m_stAgreeBlob.m_szData, rstFriendWholedata.m_stAgreeBlob.m_iLen, &ulUseSize, dwVersion);
    if (TdrError::TDR_NO_ERROR != iRet)
    {
        LOGERR_r("Uin<%lu> unpack m_stAgreeBlob failed, Ret=%d", m_ullUin, iRet);
        return false;
    }
	if (m_stAgreeInfo.m_wCount > 0)
	{
		sort(m_stAgreeInfo.m_List, m_stAgreeInfo.m_List + m_stAgreeInfo.m_wCount);
	}
	

    //解压 申请包
    iRet = m_stApplyInfo.unpack((char*)rstFriendWholedata.m_stApplyBlob.m_szData, rstFriendWholedata.m_stApplyBlob.m_iLen, &ulUseSize, dwVersion);
    if (TdrError::TDR_NO_ERROR != iRet)
    {
        LOGERR_r("Uin<%lu> unpack m_stApplyBlob failed, Ret=%d", m_ullUin, iRet);
        return false;
    }
	if (m_stApplyInfo.m_wCount > 0)
	{
		sort(m_stApplyInfo.m_List, m_stApplyInfo.m_List + m_stApplyInfo.m_wCount);
	}
	//解压 已赠送体力包
	iRet = m_stSendInfo.unpack((char*)rstFriendWholedata.m_stSendBlob.m_szData, rstFriendWholedata.m_stSendBlob.m_iLen, &ulUseSize, dwVersion);
	if (TdrError::TDR_NO_ERROR != iRet)
	{
		LOGERR_r("Uin<%lu> unpack m_stSendBlob failed, Ret=%d", m_ullUin, iRet);
		return false;
	}

	if (m_stSendInfo.m_wCount > 0)
	{
		sort(m_stSendInfo.m_List, m_stSendInfo.m_List + m_stSendInfo.m_wCount);
	}

	//解压 未领取的被赠送体力包
	iRet = m_stRecvInfo.unpack((char*)rstFriendWholedata.m_stRecvBlob.m_szData, rstFriendWholedata.m_stRecvBlob.m_iLen, &ulUseSize, dwVersion);
	if (TdrError::TDR_NO_ERROR != iRet)
	{
		LOGERR_r("Uin<%lu> unpack m_stRecvBlob failed, Ret=%d", m_ullUin, iRet);
		return false;
	}

	if (m_stRecvInfo.m_wCount > 0)
	{
		sort(m_stRecvInfo.m_List, m_stRecvInfo.m_List + m_stRecvInfo.m_wCount);
	}

    return true;
}


bool Friend::IsInApplyList(uint64_t ullUin)
{
    int i = 0 ;
    for (; i < m_stApplyInfo.m_wCount; i++)
    {
        if (ullUin == m_stApplyInfo.m_List[i])
        {
            return true;
        }
    }
    return false;
}

bool Friend::IsInAgreeList(uint64_t ullUin)
{
    int iEqual = 0;
    MyBSearch(&ullUin, m_stAgreeInfo.m_List, m_stAgreeInfo.m_wCount, sizeof(uint64_t), &iEqual, CompareUin);
    return iEqual;
}

int Friend::Add2ApplyList(uint64_t ullUin)
{
    if (IsInApplyList(ullUin))
    {//在申请列表里, 这里策划说允许, 不添加,返回正常
        return ERR_NONE;
    }
    if (IsInAgreeList(ullUin))
    {//在好友列表里
        return ERR_NONE;
    }
    if (m_stApplyInfo.m_wCount >= MAX_FRIEND_APPLY_LIST_CNT)
    {//申请列表已满
        return ERR_FRIEND_APPLY_FULL;
    }
    m_stApplyInfo.m_List[m_stApplyInfo.m_wCount++] = ullUin;
    return ERR_NONE;
}

int Friend::Add2AgreeList(uint64_t ullUin)
{
    if (IsInAgreeList(ullUin))
    {//已是好友
        return ERR_FRIEND_ALREADY_AGREE;
    }
    if (m_stAgreeInfo.m_wCount >= MAX_FRIEND_AGREE_LIST_CNT)
    {//好友列表已满
        return ERR_FRIEND_AGREE_FULL;
    }

    size_t nmemb = (size_t)m_stAgreeInfo.m_wCount;
    if (!MyBInsert(&ullUin, m_stAgreeInfo.m_List, &nmemb, sizeof(uint64_t), 1, CompareUin))
    {
        LOGERR_r("Uin<%lu> Add <%lu> to AgreeList  err!", m_ullUin, ullUin);
        return ERR_SYS;
    }
    m_stAgreeInfo.m_wCount = (uint16_t)nmemb;
    return ERR_NONE;
}

void Friend::DeleteApplyList(uint64_t ullUin)
{
    if (m_stApplyInfo.m_wCount < 1)
    {
        return;
    }
    bool bDel = false;
    m_stApplyInfo.m_wCount--;
    for (int i = 0; i < m_stApplyInfo.m_wCount; i++)
    {
        if (bDel==false && m_stApplyInfo.m_List[i] == ullUin)
        {
            m_stApplyInfo.m_List[i] = m_stApplyInfo.m_List[i+1];
            bDel = true;
            continue;
        }
        if (bDel)
        {
            m_stApplyInfo.m_List[i] = m_stApplyInfo.m_List[i+1];
        }
    }
    m_stApplyInfo.m_List[m_stApplyInfo.m_wCount] = 0;  
}

void Friend::DeleteAgreeList(uint64_t ullUin)
{
    size_t nmemb = (size_t)m_stAgreeInfo.m_wCount;
    if (!MyBDelete(&ullUin, m_stAgreeInfo.m_List, &nmemb, sizeof(uint64_t), CompareUin))
    {
        LOGERR_r("Uin<%lu> Delete <%lu> rstAgreeInfo  err!", m_ullUin, ullUin);
        return;
    }
    m_stAgreeInfo.m_wCount = (uint16_t)nmemb;
}

void Friend::UpdatePlayerInfo(DT_FRIEND_PLAYER_INFO& rstPlayerInfo)
{
    memcpy(&m_stPlayerInfo, &rstPlayerInfo, sizeof(DT_FRIEND_PLAYER_INFO));
}

void Friend::InitFriend(DT_FRIEND_PLAYER_INFO& rstPlayerInfo)
{
    this->m_ullUin = rstPlayerInfo.m_ullUin;
    StrCpy(this->m_szName, rstPlayerInfo.m_szName, MAX_NAME_LENGTH);
    UpdatePlayerInfo(rstPlayerInfo);
}



int Friend::PackWholeData(OUT DT_FRIEND_WHOLE_DATA& rstWholeData)
{
    size_t ulUseSize = 0;
    int iRet = 0;
    //BaseInfo
    rstWholeData.m_stBaseInfo.m_ullUin = m_ullUin;
    StrCpy(rstWholeData.m_stBaseInfo.m_szName, m_szName, MAX_NAME_LENGTH);
    
    //申请列表 整理
    iRet = m_stApplyInfo.pack((char*)rstWholeData.m_stApplyBlob.m_szData, MAX_LEN_FRIEND_APPLY_BLOB, &ulUseSize);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Uin<%lu> pack m_stApplyInfo failed, Ret=%d", m_ullUin , iRet);
        return ERR_SYS;
    }
    rstWholeData.m_stApplyBlob.m_iLen = (int)ulUseSize;

    //好友列表  整理
    iRet = m_stAgreeInfo.pack((char*)rstWholeData.m_stAgreeBlob.m_szData, MAX_LEN_FRIEND_AGREE_BLOB, &ulUseSize);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Uin<%lu> pack m_stFriendAgreeFrontInfo failed, Ret=%d", m_ullUin, iRet);
        return ERR_SYS;
    }
    rstWholeData.m_stAgreeBlob.m_iLen = (int)ulUseSize;

	//已送过体力的好友列表 整理
	iRet = m_stSendInfo.pack((char*)rstWholeData.m_stSendBlob.m_szData, MAX_LEN_FRIEND_SEND_BLOB, &ulUseSize);
	if( iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR_r("Uin<%lu> pack m_stSendInfo failed, Ret=%d", m_ullUin, iRet);
		return ERR_SYS;
	}
	rstWholeData.m_stSendBlob.m_iLen = (int)ulUseSize;

	//该列表中的好友赠送的体力尚未被该玩家收取 整理
	iRet = m_stRecvInfo.pack((char*)rstWholeData.m_stRecvBlob.m_szData, MAX_LEN_FRIEND_RECV_BLOB, &ulUseSize);
	if( iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR_r("Uin<%lu> pack m_stRecvInfo failed, Ret=%d", m_ullUin, iRet);
		return ERR_SYS;
	}
	rstWholeData.m_stRecvBlob.m_iLen = (int)ulUseSize;

    return ERR_NONE;
}

bool Friend::IsInSendApList(uint64_t ullUin)
{
	int iEqual = 0;
	MyBSearch(&ullUin, m_stSendInfo.m_List, m_stSendInfo.m_wCount, sizeof(uint64_t), &iEqual, CompareUin);
	return iEqual;
}

bool Friend::IsInRecvApList(uint64_t ullUin)
{
	int iEqual = 0;
	MyBSearch(&ullUin, m_stRecvInfo.m_List, m_stRecvInfo.m_wCount, sizeof(uint64_t), &iEqual, CompareUin);
	return iEqual;
}

int Friend::Add2SendApList(uint64_t ullUin)
{
	if (IsInSendApList(ullUin))
	{//已存在
		return ERR_DEFAULT;
	}
	if (m_stSendInfo.m_wCount >= MAX_FRIEND_AGREE_LIST_CNT)
	{//列表已满
		return ERR_DEFAULT;
	}

	size_t nmemb = (size_t)m_stSendInfo.m_wCount;
	if (!MyBInsert(&ullUin, m_stSendInfo.m_List, &nmemb, sizeof(uint64_t), 1, CompareUin))
	{
		LOGERR_r("Uin<%lu>  Add <%lu> to SendApList  err!", m_ullUin, ullUin);
		return ERR_SYS;
	}
	m_stSendInfo.m_wCount = (uint16_t)nmemb;
	return ERR_NONE;
}

int Friend::Add2RecvApList(uint64_t ullUin)
{
	if (IsInRecvApList(ullUin))
	{//已存在
		return ERR_DEFAULT;
	}
	if (m_stRecvInfo.m_wCount >= MAX_FRIEND_AGREE_LIST_CNT)
	{//列表已满
		return ERR_DEFAULT;
	}

	size_t nmemb = (size_t)m_stRecvInfo.m_wCount;
	if (!MyBInsert(&ullUin, m_stRecvInfo.m_List, &nmemb, sizeof(uint64_t), 1, CompareUin))
	{
		LOGERR_r("Uin<%lu>  Add <%lu> to RecvApList  err!", m_ullUin, ullUin);
		return ERR_SYS;
	}
	m_stRecvInfo.m_wCount = (uint16_t)nmemb;
	return ERR_NONE;
}

void Friend::DeleteSendApList(uint64_t ullUin)
{
	size_t nmemb = (size_t)m_stSendInfo.m_wCount;
	if (!MyBDelete(&ullUin, m_stSendInfo.m_List, &nmemb, sizeof(uint64_t), CompareUin))
	{
		LOGERR_r("Uin<%lu>  Delete <%lu> from rstSendInfo  err!", m_ullUin, ullUin);
		return;
	}
	m_stSendInfo.m_wCount = (uint16_t)nmemb;
}

void Friend::DeleteRecvApList(uint64_t ullUin)
{
	size_t nmemb = (size_t)m_stRecvInfo.m_wCount;
	if (!MyBDelete(&ullUin, m_stRecvInfo.m_List, &nmemb, sizeof(uint64_t), CompareUin))
	{
		LOGERR_r("Uin<%lu>  Delete <%lu> from rstRecvInfo  err!", m_ullUin, ullUin);
		return;
	}
	m_stRecvInfo.m_wCount = (uint16_t)nmemb;
}

void Friend::ClearSendApList()
{
	m_stSendInfo.m_wCount = 0;
	return;
}


