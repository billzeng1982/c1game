#include "GFightMgrState.h"
#include "LogMacros.h"
#include "GFightMgrStateMachine.h"
#include "common_proto.h"
#include "GFightArenaMgr.h"
#include "GuildFightArena.h"

using namespace PKGMETA;

void GFightMgrState::ChangeState(GuildFightMgr* poGFightMgr, int iNewState)
{
    assert(poGFightMgr);
    LOGRUN_r("GuildFightMgr change state from %d to %d", poGFightMgr->GetState(), iNewState);

    //退出旧状态
    int iOldState = poGFightMgr->GetState();
    GFightMgrState* poOldState = GFightMgrStateMachine::Instance().GetState(iOldState);
    assert(poOldState);
    poOldState->Exit(poGFightMgr);

    //进入新状态
    GFightMgrState* poNewState = GFightMgrStateMachine::Instance().GetState(iNewState);
    assert(poNewState);
    poGFightMgr->SetState(iNewState);
    poNewState->Enter(poGFightMgr);

    //保存进度
    poGFightMgr->_SaveSchedule();
}



void GFightMgrState_NotOpen::Enter(GuildFightMgr* poGFightMgr)
{

}

void GFightMgrState_NotOpen::Update(GuildFightMgr* poGFightMgr, int iDeltaTime)
{
    if (poGFightMgr->m_ullTimeMs >= poGFightMgr->m_ullChgStateTime)
    {
        GFightMgrStateMachine::Instance().ChangeState(poGFightMgr, GUILD_FIGHT_STATE_NONE);
    }
}

void GFightMgrState_NotOpen::Exit(GuildFightMgr* poGFightMgr)
{

}

void GFightMgrState_None::Enter(GuildFightMgr* poGFightMgr)
{
    if (poGFightMgr->m_ullTimeMs > poGFightMgr->m_ullApplyStartTime + poGFightMgr->m_ullApplyTime)
    {
        poGFightMgr->m_ullApplyStartTime += SECONDS_OF_WEEK * 1000;
    }

    poGFightMgr->m_ullChgStateTime = poGFightMgr->m_ullApplyStartTime;
}

void GFightMgrState_None::Update(GuildFightMgr* poGFightMgr, int iDeltaTime)
{
    if (poGFightMgr->m_ullTimeMs >= poGFightMgr->m_ullChgStateTime)
    {
        GFightMgrStateMachine::Instance().ChangeState(poGFightMgr, GUILD_FIGHT_STATE_APPLY_START);
    }
}

void GFightMgrState_None::Exit(GuildFightMgr* poGFightMgr)
{
    poGFightMgr->Reset();
    poGFightMgr->SendApplyListSyn();
    poGFightMgr->SendAgainstListSyn();
}

/*报名阶段*/
void GFightMgrState_ApplyStart::Enter(GuildFightMgr* poGFightMgr)
{
    poGFightMgr->m_ullChgStateTime += poGFightMgr->m_ullApplyTime;

	poGFightMgr->m_bFirstRest = true;
}

void GFightMgrState_ApplyStart::Update(GuildFightMgr* poGFightMgr, int iDeltaTime)
{
    if (poGFightMgr->m_ullTimeMs >= poGFightMgr->m_ullChgStateTime)
    {
        GFightMgrStateMachine::Instance().ChangeState(poGFightMgr, GUILD_FIGHT_STATE_APPLY_END);
    }
}

void GFightMgrState_ApplyStart::Exit(GuildFightMgr* poGFightMgr)
{
    //报名结束后生成初始对阵表
    poGFightMgr->GenerateAgainstList();

    //报名结束后的一些处理
    poGFightMgr->OnApplyEnd();
}

/*报名结束阶段*/
void GFightMgrState_ApplyEnd::Enter(GuildFightMgr* poGFightMgr)
{
    if (poGFightMgr->m_iFightStage == GUILD_FIGHT_STAGE_QUARTERFINAL)
    {
        poGFightMgr->m_ullChgStateTime += poGFightMgr->m_ullApplyRestTime;
    }
    else
    {
		if (poGFightMgr->m_bFirstRest)
		{
			poGFightMgr->m_bFirstRest = false;
			poGFightMgr->m_ullChgStateTime += poGFightMgr->m_ullFightRestTime;
		}
		else
		{
			poGFightMgr->m_ullChgStateTime += poGFightMgr->m_ullFightRestTimeFollow;
		}
    }
}

void GFightMgrState_ApplyEnd::Update(GuildFightMgr* poGFightMgr, int iDeltaTime)
{
    if (poGFightMgr->m_ullTimeMs >= poGFightMgr->m_ullChgStateTime)
    {
        GFightMgrStateMachine::Instance().ChangeState(poGFightMgr, GUILD_FIGHT_STATE_FIGHT_PREPARE);
    }
}

void GFightMgrState_ApplyEnd::Exit(GuildFightMgr* poGFightMgr)
{

}

/*战斗准备阶段*/
void GFightMgrState_FightPrepare::Enter(GuildFightMgr* poGFightMgr)
{
    poGFightMgr->m_ullChgStateTime += poGFightMgr->m_ullFightPrepareTime;

    //准备阶段生成战场
    DT_GUILD_FIGHT_ONE_AGAINST* pstAgainstList = poGFightMgr->m_stFightAgainstList.m_astAgainstList;
    int iStartIndex = poGFightMgr->m_iFightStage -1;
    int iEndIndex = (poGFightMgr->m_iFightStage==GUILD_FIGHT_STAGE_FINALS) ? 0 : (poGFightMgr->m_iFightStage/2);

    //根据对阵表生成战场
    for (int i=iStartIndex; i>=iEndIndex; i--)
    {
        GuildFightArena* poArena = GFightArenaMgr::Instance().New();

        //有轮空的直接产生结果
        if ((pstAgainstList[i].m_GuildList[0]==0) ||(pstAgainstList[i].m_GuildList[1]==0))
        {
            pstAgainstList[i].m_bFightStat = GUILD_FIGHT_AGAINST_END;
            pstAgainstList[i].m_ullWinnerId = (pstAgainstList[i].m_GuildList[0]==0) ? pstAgainstList[i].m_GuildList[1] : pstAgainstList[i].m_GuildList[0];
        }
        else
        {
            if (!poArena->Init(pstAgainstList[i].m_GuildList, MAX_NUM_ONE_GUILD_FIGHT_JOIN_GUILD, pstAgainstList[i].m_bAgainstId))
            {
                LOGERR_r("Create Arena failed, Guild1=(%lu), Guild2=(%lu)", pstAgainstList[i].m_GuildList[0], pstAgainstList[i].m_GuildList[1]);
                continue;
            }
            pstAgainstList[i].m_bFightStat = GUILD_FIGHT_AGAINST_FIGHTING;
            GFightArenaMgr::Instance().AddArena(poArena);
        }
    }

    poGFightMgr->SendAgainstListSyn();
}

void GFightMgrState_FightPrepare::Update(GuildFightMgr* poGFightMgr, int iDeltaTime)
{
    //战场Update
    for (GFightArenaMgr::Instance().m_listArenaIter = GFightArenaMgr::Instance().m_listArena.begin();
        GFightArenaMgr::Instance().m_listArenaIter!= GFightArenaMgr::Instance().m_listArena.end();
         GFightArenaMgr::Instance().m_listArenaIter++)
    {
        GuildFightArena* pstArena = *(GFightArenaMgr::Instance().m_listArenaIter);
        assert(pstArena);
        pstArena->Update(iDeltaTime);
    }

    if (poGFightMgr->m_ullTimeMs >= poGFightMgr->m_ullChgStateTime)
    {
        GFightMgrStateMachine::Instance().ChangeState(poGFightMgr, GUILD_FIGHT_STATE_FIGHT_START);
    }
}

void GFightMgrState_FightPrepare::Exit(GuildFightMgr* poGFightMgr)
{

}

/*战斗开始阶段*/
void GFightMgrState_FightStart::Enter(GuildFightMgr* poGFightMgr)
{
    poGFightMgr->m_ullChgStateTime += poGFightMgr->m_ullFightTime;

    //将战场状态从准备切到开战状态
    GFightArenaMgr::Instance().m_listArenaIter = GFightArenaMgr::Instance().m_listArena.begin();
    for (; GFightArenaMgr::Instance().m_listArenaIter!= GFightArenaMgr::Instance().m_listArena.end(); GFightArenaMgr::Instance().m_listArenaIter++)
    {
        GuildFightArena* poArena = *GFightArenaMgr::Instance().m_listArenaIter;
        poArena->Start();
    }
}

void GFightMgrState_FightStart::Update(GuildFightMgr* poGFightMgr, int iDeltaTime)
{
    bool bFlag = false;
    DT_GUILD_FIGHT_ONE_AGAINST* pstAgainstList = poGFightMgr->m_stFightAgainstList.m_astAgainstList;

    //战场Update，同时查看是否有已结束的战场
    for (GFightArenaMgr::Instance().m_listArenaIter = GFightArenaMgr::Instance().m_listArena.begin();
        GFightArenaMgr::Instance().m_listArenaIter!= GFightArenaMgr::Instance().m_listArena.end();
         GFightArenaMgr::Instance().m_listArenaIter++)
    {
        GuildFightArena* pstArena = *(GFightArenaMgr::Instance().m_listArenaIter);
        assert(pstArena);
        pstArena->Update(iDeltaTime);

        if (pstArena->m_iState == GUILD_FIGHT_ARENA_STATE_END)
        {
            uint64_t ullWinnerId = pstArena->m_ullWinGuild;
            pstAgainstList[pstArena->m_bAgainstId].m_bFightStat = GUILD_FIGHT_AGAINST_END;
            pstAgainstList[pstArena->m_bAgainstId].m_ullWinnerId = ullWinnerId;
            GFightArenaMgr::Instance().Delete(pstArena);
            bFlag = true;
        }
    }
    if (bFlag)
    {
        poGFightMgr->SendAgainstListSyn();
    }

    if (poGFightMgr->m_ullTimeMs >= poGFightMgr->m_ullChgStateTime)
    {
        //当前是决赛，结束后切为NONE状态
        if (poGFightMgr->m_iFightStage == GUILD_FIGHT_STAGE_FINALS)
        {
            GFightMgrStateMachine::Instance().ChangeState(poGFightMgr, GUILD_FIGHT_STATE_NONE);
        }
        else
        {
            GFightMgrStateMachine::Instance().ChangeState(poGFightMgr, GUILD_FIGHT_STATE_APPLY_END);
        }
    }
}

void GFightMgrState_FightStart::Exit(GuildFightMgr* poGFightMgr)
{
    //战斗结束，结算
    DT_GUILD_FIGHT_ONE_AGAINST* pstAgainstList = poGFightMgr->m_stFightAgainstList.m_astAgainstList;
    for (GFightArenaMgr::Instance().m_listArenaIter = GFightArenaMgr::Instance().m_listArena.begin();
        GFightArenaMgr::Instance().m_listArenaIter!= GFightArenaMgr::Instance().m_listArena.end();
         GFightArenaMgr::Instance().m_listArenaIter++)
    {
        GuildFightArena* pstArena = *GFightArenaMgr::Instance().m_listArenaIter;
        uint64_t ullWinnerId = pstArena->Settle();
        pstAgainstList[pstArena->m_bAgainstId].m_bFightStat = GUILD_FIGHT_AGAINST_END;
        pstAgainstList[pstArena->m_bAgainstId].m_ullWinnerId = ullWinnerId;
    }
    GFightArenaMgr::Instance().Clear();

    //非决赛，需要生成新一轮的晋级表
    poGFightMgr->m_iFightStage /= 2;
    int iFightStage = poGFightMgr->m_iFightStage;
    if (iFightStage == 1)
    {
        poGFightMgr->SendAgainstListSyn();
        return;
    }

    poGFightMgr->m_stFightAgainstList.m_bPhase = iFightStage;
    for(int i=iFightStage-1; i>=iFightStage/2; i--)
    {
        for(int j=0; j<MAX_NUM_ONE_GUILD_FIGHT_JOIN_GUILD; j++)
        {
            pstAgainstList[i].m_GuildList[j] = pstAgainstList[i*2+j].m_ullWinnerId;
        }
        pstAgainstList[i].m_bFightStat = GUILD_FIGHT_AGAINST_IDLE;
        pstAgainstList[i].m_ullWinnerId= 0;
        pstAgainstList[i].m_bAgainstId = i;
    }

    //半决赛特殊处理,需要进行季军争夺
    if (iFightStage == GUILD_FIGHT_STAGE_FINALS)
    {
        for(int j=0; j<MAX_NUM_ONE_GUILD_FIGHT_JOIN_GUILD; j++)
        {
            if (pstAgainstList[2+j].m_ullWinnerId == 0)
            {
                pstAgainstList[0].m_GuildList[j] = 0;
            }
            else if(pstAgainstList[2+j].m_ullWinnerId==pstAgainstList[2+j].m_GuildList[0])
            {
                pstAgainstList[0].m_GuildList[j] = pstAgainstList[2+j].m_GuildList[1];
            }
            else
            {
                pstAgainstList[0].m_GuildList[j] = pstAgainstList[2+j].m_GuildList[0];
            }
        }
        pstAgainstList[0].m_bFightStat = GUILD_FIGHT_AGAINST_IDLE;
        pstAgainstList[0].m_ullWinnerId = 0;
        pstAgainstList[0].m_bAgainstId = 0;
    }

    poGFightMgr->SendAgainstListSyn();
}

