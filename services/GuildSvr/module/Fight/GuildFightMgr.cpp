#include <stdlib.h>
#include "GuildFightMgr.h"
#include "LogMacros.h"
#include "../../framework/GuildSvrMsgLayer.h"
#include "strutil.h"
#include "GameTime.h"
#include "GFightMgrStateMachine.h"
#include "../Guild/GuildMgr.h"
#include "../../gamedata/GameDataMgr.h"
#include "unistd.h"

using namespace PKGMETA;

int GuildCmp(const void *pstFirst, const void *pstSecond)
{
    DT_GUILD_FIGHT_APPLY_INFO* pstItemFirst = (DT_GUILD_FIGHT_APPLY_INFO*)pstFirst;
    DT_GUILD_FIGHT_APPLY_INFO* pstItemSecond = (DT_GUILD_FIGHT_APPLY_INFO*)pstSecond;

    if (pstItemSecond->m_ullGuildId > pstItemFirst->m_ullGuildId)
    {
        return 1;
    }
    else if (pstItemSecond->m_ullGuildId < pstItemFirst->m_ullGuildId)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int FightApplyCmp(const void * pstFirst, const void * pstSecond)
{
    DT_GUILD_FIGHT_APPLY_INFO* pstItemFirst = (DT_GUILD_FIGHT_APPLY_INFO*)pstFirst;
    DT_GUILD_FIGHT_APPLY_INFO* pstItemSecond = (DT_GUILD_FIGHT_APPLY_INFO*)pstSecond;

    if (pstItemFirst->m_dwApplyFund != pstItemSecond->m_dwApplyFund)
    {
        return (int)pstItemSecond->m_dwApplyFund -(int)pstItemFirst->m_dwApplyFund;
    }

    if (pstItemSecond->m_ullGuildId > pstItemFirst->m_ullGuildId)
    {
        return 1;
    }
    else if (pstItemSecond->m_ullGuildId < pstItemFirst->m_ullGuildId)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

void GuildFightMgr::Update()
{
    int iDeltaTime = CGameTime::Instance().GetCurrTimeMs() - m_ullTimeMs;
    if (iDeltaTime > m_iUptTimeVal)
    {
        m_ullTimeMs += iDeltaTime;
        GFightMgrStateMachine::Instance().Update(this, iDeltaTime);
    }
}


void GuildFightMgr::Reset()
{
    for (int i=0; i<MAX_NUM_GUILD_FIGHT_JOIN_AGAINST; i++)
    {
        if (m_stFightAgainstList.m_astGuildList[i].m_ullGuildId != 0)
        {
            Guild* poGuild = GuildMgr::Instance().GetGuild(m_stFightAgainstList.m_astGuildList[i].m_ullGuildId);
            if (poGuild!=NULL)
            {
                poGuild->SetCanSwap(true);
            }
        }
    }

    //获取本轮冠军的信息
    DT_GUILD_FIGHT_APPLY_INFO stFightApplyInfo = {0};
    stFightApplyInfo.m_ullGuildId = m_stFightAgainstList.m_astAgainstList[1].m_ullWinnerId;
    if (stFightApplyInfo.m_ullGuildId != 0)
    {
        Guild* poGuild = GuildMgr::Instance().GetGuild(stFightApplyInfo.m_ullGuildId);
        if (poGuild!=NULL)
        {
            poGuild->SetCanSwap(false);
            memcpy(&stFightApplyInfo.m_szGuildName, poGuild->GetGuildName(), MAX_NAME_LENGTH);
        }
    }

    bzero(&m_stFightApplyList, sizeof(m_stFightApplyList));
    bzero(&m_stFightAgainstList, sizeof(m_stFightAgainstList));

    memcpy(&m_stFightApplyList.m_stLastWinner, &stFightApplyInfo, sizeof(DT_GUILD_FIGHT_APPLY_INFO));

    m_iFightStage = GUILD_FIGHT_STAGE_QUARTERFINAL;
}


bool GuildFightMgr::_FirstInit()
{
    //开服后首次开启军团战的时间
    m_ullChgStateTime = m_ullTimeMs;

    m_iState = GUILD_FIGHT_STATE_NOT_OPEN;
    m_iFightStage = GUILD_FIGHT_STAGE_QUARTERFINAL;

    m_fp = fopen(m_szFileName, "wb+");
    if (!m_fp)
    {
        LOGERR_r("FirstInit failed, create file(%s) failed.", m_szFileName);
        return false;
    }

    this->_SaveSchedule();

    return true;
}



bool GuildFightMgr::_InitFromFile()
{
    m_fp = fopen(m_szFileName, "rb+");
    if (!m_fp)
    {
        LOGERR_r("InitFromFile failed, open file(%s) failed.", m_szFileName);
        return false;
    }

    fseek(m_fp, 0, SEEK_SET);
    //从文件中读下次状态改变时间
    if(fread(&m_ullChgStateTime, sizeof(m_ullChgStateTime), 1, m_fp) != 1)
    {
        LOGERR_r("Read m_ullStartTime from file failed");
        return false;
    }

    //从文件中读状态
    if(fread(&m_iState, sizeof(m_iState), 1, m_fp) != 1)
    {
        LOGERR_r("Read m_iState from file failed");
        return false;
    }

    //从文件中读当前比赛阶段
    if(fread(&m_iFightStage, sizeof(m_iFightStage), 1, m_fp) != 1)
    {
        LOGERR_r("Read m_iFightStage from file failed");
        return false;
    }

    //从文件中读报名表
    if(fread(&m_stFightApplyList, sizeof(m_stFightApplyList), 1, m_fp) != 1)
    {
        LOGERR_r("Read m_stFightApplyList from file failed");
        return false;
    }

    //从文件中读对阵表
    if(fread(&m_stFightAgainstList, sizeof(m_stFightAgainstList), 1, m_fp) != 1)
    {
        LOGERR_r("Read m_stFightAgainstList from file failed");
        return false;
    }

    return true;
}


bool GuildFightMgr::Init(int iUptTimeVal, const char* pszFileName)
{
    bzero(&m_stFightApplyList, sizeof(m_stFightApplyList));
    bzero(&m_stFightAgainstList, sizeof(m_stFightAgainstList));

    //各种时间参数
    RESGUILDFIGHTPARAM *pPhaseTime = CGameDataMgr::Instance().GetResGuildFightParamMgr().Find((uint32_t)GUILD_FIGHT_PARAM_PHASE_TIME);
    if(NULL == pPhaseTime)
    {
        LOGERR_r("not exist phase time");
        return false;
    }
    m_ullApplyTime = (uint64_t)pPhaseTime->m_paramList[0] * 1000; //报名持续时间
    m_ullApplyRestTime = (uint64_t)pPhaseTime->m_paramList[1] * 1000; //报名结束到战斗开始的持续时间
    m_ullFightRestTime = (uint64_t)pPhaseTime->m_paramList[2] * 1000; //战斗结束到下次战斗开始的持续时间（首次）
    m_ullFightPrepareTime = (uint64_t)pPhaseTime->m_paramList[3] * 1000; //战斗准备时间
    m_ullFightTime = (uint64_t)pPhaseTime->m_paramList[4] * 1000; //战斗持续时间
    m_ullFightRestTimeFollow = (uint64_t)pPhaseTime->m_paramList[6] * 1000; //战斗结束到下次战斗开始的持续时间（后续）

    //报名开始时间
    RESGUILDFIGHTPARAM *pStartTime = CGameDataMgr::Instance().GetResGuildFightParamMgr().Find(6);
    if(NULL == pStartTime)
    {
        LOGERR_r("not exist pStartTime time");
        return false;
    }
    m_ullApplyStartTime = CGameTime::Instance().GetSecByWeekDay(pStartTime->m_paramList[0], pStartTime->m_paramList[1]) + pStartTime->m_paramList[2] * 60;
    m_ullApplyStartTime *= 1000;

    //当前时间
    m_ullTimeMs = CGameTime::Instance().GetCurrTimeMs();

    /*pStartTime = CGameDataMgr::Instance().GetResGuildFightParamMgr().Find(7);
    if(NULL == pStartTime)
    {
        LOGERR_r("not exist pStartTime time");
        return false;
    }
    ullStartTime = CGameTime::Instance().GetSecByWeekDay(pStartTime->m_paramList[0], pStartTime->m_paramList[1]);
    ullStartTime += pStartTime->m_paramList[2] * 60;
    m_TimeList.push_back(ullStartTime * 1000);*/

    //Update间隔，即每一帧的时间
    m_iUptTimeVal = iUptTimeVal;

    //存储进度的文件名
    StrCpy(m_szFileName, pszFileName, MAX_NAME_LENGTH);

    if (access(m_szFileName, F_OK))
    {
        if (!this->_FirstInit())
        {
            LOGERR_r("GuildFightMgr first init failed.");
            return false;
        }
    }
    else
    {
       if (!this->_InitFromFile())
       {
           LOGERR_r("GuildFightMgr init from file failed.");
           return false;
       }
    }

    this->SendAgainstListSyn();

    this->SendApplyListSyn();

    return true;
}

void GuildFightMgr::Fini()
{
    //写入文件
    this->_SaveSchedule();
    LOGRUN_r("GuildFightMgr Fini success");
}


//刷新公会战报名列表
void GuildFightMgr::RefreshApplyList(Guild* poGuild)
{
    int iFund = poGuild->GetFightApplyFund();

    if (m_stFightApplyList.m_bCount >= MAX_LEN_GUILD_FIGHT_APLLY_LIST)
    {
        return;
    }

    int iEqual = 0;
    DT_GUILD_FIGHT_APPLY_INFO stFightApplyInfo;
    stFightApplyInfo.m_ullGuildId = poGuild->GetGuildId();
    stFightApplyInfo.m_dwApplyFund = iFund;
    StrCpy(stFightApplyInfo.m_szGuildName, poGuild->GetGuildName(), MAX_NAME_LENGTH);
    size_t nmemb = (size_t)m_stFightApplyList.m_bCount;

    int iIdx = MyBSearch(&stFightApplyInfo, m_stFightApplyList.m_astApplyList, m_stFightApplyList.m_bCount, sizeof(DT_GUILD_FIGHT_APPLY_INFO), &iEqual, GuildCmp);
    if (iEqual)
    {
        m_stFightApplyList.m_astApplyList[iIdx].m_dwApplyFund = iFund;
    }
    else
    {
        nmemb = nmemb >= MAX_LEN_GUILD_FIGHT_APLLY_LIST ? nmemb -1 : nmemb;
        MyBInsert(&stFightApplyInfo, m_stFightApplyList.m_astApplyList, &nmemb, sizeof(DT_GUILD_FIGHT_APPLY_INFO), 1, GuildCmp);
        m_stFightApplyList.m_bCount = nmemb;
    }
}


//根据晋级表生成对战表
void GuildFightMgr::GenerateAgainstList()
{
    //首先对报名表进行排序
    qsort(m_stFightApplyList.m_astApplyList, m_stFightApplyList.m_bCount, sizeof(DT_GUILD_FIGHT_APPLY_INFO), FightApplyCmp);

    bzero(&m_stFightAgainstList, sizeof(m_stFightAgainstList));
    m_stFightAgainstList.m_bPhase = MAX_NUM_GUILD_FIGHT_JOIN_AGAINST;
    //第一次生成对阵表时需要初始化对阵表中的公会信息
    if (m_stFightApplyList.m_stLastWinner.m_ullGuildId == 0)
    {
        memcpy(m_stFightAgainstList.m_astGuildList, m_stFightApplyList.m_astApplyList, sizeof(m_stFightAgainstList.m_astGuildList));
    }
    else
    {
        memcpy(&m_stFightAgainstList.m_astGuildList[0], &m_stFightApplyList.m_stLastWinner, sizeof(DT_GUILD_FIGHT_APPLY_INFO));
        memcpy(&m_stFightAgainstList.m_astGuildList[1], m_stFightApplyList.m_astApplyList, sizeof(DT_GUILD_FIGHT_APPLY_INFO)*(MAX_NUM_GUILD_FIGHT_JOIN_AGAINST-1));
    }

    //初始化对阵表
    DT_GUILD_FIGHT_ONE_AGAINST* pstAgainstList = m_stFightAgainstList.m_astAgainstList;
    DT_GUILD_FIGHT_APPLY_INFO* pstApplyList = m_stFightAgainstList.m_astGuildList;
    for(int i=m_iFightStage-1; i>=m_iFightStage/2; i--)
    {
        pstAgainstList[i].m_bFightStat = GUILD_FIGHT_AGAINST_IDLE;
        pstAgainstList[i].m_ullWinnerId = 0;
        pstAgainstList[i].m_bAgainstId = i;
    }

    //初始化对阵
    pstAgainstList[4].m_GuildList[0] = pstApplyList[0].m_ullGuildId;
    pstAgainstList[4].m_GuildList[1] = pstApplyList[7].m_ullGuildId;
    pstAgainstList[5].m_GuildList[0] = pstApplyList[3].m_ullGuildId;
    pstAgainstList[5].m_GuildList[1] = pstApplyList[4].m_ullGuildId;
    pstAgainstList[6].m_GuildList[0] = pstApplyList[1].m_ullGuildId;
    pstAgainstList[6].m_GuildList[1] = pstApplyList[6].m_ullGuildId;
    pstAgainstList[7].m_GuildList[0] = pstApplyList[2].m_ullGuildId;
    pstAgainstList[7].m_GuildList[1] = pstApplyList[5].m_ullGuildId;


    for(int i=m_iFightStage/2-1; i>=0; i--)
    {
        pstAgainstList[i].m_bAgainstId = i;
    }

    this->SendAgainstListSyn();
}


void GuildFightMgr::SendAgainstListSyn()
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_FIGHT_AGAINST_INFO_NTF;
    SS_PKG_GUILD_FIGHT_AGAINST_INFO_NTF& rstSsPkgBody = m_stSsPkg.m_stBody.m_stGuildFightAgainstInfoNtf;
    memcpy(&rstSsPkgBody.m_stAgainstInfo, &m_stFightAgainstList, sizeof(m_stFightAgainstList));
    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
}

void GuildFightMgr::SendApplyListSyn()
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_FIGHT_APPLY_LIST_NTF;
    SS_PKG_GUILD_FIGHT_APPLY_LIST_NTF& rstSsPkgBody = m_stSsPkg.m_stBody.m_stGuildFightApplyListNtf;
    memcpy(&rstSsPkgBody.m_stApplyListInfo, &m_stFightApplyList, sizeof(m_stFightApplyList));
    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
}

int GuildFightMgr::FightApply(Guild* poGuild, int iFund)
{
    assert(poGuild);

    if (m_iState != GUILD_FIGHT_STATE_APPLY_START)
    {
        return ERR_DEFAULT;
    }

    if (poGuild->GetGuildId() == m_stFightApplyList.m_stLastWinner.m_ullGuildId)
    {
        return ERR_DEFAULT;
    }

    this->RefreshApplyList(poGuild);
    this->SendApplyListSyn();

    return ERR_NONE;
}


//公会战开始后，发通知邮件
void GuildFightMgr::OnApplyEnd()
{
    uint8_t bAgainstCount = MAX_NUM_GUILD_FIGHT_JOIN_AGAINST; //参加军团战的公会数
    if (m_stFightApplyList.m_stLastWinner.m_ullGuildId != 0)
    {
        bAgainstCount--;
    }

    for (int i=0; i<m_stFightApplyList.m_bCount; i++)
    {
        Guild* poGuild = GuildMgr::Instance().GetGuild(m_stFightApplyList.m_astApplyList[i].m_ullGuildId);
        if (poGuild==NULL)
        {
            continue;
        }

        if (i<bAgainstCount)
        {
            poGuild->SendGuildMail(1);
        }
        else
        {
            poGuild->SendGuildMail(2);
        }
    }
}


void GuildFightMgr::FightSettle(DT_GUILD_FIGHT_ARENA_GUILD_INFO* stGuildInfoList, uint64_t ullWinnerId)
{

	if (m_iFightStage == GUILD_FIGHT_STAGE_FINALS)
	{
        //一二名发奖
		_SendReward(stGuildInfoList[0].m_ullGuildId, ullWinnerId);
		_SendReward(stGuildInfoList[1].m_ullGuildId, ullWinnerId);
	}
	else
	{
        //三四名发奖
		_SendReward(stGuildInfoList[0].m_ullGuildId, ullWinnerId, true);
		_SendReward(stGuildInfoList[1].m_ullGuildId, ullWinnerId, true);
	}
}


//保存军团战进度，在进程关闭或者进度发生变化时保存
bool GuildFightMgr::_SaveSchedule()
{
    assert(m_fp);

    fseek(m_fp, 0, SEEK_SET);
    //下次状态改变时间写入文件
    if(fwrite(&m_ullChgStateTime, sizeof(m_ullChgStateTime), 1, m_fp) != 1)
    {
        LOGERR_r("Write m_ullChgStateTime to file failed");
        return false;
    }
    LOGRUN_r("Write m_ullChgStateTime to file success");

    //状态写入文件
    if(fwrite(&m_iState, sizeof(m_iState), 1, m_fp) != 1)
    {
        LOGERR_r("Write m_iState to file failed");
        return false;
    }
    LOGRUN_r("Write m_iState to file success");

    //当前比赛阶段写入文件
    if(fwrite(&m_iFightStage, sizeof(m_iFightStage), 1, m_fp) != 1)
    {
        LOGERR_r("Write m_iFightStage to file failed");
        return false;
    }
    LOGRUN_r("Write m_iFightStage to file success");

    //报名表写入文件
    if(fwrite(&m_stFightApplyList, sizeof(m_stFightApplyList), 1, m_fp) != 1)
    {
        LOGERR_r("Write m_stFightApplyList to file failed");
        return false;
    }
    LOGRUN_r("Write m_stFightApplyList to file success");

    //对阵表写入文件
    if(fwrite(&m_stFightAgainstList, sizeof(m_stFightAgainstList), 1, m_fp) != 1)
    {
        LOGERR_r("Write m_stFightAgainstList to file failed");
        return false;
    }
    LOGRUN_r("Write m_stFightAgainstList to file success");

    fflush(m_fp);
    return true;
}


void GuildFightMgr::_SendReward(uint64_t ullGuildId, uint64_t ullWinnerId, bool bThird)
{
    if (ullGuildId == 0)
    {
        return;
    }

    Guild* poGuild = GuildMgr::Instance().GetGuild(ullGuildId);
    if (poGuild == NULL)
    {
        LOGERR_r("poGuild is NULL, Guild(%lu) not found", ullGuildId);
        return;
    }

    switch (m_iFightStage)
    {
    case GUILD_FIGHT_STAGE_QUARTERFINAL:
        if (ullGuildId == ullWinnerId)
        {
            poGuild->SendGuildMail(4);
        }
        else
        {
            poGuild->SendGuildMail(5);
            poGuild->SendGuildLeaderMail(14);
        }
        break;
    case GUILD_FIGHT_STAGE_SEMIFINALS:
        if (ullGuildId == ullWinnerId)
        {
            poGuild->SendGuildMail(6);
        }
        else
        {
            poGuild->SendGuildMail(7);
        }
        break;
    case GUILD_FIGHT_STAGE_FINALS:
        if (!bThird)
        {
            if (ullGuildId == ullWinnerId)
            {
                //冠军
                poGuild->SendGuildMail(10);
                poGuild->SendGuildLeaderMail(15);
            }
            else
            {
                //亚军
                poGuild->SendGuildMail(11);
                poGuild->SendGuildLeaderMail(16);
            }
        }
        else
        {
            if (ullGuildId == ullWinnerId)
            {
                //季军
                poGuild->SendGuildMail(12);
                poGuild->SendGuildLeaderMail(17);
            }
            else
            {
                //殿军
                poGuild->SendGuildMail(13);
                poGuild->SendGuildLeaderMail(18);
            }
        }
        break;
    default:
        LOGERR_r("FightStage is error");
        break;
    }
}


uint32_t GuildFightMgr::GetFightApplyFund(uint64_t ullGuildId)
{
    if (m_stFightApplyList.m_stLastWinner.m_ullGuildId == ullGuildId)
    {
        return 0;
    }

    for (uint8_t i=0; i<m_stFightApplyList.m_bCount && i<MAX_NUM_GUILD_FIGHT_JOIN_AGAINST; i++)
    {
        if (m_stFightApplyList.m_astApplyList[i].m_ullGuildId == ullGuildId)
        {
            return m_stFightApplyList.m_astApplyList[i].m_dwApplyFund;
        }
    }

    return 0;
}

