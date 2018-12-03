#include "Guild.h"
#include "utils/strutil.h"
#include "log/LogMacros.h"
#include "ss_proto.h"
#include "common_proto.h"
#include "../../gamedata/GameDataMgr.h"
#include "../Fight/GuildFightMgr.h"
#include "GuildRank.h"
#include "../Fight/GuildFightArena.h"
#include "../Fight/GFightArenaMgr.h"
#include "../../transaction/GuildTransFrame.h"
#include "../Player/PlayerMgr.h"
#include "GuildDataDynPool.h"
#include "../../GuildSvr.h"
#include "GuildBossMgr.h"
#include "GuildSociety.h"
#include "PKGMETA_metalib.h"
#include "utils/oi_misc.h"

using namespace PKGMETA;

Guild::Guild()
{
}

Guild::~Guild()
{
}

static int UinCmp(const void * pFirst, const void * pSecond)
{
    if (((DT_GUILD_BOSS_FIGHT_TIMES *)pFirst)->m_ullUin > ((DT_GUILD_BOSS_FIGHT_TIMES *)pSecond)->m_ullUin)
    {
        return 1;
    }
    else if (((DT_GUILD_BOSS_FIGHT_TIMES *)pFirst)->m_ullUin < ((DT_GUILD_BOSS_FIGHT_TIMES *)pSecond)->m_ullUin)
    {
        return -1;
    }
    return 0;
}

static int HangLogCmp(const void *pstFirst, const void *pstSecond)
{
    if (((DT_GUILD_HANG_LOG_INFO*)pstSecond)->m_ullTimestamp > ((DT_GUILD_HANG_LOG_INFO*)pstFirst)->m_ullTimestamp)
    {
        return 1;
    }
    else if (((DT_GUILD_HANG_LOG_INFO*)pstSecond)->m_ullTimestamp < ((DT_GUILD_HANG_LOG_INFO*)pstFirst)->m_ullTimestamp)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

static int StarUinCmp(const void *pstFirst, const void *pstSecond)
{
    if (*(uint64_t*)pstFirst > *(uint64_t*)pstSecond)
    {
        return 1;
    }
    else if (*(uint64_t*)pstFirst < *(uint64_t*)pstSecond)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

bool Guild::InitFromDB(DT_GUILD_WHOLE_DATA& rstGuildWholeData)
{
    m_iDeputyNum = 0;
	m_wUploadExpeditionCtrl = 0;
    memcpy(&m_stGuildBaseInfo, &rstGuildWholeData.m_stBaseInfo, sizeof(m_stGuildBaseInfo));

    uint64_t ullGuildId = m_stGuildBaseInfo.m_ullGuildId;

    uint16_t wVersion = m_stGuildBaseInfo.m_wVersion;
    size_t ulUseSize = 0;
    m_stGuildBaseInfo.m_wVersion = PKGMETA::MetaLib::getVersion();

    //unpack GuildGlobal
    int iRet = m_stGuildGlobalInfo.unpack((char*)rstGuildWholeData.m_stGlobalBlob.m_szData, sizeof(rstGuildWholeData.m_stGlobalBlob.m_szData), &ulUseSize, wVersion);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("Guild(%lu) unpack DT_GUILD_GLOBAL_BLOB failed, Ret=%d", ullGuildId, iRet);
        return false;
    }

    //bugfix
    ResGuildLevelMgr_t& rstResGuildLevelMgr = CGameDataMgr::Instance().GetResGuildLevelMgr();
    RESGUILDLEVEL* pstResGuildLevel = rstResGuildLevelMgr.Find(m_stGuildGlobalInfo.m_bGuildLevel);
    assert(pstResGuildLevel);
    m_stGuildGlobalInfo.m_wMaxMemberNum = pstResGuildLevel->m_dwMemberNum + this->m_oGuildSociety.GetValue(GUILD_SOCIETY_TYPE_MEMBER_NUM);
    m_stGuildGlobalInfo.m_wMaxEliteNum = pstResGuildLevel->m_dwMemberNum + this->m_oGuildSociety.GetValue(GUILD_SOCIETY_TYPE_ELITE_NUM);

    //unpack GuildMember
    if (!m_oGuildMember.InitFromDB(rstGuildWholeData.m_stMemberBlob, ullGuildId, (int)m_stGuildGlobalInfo.m_wMaxMemberNum, &m_iDeputyNum, &m_iEliteNum, wVersion))
    {
        return false;
    }

    //unpack GuildApply
    if (!m_oGuildApply.InitFromDB(rstGuildWholeData.m_stApplyBlob, ullGuildId, MAX_NUM_APPLY, wVersion))
    {
        return false;
    }

    //unpack GuildReplay
    if (!m_oGuildReplay.InitFromDB(rstGuildWholeData.m_stReplayBlob, ullGuildId, wVersion))
    {
        return false;
    }

    //unpack m_oGuildBoss
    if (!m_oGuildBoss.InitFromDB(rstGuildWholeData.m_stBossBlob, ullGuildId, wVersion))
    {
        return false;
    }

	//upack m_oGuildSociety
	if (!m_oGuildSociety.InitFromDB(rstGuildWholeData.m_stSocietyBlob, ullGuildId, wVersion))
	{
        return false;
	}

    //init GuildRoom
    if (!m_oGuildRoom.Init(ullGuildId))
    {
        return false;
    }

    //获取军团战当前状态
    GuildFightMgr::Instance().GetState(m_stFightState);

    m_bCanSwap = true;

    m_ullLastCheckTime = 0;

    return true;
}

bool Guild::NewInit()
{
    if (!m_oGuildBoss.NewInit())
    {
        return false;
    }

    if (!m_oGuildSociety.Init())
    {
        return false;
    }
	m_wUploadExpeditionCtrl = 0;
    GuildMgr::Instance().AddToDirtyList(this);

    return true;
}

bool Guild::PackGuildWholeData(OUT DT_GUILD_WHOLE_DATA& rstGuildWholeData, uint16_t wVersion)
{
    memcpy(&rstGuildWholeData.m_stBaseInfo, &m_stGuildBaseInfo, sizeof(m_stGuildBaseInfo));

    //pack GuildGlobal
    size_t ulUseSize = 0;
    int iRet = m_stGuildGlobalInfo.pack((char*)rstGuildWholeData.m_stGlobalBlob.m_szData, MAX_LEN_GUILD_GLOBAL, &ulUseSize, wVersion);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Guild(%lu) pack DT_GUILD_GLOBAL_BLOB failed, Ret=%d", GetGuildId(), iRet);
        return false;
    }
    rstGuildWholeData.m_stGlobalBlob.m_iLen = (int)ulUseSize;

    //pack GuildMember
    if (!m_oGuildMember.PackGuildMemberInfo(rstGuildWholeData.m_stMemberBlob, wVersion))
    {
        return false;
    }

    //pack GuildApply
    if (!m_oGuildApply.PackGuildApplyInfo(rstGuildWholeData.m_stApplyBlob, wVersion))
    {
        return false;
    }

    //pack GuildReplay
    if (!m_oGuildReplay.PackGuildReplayInfo(rstGuildWholeData.m_stReplayBlob, wVersion))
    {
        return false;
    }

    //pack GuildBoss
    if (!m_oGuildBoss.PackGuildBossInfo(rstGuildWholeData.m_stBossBlob, wVersion))
    {
        return false;
    }

	//pack GuildSociety
	if (!m_oGuildSociety.PackGuildSocietyInfo(rstGuildWholeData.m_stSocietyBlob, wVersion))
	{
		return false;
	}

    return true;
}


void Guild::GetGuildExpeditionInfo(OUT DT_GUILD_EXPEDITION_GUILD_UPLOAD_INFO& rstInfo)
{

	rstInfo.m_dwAddr = GuildSvr::Instance().GetConfig().m_iProcID;
	rstInfo.m_ullGuildId = m_stGuildBaseInfo.m_ullGuildId;
	StrCpy(rstInfo.m_szName, m_stGuildBaseInfo.m_szName, sizeof(rstInfo.m_szName));
	//m_oGuildMember.GetGuildExpeditionMemberInfo(rstInfo.m_chMemberCount, rstInfo.m_astMemberList);
}

void Guild::Update(uint64_t ullBossUptTime, uint64_t ullBossSingleUptTime, uint64_t ullFundUptTime)
{
    //军团战状态发生变化时，发通知
    uint8_t bState = GuildFightMgr::Instance().GetState();
    if (bState != m_stFightState.m_bState)
    {
        GuildFightMgr::Instance().GetState(m_stFightState);
        _SendFightStateChgNtf();
    }

    //军团战自动报名
    this->FightApply();

    //申请列表需要刷新时，刷新申请列表
    uint64_t ullTimestamp = PlayerMgr::Instance().GetUptTimestamp();
    if (m_oGuildApply.GetUptTimeStamp() < ullTimestamp)
    {
        m_oGuildApply.Clear();
        m_oGuildApply.SetUptTimeStamp(ullTimestamp);
#if 0
        //发送清空申请列表的广播
        DT_ONE_GUILD_MEMBER stOneApply;
        stOneApply.m_ullUin = 0;
        this->_SendUptGuildApplyNtf(0, stOneApply);
#endif
        //数据变化，需要回写数据库
        GuildMgr::Instance().AddToDirtyList(this);
    }

    //  公会Boss刷新
    DT_GUILD_BOSS_INFO& oGuildBossInfo = m_oGuildBoss.GetBossInfo();
    PKGMETA::SSPKG m_stSsPkg;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_BOSS_GET_RANK_LIST_REQ;
    m_stSsPkg.m_stBody.m_stGuildBossGetRankListReq.m_bType = RANK_TYPE_GUILD;
    uint32_t dwCurBossId = m_oGuildBoss.GetBossInfo().m_dwCurBossId;
    if (oGuildBossInfo.m_ullLastUptTime < ullBossUptTime)
    {
        m_oGuildBoss.SendDamageRankRwd(dwCurBossId);
        m_oGuildBoss.SendSubsectionRwd(dwCurBossId);
        m_oGuildBoss.Reset();
        oGuildBossInfo.m_ullLastUptTime = ullBossUptTime;
        oGuildBossInfo.m_bResetNum = 0;
        LOGRUN_r("GuildBoss sys reset Gid<%lu> ", m_stGuildBaseInfo.m_ullGuildId);
        GuildSvrMsgLayer::Instance().SendToRankSvr(m_stSsPkg);
        _SendGuildBossNtf();
    }
    else if (oGuildBossInfo.m_ullLastSingleUptTime < ullBossSingleUptTime)
    {
        m_oGuildBoss.SendDamageRankRwd(dwCurBossId);
        m_oGuildBoss.SendSubsectionRwd(dwCurBossId);
        m_oGuildBoss.SingleReset();
        oGuildBossInfo.m_ullLastSingleUptTime = ullBossSingleUptTime;
        LOGRUN_r("GuildBoss Single reset Gid<%lu> ", m_stGuildBaseInfo.m_ullGuildId);
        GuildSvrMsgLayer::Instance().SendToRankSvr(m_stSsPkg);
        _SendGuildBossNtf();
    }

    this->_UpdateReplay();

	//捐献军团资金每日上限 以及成员每日捐献记录刷新
	if (m_stGuildGlobalInfo.m_ullTimeStampFund < ullFundUptTime)
	{
		m_stGuildGlobalInfo.m_dwFundOneDay = 0;
		m_stGuildGlobalInfo.m_ullTimeStampFund = ullFundUptTime;
		LOGRUN_r("GuildFund per day reset Gid<%lu> ", m_stGuildBaseInfo.m_ullGuildId);

		//成员每日贡献置零 已加速列表清零
		for (int i=0; i<MAX_NUM_MEMBER; i++)
		{
			uint64_t ullUin = m_oGuildMember.GetMemberList()[i];
			DT_ONE_GUILD_MEMBER* pstMemInfo = FindMember(ullUin);
			if (pstMemInfo==NULL)
			{
				continue;
			}

			pstMemInfo->m_dwFundToday = 0;
			pstMemInfo->m_bSalaryIdentityToday = pstMemInfo->m_bIdentity;

            pstMemInfo->m_stHangSpeedPartnerInfo.m_bCount = 0;
            pstMemInfo->m_stHangBeSpeededInfo.m_bCount = 0;
			pstMemInfo->m_stHangBeSpeededInfo.m_bSpeededToday = 0;
		}
        m_oGuildBoss.ResetMemFightTimesToZero();
	}
    m_oGuildRoom.Update();

    this->_CheckLeader();
}


void Guild::_CheckLeader()
{
    if (CGameTime::Instance().GetCurrSecond() - m_ullLastCheckTime < CHECK_TIMEVAL)
    {
        return;
    }

    m_ullLastCheckTime = CGameTime::Instance().GetCurrSecond();

    DT_ONE_GUILD_MEMBER* pstOneMember = m_oGuildMember.Find(m_stGuildGlobalInfo.m_ullLeaderUin);
    if (!pstOneMember)
    {
		return;
    }

    RESBASIC* pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(9403);
    assert(pResBasic);

    uint32_t dwTimeLimit = (uint32_t)pResBasic->m_para[0];
    if (CGameTime::Instance().GetCurrSecond() - (pstOneMember->m_ullLastLogoutTimeStap/1000) > dwTimeLimit)
    {
        DT_ONE_GUILD_MEMBER* pstLeader = m_oGuildMember.GetNextLeader();
        if (!pstLeader)
        {
            return;
        }

        SetGuildMemJob(pstOneMember, pstLeader->m_ullUin, GUILD_IDENTITY_LEADER);
    }
}

void Guild::Clear()
{
    m_oGuildApply.Clear();
    m_oGuildMember.Clear();
    m_oGuildReplay.Clear();
    m_oGuildRoom.Clear();
    m_oGuildBoss.Clear();
	m_oGuildSociety.Clear();
	m_wUploadExpeditionCtrl = 0;
}

int Guild::AddMember(DT_ONE_GUILD_MEMBER& rstOneMember, DT_ONE_GUILD_MEMBER* pstOneOfficer)
{
    int iRet = m_oGuildMember.Add(rstOneMember);

    if (ERR_NONE == iRet)
    {
        //添加成员后更新公会排名信息
        //GuildRankMgr::Instance().UpdateRank(this->GetGuildRankInfo());
		GuildRankMgr::Instance().UpdateGuildRankNtf(this, this->GetGuildRankInfo());

        //发送玩家加入的广播
        this->_SendUptGuildMemberNtf(GUILD_JOB_OP_TYPE_ADD, rstOneMember, pstOneOfficer);

		//发送加入公会时间公告
		this->_SendGuildJoinOrLeaveEventNtf(rstOneMember, GUILD_EVENT_JOIN);

        //将新加入成员信息写入GuildBoss战斗次数中
        if (this->GetGuildMemInfo().GetCurMemberNum() > 0 && this->GetGuildMemInfo().GetCurMemberNum() < MAX_GUILD_MEMBER_MAX_NUM)
        {
            size_t nMemNum = this->GetGuildMemInfo().GetCurMemberNum() - 1;
            iRet = m_oGuildBoss.AddMemFightTimesInfo(nMemNum, rstOneMember.m_ullUin);
        }
        //数据变化，需要回写数据库
        GuildMgr::Instance().AddToDirtyList(this);
    }

    return iRet;
}

int Guild::DelMember(uint64_t ullUin)
{
    //判断能否删除
    DT_ONE_GUILD_MEMBER* pstMemberInfo = m_oGuildMember.Find(ullUin);
    if (pstMemberInfo == NULL)
    {
        return ERR_NOT_FOUND_MEMBER;
    }
    if (pstMemberInfo->m_bIdentity == GUILD_IDENTITY_LEADER)
    {
        return ERR_LEADER_CANT_QUIT;
    }

    //发送玩家退出的广播(先发广播,后删成员,否则被删除的成员不会收到广播,
    //由于前面已经进行了判断，因此删除操作一定是可以成功的)!!!!!!!!
    this->_SendUptGuildMemberNtf(GUILD_JOB_OP_TYPE_DEL, *pstMemberInfo);

    //删除副会长
    if (pstMemberInfo->m_bIdentity == GUILD_IDENTITY_DEPUTY_LEADER)
    {
        m_iDeputyNum--;
    }
	if (pstMemberInfo->m_bIdentity == GUILD_IDENTITY_ELDERS)
	{
		m_iEliteNum--;
	}

    m_oGuildMember.Del(ullUin);

    //删除成员后更新公会排名信息
	GuildRankMgr::Instance().UpdateGuildRankNtf(this, this->GetGuildRankInfo());

    //删除玩家在公会中的Boss战斗次数
    m_oGuildBoss.DelMemFightTimesInfo(ullUin);
	//发送离开公告
	this->_SendGuildJoinOrLeaveEventNtf(*pstMemberInfo, GUILD_EVENT_LEAVE);

    //数据变化，需要回写数据库
    GuildMgr::Instance().AddToDirtyList(this);

    return ERR_NONE;
}

DT_ONE_GUILD_MEMBER* Guild::FindMember(uint64_t ullUin)
{
    return m_oGuildMember.Find(ullUin);
}

int Guild::AddApply(DT_ONE_GUILD_MEMBER& rstOneApply)
{
    int iRet = m_oGuildApply.Add(rstOneApply);

    if (iRet == ERR_NONE)
    {
        //发送新申请的广播
        this->_SendUptGuildApplyNtf(1, rstOneApply);

        //数据变化，需要回写数据库
        GuildMgr::Instance().AddToDirtyList(this);
    }

    return iRet;
}

int Guild::DelApply(uint64_t ullUin)
{
    int iRet = m_oGuildApply.Del(ullUin);

    if (iRet == ERR_NONE)
    {
        //发送删除申请的广播
        DT_ONE_GUILD_MEMBER stOneApply;
        stOneApply.m_ullUin = ullUin;
        this->_SendUptGuildApplyNtf(0, stOneApply);

        //数据变化，需要回写数据库
        GuildMgr::Instance().AddToDirtyList(this);
    }

    return iRet;
}

void Guild::SetNeedApply(uint8_t bNeedApply)
{
    m_stGuildGlobalInfo.m_bNeedApply = bNeedApply;

    //数据变化，需要回写数据库
    GuildMgr::Instance().AddToDirtyList(this);

}

DT_ONE_GUILD_MEMBER* Guild::FindApply(uint64_t ullUin)
{
    return m_oGuildApply.Find(ullUin);
}

int Guild::RefreshMemberInfo(SS_PKG_REFRESH_MEMBERINFO_NTF& rstRefreshNtf)
{
    DT_ONE_GUILD_MEMBER* pstOneMember = m_oGuildMember.Find(rstRefreshNtf.m_ullUin);
    if (pstOneMember==NULL)
    {
        LOGERR_r("Refresh MemberInfo failed, Player(%lu) is not found in Guild(%s)(%lu)", rstRefreshNtf.m_ullUin, GetGuildName(), GetGuildId());
        return ERR_NOT_FOUND;
    }

    //活跃度不为0，为活跃度更新，不用更新其他信息
    if (rstRefreshNtf.m_stPlayerInfo.m_dwGuildVitality != 0)
    {
        return AddVitality(rstRefreshNtf.m_stPlayerInfo.m_dwGuildVitality, pstOneMember);
    }

    //更新玩家信息
    uint8_t bOldLi = pstOneMember->m_stExtraInfo.m_dwLi;
    uint32_t dwVitality = pstOneMember->m_stExtraInfo.m_dwGuildVitality;
    pstOneMember->m_stExtraInfo = rstRefreshNtf.m_stPlayerInfo;
    pstOneMember->m_stExtraInfo.m_dwGuildVitality = dwVitality;
	StrCpy(pstOneMember->m_szName, rstRefreshNtf.m_szName, PKGMETA::MAX_NAME_LENGTH);

    //检查星级变化,变化更新排行榜
    if (pstOneMember->m_stExtraInfo.m_dwLi != bOldLi)
    {
		GuildRankMgr::Instance().UpdateGuildRankNtf(this, this->GetGuildRankInfo());
    }

    //登出
    if (rstRefreshNtf.m_stPlayerInfo.m_bIsOnline == 0)
    {
        //置登出时间
        pstOneMember->m_ullLastLogoutTimeStap = CGameTime::Instance().GetCurrTimeMs();

        //如果在军团战中登出,需要踢出战场
        if (m_stFightState.m_bState==GUILD_FIGHT_STATE_FIGHT_START ||m_stFightState.m_bState==GUILD_FIGHT_STATE_FIGHT_PREPARE)
        {
            GuildFightArena* poArena = GFightArenaMgr::Instance().GetArenaByGuildId(this->GetGuildId());
            if (poArena != NULL)
            {
                poArena->Quit(rstRefreshNtf.m_ullUin);
            }
        }
    }

    //发送玩家信息更新的广播消息
    this->_SendUptGuildMemberNtf(GUILD_JOB_OP_TYPE_EXTRA, *pstOneMember);

    //数据变化，需要回写数据库
    GuildMgr::Instance().AddToDirtyList(this);

    return ERR_NONE;
}

int Guild::RefreshRoomInfo(SS_PKG_REFRESH_GUILD_PVP_ROOM_NTF& rstRefreshNtf)
{
    int iRet = ERR_NONE;
    uint8_t bIsAdd = 0;

    if (rstRefreshNtf.m_stRoomInfo.m_bPlayerCount == 0)
    {
        iRet = m_oGuildRoom.DelRoom(rstRefreshNtf.m_stRoomInfo);
    }
    else
    {
        bIsAdd = 1;
        iRet = m_oGuildRoom.UpdateRoom(rstRefreshNtf.m_stRoomInfo);
    }

    if (iRet == ERR_NONE)
    {
        this->_SendUptGuildRoomNtf(bIsAdd, rstRefreshNtf.m_stRoomInfo);
    }

    return iRet;
}

void Guild::GetGuildRoomInfo(DT_GUILD_ROOM_INFO& rstGuildRoom)
{
    m_oGuildRoom.GetGuildRoomInfo(rstGuildRoom);
}

int Guild::UptGuildNotice(const char* pszNotice)
{
    if ((NULL == pszNotice) || (pszNotice[0] == '\0'))
    {
        return ERR_SYS;
    }

    StrCpy(m_stGuildGlobalInfo.m_szNotice, pszNotice, MAX_MSG_LENGTH);

    //将公告变更广播给公会成员
    this->_SendUptGuildGlobalNtf();

    //数据变化，需要回写数据库
    GuildMgr::Instance().AddToDirtyList(this);

    return ERR_NONE;
}

int Guild::SetGuildMemJob(DT_ONE_GUILD_MEMBER* pstOfficer, uint64_t ullMemberId, uint8_t bJob)
{
    DT_ONE_GUILD_MEMBER* pstMemberInfo = m_oGuildMember.Find(ullMemberId);
    if (pstMemberInfo==NULL)
    {
        return ERR_DEFAULT;
    }

	uint8_t bOldIdentity = pstMemberInfo->m_bIdentity;

	if (bJob == pstMemberInfo->m_bIdentity)
	{
		return ERR_DEFAULT;
	}

	if (pstOfficer->m_bIdentity != GUILD_IDENTITY_LEADER)
	{
		return ERR_GUILD_PERMISSION_DENIED;
	}

	//会长移交
	if (pstOfficer->m_bIdentity == GUILD_IDENTITY_LEADER &&
		bJob == GUILD_IDENTITY_LEADER)
	{
		pstOfficer->m_bIdentity = pstMemberInfo->m_bIdentity;
		pstMemberInfo->m_bIdentity = GUILD_IDENTITY_LEADER;

        m_stGuildGlobalInfo.m_ullLeaderUin = ullMemberId;
        this->_SendUptGuildGlobalNtf();
	}
    //由其他职位升为副会长
    else if (pstMemberInfo->m_bIdentity > GUILD_IDENTITY_DEPUTY_LEADER && bJob == GUILD_IDENTITY_DEPUTY_LEADER)
    {
        if (m_iDeputyNum >= this->_GetTitleNum(GUILD_IDENTITY_DEPUTY_LEADER))
        {
            return ERR_MAX_DEPUTY_LEADER;
        }
        else
        {
            m_iDeputyNum++;
        }

		//若本职为长老
		if (pstMemberInfo->m_bIdentity == GUILD_IDENTITY_ELDERS)
		{
			m_iEliteNum--;
		}
    }

    //由副会长降为其他职位
    else if (pstMemberInfo->m_bIdentity == GUILD_IDENTITY_DEPUTY_LEADER && bJob > GUILD_IDENTITY_DEPUTY_LEADER)
    {
		//若降为长老
		if (bJob == GUILD_IDENTITY_ELDERS)
		{
			if (m_iEliteNum >= this->_GetTitleNum(GUILD_IDENTITY_ELDERS))
			{
				LOGRUN("m_iEliteNum<%d>, this->_GetTitleNum(GUILD_IDENTITY_ELDERS)<%d>", m_iEliteNum, this->_GetTitleNum(GUILD_IDENTITY_ELDERS));
				return ERR_GUILD_MAX_ELITE;
			}
			else
			{
				m_iEliteNum++;
			}
		}

		m_iDeputyNum--;
    }

	//由长老降为其他职位
	else if (pstMemberInfo->m_bIdentity == GUILD_IDENTITY_ELDERS && bJob > GUILD_IDENTITY_ELDERS)
	{
		m_iEliteNum--;
	}

	//升为长老
	else if (pstMemberInfo->m_bIdentity > GUILD_IDENTITY_ELDERS && bJob == GUILD_IDENTITY_ELDERS)
	{
		if (m_iEliteNum >= this->_GetTitleNum(GUILD_IDENTITY_ELDERS))
		{
			LOGRUN("m_iEliteNum<%d>, this->_GetTitleNum(GUILD_IDENTITY_ELDERS)<%d>", m_iEliteNum, this->_GetTitleNum(GUILD_IDENTITY_ELDERS));
			return ERR_GUILD_MAX_ELITE;
		}
		else
		{
			m_iEliteNum++;
		}
	}

    pstMemberInfo->m_bIdentity = bJob;

	uint8_t bUptType;
	if (bJob == GUILD_IDENTITY_LEADER)
	{
		bUptType = GUILD_JOB_OP_TYPE_TOP;
	}
	else if (bJob < bOldIdentity)
	{
		bUptType = GUILD_JOB_OP_TYPE_UP;
	}
	else
	{
		bUptType = GUILD_JOB_OP_TYPE_DOWN;
	}

    //将晋升/降职的消息广播给公会内所有在线玩家
    this->_SendUptGuildMemberNtf(bUptType, *pstMemberInfo, pstOfficer);

	//发送降升职公告
	this->_SendGuildIdentityChangeNtf(*pstMemberInfo, bUptType);

    //数据变化，需要回写数据库
    GuildMgr::Instance().AddToDirtyList(this);

    return ERR_NONE;
}

uint8_t Guild::GetGuildMemJob(uint64_t ullMemberId)
{
    DT_ONE_GUILD_MEMBER* pstMemberInfo = m_oGuildMember.Find(ullMemberId);
    if (pstMemberInfo==NULL)
    {
        return 0;
    }
    return pstMemberInfo->m_bIdentity;
}

int Guild::KickGuildMem(DT_ONE_GUILD_MEMBER* pstOfficer, uint64_t ullMemberId)
{
	DT_ONE_GUILD_MEMBER* pstMember = FindMember(ullMemberId);
	if (pstMember==NULL)
	{
		return ERR_GUILD_PERMISSION_DENIED;
	}

	if (pstOfficer->m_bIdentity >= pstMember->m_bIdentity)
	{
		return ERR_GUILD_PERMISSION_DENIED;
	}

	if (pstOfficer->m_bIdentity == GUILD_IDENTITY_ELDERS &&
		pstOfficer->m_bKickCount >= MAX_ELITE_KICK_TIMES)
	{
		return ERR_MAX_KICK_TIME;
	}


	int iRet = ERR_NONE;

	iRet = DelMember(ullMemberId);
	if (iRet == ERR_NONE && pstOfficer->m_bIdentity == GUILD_IDENTITY_ELDERS)
	{
		pstOfficer->m_bKickCount++;
	}
	//通知远征服务器,某人退出公会
	SendGuildStateToExpeditionSvr(2, ullMemberId);
	return iRet;
}


int Guild::Dissolve()
{
    this->_SendGuildDissolveNtf();
	//通知远征服务器,公会解散
	SendGuildStateToExpeditionSvr(1, 0);
    //公会解散后，需要将全体公会成员的GuildID清空，否则玩家申请不了新的公会
    uint64_t* pMemberList = m_oGuildMember.GetMemberList();
    for (int i=0; i<m_oGuildMember.GetCurMemberNum(); i++)
    {
        GuildTransFrame::Instance().AddRstPlayerTrans(pMemberList[i]);
    }

    GuildMgr::Instance().DeleteGuild(GetGuildId());

    return ERR_NONE;
}

int Guild::AddFund(int iFund, DT_GUILD_DONATE_INFO* pstDonateInfo/*=NULL*/)
{
	ResGuildLevelMgr_t& rstResGuildLevelMgr = CGameDataMgr::Instance().GetResGuildLevelMgr();
	RESGUILDLEVEL* pstResGuildLevel = rstResGuildLevelMgr.Find(m_stGuildGlobalInfo.m_bGuildLevel);
	if (!pstResGuildLevel)
	{
		LOGERR("pstResGuildLevel is null. line<%d>", m_stGuildGlobalInfo.m_bGuildLevel);
		return ERR_SYS;
	}

	bool bFundFullPerDay = false;
	if (pstResGuildLevel->m_dwFundLimit <= m_stGuildGlobalInfo.m_dwFundOneDay)
	{
		bFundFullPerDay = true;
	}

	if (pstDonateInfo!=NULL && bFundFullPerDay)
	{
		//公会资金没有变动，发广播只为全团广播捐献事件
		this->_SendUptGuildGlobalNtf(GUILD_CHANGE_TYPE_DONATE, pstDonateInfo);

		return ERR_NONE;
	}

	//每日捐献上限
	if (iFund > 0 && pstDonateInfo!=NULL)
	{
		m_stGuildGlobalInfo.m_dwFundOneDay += iFund;
		if (pstResGuildLevel->m_dwFundLimit <= m_stGuildGlobalInfo.m_dwFundOneDay)
		{
			iFund -= (m_stGuildGlobalInfo.m_dwFundOneDay - pstResGuildLevel->m_dwFundLimit);
			m_stGuildGlobalInfo.m_dwFundOneDay = pstResGuildLevel->m_dwFundLimit;
		}

	}

    int iTempFund = (int)m_stGuildGlobalInfo.m_dwGuildFund + iFund;
    if (iTempFund < 0)
    {
       iTempFund = 0;
    }
    m_stGuildGlobalInfo.m_dwGuildFund = (uint32_t)iTempFund;

	//自动升级
	this->_LevelUp();

    //将资金变更广播给公会成员
	if (pstDonateInfo==NULL)
	{
		this->_SendUptGuildGlobalNtf();
	}
	else
	{
		this->_SendUptGuildGlobalNtf(GUILD_CHANGE_TYPE_DONATE, pstDonateInfo);
	}

    //数据变化，需要回写数据库
    GuildMgr::Instance().AddToDirtyList(this);

    return ERR_NONE;
}

int Guild::AddVitality(int iVitality, DT_ONE_GUILD_MEMBER* pstOneMember)
{
    //这里不用同步整体信息，只需要同步单个玩家的变化
    m_stGuildGlobalInfo.m_dwGuildVitality += iVitality;

    pstOneMember->m_stExtraInfo.m_dwGuildVitality += iVitality;

    this->_SendUptGuildMemberNtf(GUILD_JOB_OP_TYPE_EXTRA, *pstOneMember);

    return 0;
}

void Guild::SendVitalityReward()
{
    m_oGuildMember.SendVitalityReward();
}

int Guild::AddSocietyExp(uint8_t bType, int iExp, DT_GUILD_DONATE_INFO* pstDonateInfo/*=NULL*/)
{
	if (bType < 1 || bType>MAX_GUILD_SOCIETY_TYPE)
	{
		return ERR_WRONG_PARA;
	}

	if (m_oGuildSociety.IsLvMax(bType))
	{
		return ERR_TOP_LEVEL;
	}


	uint8_t bOldLv = m_oGuildSociety.GetLv(bType);

	int iRet = m_oGuildSociety.AddExp(bType, iExp);

	if (bOldLv != m_oGuildSociety.GetLv(bType))
	{
		ResGuildLevelMgr_t& rstResGuildLevelMgr = CGameDataMgr::Instance().GetResGuildLevelMgr();
		RESGUILDLEVEL* pstResGuildLevel = rstResGuildLevelMgr.Find(m_stGuildGlobalInfo.m_bGuildLevel);
		assert(pstResGuildLevel);

		switch (bType)
		{
		case GUILD_SOCIETY_TYPE_MEMBER_NUM:
			m_stGuildGlobalInfo.m_wMaxMemberNum = pstResGuildLevel->m_dwMemberNum + this->m_oGuildSociety.GetValue(bType);
			break;
		case GUILD_SOCIETY_TYPE_ELITE_NUM:
			m_stGuildGlobalInfo.m_wMaxEliteNum = pstResGuildLevel->m_dwElite + this->m_oGuildSociety.GetValue(bType);
			break;
		default:
			break;
		}
	}


	if (iRet!=ERR_NONE)
	{
		return iRet;
	}


	//将科技变更广播给公会成员
	this->_SendGuildSocietyNtf(bType, pstDonateInfo);

	//数据变化，需要回写数据库
	GuildMgr::Instance().AddToDirtyList(this);

	return iRet;
}

int Guild::LevelUp()
{
    if (m_stGuildGlobalInfo.m_bGuildLevel >= MAX_GUILD_LEVEL)
    {
        return ERR_TOP_LEVEL;
    }
    ResGuildLevelMgr_t& rstResGuildLevelMgr = CGameDataMgr::Instance().GetResGuildLevelMgr();
    RESGUILDLEVEL* pstResGuildLevel = rstResGuildLevelMgr.Find(m_stGuildGlobalInfo.m_bGuildLevel);
    assert(pstResGuildLevel);

    if (m_stGuildGlobalInfo.m_dwGuildFund < pstResGuildLevel->m_dwCost)
    {
        return ERR_NOT_ENOUGH_FUND;
    }

    m_stGuildGlobalInfo.m_dwGuildFund -= pstResGuildLevel->m_dwCost;
    m_stGuildGlobalInfo.m_bGuildLevel++;

    pstResGuildLevel = rstResGuildLevelMgr.Find(m_stGuildGlobalInfo.m_bGuildLevel);
    assert(pstResGuildLevel);
    m_stGuildGlobalInfo.m_wMaxMemberNum = pstResGuildLevel->m_dwMemberNum + this->m_oGuildSociety.GetValue(GUILD_SOCIETY_TYPE_MEMBER_NUM);
	m_stGuildGlobalInfo.m_wMaxEliteNum = pstResGuildLevel->m_dwElite + this->m_oGuildSociety.GetValue(GUILD_SOCIETY_TYPE_ELITE_NUM);
	m_stGuildGlobalInfo.m_wMaxViceLeaderNum = pstResGuildLevel->m_dwViceLeaderNum;
    //提升人数上限
    m_oGuildMember.SetMaxMemberNum(pstResGuildLevel->m_dwMemberNum + this->m_oGuildSociety.GetValue(GUILD_SOCIETY_TYPE_MEMBER_NUM));

    //将等级变更广播给公会成员
    this->_SendUptGuildGlobalNtf();

    //数据变化，需要回写数据库
    GuildMgr::Instance().AddToDirtyList(this);

    return ERR_NONE;
}

void Guild::_LevelUp()
{
	if (m_stGuildGlobalInfo.m_bGuildLevel >= MAX_GUILD_LEVEL)
	{
		return;
	}
	ResGuildLevelMgr_t& rstResGuildLevelMgr = CGameDataMgr::Instance().GetResGuildLevelMgr();
	RESGUILDLEVEL* pstResGuildLevel = rstResGuildLevelMgr.Find(m_stGuildGlobalInfo.m_bGuildLevel);
	assert(pstResGuildLevel);

	if (m_stGuildGlobalInfo.m_dwGuildFund < pstResGuildLevel->m_dwCost)
	{
		return;
	}

	m_stGuildGlobalInfo.m_dwGuildFund -= pstResGuildLevel->m_dwCost;
	m_stGuildGlobalInfo.m_bGuildLevel++;

	pstResGuildLevel = rstResGuildLevelMgr.Find(m_stGuildGlobalInfo.m_bGuildLevel);
	assert(pstResGuildLevel);

	m_stGuildGlobalInfo.m_wMaxMemberNum = pstResGuildLevel->m_dwMemberNum + this->m_oGuildSociety.GetValue(GUILD_SOCIETY_TYPE_MEMBER_NUM);
	m_stGuildGlobalInfo.m_wMaxEliteNum = pstResGuildLevel->m_dwElite + this->m_oGuildSociety.GetValue(GUILD_SOCIETY_TYPE_ELITE_NUM);
	m_stGuildGlobalInfo.m_wMaxViceLeaderNum = pstResGuildLevel->m_dwViceLeaderNum;

	//提升人数上限
	m_oGuildMember.SetMaxMemberNum(m_stGuildGlobalInfo.m_wMaxMemberNum);

	//广播与数据回写同AddFund()一起处理
	return;
}

int Guild::FightApply()
{
    if (m_stFightState.m_bState != GUILD_FIGHT_STATE_APPLY_START &&
        m_stGuildGlobalInfo.m_dwFightApplyFund != 0)
    {
        this->ResetFightApplyFund();
    }

    if (m_stFightState.m_bState == GUILD_FIGHT_STATE_APPLY_START &&
        m_stGuildGlobalInfo.m_dwFightApplyFund != m_stGuildGlobalInfo.m_dwGuildVitality)
    {
        m_stGuildGlobalInfo.m_dwFightApplyFund = m_stGuildGlobalInfo.m_dwGuildVitality;
        GuildFightMgr::Instance().FightApply(this, m_stGuildGlobalInfo.m_dwGuildVitality);
    }

    return ERR_NONE;
}

void Guild::ResetFightApplyFund()
{
    m_stGuildGlobalInfo.m_dwFightApplyFund = 0;
    m_stGuildGlobalInfo.m_dwGuildVitality = 0;

    //清空活跃度同步到客户端
    this->_SendUptGuildGlobalNtf();

    m_oGuildMember.SendVitalityReward();
    //清空玩家的活跃度
    m_oGuildMember.ClearVitality();

    //数据变化，需要回写数据库
    GuildMgr::Instance().AddToDirtyList(this);

    return;
}

void Guild::SendBroadcastMsg(DT_GUILD_NTF_MSG& rstGuilNtfMsg, uint16_t MsgId)
{
    SSPKG stSsPkg;
    stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_BROADCAST_NTF;
    SS_PKG_GUILD_BROADCAST_NTF& rstGuildBroadCastNtf = stSsPkg.m_stBody.m_stGuildBroadcastNtf;
    rstGuildBroadCastNtf.m_wMsgId = MsgId;
    rstGuildBroadCastNtf.m_ullGuildId = this->GetGuildId();
    memcpy(&rstGuildBroadCastNtf.m_stNtfMsg, &rstGuilNtfMsg, sizeof(DT_GUILD_NTF_MSG));

    rstGuildBroadCastNtf.m_bPlayerCount = m_oGuildMember.GetCurMemberNum();
    uint64_t* pMemberList = m_oGuildMember.GetMemberList();
    for (int i=0; i<rstGuildBroadCastNtf.m_bPlayerCount; i++)
    {
        rstGuildBroadCastNtf.m_PlayerList[i] = pMemberList[i];
    }

    GuildSvrMsgLayer::Instance().SendToZoneSvr(stSsPkg);
}


void Guild::_SendUptGuildGlobalNtf(uint8_t bChangeType/* = GUILD_CHANGE_TYPE_DEFAULT*/, DT_GUILD_DONATE_INFO* pstDonateInfo/*=NULL*/)
{
    DT_GUILD_NTF_MSG stGuildNtfMsg;
    DT_GUILD_PKG_UPDATE_GUILD_GLOBAL& rstUptGlobalInfo = stGuildNtfMsg.m_stUptGuildGlobal;
	rstUptGlobalInfo.m_bChangeType = bChangeType;
    memcpy(&rstUptGlobalInfo.m_stGlobalInfo, &m_stGuildGlobalInfo, sizeof(DT_GUILD_GLOBAL_INFO));
	if (bChangeType==GUILD_CHANGE_TYPE_DONATE &&
		pstDonateInfo!=NULL)
	{
		memcpy(&rstUptGlobalInfo.m_stDonateInfo, pstDonateInfo, sizeof(DT_GUILD_DONATE_INFO));
	}
	else
	{
		bzero(&rstUptGlobalInfo.m_stDonateInfo, sizeof(DT_GUILD_DONATE_INFO));
	}


    this->SendBroadcastMsg(stGuildNtfMsg, DT_GUILD_MSG_UPDATE_GUILD_GLOBAL);
}

void Guild::_SendUptGuildMemberNtf(uint8_t bIsAdd, DT_ONE_GUILD_MEMBER& rstOneMember, DT_ONE_GUILD_MEMBER* pstOneMemberOfficer)
{
    //发送公会成员更新的广播
    DT_GUILD_NTF_MSG stGuildNtfMsg;
    DT_GUILD_PKG_UPDATE_GUILD_MEMBER& rstUptMember = stGuildNtfMsg.m_stUptGuildMember;
    rstUptMember.m_bIsAdd = bIsAdd;
    if (bIsAdd == GUILD_JOB_OP_TYPE_DEL)
    {
        rstUptMember.m_stMemberInfo.m_ullUin = rstOneMember.m_ullUin;
    }
	else
    {
        memcpy(&rstUptMember.m_stMemberInfo, &rstOneMember, sizeof(DT_ONE_GUILD_MEMBER));
    }

	if (bIsAdd == GUILD_JOB_OP_TYPE_TOP || ((bIsAdd == GUILD_JOB_OP_TYPE_ADD) && pstOneMemberOfficer != NULL) )
	{
		memcpy(&rstUptMember.m_stOfficerInfo, pstOneMemberOfficer, sizeof(DT_ONE_GUILD_MEMBER));
	}
	else
	{
		bzero(&rstUptMember.m_stOfficerInfo, sizeof(DT_ONE_GUILD_MEMBER));
	}

    this->SendBroadcastMsg(stGuildNtfMsg, DT_GUILD_MSG_UPDATE_GUILD_MEMBER);

}

void Guild::_SendUptGuildApplyNtf(uint8_t bIsAdd, DT_ONE_GUILD_MEMBER& rstOneApply)
{
    //发送申请更新的广播
    DT_GUILD_NTF_MSG stGuildNtfMsg;
    DT_GUILD_PKG_UPDATE_GUILD_APPLY& rstUptApply = stGuildNtfMsg.m_stUptGuildApply;
    rstUptApply.m_bIsAdd = bIsAdd;
    if (bIsAdd == 0)
    {
        rstUptApply.m_stApplyInfo.m_ullUin = rstOneApply.m_ullUin;
    }
    else
    {
        memcpy(&rstUptApply.m_stApplyInfo, &rstOneApply, sizeof(DT_ONE_GUILD_MEMBER));
    }

    this->SendBroadcastMsg(stGuildNtfMsg, DT_GUILD_MSG_UPDATE_GUILD_APPLY);
}

void Guild::_SendUptGuildRoomNtf(uint8_t bIsAdd, DT_PVP_ROOM_INFO& rstRoomInfo)
{
    //发送房间更新的广播
    DT_GUILD_NTF_MSG stGuildNtfMsg;
    DT_GUILD_PKG_UPDATE_GUILD_ROOM& rstUptRoom = stGuildNtfMsg.m_stUptPvpRoom;
    rstUptRoom.m_bIsAdd = bIsAdd;
    rstUptRoom.m_stRoomInfo = rstRoomInfo;
    this->SendBroadcastMsg(stGuildNtfMsg, DT_GUILD_MSG_UPDATE_GUILD_ROOM);
}

void Guild::_SendUptGuildReplayNtf(DT_REPLAY_INFO& rstOneReplay)
{
    //发送录像更新的广播
    DT_GUILD_NTF_MSG stGuildNtfMsg;
    DT_GUILD_PKG_UPDATE_GUILD_REPLAY& rstUptReplay = stGuildNtfMsg.m_stUptGuildReplay;
    rstUptReplay.m_stReplayInfo = rstOneReplay;
    this->SendBroadcastMsg(stGuildNtfMsg, DT_GUILD_MSG_UPDATE_GUILD_REPLAY);
}

void Guild::_SendFightStateChgNtf()
{
    //将公会战状态变化的消息广播给公会成员
    DT_GUILD_NTF_MSG stGuildNtfMsg;
    DT_GUILD_PKG_UPDATE_FIGHT_STATE& rstUptFightState = stGuildNtfMsg.m_stUptFightState;
    rstUptFightState.m_bState = m_stFightState.m_bState;
    rstUptFightState.m_ullTimeStamp = m_stFightState.m_ullTimeStamp;
    this->SendBroadcastMsg(stGuildNtfMsg, DT_GUILD_MSG_UPDATE_FIGHT_STATE);
}

void Guild::_SendGuildDissolveNtf()
{
    DT_GUILD_NTF_MSG stGuildNtfMsg;
    this->SendBroadcastMsg(stGuildNtfMsg, DT_GUILD_MSG_DISSOLVE_GUILD);
}

int Guild::GetBriefInfo(OUT DT_GUILD_BRIEF_INFO& rstBriefInfo)
{
    rstBriefInfo.m_ullGuildId = this->GetGuildId();
    StrCpy(rstBriefInfo.m_szName, this->GetGuildName(), MAX_NAME_LENGTH);
    rstBriefInfo.m_dwStarNum = m_oGuildMember.GetStarSumNum();
    rstBriefInfo.m_wCurrMemberNum = m_oGuildMember.GetCurMemberNum();
    rstBriefInfo.m_wMaxMemberNum = m_stGuildGlobalInfo.m_wMaxMemberNum;
    rstBriefInfo.m_bNeedApply = m_stGuildGlobalInfo.m_bNeedApply;
	rstBriefInfo.m_bLvLimit = m_stGuildGlobalInfo.m_bLvLimit;
    return ERR_NONE;
}

void Guild::SetLeader(DT_ONE_GUILD_MEMBER& rstOneMember)
{
    m_stGuildGlobalInfo.m_ullLeaderUin = rstOneMember.m_ullUin;
    StrCpy(m_stGuildGlobalInfo.m_szLeaderName, rstOneMember.m_szName, MAX_NAME_LENGTH);

    //数据变化，需要回写数据库
    GuildMgr::Instance().AddToDirtyList(this);
}

void Guild::SendGuildMail(int iMailId)
{
    SSPKG stSsPkg = {0};
    stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_BY_ID_REQ;
    SS_PKG_MAIL_ADD_BY_ID_REQ& rstMailAddReq = stSsPkg.m_stBody.m_stMailAddByIdReq;

    rstMailAddReq.m_nUinCount= m_oGuildMember.GetCurMemberNum();
    uint64_t* pMemberList = m_oGuildMember.GetMemberList();
    for (int i=0; i<rstMailAddReq.m_nUinCount; i++)
    {
        rstMailAddReq.m_UinList[i] = pMemberList[i];
    }
    rstMailAddReq.m_dwMailResId = iMailId;

    GuildSvrMsgLayer::Instance().SendToMailSvr(stSsPkg);

    return;
}

void Guild::SendGuildLeaderMail(int iMailId, bool bVice)
{
    SSPKG stSsPkg = {0};
    stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_BY_ID_REQ;
    SS_PKG_MAIL_ADD_BY_ID_REQ& rstMailAddReq = stSsPkg.m_stBody.m_stMailAddByIdReq;

    rstMailAddReq.m_nUinCount = 1;
    rstMailAddReq.m_UinList[0] = m_stGuildGlobalInfo.m_ullLeaderUin;
    rstMailAddReq.m_dwMailResId = iMailId;

    //  是否需要给副军团长发送邮件
    if (bVice)
    {
        int iCurrNum = m_oGuildMember.GetCurMemberNum();
        uint64_t* pMemberList = m_oGuildMember.GetMemberList();
        for (int i=0; i < iCurrNum; i++)
        {
            if (m_oGuildMember.IsDeputy(pMemberList[i]))
            {
                rstMailAddReq.m_UinList[rstMailAddReq.m_nUinCount] = m_stGuildGlobalInfo.m_ullLeaderUin;
                rstMailAddReq.m_nUinCount++;
            }
        }
        rstMailAddReq.m_dwMailResId = iMailId;
    }

    GuildSvrMsgLayer::Instance().SendToMailSvr(stSsPkg);

    return;
}

DT_GUILD_RANK_INFO& Guild::GetGuildRankInfo()
{
    m_stGuildRankInfo.m_ullGuildId = m_stGuildBaseInfo.m_ullGuildId;
    m_stGuildRankInfo.m_bGuildLevel = m_stGuildGlobalInfo.m_bGuildLevel;
    m_stGuildRankInfo.m_dwStarNum = m_oGuildMember.GetLiSum();
    m_stGuildRankInfo.m_wCurrMemberNum = m_oGuildMember.GetCurMemberNum();
    m_stGuildRankInfo.m_wMaxMemberNum = m_stGuildGlobalInfo.m_wMaxMemberNum;

    StrCpy(m_stGuildRankInfo.m_szGuildName, m_stGuildBaseInfo.m_szName, MAX_NAME_LENGTH);
    StrCpy(m_stGuildRankInfo.m_szLeaderName, m_stGuildGlobalInfo.m_szLeaderName, MAX_NAME_LENGTH);

    return m_stGuildRankInfo;
}

DT_GFIGHT_RANK_INFO& Guild::GetGFightRankInfo()
{
    m_stGFightRankInfo.m_ullGuildId = m_stGuildBaseInfo.m_ullGuildId;
    m_stGFightRankInfo.m_bGuildLevel = m_stGuildGlobalInfo.m_bGuildLevel;
    m_stGFightRankInfo.m_wScore = m_stGuildGlobalInfo.m_dwTotalScore;
    m_stGFightRankInfo.m_wCurrMemberNum = m_oGuildMember.GetCurMemberNum();
    m_stGFightRankInfo.m_wMaxMemberNum = m_stGuildGlobalInfo.m_wMaxMemberNum;

    StrCpy(m_stGuildRankInfo.m_szGuildName, m_stGuildBaseInfo.m_szName, MAX_NAME_LENGTH);
    StrCpy(m_stGuildRankInfo.m_szLeaderName, m_stGuildGlobalInfo.m_szLeaderName, MAX_NAME_LENGTH);

    return m_stGFightRankInfo;
}

void Guild::AddGFightScore(int iScoreDeta)
{
    m_stGuildGlobalInfo.m_dwTotalScore += iScoreDeta;
    GuildMgr::Instance().AddToDirtyList(this);

    //数据变化，需要回写数据库
    //GFightRankMgr::Instance().UpdateRank(GetGFightRankInfo());
	GFightRankMgr::Instance().UpdateGFightRankNtf(this, this->GetGFightRankInfo());
}

void Guild::GmUpdateGuildInfo(SS_PKG_GUILD_GM_UPDATE_INFO_REQ& rstReq)
{
#if 0
    LOGWARN_r("Gm:Guild::GmUpdateGuildInfo, <%lu>",rstReq.m_ullGuildId);
    DT_GUILD_GLOBAL_INFO& rstGlobalInfo = m_oGuildData.GetGlobalInfo();
    if (0 < rstReq.m_bGuildLevel)
    {
        rstGlobalInfo.m_bGuildLevel = rstReq.m_bGuildLevel;
    }
    if (0 < rstReq.m_dwGuildFund)
    {
        rstGlobalInfo.m_dwGuildFund = rstReq.m_dwGuildFund;
    }
    if ( 0 == rstReq.m_bGuildLevel  && 0 == rstReq.m_dwGuildFund )
    {//没变化
        LOGERR_r("Gm UpdateGuildInfo error, the para is invalid!");
        return;
    }
    //广播给成员
    DT_GUILD_PKG_UPDATE_GUILD_GLOBAL& rstUptGlobalInfo = m_stGuildNtfMsg.m_stUptGuildGlobal;
    memcpy(&rstUptGlobalInfo.m_stGlobalInfo, &rstGlobalInfo, sizeof(rstGlobalInfo));
    this->SendBroadcastMsg(m_stGuildNtfMsg, DT_GUILD_MSG_UPDATE_GUILD_GLOBAL);
#endif
}

int Guild::UploadReplay(DT_REPLAY_RECORD_FILE_HEADER* pstReplayHead, char* pszURL)
{
    DT_REPLAY_INFO * pstReplayInfo = GuildDataDynPool::Instance().GuildReplayPool().Get();
    if (pstReplayInfo==NULL)
    {
        return ERR_SYS;
    }

    memcpy(&pstReplayInfo->m_stFileHead, pstReplayHead, sizeof(DT_REPLAY_RECORD_FILE_HEADER));
    pstReplayInfo->m_ullUpLoadTimeStamp = CGameTime::Instance().GetCurrSecond();

    uint32_t dwRaceNumber = pstReplayHead->m_dwRaceNumber;
    sprintf(pstReplayInfo->m_szURL, "%s/%u", GuildSvr::Instance().GetConfig().m_szURLRoot, dwRaceNumber);

    memcpy(pszURL, pstReplayInfo->m_szURL, URLROOT_MAXLEN);

    //允许上传，录像信息 加入ToBeChecked列表
    m_listToCheckedReplay.push_back(pstReplayInfo);

    return ERR_NONE;
}

void Guild::_UpdateReplay()
{
    int iCounter = 0;
    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();

    while(!m_listToCheckedReplay.empty() && iCounter++ < MAX_DEAL_NUM_PER_SEC)
    {
        DT_REPLAY_INFO * pstReplayInfo = m_listToCheckedReplay.front();
        assert(pstReplayInfo);

        if ((int)(ullCurTime - pstReplayInfo->m_ullUpLoadTimeStamp) < GuildSvr::Instance().GetConfig().m_iCheckInterval)
        {
            break;
        }

        m_listToCheckedReplay.pop_front();

        //录像文件不存在，可能是上传失败，不加入ReplayMap
        if (!_CheckReplayExist(pstReplayInfo->m_stFileHead.m_dwRaceNumber))
        {
            LOGERR("Guild(%lu) Replay(%u) Upload failed", GetGuildId(), pstReplayInfo->m_stFileHead.m_dwRaceNumber);
            GuildDataDynPool::Instance().GuildReplayPool().Release(pstReplayInfo);
            continue;
        }

        //添加录像
        m_oGuildReplay.Add(pstReplayInfo);

        //发送广播
        this->_SendUptGuildReplayNtf(*pstReplayInfo);

        //数据变化，需要回写数据库
        GuildMgr::Instance().AddToDirtyList(this);
    }
}

bool Guild::_CheckReplayExist(uint32_t dwRaceNumber)
{
    //return true;
    char szFilePath[MAX_LEN_FILEPATH];
    snprintf(szFilePath, MAX_LEN_FILEPATH, "%s/%u", GuildSvr::Instance().GetConfig().m_szRootDir, dwRaceNumber);
    return (access(szFilePath, F_OK) == 0);
}

void Guild::SettleBoss(uint64_t ullUin, SS_PKG_GUILD_BOSS_FIGHT_SETTLE_NTF& rstSsPkgNtf)
{
    uint8_t bKilled = 0;
    uint32_t dwBossId = 0;
    m_oGuildBoss.AddDamage(ullUin, rstSsPkgNtf.m_dwFigheLevelId, rstSsPkgNtf.m_dwDamageHp, bKilled, dwBossId);
    int iBossIndex = this->GetGuildBoss().FindBossIndex(dwBossId);

    uint16_t wMemNum = this->GetGuildMemInfo().GetCurMemberNum();
    DT_GUILD_BOSS_FIGHT_TIMES *pstFightTimes = this->GetGuildBoss().GetBossInfo().m_astMemFightBossTimes;
    DT_GUILD_BOSS_FIGHT_TIMES stFightTimes;
    stFightTimes.m_ullUin = ullUin;
    int iEqual = 0;
    int iIndex = MyBSearch(&stFightTimes, this->GetGuildBoss().GetBossInfo().m_astMemFightBossTimes, wMemNum,
        sizeof(DT_GUILD_BOSS_FIGHT_TIMES), &iEqual, UinCmp);
    if (iEqual != 0)
    {
        ++pstFightTimes[iIndex].m_bFightTimes;
    }
    else
    {
        LOGERR_r("player <%lu> isn't in this guild <%lu>.", ullUin, this->GetGuildId());
    }

    if (bKilled)
    {
        m_oGuildBoss.UnlockNextBoss(m_stGuildGlobalInfo.m_bGuildLevel);
        GuildBossMgr::Instance().SetPassedGuildInfo(this->GetGuildId(), iBossIndex);
        m_oGuildBoss.GetOneBossInfo(dwBossId)->m_ullPassedTimeStamp = CGameTime::Instance().GetCurrSecond();
    }
    _SendGuildBossNtf(bKilled, dwBossId);
}

void Guild::ResetBoss(uint64_t ullUin)
{
    m_oGuildBoss.Reset( );
    _SendGuildBossNtf();
    LOGRUN(" Uin<%lu> ResetBoss GuildId<%lu> OK! ", ullUin, m_stGuildBaseInfo.m_ullGuildId);
}

void Guild::_SendGuildBossNtf(uint8_t bKilled, uint32_t dwBossId)
{
    uint32_t dwCurBossId = this->GetGuildBoss().GetBossInfo().m_dwCurBossId;
    int iCurBossIndex = this->GetGuildBoss().FindBossIndex(dwCurBossId);
    if (iCurBossIndex >= 0)
    {
        m_oGuildBoss.GetBossInfo().m_astBossList[iCurBossIndex].m_wPassedGuildNum =
            GuildBossMgr::Instance().GetPassedGuildNum(iCurBossIndex);
    }
    DT_GUILD_NTF_MSG stNtf ;
    stNtf.m_stUptGuildBoss.m_bKilled = bKilled;
    stNtf.m_stUptGuildBoss.m_dwKilledBossId = dwBossId;
    stNtf.m_stUptGuildBoss.m_stBossInfo = m_oGuildBoss.GetBossInfo();

    SendBroadcastMsg(stNtf , DT_GUILD_MSG_UPDATE_GUILD_BOSS);
}

int Guild::EnterBossFight(uint64_t ullUin, uint32_t dwFLevelId, OUT SS_PKG_GUILD_BOSS_ENTER_FIGHT_RSP& rstSsPkgRsp)
{
    rstSsPkgRsp.m_nErrNo = m_oGuildBoss.GetBossLeftHp(ullUin, dwFLevelId, rstSsPkgRsp.m_dwBossHpLeft);
    rstSsPkgRsp.m_dwFLevelId = dwFLevelId;
    return 0;
}

void Guild::_UpdateBossInfo()
{

}

void Guild::SetJoinLevel(uint8_t bLvLimit)
{
	m_stGuildGlobalInfo.m_bLvLimit = bLvLimit;

	//数据变化，需要回写数据库
	GuildMgr::Instance().AddToDirtyList(this);
}

void Guild::_SendGuildSocietyNtf(uint8_t bType, DT_GUILD_DONATE_INFO* pstDonateInfo/*=NULL*/)
{
	DT_GUILD_NTF_MSG stNtf;
	stNtf.m_stUptGuildSociety.m_bType = bType;
	stNtf.m_stUptGuildSociety.m_bLv = m_oGuildSociety.GetLv(bType);
	stNtf.m_stUptGuildSociety.m_dwExp = m_oGuildSociety.GetExp(bType);
	stNtf.m_stUptGuildSociety.m_bUpData = m_oGuildSociety.GetValue(bType);
	if (pstDonateInfo!=NULL)
	{
		memcpy(&stNtf.m_stUptGuildSociety.m_stDonateInfo, pstDonateInfo, sizeof(DT_GUILD_DONATE_INFO));
	}
	else
	{
		bzero(&stNtf.m_stUptGuildSociety.m_stDonateInfo, sizeof(DT_GUILD_DONATE_INFO));
	}


	SendBroadcastMsg(stNtf , DT_GUILD_MSG_UPDATE_GUILD_SOCIETY);
}

int Guild::_GetTitleNum(uint8_t bTitle)
{
	if (bTitle == GUILD_IDENTITY_LEADER)
	{
		return 1;
	}

	if (bTitle == GUILD_IDENTITY_DEPUTY_LEADER)
	{
		return this->m_stGuildGlobalInfo.m_wMaxViceLeaderNum;
	}

	if (bTitle == GUILD_IDENTITY_ELDERS)
	{
		return this->m_stGuildGlobalInfo.m_wMaxEliteNum;
	}

	if (bTitle == GUILD_IDENTITY_MEMBER)
	{
		return this->m_stGuildGlobalInfo.m_wMaxMemberNum;
	}

	return ERR_WRONG_PARA;
}


void Guild::_SendGuildIdentityChangeNtf(DT_ONE_GUILD_MEMBER& rstOneMember, uint8_t bType)
{
	//发事件广播
	DT_GUILD_NTF_MSG stNtfMsg;
	DT_GUILD_PKG_GUILD_LEVEL_EVENT& rstLevelEvent = stNtfMsg.m_stLevelEventBraod;

	rstLevelEvent.m_ullUin = rstOneMember.m_ullUin;

	switch (bType)
	{
	case GUILD_JOB_OP_TYPE_UP:
		{
			rstLevelEvent.m_bType = GUILD_EVENT_UP;
			DT_GUILD_PKG_EVENT_UP& rstEventUp = rstLevelEvent.m_stGuildEvent.m_stGuildEventUp;
			rstEventUp.m_bIdentity = rstOneMember.m_bIdentity;
			break;
		}
	case GUILD_JOB_OP_TYPE_TOP:
		rstLevelEvent.m_bType = GUILD_EVENT_TOP;
		break;
	default:
		return;
		break;
	}

	this->SendBroadcastMsg(stNtfMsg, DT_GUILD_MSG_GUILD_LEVEL_EVENT);
}

void Guild::_SendGuildJoinOrLeaveEventNtf(DT_ONE_GUILD_MEMBER& rstOneMember, uint8_t bType)
{
	//发事件广播
	DT_GUILD_NTF_MSG stNtfMsg;
	DT_GUILD_PKG_GUILD_LEVEL_EVENT& rstLevelEvent = stNtfMsg.m_stLevelEventBraod;

	rstLevelEvent.m_ullUin = rstOneMember.m_ullUin;

	switch (bType)
	{
	case GUILD_EVENT_JOIN:
		rstLevelEvent.m_bType = GUILD_EVENT_JOIN;
		break;
	case GUILD_EVENT_LEAVE:
		rstLevelEvent.m_bType = GUILD_EVENT_LEAVE;
		break;
	default:
		return;
		break;
	}

	this->SendBroadcastMsg(stNtfMsg, DT_GUILD_MSG_GUILD_LEVEL_EVENT);
}

void Guild::BroadMemberInfoChange(DT_ONE_GUILD_MEMBER& rstOneMember)
{
	_SendUptGuildMemberNtf(GUILD_JOB_OP_TYPE_EXTRA, rstOneMember);
}

//军团练兵场为他人加速
int Guild::SpeedPartner(uint64_t ullSpeeder, uint64_t ullSpeeded)
{
    DT_ONE_GUILD_MEMBER* poSpeederMemInfo = m_oGuildMember.Find(ullSpeeder);
    DT_ONE_GUILD_MEMBER* poSpeededMemInfo = m_oGuildMember.Find(ullSpeeded);

    int iRet = ERR_NONE;
    do
    {
        if (poSpeederMemInfo == NULL)
        {
            LOGERR("poSpeederMemInfo is null. uin<%lu>", ullSpeeder);
            iRet = ERR_NOT_FOUND_MEMBER;
            break;
        }
        if (poSpeededMemInfo == NULL)
        {
            LOGERR("poSpeededMemInfo is null. uin<%lu>", ullSpeeded);
            iRet = ERR_NOT_FOUND_MEMBER;
            break;
        }

        DT_GUILD_HANG_SPEED_PARTNER_INFO& rstSpeederInfo = poSpeederMemInfo->m_stHangSpeedPartnerInfo;
        DT_GUILD_HANG_BE_SPEEDED_INFO& rstBeSpeededInfo = poSpeededMemInfo->m_stHangBeSpeededInfo;

        if (rstSpeederInfo.m_bCount >= GUILD_HANG_SPEED_PARTNER_NUM)
        {
            LOGERR("speed partner time is used up. uin<%lu>", ullSpeeder);
            iRet = ERR_MAX_SPEED_TIME;
            break;
        }
        if (rstBeSpeededInfo.m_bSpeededToday >= GUILD_HANG_BE_SPEEDED_NUM)
        {
            LOGERR("be speeded time is used up. uin<%lu>", ullSpeeded);
            iRet = ERR_MAX_BE_SPEEDED_TIME;
            break;
        }

        if (_IsInSpeederInfo(rstSpeederInfo, ullSpeeded))
        {
            LOGERR("the member has been speeded today. uin<%lu>", ullSpeeded);
            iRet = ERR_ALREADY_SPEED_TODAY;
            break;
        }

        DT_GUILD_HANG_LOG_INFO oOneHangLogInfo;
        oOneHangLogInfo.m_ullUin = ullSpeeder;
        oOneHangLogInfo.m_ullTimestamp = CGameTime::Instance().GetCurrSecond();
        //记录最多保留GUILD_HANG_SPEED_LOG_NUM
        size_t nmemb = (size_t)rstBeSpeededInfo.m_bCount;
        nmemb = (nmemb >= GUILD_HANG_SPEED_LOG_NUM) ? nmemb -1 : nmemb;
        if (!MyBInsert(&oOneHangLogInfo, rstBeSpeededInfo.m_astLogInfoList, &nmemb, sizeof(DT_GUILD_HANG_LOG_INFO), 1, HangLogCmp))
        {
            LOGERR_r("Add2rstBeSpeededInfo  err!");
            iRet = ERR_SYS;
            break;
        }
        rstBeSpeededInfo.m_bCount = (uint8_t)nmemb;
        rstBeSpeededInfo.m_bSpeededToday++;
        //最多存255点加速次数
        if (rstBeSpeededInfo.m_bTotalNum != 255)
        {
            rstBeSpeededInfo.m_bTotalNum++;
        }

        rstSpeederInfo.m_UinList[rstSpeederInfo.m_bCount++] = ullSpeeded;

    } while (false);

    if (iRet == ERR_NONE)
    {
        GuildMgr::Instance().AddToDirtyList(this);
        this->_SendUptGuildMemberNtf(GUILD_JOB_OP_TYPE_EXTRA, *poSpeederMemInfo);
        this->_SendUptGuildMemberNtf(GUILD_JOB_OP_TYPE_EXTRA, *poSpeededMemInfo);
    }

    return iRet;

}

int Guild::GetSpeedInfo(uint64_t ullUin, uint8_t& bCount)
{
    DT_ONE_GUILD_MEMBER* poMember = m_oGuildMember.Find(ullUin);
    if (!poMember)
    {
        LOGERR("poMember is null. Uin<%lu>", ullUin);
        return ERR_NOT_FOUND_MEMBER;
    }

    bCount = poMember->m_stHangBeSpeededInfo.m_bTotalNum;
    poMember->m_stHangBeSpeededInfo.m_bTotalNum = 0;

    this->_SendUptGuildMemberNtf(GUILD_JOB_OP_TYPE_EXTRA, *poMember);
    GuildMgr::Instance().AddToDirtyList(this);

    return ERR_NONE;
}

bool Guild::_IsInSpeederInfo(DT_GUILD_HANG_SPEED_PARTNER_INFO& rstSpeederInfo, uint64_t ullUin)
{
    int iRet = false;
    for (int i = 0; i < rstSpeederInfo.m_bCount; i++)
    {
        if (rstSpeederInfo.m_UinList[i]==ullUin)
        {
            iRet = true;
            break;
        }
    }

    return iRet;
}

int Guild::ChangeStar(uint64_t ullOperator, uint64_t ullTarget, uint8_t bType, DT_GUILD_HANG_STAR_INFO& rstHangStarInfo)
{
    DT_ONE_GUILD_MEMBER* poOperator = m_oGuildMember.Find(ullOperator);
    if (!poOperator)
    {
        LOGERR("poOperator is null. Uin<%lu>", ullOperator);
        return ERR_NOT_FOUND_MEMBER;
    }

    if (!m_oGuildMember.Find(ullTarget))
    {
        LOGERR("poTarget is null. Uin<%lu>", ullTarget);
        return ERR_NOT_FOUND_MEMBER;
    }

    DT_GUILD_HANG_STAR_INFO& rstStarInfo = poOperator->m_stHangStarInfo;

    size_t nmenb = rstStarInfo.m_bCount;
    if (bType == 0)
    {
        MyBDelete(&ullTarget, rstStarInfo.m_UinList, &nmenb, sizeof(ullTarget), StarUinCmp);
        rstStarInfo.m_bCount = (uint8_t)nmenb;
    }
    if (bType == 1)
    {
        if (rstStarInfo.m_bCount >= MAX_GUILD_HANG_STAR_NUM)
        {
            LOGERR("rstStarInfo.m_bCount<%d> reach the max", rstStarInfo.m_bCount);
            return ERR_MAX_STAR_NUM;
        }

        MyBInsert(&ullTarget, rstStarInfo.m_UinList, &nmenb, sizeof(ullTarget), 1, StarUinCmp);
        rstStarInfo.m_bCount = (uint8_t)nmenb;
    }

    memcpy(&rstHangStarInfo, &rstStarInfo, sizeof(DT_GUILD_HANG_STAR_INFO));

    return ERR_NONE;
}

void Guild::UploadGuildExpeditionInfo()
{
	if (m_wUploadExpeditionCtrl++ % 10 != 0)
	{
		return;
	}

	SSPKG& rstPkgNew = GuildSvrMsgLayer::Instance().GetNtfPkg();
	rstPkgNew.m_stHead.m_wMsgId = SS_MSG_GUILD_EXPEDITION_UPLOAD_GUILD_INFO_NTF;

	SS_PKG_GUILD_EXPEDITION_UPLOAD_GUILD_INFO_NTF& rstNtf = rstPkgNew.m_stBody.m_stGuildExpeditionUploadGulidInfoNtf;
	this->GetGuildExpeditionInfo(rstNtf.m_stInfo);
	rstPkgNew.m_stHead.m_ullReservId = 0;
	GuildSvrMsgLayer::Instance().SendToGuildExpeditionSvr(rstPkgNew);
	m_ullLastUploadGuildExpeditionTime = CGameTime::Instance().GetCurrSecond();
}

void Guild::HandleExpeditionAward(uint8_t bAwardType)
{
	if ((bAwardType & 1) == 1)
	{
		m_oGuildMember.SettleExpeditionAward(1);
		LOGRUN_r("Guild<%lu> ExpeditionAward Type<1> ok", GetGuildId());
	}
	if ((bAwardType & 2) == 2)
	{
		m_oGuildMember.SettleExpeditionAward(2);
		LOGRUN_r("Guild<%lu> ExpeditionAward Type<2> ok", GetGuildId());
	}

}

void Guild::UpdateExpeidtionDefendInfo(DT_GUILD_PKG_EXPEDITION_UPDATE_DEFEND_INFO& rstDefendInfo)
{
	m_oGuildMember.UpdateDefendInfo(rstDefendInfo);
}

void Guild::SendGuildStateToExpeditionSvr(uint8_t bType, uint64_t ullUin)
{
	SSPKG& rstPkgNew = GuildSvrMsgLayer::Instance().GetNtfPkg();
	rstPkgNew.m_stHead.m_wMsgId = SS_MSG_GUILD_EXPEDITION_GUILD_STATE_NTF;
	SS_PKG_GUILD_EXPEDITION_GUILD_STATE_NTF& rstNtf = rstPkgNew.m_stBody.m_stGuildExpeditionGuildStateNtf;
	rstNtf.m_ullGuildId = GetGuildId();
	rstNtf.m_bType = bType;
	rstNtf.m_ullUllMemUin = ullUin;
}



