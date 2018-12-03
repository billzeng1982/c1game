#include "GuildMember.h"
#include "LogMacros.h"
#include "GuildDataDynPool.h"
#include "../gamedata/GameDataMgr.h"
#include "ss_proto.h"
#include "GuildSvrMsgLayer.h"
#include "strutil.h"
#include "GuildMgr.h"

using namespace PKGMETA;

GuildMember::GuildMember()
{
    m_iCount = 0;
    m_iMaxNum = INIT_NUM_MEMBER;
}

bool GuildMember::InitFromDB(DT_GUILD_MEMBER_BLOB & rstBlob, uint64_t ullGuildId, int iMaxNum, int* pDeputyNum, int* pEliteNum, uint16_t wVersion)
{
    DT_GUILD_MEMBER_INFO stGuildMemberInfo;
    size_t ulUseSize = 0;
    int iRet = stGuildMemberInfo.unpack((char*)rstBlob.m_szData, sizeof(rstBlob.m_szData), &ulUseSize, wVersion);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Guild(%lu) init Member failed, unpack DT_GUILD_MEMBER_BLOB failed, Ret=%d", ullGuildId, iRet);
        return false;
    }

    m_iCount = 0;
    m_iMaxNum = iMaxNum;
    for (int i=0; i<stGuildMemberInfo.m_wCount; i++)
    {
        //初始化所有玩家为不在线，玩家通过心跳刷新在线状态
        stGuildMemberInfo.m_astMemberList[i].m_stExtraInfo.m_bIsOnline = 0;
        iRet = this->Add(stGuildMemberInfo.m_astMemberList[i]);
        if (iRet != ERR_NONE)
        {
            LOGERR_r("Guild(%lu) init Member failed, add member(%lu) failed, Ret=%d", ullGuildId, stGuildMemberInfo.m_astMemberList[i].m_ullUin, iRet);
            return false;
        }

        if (stGuildMemberInfo.m_astMemberList[i].m_bIdentity == GUILD_IDENTITY_DEPUTY_LEADER)
        {
            (*pDeputyNum)++;
        }
		if (stGuildMemberInfo.m_astMemberList[i].m_bIdentity == GUILD_IDENTITY_ELDERS)
		{
			(*pEliteNum)++;
		}
    }

    m_ullGuildId = ullGuildId;
    return true;
}


bool GuildMember::PackGuildMemberInfo(DT_GUILD_MEMBER_BLOB& rstBlob, uint16_t wVersion)
{
    DT_GUILD_MEMBER_INFO stGuildMemberInfo;
    stGuildMemberInfo.m_wCount = m_iCount;

    m_oUinToMemberMapIter = m_oUinToMemberMap.begin();
    for (int i=0; m_oUinToMemberMapIter!= m_oUinToMemberMap.end() && i<m_iCount; m_oUinToMemberMapIter++, i++)
    {
        memcpy(&stGuildMemberInfo.m_astMemberList[i], m_oUinToMemberMapIter->second, sizeof(DT_ONE_GUILD_MEMBER));
    }

    size_t ulUseSize = 0;
    int iRet = stGuildMemberInfo.pack((char*)rstBlob.m_szData, MAX_LEN_GUILD_MEMBER, &ulUseSize, wVersion);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Guild(%lu) pack DT_GUILD_MEMBER_INFO failed, iRet=%d, UsedSize=%lu", m_ullGuildId, iRet, ulUseSize);
        return false;
    }
    rstBlob.m_iLen = (int)ulUseSize;

    return true;
}


int GuildMember::Add(DT_ONE_GUILD_MEMBER& rstOneMember)
{
    if (m_iCount >= m_iMaxNum)
    {
        return ERR_MEMBER_FULL;
    }

    m_oUinToMemberMapIter = m_oUinToMemberMap.find(rstOneMember.m_ullUin);
    if (m_oUinToMemberMapIter != m_oUinToMemberMap.end())
    {
        return ERR_ALREADY_EXIST_MEMBER;
    }

    DT_ONE_GUILD_MEMBER* pstOneMember = GuildDataDynPool::Instance().GuildMemberPool().Get();
    if (pstOneMember == NULL)
    {
        LOGERR_r("Guild(%lu) get DT_ONE_GUILD_MEMBER from GuildDataDynPool failed", m_ullGuildId);
        return ERR_SYS;
    }
    memcpy(pstOneMember, &rstOneMember, sizeof(DT_ONE_GUILD_MEMBER));

    m_oUinToMemberMap.insert(map<uint64_t, DT_ONE_GUILD_MEMBER*>::value_type(rstOneMember.m_ullUin, pstOneMember));
    m_MemberList[m_iCount++] = rstOneMember.m_ullUin;

    return ERR_NONE;
}


int GuildMember::Del(uint64_t ullUin)
{
    if (m_iCount <= 0)
    {
        return ERR_NOT_FOUND_MEMBER;
    }

    m_oUinToMemberMapIter = m_oUinToMemberMap.find(ullUin);
    if (m_oUinToMemberMapIter == m_oUinToMemberMap.end())
    {
        return ERR_NOT_FOUND_MEMBER;
    }

    //会长不能删除
    DT_ONE_GUILD_MEMBER* pstOneMember = m_oUinToMemberMapIter->second;
    if (pstOneMember->m_bIdentity == GUILD_IDENTITY_LEADER)
    {
        return ERR_LEADER_CANT_QUIT;
    }

     int i = this->_FindMember(ullUin);
    //assert(i>=0);
    if( i < 0 )
    {
        LOGERR_r("_FindMember return i < 0, ullUin=%lu", ullUin);
        assert(false);
        return ERR_WRONG_PARA;
    }

    GuildDataDynPool::Instance().GuildMemberPool().Release(pstOneMember);
    m_oUinToMemberMap.erase(m_oUinToMemberMapIter);
    m_MemberList[i] = m_MemberList[--m_iCount];

    return ERR_NONE;
}


DT_ONE_GUILD_MEMBER* GuildMember::Find(uint64_t ullUin)
{
    m_oUinToMemberMapIter = m_oUinToMemberMap.find(ullUin);
    if (m_oUinToMemberMapIter == m_oUinToMemberMap.end())
    {
        return NULL;
    }
    else
    {
        return m_oUinToMemberMapIter->second;
    }
}


int GuildMember::_FindMember(uint64_t ullUin)
{
    for (int i=0; i<m_iCount; i++)
    {
        if (m_MemberList[i] == ullUin)
        {
            return i;
        }
    }

    return -1;
}


uint64_t* GuildMember::GetMemberList()
{
    return m_MemberList;
}


uint32_t GuildMember::GetStarSumNum()
{
    uint32_t dwSum = 0;

    m_oUinToMemberMapIter = m_oUinToMemberMap.begin();
    for (; m_oUinToMemberMapIter!= m_oUinToMemberMap.end(); m_oUinToMemberMapIter++)
    {
        DT_ONE_GUILD_MEMBER* pstOneMember = m_oUinToMemberMapIter->second;
        dwSum += pstOneMember->m_stExtraInfo.m_bStarNum;
    }

    return dwSum;
}

uint32_t GuildMember::GetLiSum()
{
    uint32_t dwSum = 0;

    m_oUinToMemberMapIter = m_oUinToMemberMap.begin();
    for (; m_oUinToMemberMapIter!= m_oUinToMemberMap.end(); m_oUinToMemberMapIter++)
    {
        DT_ONE_GUILD_MEMBER* pstOneMember = m_oUinToMemberMapIter->second;
        dwSum += pstOneMember->m_stExtraInfo.m_dwLi;
    }

    return dwSum;
}

void GuildMember::Clear()
{
    m_oUinToMemberMapIter = m_oUinToMemberMap.begin();
    for (; m_oUinToMemberMapIter!= m_oUinToMemberMap.end(); m_oUinToMemberMapIter++)
    {
        DT_ONE_GUILD_MEMBER* pstOneMember = m_oUinToMemberMapIter->second;
        GuildDataDynPool::Instance().GuildMemberPool().Release(pstOneMember);
    }

    m_oUinToMemberMap.clear();
    m_iCount = 0;
}

bool GuildMember::IsDeputy(uint64_t ullUin)
{
    DT_ONE_GUILD_MEMBER* poMemInfo = m_oUinToMemberMap[ullUin];
    if (poMemInfo && GUILD_IDENTITY_DEPUTY_LEADER == poMemInfo->m_bIdentity )
    {
        return true;
    }
    return false;
}

void GuildMember::ClearVitality()
{
    m_oUinToMemberMapIter = m_oUinToMemberMap.begin();
    for (; m_oUinToMemberMapIter!= m_oUinToMemberMap.end(); m_oUinToMemberMapIter++)
    {
        DT_ONE_GUILD_MEMBER* pstOneMember = m_oUinToMemberMapIter->second;
        pstOneMember->m_stExtraInfo.m_dwGuildVitality = 0;
    }
}

void GuildMember::SendVitalityReward()
{
    //根据活跃度排序
    DT_ONE_GUILD_MEMBER* pstOneMember, *temp;
    DT_ONE_GUILD_MEMBER* astMemList[MAX_NUM_MEMBER];
    m_oUinToMemberMapIter = m_oUinToMemberMap.begin();
    for (int i=0; m_oUinToMemberMapIter!= m_oUinToMemberMap.end(); m_oUinToMemberMapIter++, i++)
    {
        pstOneMember = m_oUinToMemberMapIter->second;
        astMemList[i] = pstOneMember;
        for (int j = i; j > 0; j--)
        {
            if (astMemList[j]->m_stExtraInfo.m_dwGuildVitality > astMemList[j-1]->m_stExtraInfo.m_dwGuildVitality)
            {
                temp = astMemList[j-1];
                astMemList[j-1] = astMemList[j];
                astMemList[j] = temp;
            }
            else
            {
                break;
            }
        }
    }

    //发送活跃度奖励
    RESPRIMAIL* poResPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(21);
    if (!poResPriMail)
    {
        LOGERR_r("Can't find the mail <21>");
        return;
    }

    SSPKG stSsPkg;
    stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
    SS_PKG_MAIL_ADD_REQ& rstMailAddReq = stSsPkg.m_stBody.m_stMailAddReq;
    rstMailAddReq.m_nUinCount = 0;
    DT_MAIL_DATA& rstMailData = rstMailAddReq.m_stMailData;
    rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
    rstMailData.m_bState = MAIL_STATE_UNOPENED;
    StrCpy(rstMailData.m_szTitle, poResPriMail->m_szTitle, MAX_MAIL_TITLE_LEN);
    StrCpy(rstMailData.m_szContent, poResPriMail->m_szContent, MAX_MAIL_CONTENT_LEN);
    rstMailData.m_ullFromUin = 0;
    rstMailData.m_ullTimeStampMs = CGameTime::Instance().GetCurrSecond();

    ResGuildVitalityRankRewardMgr_t& rstResRankMgr = CGameDataMgr::Instance().GetResGuildVitalityRankRewardMgr();
    int iResCount = rstResRankMgr.GetResNum();

    int iStartRank = 1;
    int iEndRank = m_iCount;

    for (int i = 0; i < iResCount; i++)
    {
        RESGUILDVITALITYRANKREWARD* pResReward = rstResRankMgr.GetResByPos(i);
        rstMailData.m_bAttachmentCount = pResReward->m_bCount;
        for (int k = 0; k < pResReward->m_bCount; k++)
        {
            rstMailData.m_astAttachmentList[k].m_bItemType = pResReward->m_szPropsType[k];
            rstMailData.m_astAttachmentList[k].m_dwItemId = pResReward->m_propsId[k];
            rstMailData.m_astAttachmentList[k].m_iValueChg = pResReward->m_propsNum[k];
        }

        for (int j = iStartRank; (j <= (int)pResReward->m_dwRank) && (j <= iEndRank); j++)
        {
            if (astMemList[j-1]->m_stExtraInfo.m_dwGuildVitality == 0)
            {
                break;
            }

            rstMailAddReq.m_UinList[rstMailAddReq.m_nUinCount++] = astMemList[j-1]->m_ullUin;
        }
        iStartRank = pResReward->m_dwRank + 1;

        if (rstMailAddReq.m_nUinCount >= 0)
        {
            GuildSvrMsgLayer::Instance().SendToMailSvr(stSsPkg);
            rstMailAddReq.m_nUinCount = 0;
        }
    }
}

DT_ONE_GUILD_MEMBER*  GuildMember::GetNextLeader()
{
    uint32_t dwVitality = 0;
    DT_ONE_GUILD_MEMBER* pstOneMember = NULL;
    for (m_oUinToMemberMapIter = m_oUinToMemberMap.begin(); m_oUinToMemberMapIter!= m_oUinToMemberMap.end(); m_oUinToMemberMapIter++)
    {
        DT_ONE_GUILD_MEMBER* pstTemp = m_oUinToMemberMapIter->second;
        if (pstTemp->m_stExtraInfo.m_dwGuildVitality > dwVitality)
        {
            dwVitality = pstTemp->m_stExtraInfo.m_dwGuildVitality;
            pstOneMember = pstTemp;
        }
    }

	return pstOneMember;
}


void GuildMember::GetGuildExpeditionMemberInfo(OUT int8_t& chCount, OUT DT_GUILD_EXPEDITION_ONE_MEMBER_INFO* pstInfoList)
{
	chCount = 0;
	m_oUinToMemberMapIter = m_oUinToMemberMap.begin();
	for (; chCount < MAX_NUM_MEMBER && m_oUinToMemberMapIter != m_oUinToMemberMap.end(); chCount++, m_oUinToMemberMapIter++)
	{
		pstInfoList[chCount].m_ullUin = m_oUinToMemberMapIter->second->m_ullUin;
		pstInfoList[chCount].m_bIsDefend = m_oUinToMemberMapIter->second->m_bIsSetExpeditionArray;
	}

}

void GuildMember::SettleExpeditionAward(uint8_t bAwardType)
{

	uint32_t dwMailId = bAwardType == 1 ? 10301 : 10302;
	RESPRIMAIL* poResPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(dwMailId);
	if (!poResPriMail)
	{
		LOGERR_r("GuildId<%lu> Can't find the mail <%u>", m_ullGuildId ,dwMailId);
		return;
	}
	int UpdateHour = GuildMgr::Instance().GetDailyUpdateHour();
	SSPKG stSsPkg;
	stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_BY_ID_REQ;
	SS_PKG_MAIL_ADD_BY_ID_REQ& rstMailAddReq = stSsPkg.m_stBody.m_stMailAddByIdReq;

	rstMailAddReq.m_nUinCount = 0;
	rstMailAddReq.m_dwMailResId = dwMailId;
	m_oUinToMemberMapIter = m_oUinToMemberMap.begin();
	for (; m_oUinToMemberMapIter != m_oUinToMemberMap.end(); m_oUinToMemberMapIter++)
	{
		//加入公会下一个刷新日才能得到奖励
		if (CGameTime::Instance().GetSecOfCycleHourInCurrday(UpdateHour)
			== CGameTime::Instance().GetSecOfCycleHourInSomeDay(m_oUinToMemberMapIter->second->m_ullJoinGuildTimeStap / 1000, UpdateHour))
		{
			continue;
		}
		rstMailAddReq.m_UinList[rstMailAddReq.m_nUinCount++] = m_oUinToMemberMapIter->second->m_ullUin;
		LOGWARN_r("Uin<%lu> get expedition award mail<%u> Guild<%lu>", m_oUinToMemberMapIter->second->m_ullUin, dwMailId, m_ullGuildId);
	}
	GuildSvrMsgLayer::Instance().SendToMailSvr(stSsPkg);
	LOGWARN_r("Guild<%lu> Expedition Award<%u> OK", m_ullGuildId, dwMailId);
}

int GuildMember::UpdateDefendInfo(DT_GUILD_PKG_EXPEDITION_UPDATE_DEFEND_INFO& rstDefendInfo)
{
	for (size_t i = 0; i < rstDefendInfo.m_bMemberCount; i++)
	{
		m_oUinToMemberMapIter = m_oUinToMemberMap.find(rstDefendInfo.m_MemberList[i]);
		if (m_oUinToMemberMapIter == m_oUinToMemberMap.end())
		{
			LOGERR_r("Guild<%lu> update defend info error, cant find the member<%lu>", m_ullGuildId, rstDefendInfo.m_MemberList[i]);
			continue;
		}
		m_oUinToMemberMapIter->second->m_bIsSetExpeditionArray = 1;
	}
	return ERR_NONE;
}
