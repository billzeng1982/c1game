#include "Marquee.h"
#include "sys/GameTime.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "player/PlayerMgr.h"

#include "ov_res_keywords.h"
#include "ov_res_public.h"
#include "common_proto.h"

using namespace PKGMETA;

static int MarqueeIdCmp(const void *pstFirst, const void *pstSecond)
{
    int iRet = ((DT_GM_MARQUEE_ON_TIME_ADD*)pstFirst)->m_ullMarqueeMsgId - ((DT_GM_MARQUEE_ON_TIME_ADD*)pstSecond)->m_ullMarqueeMsgId;
    return iRet;
}

Marquee::Marquee()
{

}

Marquee::~Marquee()
{

}

bool Marquee::Init()
{
	m_iHead = 0;
	m_iTail = 0;
	m_ullSendInterval = (int)CGameDataMgr::Instance().GetResBasicMgr().Find((int)SEND_MARQUEE_MSG_INTERVAL)->m_para[0]; //消息发送间隔读表
    m_stMarqueeOnTimeInfo.m_iCount = 0;

	bzero(m_stMarqueeMsgList, sizeof(DT_MARQUEE_INFO) * MAX_MSG_LIST_LEN);

	return true;
}

void Marquee::SaveMarqueeForGCard(PlayerData* pstData, uint8_t bType, uint32_t dwId, uint32_t dwStar)
{
    ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
    RESGENERAL* pstResGeneral = rstResGeneralMgr.Find(dwId);
    if (!pstResGeneral)
    {
        return;
    }
    SaveMarqueeForGCard(pstData, bType, pstResGeneral, dwStar);

}

void Marquee::SaveMarqueeForGCard(PlayerData * pstData, uint8_t bType, RESGENERAL * pResGCard, uint32_t dwStar)
{
    assert(pResGCard);
    if (pResGCard->m_bMarquee == 1)
    {
        if (bType == GCARD_GAIN)
        {
            //获取武将
            //神将
            AddLocalMsg(pstData->GetRoleBaseInfo().m_szRoleName, GAIN_GOD_GENERAL, pResGCard->m_dwId, dwStar);
        }
        else
        {   //培养武将
            //神将
            AddLocalMsg(pstData->GetRoleBaseInfo().m_szRoleName, PINK_GENERAL_OPT, pResGCard->m_dwId, dwStar);
        }
        
    }
}

void Marquee::UpdateServer()
{
	uint64_t ullDiff = (uint64_t)CGameTime::Instance().GetCurrSecond() - m_ullLastSendTime;

	if (ullDiff >= m_ullSendInterval)
	{
		m_ullLastSendTime = (uint64_t)CGameTime::Instance().GetCurrSecond();
		//send to client
		if (m_iTail != m_iHead)
		{
			_SendMsgToClient(m_iHead, m_iTail);
		}
	}

    _UpdateMarqueeOnTime();
}

void Marquee::AddGMMsg(DT_MARQUEE_INFO& stMarqueeInfo)
{
	m_stMarqueeMsgList[m_iTail++] = stMarqueeInfo;

	if (_Check())
	{
		_SendMsgToClient(m_iHead, m_iTail);
	}

	if (m_iTail == MAX_MSG_LIST_LEN)
	{
		m_iTail = 0;
	}
}

void Marquee::AddLocalMsg(const char* pszName, uint8_t bType, uint32_t dwParam, uint32_t dwParam2, const char* pszContent)
{
    if(GAIN_GOD_GENERAL == bType || GAIN_FAMOUS_GENERAL == bType)
    {

        if(_IsScreenGeneral(dwParam))
            return;
    }

	DT_MARQUEE_INFO& rstMarqueeInfo = m_stMarqueeMsgList[m_iTail++];
	rstMarqueeInfo.m_bType = bType;
	switch (rstMarqueeInfo.m_bType)
	{
	case PINK_GENERAL_OPT:
		StrCpy(rstMarqueeInfo.m_stMarqueeData.m_stPinkGeneralData.m_szName, pszName, PKGMETA::MAX_NAME_LENGTH);
		rstMarqueeInfo.m_stMarqueeData.m_stPinkGeneralData.m_dwParam1 = dwParam;
        rstMarqueeInfo.m_stMarqueeData.m_stGodGeneralData.m_dwParam2 = dwParam2;
		break;
	case ORANGE_GENERAL_OPT:
		StrCpy(rstMarqueeInfo.m_stMarqueeData.m_stOrangeGeneralData.m_szName, pszName, PKGMETA::MAX_NAME_LENGTH);
		rstMarqueeInfo.m_stMarqueeData.m_stOrangeGeneralData.m_dwParam1 = dwParam;
        rstMarqueeInfo.m_stMarqueeData.m_stGodGeneralData.m_dwParam2 = dwParam2;
		break;
	case GAIN_GOD_GENERAL:
		StrCpy(rstMarqueeInfo.m_stMarqueeData.m_stGodGeneralData.m_szName, pszName, PKGMETA::MAX_NAME_LENGTH);
		rstMarqueeInfo.m_stMarqueeData.m_stGodGeneralData.m_dwParam1 = dwParam;
        rstMarqueeInfo.m_stMarqueeData.m_stGodGeneralData.m_dwParam2 = dwParam2;
		break;
	case GAIN_FAMOUS_GENERAL:
		StrCpy(rstMarqueeInfo.m_stMarqueeData.m_stFamousGeneralData.m_szName, pszName, PKGMETA::MAX_NAME_LENGTH);
		rstMarqueeInfo.m_stMarqueeData.m_stFamousGeneralData.m_dwParam1 = dwParam;
        rstMarqueeInfo.m_stMarqueeData.m_stGodGeneralData.m_dwParam2 = dwParam2;
		break;
	case PVP_RANK:
		StrCpy(rstMarqueeInfo.m_stMarqueeData.m_stPvpRankData.m_szName, pszName, PKGMETA::MAX_NAME_LENGTH);
		rstMarqueeInfo.m_stMarqueeData.m_stPvpRankData.m_dwParam1 = dwParam;
		break;
	case GUILD_FIGHT_WIN:
		StrCpy(rstMarqueeInfo.m_stMarqueeData.m_stGuildFightData.m_szName, pszName, PKGMETA::MAX_NAME_LENGTH);
		rstMarqueeInfo.m_stMarqueeData.m_stGuildFightData.m_dwParam1 = dwParam;
		break;
	case NINE_LEVEL_GEM:
		StrCpy(rstMarqueeInfo.m_stMarqueeData.m_stNineLevelGemData.m_szName, pszName, PKGMETA::MAX_NAME_LENGTH);
		rstMarqueeInfo.m_stMarqueeData.m_stNineLevelGemData.m_dwParam1 = dwParam;
		break;
	case HORN_SPEAKER:
		StrCpy(rstMarqueeInfo.m_stMarqueeData.m_stHornSpeaker.m_szName, pszName, PKGMETA::MAX_NAME_LENGTH);
		StrCpy(rstMarqueeInfo.m_stMarqueeData.m_stHornSpeaker.m_szContent, pszContent, PKGMETA::MAX_MESSAGE_RECORD_LEN);
		break;
	default:
		break;
	}

	if (_Check())
	{
		_SendMsgToClient(m_iHead, m_iTail);
	}

	if (m_iTail == MAX_MSG_LIST_LEN)
	{
		m_iTail = 0;
	}
}

void Marquee::_SendMsgToClient(int iStart, int iEnd)
{
	m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MARQUEE_NTF;
	SC_PKG_MARQUEE_NTF& stScMarqueeNtf = m_stScPkg.m_stBody.m_stMarqueeNtf;

	if (iStart == iEnd)
	{
		//no msg to send
		return;
	}
	else if (iStart < iEnd)
	{
		stScMarqueeNtf.m_bCount = iEnd - iStart;
		for (int i = 0; i < stScMarqueeNtf.m_bCount; i++)
		{
			stScMarqueeNtf.m_astMarqueeInfo[i] = m_stMarqueeMsgList[iStart + i];
		}
	}
	else
	{
		stScMarqueeNtf.m_bCount = MAX_MSG_LIST_LEN - iStart + iEnd;
		for (int i = 0; i < stScMarqueeNtf.m_bCount; i++)
		{
			stScMarqueeNtf.m_astMarqueeInfo[i] = (i + iStart) < MAX_MSG_LIST_LEN ? m_stMarqueeMsgList[i + iStart] : m_stMarqueeMsgList[i + iStart - MAX_MSG_LIST_LEN];
		}
	}

	ZoneSvrMsgLayer::Instance().BroadcastToClient(&m_stScPkg);

	m_iHead = iEnd;
}

bool Marquee::_Check()
{
	int iLen = 0;

	if (m_iHead <= m_iTail)
	{
		iLen = m_iTail - m_iHead;
	}
	else
	{
		iLen = MAX_MSG_LIST_LEN - m_iHead + m_iTail;
	}

	if (iLen >= MAX_NUM_MARQUEE_INFO_PUR_NTF)
	{
		return true;
	}

	return false;
}

bool Marquee::_IsScreenGeneral(uint32_t dwId)
{
    ResMarqueeScreenMgr_t& rstMarqueeScreenMgr = CGameDataMgr::Instance().GetResMarqueeScreenMgr();
    int iColNum = rstMarqueeScreenMgr.GetResNum();
    for(int i = 0; i < iColNum; i++)
    {
          RESMARQUEESCREEN* pstMarqueeScreen = rstMarqueeScreenMgr.GetResByPos(i);
          if(pstMarqueeScreen)
          {
                if(dwId == pstMarqueeScreen->m_dwGeneralId)
                {

                    //is screened
                    return true;
                }
          }
    }
    return false;
}


void Marquee::SaveMarqueeForHorn(PlayerData* pstData, const char* pszContent)
{
	AddLocalMsg(pstData->GetRoleBaseInfo().m_szRoleName, HORN_SPEAKER, 0, 0, pszContent);
}

int Marquee::AddMarqueeOnTime(DT_GM_MARQUEE_ON_TIME_ADD& rstMarqueeOnTimeAdd)
{
    int iRet = ERR_NONE;
    do 
    {
        if (m_stMarqueeOnTimeInfo.m_iCount >= MAX_GM_MARQUEE_ON_TIME_NUM)
        {
            LOGERR("the m_stMarqueeOnTimeInfo.m_iCount<%d> already reached the max.", m_stMarqueeOnTimeInfo.m_iCount);
            iRet = ERR_SYS;
            break;
        }

        if(!_CheckMarqueeOnTime(rstMarqueeOnTimeAdd))
        {
            LOGERR("the marquee<%lu> is not legal.", rstMarqueeOnTimeAdd.m_ullMarqueeMsgId);
            iRet = ERR_WRONG_PARA;
            break;
        }

        rstMarqueeOnTimeAdd.m_ullMarqueeMsgId = CGameTime::Instance().GetCurrTimeMs();
        size_t nmenb = m_stMarqueeOnTimeInfo.m_iCount;
        MyBInsert(&rstMarqueeOnTimeAdd, m_stMarqueeOnTimeInfo.m_astData, &nmenb, sizeof(DT_GM_MARQUEE_ON_TIME_ADD), 1, MarqueeIdCmp);
        m_stMarqueeOnTimeInfo.m_iCount = (int)nmenb;
    } while (false);
    
    return iRet;
}

int Marquee::DelMarqueeOnTime(uint64_t ullMarqueeMsgId)
{
    int iRet = ERR_NONE;
    do 
    {
        size_t nmenb = m_stMarqueeOnTimeInfo.m_iCount;
        m_stOneMarqueeOnTime.m_ullMarqueeMsgId = ullMarqueeMsgId;
        if ( !MyBDelete(&m_stOneMarqueeOnTime, m_stMarqueeOnTimeInfo.m_astData, &nmenb, sizeof(DT_GM_MARQUEE_ON_TIME_ADD), MarqueeIdCmp) )
        {
            LOGRUN("Delete m_ullMarqueeMsgId<%lu> fail. m_stMarqueeOnTimeInfo.m_iCount<%d>", m_stOneMarqueeOnTime.m_ullMarqueeMsgId, m_stMarqueeOnTimeInfo.m_iCount);
            iRet = ERR_SYS;
            break;
        }
        
        m_stMarqueeOnTimeInfo.m_iCount = (int)nmenb;
    } while (false);
    
    return iRet;
}

void Marquee::_UpdateMarqueeOnTime()
{
    uint64_t ullCurrTime = CGameTime::Instance().GetCurrSecond();

    for (int i = 0; i < m_stMarqueeOnTimeInfo.m_iCount; i++)
    {
        DT_GM_MARQUEE_ON_TIME_ADD& rstMarqueeAdd = m_stMarqueeOnTimeInfo.m_astData[i];

        //是否需要加入播放列表
        if ( ullCurrTime >= rstMarqueeAdd.m_ullStartTimestamp
             && ullCurrTime <= rstMarqueeAdd.m_ullEndTimestamp
             && (ullCurrTime - rstMarqueeAdd.m_ullLastRunTimestamp)>=rstMarqueeAdd.m_ullTimeInterval)
        {
            AddGMMsg(rstMarqueeAdd.m_stMarqueeInfo);
            rstMarqueeAdd.m_ullLastRunTimestamp = ullCurrTime;
        }
        
        //移除失效的跑马灯
        if (!_CheckMarqueeOnTime(rstMarqueeAdd))
        {
            LOGRUN("the marquee<%lu> is out of time. endtime<%lu>. currenttime<%lu>", rstMarqueeAdd.m_ullMarqueeMsgId, rstMarqueeAdd.m_ullEndTimestamp, ullCurrTime);
            if(DelMarqueeOnTime(rstMarqueeAdd.m_ullMarqueeMsgId) != ERR_NONE)
            {
                LOGERR("Delete m_ullMarqueeMsgId<%lu> fail. m_stMarqueeOnTimeInfo.m_iCount<%d>", m_stOneMarqueeOnTime.m_ullMarqueeMsgId, m_stMarqueeOnTimeInfo.m_iCount);
            }
        }
    }
}

#define MIN_INTERVAL_TIME (20)
bool Marquee::_CheckMarqueeOnTime(DT_GM_MARQUEE_ON_TIME_ADD& rstMarqueeAdd)
{
    uint64_t ullCurrTime = CGameTime::Instance().GetCurrSecond();

    if (rstMarqueeAdd.m_ullEndTimestamp < rstMarqueeAdd.m_ullStartTimestamp
        || rstMarqueeAdd.m_ullEndTimestamp <= ullCurrTime
        || rstMarqueeAdd.m_ullTimeInterval <= MIN_INTERVAL_TIME)
    {
        return false;
    }

    return true;    
}
