#include "Tutorial.h"
#include "../gamedata/GameDataMgr.h"
#include "LogMacros.h"
#include "GeneralCard.h"
#include "Item.h"
#include "player/Player.h"
#include "ZoneLog.h"
#include "dwlog_svr.h"
#include "ZoneSvrMsgLayer.h"
#include "SvrTime.h"

using namespace std;
using namespace PKGMETA;
using namespace DWLOG;

Tutorial::Tutorial()
{

}

Tutorial::~Tutorial()
{

}

bool Tutorial::Init()
{
    ResTutorialBonusMgr_t& rstResTutorialBonusMgr = CGameDataMgr::Instance().GetResTutorialBonusMgr();
    int iSumCount = rstResTutorialBonusMgr.GetResNum();
    RESTUTORIALBONUS* pstResRecord = NULL;
    for (int i=0; i<iSumCount; i++)
    {
		pstResRecord = rstResTutorialBonusMgr.GetResByPos(i);
		std::list<uint32_t> generalList;
        int iCount = (int)pstResRecord->m_dwGeneralCount;
		for (int j= 0; j<iCount; j++)
		{
			generalList.push_back(pstResRecord->m_generalIdList[j]);
		}
		m_GeneralMap.insert(TutorialGeneralMap_t::value_type((uint32_t)i, generalList));
    }

    return true;
}

int Tutorial::DrawBonus(PlayerData* pstData, uint8_t& rbBonusReq, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
	DT_ROLE_MISC_INFO& rstInfo = pstData->GetMiscInfo();

	// 请求领奖不合理
	if (rbBonusReq != rstInfo.m_bTutorialBonusCnt)
	{
		rbBonusReq = rstInfo.m_bTutorialBonusCnt;
		return ERR_BONUS_ALREADY_GET;
	}

	// 超过领取次数
	if (rstInfo.m_bTutorialBonusCnt >= m_GeneralMap.size())
	{
		rbBonusReq = rstInfo.m_bTutorialBonusCnt;
		return ERR_BONUS_ALREADY_GET;
	}

	// 根据领取次数获取武将列表
	list<uint32_t> generalList = m_GeneralMap[rstInfo.m_bTutorialBonusCnt];

    for (list<uint32_t>::iterator iter = generalList.begin(); iter != generalList.end(); ++iter)
    {
        int iRet = Item::Instance().RewardItem(pstData, ITEM_TYPE_GCARD, *iter, 1, rstSyncItemInfo, METHOD_TUTORIAL_AWARD);
        if (iRet < 0)
        {
            LOGERR("tutorial bonus is err. id-%u", *iter);
            continue;
        }
    }

	// 增加领取次数
    rstInfo.m_bTutorialBonusCnt++;

	rbBonusReq = rstInfo.m_bTutorialBonusCnt;

    return ERR_NONE;
}

int Tutorial::RecordTutorialData(PlayerData* pstData, uint8_t bFlag, uint64_t ullDataValue)
{
    DT_ROLE_MISC_INFO& rstInfo = pstData->GetMiscInfo();

    if (ACTIVATE_PAY_FDAY_AWARD_ID == ullDataValue)
    {
        rstInfo.m_stPayInfo.m_ullFDayPayStartTime = CGameTime::Instance().GetCurrSecond();
    }

    if (0 == bFlag)
    {
        rstInfo.m_ullTutorialStep = ullDataValue;
    }
    else if (1 == bFlag)
    {
        rstInfo.m_ullTutorialTrigger = ullDataValue;
    }
    else if (2 == bFlag)
    {
        // 记录新手Step日志
        ZoneLog::Instance().WriteTutorialStepLog(pstData, bFlag, ullDataValue);
    }

    LOGRUN("syn tutorial, Uin-%lu, RoleName-%s, step-%lu",
        pstData->GetRoleBaseInfo().m_ullUin, pstData->GetRoleBaseInfo().m_szRoleName, rstInfo.m_ullTutorialStep);

    return ERR_NONE;
}

void Tutorial::_SendTutorialGift(PlayerData* pstData)
{
    RESPRIMAIL* poResPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(TUTORIAL_GIFT_MAIL_ID);
    assert(poResPriMail);

    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
    SS_PKG_MAIL_ADD_REQ& rstMailAddReq = m_stSsPkg.m_stBody.m_stMailAddReq;
    rstMailAddReq.m_nUinCount = 1;
    rstMailAddReq.m_UinList[0] = pstData->GetRoleBaseInfo().m_ullUin;
    DT_MAIL_DATA& rstMailData = rstMailAddReq.m_stMailData;
    rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
    rstMailData.m_bState = MAIL_STATE_UNOPENED;
    StrCpy(rstMailData.m_szTitle, poResPriMail->m_szTitle, MAX_MAIL_TITLE_LEN);
    StrCpy(rstMailData.m_szContent, poResPriMail->m_szContent, MAX_MAIL_CONTENT_LEN);
    rstMailData.m_ullFromUin = 0;
    rstMailData.m_ullTimeStampMs = CGameTime::Instance().GetCurrSecond();
    rstMailData.m_bAttachmentCount = poResPriMail->m_bAttachmentNum;
    for (int j = 0; j < rstMailData.m_bAttachmentCount; j++)
    {
        rstMailData.m_astAttachmentList[j].m_bItemType = poResPriMail->m_astAttachmentList[j].m_bType;
        rstMailData.m_astAttachmentList[j].m_dwItemId = poResPriMail->m_astAttachmentList[j].m_dwId;
        rstMailData.m_astAttachmentList[j].m_iValueChg = poResPriMail->m_astAttachmentList[j].m_dwNum;
    }

    ZoneSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);

    int iDaySinceSvrOpen = SvrTime::Instance().GetOpenSvrDay();
    if (!CGameTime::Instance().IsInSameDay(
        pstData->GetRoleBaseInfo().m_llFirstLoginTime,
        SvrTime::Instance().GetOpenSvrTime()) && iDaySinceSvrOpen > 1)
    {
        RESBASIC* poResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_AP_COMPENSATE_ID);
        assert(poResBasic);
        int iDelta = iDaySinceSvrOpen > poResBasic->m_para[1] ? poResBasic->m_para[1] + 1 : iDaySinceSvrOpen;
        RESPRIMAIL* poResPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(poResBasic->m_para[0] + iDelta - 2);
        assert(poResPriMail);
        bzero(&rstMailAddReq, sizeof(SS_PKG_MAIL_ADD_REQ));
        rstMailAddReq.m_nUinCount = 1;
        rstMailAddReq.m_UinList[0] = pstData->GetRoleBaseInfo().m_ullUin;
        DT_MAIL_DATA& rstMailData = rstMailAddReq.m_stMailData;
        rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
        rstMailData.m_bState = MAIL_STATE_UNOPENED;
        StrCpy(rstMailData.m_szTitle, poResPriMail->m_szTitle, MAX_MAIL_TITLE_LEN);
        StrCpy(rstMailData.m_szContent, poResPriMail->m_szContent, MAX_MAIL_CONTENT_LEN);
        rstMailData.m_ullFromUin = 0;
        rstMailData.m_bAttachmentCount = poResPriMail->m_bAttachmentNum;
        for (int j = 0; j < rstMailData.m_bAttachmentCount; j++)
        {
            rstMailData.m_astAttachmentList[j].m_bItemType = poResPriMail->m_astAttachmentList[j].m_bType;
            rstMailData.m_astAttachmentList[j].m_dwItemId = poResPriMail->m_astAttachmentList[j].m_dwId;
            rstMailData.m_astAttachmentList[j].m_iValueChg = poResPriMail->m_astAttachmentList[j].m_dwNum;
        }
        ZoneSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
    }
}

