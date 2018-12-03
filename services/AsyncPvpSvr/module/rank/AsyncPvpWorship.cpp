#include "AsyncPvpWorship.h"
#include "../../gamedata/GameDataMgr.h"
#include "LogMacros.h"
#include "AsyncPvpRank.h"
#include "../player/AsyncPvpPlayerMgr.h"
#include "../../framework/AsyncPvpSvrMsgLayer.h"
#include "../../transaction/AsyncPvpTransaction.h"
#include "../../transaction/AsyncPvpTransFrame.h"
#include "../../transaction/GameObjectPool.h"

bool AsyncPvpWorship::Init()
{
	//结算时间
	RESBASIC* poResBasic= CGameDataMgr::Instance().GetResBasicMgr().Find(ASYNCPVP_WORSHIPPED_SETTLE_TIME);
	if (!poResBasic)
	{
		LOGERR_r("AsyncPvpRank init failed, ResBasic id(%d) not found", ASYNCPVP_WORSHIPPED_SETTLE_TIME);
		return false;
	}
	m_iSettleTime = (int)poResBasic->m_para[0];

	m_bSettleFlag = false;

	return true;
}

void AsyncPvpWorship::Update()
{
	//每日结算
	int iHour = CGameTime::Instance().GetCurrHour();
	if ((iHour == m_iSettleTime) && !m_bSettleFlag)
	{
		this->SendWorshippedReward();
		m_bSettleFlag = true;
	}
	else if ((iHour != m_iSettleTime))
	{
		m_bSettleFlag = false;
	}
}

void AsyncPvpWorship::SendWorshippedReward()
{
	WorshipMailTransAction* poTrans = GET_GAMEOBJECT(WorshipMailTransAction, GAMEOBJ_WORSHIP_MAIL_TRANS);
	assert(poTrans);

	//创建获取前100的组合action
	CompositeAction* poCompositeAction = GET_GAMEOBJECT(CompositeAction, GAMEOBJ_COMPOSITE_ACTION);
	assert(poCompositeAction);

	for (uint8_t i = 1; i <= WORSHIPPED_CANDIDATE_MAX_NUM; i++)
	{
		uint64_t ullUin = AsyncPvpRank::Instance().GetPlayerByRank(i);
		
		//假玩家不发邮件
		if (ullUin < MAX_NUM_ASYNC_PVP_FAKE_PLAYER_UIN)
		{
			continue;
		}

		GetPlayerAction* poGetWorshippedPlayerAction = GET_GAMEOBJECT(GetPlayerAction, GAMEOBJ_GET_PLAYER_ACTION);
		assert(poGetWorshippedPlayerAction);
		poGetWorshippedPlayerAction->SetPlayerId(ullUin);

		poCompositeAction->AddAction(poGetWorshippedPlayerAction);
	}

	poTrans->AddAction(poCompositeAction);

	AsyncPvpTransFrame::Instance().ScheduleTransaction(poTrans);

	//RESPRIMAIL* poResPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(DAILY_WORSHIPPED_SETTLE_MAIL_ID);
	//if (NULL == poResPriMail)
	//{
	//	LOGERR("Can't find the mail<id %u>", DAILY_WORSHIPPED_SETTLE_MAIL_ID);
	//	return;
	//}

	//m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
	//SS_PKG_MAIL_ADD_REQ& rstMailAddReq = m_stSsPkg.m_stBody.m_stMailAddReq;
	//rstMailAddReq.m_nUinCount = 0;
	//DT_MAIL_DATA& rstMailData = rstMailAddReq.m_stMailData;
	//rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
	//rstMailData.m_bState = MAIL_STATE_UNOPENED;
	//StrCpy(rstMailData.m_szTitle, poResPriMail->m_szTitle, MAX_MAIL_TITLE_LEN);
	//StrCpy(rstMailData.m_szContent, poResPriMail->m_szContent, MAX_MAIL_CONTENT_LEN);
	//rstMailData.m_ullFromUin = 0;

	//for (int i=1; i<=WORSHIPPED_CANDIDATE_MAX_NUM; i++)
	//{
	//	uint64_t ullUin = AsyncPvpRank::Instance().GetPlayerByRank(i);

	//	//假玩家不发邮件
	//	if (ullUin < MAX_NUM_ASYNC_PVP_FAKE_PLAYER_UIN)
	//	{
	//		continue;
	//	}

	//	AsyncPvpPlayer* poPlayer = AsyncPvpPlayerMgr::Instance().GetPlayer(ullUin);
	//	if (NULL == poPlayer)
	//	{
	//		LOGERR("poPlayer is null:%lu", ullUin);
	//		return;
	//	}

	//	int iWorshippedGold = (int)poPlayer->GetWorshipGold();

	//	if ( iWorshippedGold == 0 )
	//	{
	//		continue;
	//	}

	//	rstMailData.m_bAttachmentCount = 1;
	//	rstMailData.m_astAttachmentList[0].m_bItemType = ITEM_TYPE_GOLD;
	//	rstMailData.m_astAttachmentList[0].m_dwItemId = 0;
	//	rstMailData.m_astAttachmentList[0].m_iValueChg = iWorshippedGold;
	//	poPlayer->ClearWorshipGold();
	//	rstMailAddReq.m_UinList[rstMailAddReq.m_nUinCount++] = ullUin;

	//	AsyncPvpSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
	//	rstMailAddReq.m_nUinCount = 0;
	//}

}


