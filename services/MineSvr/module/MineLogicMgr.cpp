
#include "./MineLogicMgr.h"
#include "./MineDataMgr.h"




MineLogicMgr::MineLogicMgr()
{

}

MineLogicMgr::~MineLogicMgr()
{

}

void CoroutineDailySettleFunc(void* pArgs)
{
	CoSettleArgs* pPara = (CoSettleArgs*) pArgs;
	MinePlayer* pstPlayer = NULL;
	for (int i = 0; i < pPara->m_bCount; i++)
	{
		if ((pstPlayer = MineDataMgr::Instance().GetMinePlayer(pPara->m_UinList[i])) == NULL)
		{
			LOGWARN("Uin<%lu> CoroutineDailySettleFunc is NULL", pPara->m_UinList[i]);
			continue;
		}
		pstPlayer->SettleDaily();
	}

}

bool MineLogicMgr::Init(MINESVRCFG* pstConfig)
{
	m_iDelayCheckOreSec = pstConfig->m_iDelayCheckOreSec;
    if (!LoadGameData())
    {
        LOGERR("LoadGameData error!");
        return false;
    }

	if (!m_oMineUinSet.Init("UinList.fd"))
	{
		return false;
	}
    return true;
}

bool MineLogicMgr::LoadGameData()
{
    RESBASIC* pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_MINE_MINING_BEGIN_TIME);
    if (!pResBasic)  return false;
    m_dwDailyStartSecOfDay = pResBasic->m_para[0] * 3600 + pResBasic->m_para[1] * 60 + pResBasic->m_para[2];

    pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_MINE_GUARD_TIME);
    if (!pResBasic) return false;
    m_dwChallengeGuardTime = pResBasic->m_para[0] * 60;
    m_dwOccupyOreGuardTime = pResBasic->m_para[1] * 60;
	m_dwSelectOccupyOutTime = pResBasic->m_para[2] * 60;

    pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_MINE_LOOT_PARA);
    if (!pResBasic) return false;
    m_fLootRate = pResBasic->m_para[0];
    m_fLootMaxCycleNum = pResBasic->m_para[1];
    m_dwBeLootCount = pResBasic->m_para[2];
    m_fLootLowCycleNum = pResBasic->m_para[3];

	pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_MINE_DROP_RATE);
	if (!pResBasic) return false;
	m_fDropRate = pResBasic->m_para[0];

    pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_MINE_MINING_END_TIME);
    if (!pResBasic)  return false;
    m_dwDailyEndSecOfDay = pResBasic->m_para[0] * 3600 + pResBasic->m_para[1] * 60 + pResBasic->m_para[2];

	m_dwDailySettleAwardSecOfDay = pResBasic->m_para[0] * 3600 + pResBasic->m_para[1] * 60 + pResBasic->m_para[2] + pResBasic->m_para[3] * 60;
    m_ullDailySettleAwardTime = CGameTime::Instance().GetSecOfCycleSecInCurrDay(m_dwDailySettleAwardSecOfDay);

	m_ullNowDailyStartTime = CGameTime::Instance().GetSecOfCycleSecInCurrDay(m_dwDailyStartSecOfDay);

	m_ullNowDailyEndTime = m_ullNowDailyStartTime - m_dwDailyStartSecOfDay + m_dwDailyEndSecOfDay;

	m_dwDailyMaxProduceTime = m_dwDailyEndSecOfDay - m_dwDailyStartSecOfDay;


    RESMINE* pResMine = NULL;
    for (int i = 0; i < CGameDataMgr::Instance().GetResMineMgr().GetResNum(); i++)
    {
        pResMine = CGameDataMgr::Instance().GetResMineMgr().GetResByPos(i);
        assert(pResMine);
        m_MineTypeToArryMap[pResMine->m_bOreType].push_back(pResMine->m_dwId);
    }
	m_bIsSettle = false;
	m_bIsClear = false;

	pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_MINE_SEASON_TIME);
	if (!pResBasic)  return false;
	//	一周内的开放
	m_bSeasonStartWeekDay = (uint8_t)pResBasic->m_para[0];
	m_bSeasonEndWeekDay = (uint8_t)pResBasic->m_para[1];
	m_ullSeasonStartTime = CGameTime::Instance().GetSecByWeekDay(m_bSeasonStartWeekDay, 0) + m_dwDailyStartSecOfDay;
	if ((uint64_t) CGameTime::Instance().GetCurrSecond() < m_ullSeasonStartTime)
	{
		m_ullSeasonStartTime -= SECONDS_OF_WEEK;
	}
	// 赛季结束以赛季最后一天发奖的时间点
	m_ullSeasonEndTime = CGameTime::Instance().GetSecByWeekDay(m_bSeasonEndWeekDay, 0) + m_dwDailySettleAwardSecOfDay;


	pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_MINE_NORMAL_ORE_GEN_RULE);
	if (!pResBasic)  return false;
	m_iInitNormalOreNum = (int)pResBasic->m_para[0];
	m_fNormalEmptyRate = (float)pResBasic->m_para[1];
	m_iNormalOreAddNum = (int)pResBasic->m_para[2];

	pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(BASIC_MINE_SUPPER_ORE_GEN_RULE);
	if (!pResBasic)  return false;
	m_iInitSupperOreNum = (int)pResBasic->m_para[0];
	m_fSupperEmptyRate = (float)pResBasic->m_para[1];
	m_iSupperOreAddNum = (int)pResBasic->m_para[2];

	if (CreateOre() != ERR_NONE)
	{
		return false;
	}

	m_ullLastUpdateOreNum = CGameTime::Instance().GetCurrSecond() + m_iDelayCheckOreSec; //延迟10分钟检测
	LOGRUN("DailySTime<%lu>, DailyETime<%lu>, SeansonSTime<%lu> SeaonETime<%lu>",
		m_ullNowDailyStartTime, m_ullNowDailyEndTime, m_ullSeasonStartTime, m_ullSeasonEndTime);
    return true;
}


void MineLogicMgr::Update(bool bIdle)
{
	uint64_t ullNow = CGameTime::Instance().GetCurrSecond();
	uint64_t ullTime = (uint64_t)CGameTime::Instance().GetSecOfCycleSecInCurrDay(m_dwDailyStartSecOfDay);
	if (ullTime > m_ullNowDailyStartTime)
	{
		m_ullNowDailyStartTime = ullTime;
		m_ullNowDailyEndTime = ullTime - m_dwDailyStartSecOfDay + m_dwDailyEndSecOfDay;
		LOGRUN("##  NewDailyDay DailySTime<%lu>, DailyETime<%lu>", m_ullNowDailyStartTime, m_ullNowDailyEndTime);
	}
	ullTime = (uint64_t)CGameTime::Instance().GetSecOfCycleSecInCurrDay(m_dwDailySettleAwardSecOfDay);
	if (ullTime > m_ullDailySettleAwardTime)
	{
		LOGRUN("Settle begin: LastDialySettleAwardTime<%lu> CurDailySettleAwardTime<%lu>", m_ullDailySettleAwardTime, ullTime);
		m_ullDailySettleAwardTime = ullTime;
		m_bIsSettle = true;
		m_oMineUinSet.SettleBegin();
		gettimeofday(&m_stBeginTv, NULL);
	}
	if (m_bIsSettle)
	{
		uint64_t ullUin = 0;
		for (m_CoSettleArgs.m_bCount = 0; m_CoSettleArgs.m_bCount < UIN_BATCH_COUNT; m_CoSettleArgs.m_bCount++, m_oMineUinSet.SettleNext())
		{
			if (m_oMineUinSet.SettleIsEnd())
			{
				m_bIsSettle = false;
				m_bIsClear = true;
				timeval EndTv;
				gettimeofday(&EndTv, NULL);
				uint64_t fElapsedUs = UsPass(&EndTv, &m_stBeginTv);
				LOGRUN("Settle end:  cost time <%lu> us", fElapsedUs);

				break;
			}
			ullUin = m_oMineUinSet.SettleGetCur();
			if (ullUin == 0)
				continue;
			m_CoSettleArgs.m_UinList[m_CoSettleArgs.m_bCount] = ullUin;
		}
		MineSvrMsgLayer::Instance().GetCoroutineEnv()->StartCoroutine(CoroutineDailySettleFunc, &m_CoSettleArgs);
	}
	m_oMineUinSet.Update(bIdle);

	//周一凌晨 刷新赛季开放时间
	uint64_t ullNewSeasonStartTime = CGameTime::Instance().GetSecByWeekDay(m_bSeasonStartWeekDay, 0) + m_dwDailyStartSecOfDay;
	if (ullNow < ullNewSeasonStartTime)
	{
		ullNewSeasonStartTime -= SECONDS_OF_WEEK;
	}
	if (ullNewSeasonStartTime > m_ullSeasonStartTime)
	{

		m_ullSeasonStartTime = ullNewSeasonStartTime;
		m_ullSeasonEndTime = CGameTime::Instance().GetSecByWeekDay(m_bSeasonEndWeekDay, 0) + m_dwDailySettleAwardSecOfDay;
		LOGRUN("##  New season start! STime<%lu> ETime<%lu>", m_ullSeasonStartTime, m_ullSeasonEndTime);
		//检查矿的状态
		CreateOre();
		//清理Uin,新赛季没参加的就不用结算
		m_oMineUinSet.Clear();
	}

	UpdateOreNum();
}


int MineLogicMgr::CreateOre()
{
	if (!MineDataMgr::Instance().IsMineUidSetEmpty())
	{
		return ERR_NONE;
	}
	ResMineArry_t& NormalArray = m_MineTypeToArryMap[ORE_TYPE_NORMAL];
	if (!NormalArray.empty() && MineDataMgr::Instance().GetNormalOreTotalNum() < m_iInitNormalOreNum)
	{
		int PerIdNum = (m_iInitNormalOreNum - MineDataMgr::Instance().GetNormalOreTotalNum()) / NormalArray.size() + 1;
		for (ResMineArry_t::iterator i = NormalArray.begin(); i != NormalArray.end(); i++)
		{
			MineDataMgr::Instance().CreateOre(ORE_TYPE_NORMAL, *i, PerIdNum);
		}
	}

	ResMineArry_t& SupperArray = m_MineTypeToArryMap[ORE_TYPE_SUPPER];
	if (!SupperArray.empty() && MineDataMgr::Instance().GetSupperOreTotalNum() < m_iInitSupperOreNum)
	{
		int PerIdNum = (m_iInitSupperOreNum - MineDataMgr::Instance().GetSupperOreTotalNum()) / SupperArray.size() + 1;
		for (ResMineArry_t::iterator i = SupperArray.begin(); i != SupperArray.end(); i++)
		{
			MineDataMgr::Instance().CreateOre(ORE_TYPE_SUPPER, *i, PerIdNum);
		}
	}

	ResMineArry_t& InvestigateArray = m_MineTypeToArryMap[ORE_TYPE_INVESTIGATE];
	if (!InvestigateArray.empty() && MineDataMgr::Instance().GetInvestigateOreNum() < (int)InvestigateArray.size())
	{
		//这里是为了在探索的时候保证随机不重复, 必须有MAX_MINE_EXPLORE_ORE_NUM个调查矿
		int PerIdNum = MAX_MINE_EXPLORE_ORE_NUM / InvestigateArray.size() + 1;
		for (ResMineArry_t::iterator i = InvestigateArray.begin(); i != InvestigateArray.end(); i++)
		{
			MineDataMgr::Instance().CreateOre(ORE_TYPE_INVESTIGATE, *i, PerIdNum);
		}
	}
	return ERR_NONE;
}

void MineLogicMgr::Fini()
{
	m_oMineUinSet.Fini();
}

int MineLogicMgr::GetInfo(uint64_t ullUin, char* szName, uint32_t dwAddr, uint16_t wLv, uint16_t wIconId, uint32_t dwLeaderValue, OUT PKGMETA::SS_PKG_MINE_GET_INFO_RSP& rstRsp)
{
    MinePlayer* pstPlayer = MineDataMgr::Instance().GetMinePlayer(ullUin);
    if (!pstPlayer)
    {
        LOGERR("Uin<%lu> can't find the  player", ullUin);
        return ERR_SYS;
    }
    pstPlayer->SetPlayerInfo(szName, dwAddr, wLv, wIconId, dwLeaderValue);   //更新保持最新
    pstPlayer->SettleDaily();
	m_oMineUinSet.Insert(ullUin);
    return pstPlayer->GetAllInfo(rstRsp);
}

int MineLogicMgr::Explore(uint64_t ullUin, uint32_t dwTeamLi, SS_PKG_MINE_EXPLORE_RSP& rstRsp)
{
    MinePlayer* pstPlayer = MineDataMgr::Instance().GetMinePlayer(ullUin);
    if (!pstPlayer)
    {
        LOGERR("Uin<%lu> can't find the  player", ullUin);
        return ERR_SYS;
    }
    if (ERR_NONE == MineDataMgr::Instance().RandomOre(ullUin, dwTeamLi, rstRsp.m_bExploreOreCount, rstRsp.m_astExploreOreList))
    {
        pstPlayer->UpdateExploreOreList(rstRsp.m_bExploreOreCount, rstRsp.m_astExploreOreList);
		MineDataMgr::Instance().DelFromTimeList(pstPlayer);
		MineDataMgr::Instance().AddToDirtyList(pstPlayer);
    }
    return ERR_NONE;
}

int MineLogicMgr::Occupy(uint64_t ullUin, PKGMETA::SS_PKG_MINE_DEAL_ORE_REQ& rstReq, OUT PKGMETA::DT_MINE_ORE_INFO& rstOreInfo)
{
    MineOre* pstOre = MineDataMgr::Instance().GetMineOre(rstReq.m_ullOreUid);
    MinePlayer* pstPlayer = MineDataMgr::Instance().GetMinePlayer(ullUin);
    if (!pstOre || !pstPlayer)
    {
        LOGERR("Uin<%lu> can't find the ore<%lu> or player", ullUin, rstReq.m_ullOreUid);
        return ERR_SYS;
    }

    int iRet = pstPlayer->OccupyOre(*pstOre, rstReq.m_bTroopCount, rstReq.m_astTroopInfo, rstReq.m_stMasterSkill);
    if (iRet == ERR_NONE)
    {

        pstOre->GetOreInfo(rstOreInfo);
		MineDataMgr::Instance().DelFromEmptyList(pstOre);

		MineDataMgr::Instance().DelFromTimeList(pstOre);
		MineDataMgr::Instance().AddToDirtyList(pstOre);

		MineDataMgr::Instance().DelFromTimeList(pstPlayer);
		MineDataMgr::Instance().AddToDirtyList(pstPlayer);
    }
    return iRet;
}

int MineLogicMgr::Drop(uint64_t ullUin, uint64_t ullOreUid, OUT PKGMETA::DT_MINE_ORE_INFO& rstOreInfo)
{
    MineOre* pstOre = MineDataMgr::Instance().GetMineOre(ullOreUid);
    MinePlayer* pstPlayer = MineDataMgr::Instance().GetMinePlayer(ullUin);
    if (!pstOre || !pstPlayer)
    {
        LOGERR("Uin<%lu> can't find the ore<%lu> or player", ullUin, ullOreUid);
        return ERR_SYS;
    }
    pstOre->GetOreInfo(rstOreInfo); //获取已有的
	int iRet = pstPlayer->DropOre(*pstOre);
	if (iRet == ERR_NONE)
	{
		MineDataMgr::Instance().AddToEmptyList(pstOre);

		MineDataMgr::Instance().DelFromTimeList(pstOre);
		MineDataMgr::Instance().AddToDirtyList(pstOre);

		MineDataMgr::Instance().DelFromTimeList(pstPlayer);
		MineDataMgr::Instance().AddToDirtyList(pstPlayer);
	}
	return iRet;
}

int MineLogicMgr::Modify(uint64_t ullUin, uint64_t ullOreUid, PKGMETA::DT_ITEM_MSKILL& rstMSkill, OUT PKGMETA::DT_MINE_ORE_INFO& rstOreInfo)
{
    MineOre* pstOre = MineDataMgr::Instance().GetMineOre(ullOreUid);
    MinePlayer* pstPlayer = MineDataMgr::Instance().GetMinePlayer(ullUin);
    if (!pstOre || !pstPlayer)
    {
        LOGERR("Uin<%lu> can't find the ore<%lu> or player", ullUin, ullOreUid);
        return ERR_SYS;
    }
    if (!pstPlayer->CheckOreUidInOwnList(ullOreUid))
    {
        LOGERR("Uin<%lu> the OreUid<%lu> error", ullUin, ullOreUid);
        return  ERR_MINE_INVALID_ORE;
    }
    int iRet = pstOre->Modify(ullUin, rstMSkill);
    if (iRet == ERR_NONE)
    {
        pstOre->GetOreInfo(rstOreInfo);
		MineDataMgr::Instance().DelFromTimeList(pstOre);
		MineDataMgr::Instance().AddToDirtyList(pstOre);

		MineDataMgr::Instance().DelFromTimeList(pstPlayer);
		MineDataMgr::Instance().AddToDirtyList(pstPlayer);
    }
    return iRet;
}

int MineLogicMgr::Investigate(uint64_t ullUin, uint64_t ullOreUid, OUT PKGMETA::DT_MINE_ORE_INFO& rstOreInfo)
{

    MinePlayer* pstPlayer = MineDataMgr::Instance().GetMinePlayer(ullUin);
    if ( !pstPlayer)
    {
        LOGERR("Uin<%lu> can't find the  player", ullUin);
        return ERR_SYS;
    }
	int iRet = pstPlayer->Investigate(ullOreUid, rstOreInfo);
	if (iRet == ERR_NONE)
	{
		MineDataMgr::Instance().DelFromTimeList(pstPlayer);
		MineDataMgr::Instance().AddToDirtyList(pstPlayer);
	}
	return iRet;

}


int MineLogicMgr::ChallengeRequest(uint64_t ullUin, uint64_t ullOreUid, OUT PKGMETA::DT_MINE_ORE_INFO& rstOreInfo)
{
    MineOre* pstOre = MineDataMgr::Instance().GetMineOre(ullOreUid);
    MinePlayer* pstPlayer = MineDataMgr::Instance().GetMinePlayer(ullUin);
    if (!pstOre || !pstPlayer)
    {
        LOGERR("Uin<%lu> can't find the ore<%lu> or player", ullUin, ullOreUid);
        return ERR_SYS;
    }
    int iRet = pstPlayer->ChallengeRequest(*pstOre);
	pstOre->GetOreInfo(rstOreInfo);
	if (iRet == ERR_NONE)
	{
		MineDataMgr::Instance().DelFromTimeList(pstOre);
		MineDataMgr::Instance().AddToDirtyList(pstOre);

		MineDataMgr::Instance().DelFromTimeList(pstPlayer);
		MineDataMgr::Instance().AddToDirtyList(pstPlayer);
	}
    return iRet;
}




int MineLogicMgr::RevengeRequest(uint64_t ullUin, uint64_t ullOreUid, OUT PKGMETA::DT_MINE_ORE_INFO& rstOreInfo)
{
	MineOre* pstOre = MineDataMgr::Instance().GetMineOre(ullOreUid);
	if (!pstOre)
	{
		LOGERR("Uin<%lu> can't find the ore<%lu>", ullUin, ullOreUid);
		return ERR_SYS;
	}
	pstOre->GetOreInfo(rstOreInfo);
	if (pstOre->GetOwner() == 0)
	{
		//无人矿, 无需抢夺
		return ERR_MINE_NO_ONE_OCCUPY_CANT_CHALLENGE;
	}
	uint64_t ullNow = CGameTime::Instance().GetCurrSecond();
	if (ullNow < pstOre->GetRealFightGuardTime())
	{
		LOGERR("Uin<%lu> revenge ore<Uid=%lu> error, in guard time !", ullUin, pstOre->GetUid());
		return ERR_MINE_ORE_IN_GUARD_STATE;
	}
	pstOre->SetFightGuardTime(ullNow + MineLogicMgr::Instance().m_dwChallengeGuardTime);
	pstOre->SetChallenger(ullUin);
	MineDataMgr::Instance().DelFromTimeList(pstOre);
	MineDataMgr::Instance().AddToDirtyList(pstOre);
	return ERR_NONE;
}

int MineLogicMgr::Challenge(uint64_t ullUin, uint64_t ullOreUid, uint8_t bIsWin)
{
    MineOre* pstOre = MineDataMgr::Instance().GetMineOre(ullOreUid);
    MinePlayer* pstPlayer = MineDataMgr::Instance().GetMinePlayer(ullUin);
    if (!pstOre || !pstPlayer)
    {
        LOGERR("Uin<%lu> can't find the ore<%lu> or player", ullUin, ullOreUid);
        return ERR_SYS;
    }
	int iRet = ERR_NONE;
	if (bIsWin == 1)
	{
		iRet = pstPlayer->ChallengeWin(*pstOre);
	}
	else
	{
		iRet = pstPlayer->ChallengeLost(*pstOre);
	}
	if (iRet == ERR_NONE)
	{
		MineDataMgr::Instance().DelFromTimeList(pstOre);
		MineDataMgr::Instance().AddToDirtyList(pstOre);

		MineDataMgr::Instance().DelFromTimeList(pstPlayer);
		MineDataMgr::Instance().AddToDirtyList(pstPlayer);
	}

    return iRet;
}


int MineLogicMgr::Grab(uint64_t ullUin, PKGMETA::SS_PKG_MINE_DEAL_ORE_REQ& rstReq, OUT PKGMETA::DT_MINE_ORE_INFO& rstOreInfo)
{
    MineOre* pstOre = MineDataMgr::Instance().GetMineOre(rstReq.m_ullOreUid);
    MinePlayer* pstPlayer = MineDataMgr::Instance().GetMinePlayer(ullUin);
    if (!pstOre || !pstPlayer)
    {
        LOGERR("Uin<%lu> can't find the ore<%lu> or player", ullUin, rstReq.m_ullOreUid);
        return ERR_SYS;
    }
    int iRet = pstPlayer->GrabOre(*pstOre, rstReq.m_bTroopCount, rstReq.m_astTroopInfo, rstReq.m_stMasterSkill);
    if (iRet == ERR_NONE)
    {
        pstOre->GetOreInfo(rstOreInfo);
		MineDataMgr::Instance().DelFromTimeList(pstOre);
		MineDataMgr::Instance().AddToDirtyList(pstOre);

		MineDataMgr::Instance().DelFromTimeList(pstPlayer);
		MineDataMgr::Instance().AddToDirtyList(pstPlayer);
    }
    return iRet;

}


int MineLogicMgr::GiveUpGrab(uint64_t ullUin, PKGMETA::SS_PKG_MINE_DEAL_ORE_REQ& rstReq, OUT PKGMETA::DT_MINE_ORE_INFO& rstOreInfo)
{
	MineOre* pstOre = MineDataMgr::Instance().GetMineOre(rstReq.m_ullOreUid);
	if (!pstOre)
	{
		LOGERR("Uin<%lu> can't find the ore<%lu>", ullUin, rstReq.m_ullOreUid);
		return ERR_SYS;
	}
	int iRet = pstOre->GiveUpGrab();
	if (iRet == ERR_NONE)
	{
		pstOre->GetOreInfo(rstOreInfo);
		MineDataMgr::Instance().DelFromTimeList(pstOre);
		MineDataMgr::Instance().AddToDirtyList(pstOre);

	}
	return iRet;
}

int MineLogicMgr::Revenge(uint64_t ullUin, uint64_t ullObjUin, uint64_t ullOreUid, uint8_t bIsWin, uint8_t bRevengeLogIndex)
{
	MineOre* pstOre = MineDataMgr::Instance().GetMineOre(ullOreUid);
	MinePlayer* pstPlayer = MineDataMgr::Instance().GetMinePlayer(ullUin);
	if (!pstPlayer || !pstPlayer)
	{
		LOGERR("Uin<%lu> can't find the ore<%lu> or player", ullUin, ullOreUid);
		return ERR_SYS;
	}

	int iRet = ERR_NONE;
	if (bIsWin == 1)
	{
		iRet = pstPlayer->RevengeWin(*pstOre, ullObjUin, bRevengeLogIndex);
	}
	else
	{
		iRet = pstPlayer->RevengeLost(*pstOre, ullObjUin);
	}
	if (iRet == ERR_NONE)
	{
		MineDataMgr::Instance().DelFromTimeList(pstOre);
		MineDataMgr::Instance().AddToDirtyList(pstOre);

		MineDataMgr::Instance().DelFromTimeList(pstPlayer);
		MineDataMgr::Instance().AddToDirtyList(pstPlayer);
	}
	return iRet;
}

int MineLogicMgr::GetAwardLog(uint64_t ullUin, uint8_t bType, uint8_t bIndex, OUT PKGMETA::SS_PKG_MINE_GET_AWARD_RSP& rstRsp)
{
    rstRsp.m_bAwardCount = 0;
    MinePlayer* pstPlayer = MineDataMgr::Instance().GetMinePlayer(ullUin);
    if (!pstPlayer)
    {
        LOGERR("Uin<%lu> can't find  player", ullUin);
        return ERR_SYS;
    }
    int iRet = ERR_NONE;
    if (bType == 1) //一键领取
    {
        for (int i = 0; i < pstPlayer->GetAwardLogCount(); i++)
        {
            pstPlayer->GetAwardLog(i, rstRsp.m_astAwardList[i]);
            rstRsp.m_bAwardCount++;
        }
    }
    else
    {
        iRet = pstPlayer->GetAwardLog(bIndex, rstRsp.m_astAwardList[0]);
		if (iRet != ERR_NONE)
		{
			return iRet;
		}
        rstRsp.m_bAwardCount = 1;
    }
	MineDataMgr::Instance().DelFromTimeList(pstPlayer);
	MineDataMgr::Instance().AddToDirtyList(pstPlayer);
	rstRsp.m_bType = bType;
    LOGWARN("Uin<%lu> get award count<%hhu> type<%hhu> num<%hhu ", ullUin, rstRsp.m_bAwardCount,
        rstRsp.m_astAwardList[0].m_astPropList[0].m_bItemType, rstRsp.m_astAwardList[0].m_astPropList[0].m_dwItemNum);
    return iRet;
}




void MineLogicMgr::SendNtfToClient(uint8_t bNtfType, SSPKG& rstPkg)
{
    rstPkg.m_stHead.m_wMsgId = SC_MSG_MINE_INFO_NTF;
    SS_PKG_MINE_INFO_NTF& rstNtf = rstPkg.m_stBody.m_stMineInfoNtf;
    rstNtf.m_bType = bNtfType;
    MineSvrMsgLayer::Instance().SendToClusterGate(rstPkg);
}

void MineLogicMgr::UpdateOreNum()
{
	if ((m_ullLastUpdateOreNum + 2 * 60 ) > (uint64_t )CGameTime::Instance().GetCurrSecond()) //2分钟检测一次
	{
		return;
	}

	if (MineDataMgr::Instance().GetNormalOreEmptyRate() <= m_fNormalEmptyRate)
	{
		ResMineArry_t& NormalArray = m_MineTypeToArryMap[ORE_TYPE_NORMAL];
		if (!NormalArray.empty() )
		{
			int PerIdNum = m_iNormalOreAddNum  / NormalArray.size() + 1;
			for (ResMineArry_t::iterator i = NormalArray.begin(); i != NormalArray.end(); i++)
			{
				MineDataMgr::Instance().CreateOre(ORE_TYPE_NORMAL, *i, PerIdNum);
			}
		}
	}
	if (MineDataMgr::Instance().GetSupperOreEmptyRate() <= m_fNormalEmptyRate)
	{
		ResMineArry_t& SupperArray = m_MineTypeToArryMap[ORE_TYPE_SUPPER];
		if (!SupperArray.empty())
		{
			int PerIdNum = m_iSupperOreAddNum  / SupperArray.size() + 1;
			for (ResMineArry_t::iterator i = SupperArray.begin(); i != SupperArray.end(); i++)
			{
				MineDataMgr::Instance().CreateOre(ORE_TYPE_SUPPER, *i, PerIdNum);
			}
		}
	}

	ResMineArry_t& InvestigateArray = m_MineTypeToArryMap[ORE_TYPE_INVESTIGATE];
	if (!InvestigateArray.empty() && MineDataMgr::Instance().GetInvestigateOreNum() < (int)InvestigateArray.size())
	{
		//这里是为了在探索的时候保证随机不重复, 必须有MAX_MINE_EXPLORE_ORE_NUM个调查矿
		int PerIdNum = MAX_MINE_EXPLORE_ORE_NUM / InvestigateArray.size() + 1;
		for (ResMineArry_t::iterator i = InvestigateArray.begin(); i != InvestigateArray.end(); i++)
		{
			MineDataMgr::Instance().CreateOre(ORE_TYPE_INVESTIGATE, *i, PerIdNum);
		}
	}

	m_ullLastUpdateOreNum = CGameTime::Instance().GetCurrSecond();
	LOGWARN("UpdateOreNum, SupperTotalNum<%d> SuppterEmptyNum<%d> NormalTotalNum<%d> NormalEmpty<%d> InvestigateNum<%d>",
		MineDataMgr::Instance().GetSupperOreTotalNum(), MineDataMgr::Instance().GetSupperOreEmptyNum(),
		MineDataMgr::Instance().GetNormalOreTotalNum(), MineDataMgr::Instance().GetNormalOreEmptyNum(),
		MineDataMgr::Instance().GetInvestigateOreNum());
}

