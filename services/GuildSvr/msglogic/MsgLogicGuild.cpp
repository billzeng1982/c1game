#include <string.h>
#include "strutil.h"
#include "LogMacros.h"
#include "MsgLogicGuild.h"
#include "ss_proto.h"
#include "ov_res_public.h"
#include "../framework/GuildSvrMsgLayer.h"
#include "../transaction/GuildTransFrame.h"
#include "../module/Guild/GuildMgr.h"
#include "../module/Guild/GuildRank.h"
#include "../module/Player/PlayerMgr.h"
#include "../module/Fight/GuildFightMgr.h"
#include "../module/Guild/GuildBossMgr.h"

using namespace PKGMETA;

static int UinCmp(const void * pFirst, const void * pSecond)
{
    return ((DT_GUILD_BOSS_FIGHT_TIMES *)pFirst)->m_ullUin - ((DT_GUILD_BOSS_FIGHT_TIMES *)pSecond)->m_ullUin;
}

int GuildDetailsReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    bool bFlag = true;
    Player* poPlayer = PlayerMgr::Instance().GetPlayer(rstSsPkg.m_stHead.m_ullUin);
    Guild* poGuild = NULL;

    if (poPlayer != NULL)
    {
        if (poPlayer->GetGuildId() == 0)
        {
            bFlag = false;
        }
        else
        {
            poGuild = GuildMgr::Instance().GetGuild(poPlayer->GetGuildId());
            if (poGuild != NULL)
            {
                bFlag = false;
            }
        }
    }

    if (!pvPara && bFlag)
    {
        GuildTransFrame::Instance().AddGetGuildTrans(rstSsPkg.m_stHead.m_ullUin, 0, rstSsPkg);
    }
    else
    {
        _HandleGuildDetailsReq(rstSsPkg, poGuild, poPlayer);
    }

    return 0;
}

void GuildDetailsReq_SS::_HandleGuildDetailsReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild, Player* poPlayer)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_DETAILS_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;

	//SS_PKG_GUILD_DETAILS_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildDetailsReq;
    SS_PKG_GUILD_DETAILS_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stGuildDetailsRsp;
    rstSsPkgRsp.m_nErrNo = ERR_NONE;
    rstSsPkgRsp.m_stGuildPlayerInfo.m_stBaseInfo.m_ullUin = 0;
    rstSsPkgRsp.m_stGuildPlayerInfo.m_stBaseInfo.m_ullGuildId = 0;
    rstSsPkgRsp.m_stGuildWholeInfo.m_stBaseInfo.m_ullGuildId = 0;
    rstSsPkgRsp.m_bGuildBossResetNum = 0;
    if (poPlayer != NULL)
    {
        poPlayer->PackPlayerData(rstSsPkgRsp.m_stGuildPlayerInfo, (uint16_t)rstSsPkg.m_stHead.m_ullReservId);
    }

    if (poGuild != NULL)
    {
        rstSsPkgRsp.m_ullGuildId = poGuild->GetGuildId();
        rstSsPkgRsp.m_bGuildLevel = poGuild->GetGuildLevel();
        rstSsPkgRsp.m_bGuildIdentify = poGuild->GetGuildMemJob(rstSsPkg.m_stHead.m_ullUin);
        rstSsPkgRsp.m_bGuildBossResetNum = poGuild->GetGuildBossResetNum();
        rstSsPkgRsp.m_bGuildGoldTime = poGuild->GetGuildSocietyInfo().GetValue(GUILD_SOCIETY_TYPE_GOLD_TIME);
        poGuild->PackGuildWholeData(rstSsPkgRsp.m_stGuildWholeInfo, (uint16_t)rstSsPkg.m_stHead.m_ullReservId);
        rstSsPkgRsp.m_stFightStateInfo = poGuild->GetGuildFightState();
        poGuild->GetGuildRoomInfo(rstSsPkgRsp.m_stGuildRoomInfo);
        GuildBossMgr::Instance().GetState(rstSsPkgRsp.m_stGuildBossInfo);
		GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
		
		poGuild->UploadGuildExpeditionInfo();

		
    }
    else
    {
        rstSsPkgRsp.m_stGuildWholeInfo.m_stBossBlob.m_iLen = 0;
        rstSsPkgRsp.m_stGuildWholeInfo.m_stApplyBlob.m_iLen = 0;
        rstSsPkgRsp.m_stGuildWholeInfo.m_stGlobalBlob.m_iLen = 0;
        rstSsPkgRsp.m_stGuildWholeInfo.m_stMemberBlob.m_iLen = 0;
        rstSsPkgRsp.m_stGuildWholeInfo.m_stReplayBlob.m_iLen = 0;
        rstSsPkgRsp.m_stGuildWholeInfo.m_stSocietyBlob.m_iLen = 0;
		
	}
	GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
}


int CreateGuildReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_CREATE_GUILD_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stCreateGuildReq;
    GuildTransFrame::Instance().AddCreateGuildTrans(rstSsPkg.m_stHead.m_ullUin, rstSsPkgReq.m_szName, rstSsPkg);
    return 0;
}

int RefreshGuildListReq_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_REFRESH_GUILDLIST_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    SS_PKG_REFRESH_GUILDLIST_RSP& rstSsPkgBodyRsp = m_stSsPkg.m_stBody.m_stRefreshGuildListRsp;
    int iRet = GuildMgr::Instance().RefreshGuildList(rstSsPkgBodyRsp.m_bGuildCount, rstSsPkgBodyRsp.m_astGuildList);
    rstSsPkgBodyRsp.m_nErrNo = iRet;
    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return 0;
}

int JoinGuildReq_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_JOIN_GUILD_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stJoinGuildReq;

    Player* poPlayer = PlayerMgr::Instance().GetPlayer(rstSsPkg.m_stHead.m_ullUin);
    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgReq.m_ullGuildId);

    if (!pvPara && (!poPlayer ||!poGuild))
    {
        GuildTransFrame::Instance().AddGetGuildTrans(rstSsPkg.m_stHead.m_ullUin, rstSsPkgReq.m_ullGuildId, rstSsPkg);
    }
    else
    {
        _HandleGuildJoinReq(rstSsPkg, poGuild, poPlayer);
    }

    return 0;
}

void JoinGuildReq_SS::_HandleGuildJoinReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild, Player* poPlayer)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_JOIN_GUILD_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    SS_PKG_JOIN_GUILD_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stJoinGuildReq;
    SS_PKG_JOIN_GUILD_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stJoinGuildRsp;
    rstSsPkgRsp.m_nErrNo = ERR_NONE;
    uint8_t bIsApply = rstSsPkgReq.m_bIsApply;

	//检查该工会是否设定为不允许加入
	if (2 == poGuild->GetNeedApply())
	{
		rstSsPkgRsp.m_nErrNo = ERR_GUILD_NOT_OPEN_APLLY;
		GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
		return;
	}


	//检查申请者等级是否满足该工会设定的下限
	uint8_t bLevelLimit = poGuild->GetJoinLevel();
	if ( rstSsPkgReq.m_stPlayerInfo.m_stExtraInfo.m_bMajestyLevel < bLevelLimit)
	{
		rstSsPkgRsp.m_nErrNo = ERR_MAJESTY_UN_SATISFY;
		GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
		return;
	}


    //需要军团长批准
    do
    {
        if (poGuild == NULL || poPlayer == NULL)
        {
            rstSsPkgRsp.m_nErrNo = ERR_SYS;
            break;
        }

        uint8_t bNeedApply = poGuild->GetNeedApply();
        rstSsPkgRsp.m_bNeedApply = bNeedApply;
        if (1==bNeedApply)
        {
            int iRet = 0;
            if (bIsApply)
            {
                if (poPlayer->GetGuildId() != 0)
                {
                    rstSsPkgRsp.m_nErrNo = ERR_CANT_APPLY;
                    break;
                }

                iRet = poPlayer->AddApply(rstSsPkgReq.m_ullGuildId);
                if (iRet != ERR_NONE)
                {
                    rstSsPkgRsp.m_nErrNo = iRet;
                    break;
                }

                iRet = poGuild->AddApply(rstSsPkgReq.m_stPlayerInfo);
                if (iRet != ERR_NONE)
                {
                    //没有申请成功时需要将申请从玩家数据中删除
                    poPlayer->DelApply(rstSsPkgReq.m_ullGuildId);
                }
                rstSsPkgRsp.m_nErrNo = iRet;
            }
            else
            {
                iRet = poPlayer->DelApply(rstSsPkgReq.m_ullGuildId);
                if (iRet != ERR_NONE)
                {
                    rstSsPkgRsp.m_nErrNo = iRet;
                    break;
                }
                //不需要返回DelApply的返回码
                poGuild->DelApply(rstSsPkgReq.m_stPlayerInfo.m_ullUin);
            }
        }
        else
        {
            //直接加入军团
            DT_ONE_GUILD_MEMBER& rstGuildMemberInfo = rstSsPkgReq.m_stPlayerInfo;
            rstGuildMemberInfo.m_ullJoinGuildTimeStap = CGameTime::Instance().GetCurrTimeMs();
            rstGuildMemberInfo.m_ullLastLogoutTimeStap = CGameTime::Instance().GetCurrTimeMs();
            rstGuildMemberInfo.m_stExtraInfo.m_bIsOnline = 0;
            int iRet = poGuild->AddMember(rstGuildMemberInfo, &rstGuildMemberInfo);
            if (iRet != ERR_NONE)
            {
                rstSsPkgRsp.m_nErrNo = iRet;
                break;
            }

            //更新玩家信息
            poPlayer->SetGuildId(poGuild->GetGuildId());
        }
    } while (false);

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return;
}

int GuildSetNeedApplyReq_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_GUILD_SET_NEED_APPLY_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildSetNeedApplyReq;

    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgReq.m_ullGuildId);

    if (!pvPara && !poGuild)
    {
        GuildTransFrame::Instance().AddGetGuildTrans(rstSsPkg.m_stHead.m_ullUin, rstSsPkgReq.m_ullGuildId, rstSsPkg);
    }
    else
    {
        _HandleSetNeedApplyReq(rstSsPkg, poGuild);
    }

    return 0;
}

void GuildSetNeedApplyReq_SS::_HandleSetNeedApplyReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_SET_NEED_APPLY_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    SS_PKG_GUILD_SET_NEED_APPLY_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stGuildSetNeedApplyRsp;

    rstSsPkgRsp.m_nErrNo = ERR_NONE;
	rstSsPkgRsp.m_bNeedApply = rstSsPkg.m_stBody.m_stGuildSetNeedApplyReq.m_bNeedApply;

    do
    {
        if (poGuild == NULL)
        {
            rstSsPkgRsp.m_nErrNo = ERR_SYS;
            break;
        }

        DT_ONE_GUILD_MEMBER* pstGuildMember = poGuild->FindMember(rstSsPkg.m_stHead.m_ullUin);

        if(pstGuildMember==NULL ||
            (pstGuildMember->m_bIdentity != GUILD_IDENTITY_LEADER &&
            pstGuildMember->m_bIdentity != GUILD_IDENTITY_DEPUTY_LEADER))
        {
            rstSsPkgRsp.m_nErrNo = ERR_GUILD_PERMISSION_DENIED;
            break;
        }

        poGuild->SetNeedApply(rstSsPkg.m_stBody.m_stGuildSetNeedApplyReq.m_bNeedApply);

    }while(false);

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);

    return;
}

int GuildDealApplyReq_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_GUILD_DEAL_APPLICATION_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildDealApplyReq;

    Player* poPlayer = PlayerMgr::Instance().GetPlayer(rstSsPkgReq.m_ullApplicantUin);
    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgReq.m_ullGuildId);

    if (!pvPara && (!poPlayer ||!poGuild))
    {
        GuildTransFrame::Instance().AddGetGuildTrans(rstSsPkgReq.m_ullApplicantUin, rstSsPkgReq.m_ullGuildId, rstSsPkg);
    }
    else
    {
        _HandleGuildDealApplyReq(rstSsPkg, poGuild, poPlayer);
    }

    return 0;
}

void GuildDealApplyReq_SS::_HandleGuildDealApplyReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild, Player* poPlayer)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_DEAL_APPLICATION_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    SS_PKG_GUILD_DEAL_APPLICATION_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildDealApplyReq;
    SS_PKG_GUILD_DEAL_APPLICATION_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stGuildDealApplyRsp;
    rstSsPkgRsp.m_nErrNo = ERR_NONE;

    do
    {
        if (poGuild == NULL || poPlayer == NULL)
        {
            rstSsPkgRsp.m_nErrNo = ERR_SYS;
            break;
        }

        DT_ONE_GUILD_MEMBER* pstGuildMember = poGuild->FindMember(rstSsPkg.m_stHead.m_ullUin);
        if(pstGuildMember == NULL || pstGuildMember->m_bIdentity == GUILD_IDENTITY_MEMBER)
        {
            rstSsPkgRsp.m_nErrNo = ERR_GUILD_PERMISSION_DENIED;
            break;
        }

        DT_ONE_GUILD_MEMBER* pstApplyMemberInfo = poGuild->FindApply(rstSsPkgReq.m_ullApplicantUin);
        if (pstApplyMemberInfo == NULL)
        {
            rstSsPkgRsp.m_nErrNo = ERR_SYS;
            break;
        }

        //如果是同意
        if (rstSsPkgReq.m_bIsAgreed == 1)
        {
            if (poPlayer->GetGuildId() != 0)
            {
                rstSsPkgRsp.m_nErrNo = ERR_ALREADY_JOIN_OTHER_GUILD;
                poGuild->DelApply(rstSsPkgReq.m_ullApplicantUin);
                break;
            }

            if (poPlayer->FindApply(rstSsPkgReq.m_ullGuildId) < 0)
            {
                rstSsPkgRsp.m_nErrNo = ERR_ALREADY_JOIN_OTHER_GUILD;
                poGuild->DelApply(rstSsPkgReq.m_ullApplicantUin);
                break;
            }

            //将玩家加入公会
            pstApplyMemberInfo->m_ullJoinGuildTimeStap = CGameTime::Instance().GetCurrTimeMs();
            pstApplyMemberInfo->m_ullLastLogoutTimeStap = CGameTime::Instance().GetCurrTimeMs();
            pstApplyMemberInfo->m_stExtraInfo.m_bIsOnline = 0;
            int iRet = poGuild->AddMember(*pstApplyMemberInfo, pstGuildMember);
            if (iRet != ERR_NONE)
            {
               rstSsPkgRsp.m_nErrNo = iRet;
               break;
            }

            //更新玩家信息
            poGuild->DelApply(rstSsPkgReq.m_ullApplicantUin);
            poPlayer->SetGuildId(poGuild->GetGuildId());
            poPlayer->ClearApply();
        }
        else
        {
           poGuild->DelApply(rstSsPkgReq.m_ullApplicantUin);
           //poPlayer->DelApply(poGuild->GetGuildId());
        }
    }while(false);

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return;
}

int DissolveGuildReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_DISSOLVE_GUILD_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stDissolveGuildReq;

    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgReq.m_ullGuildId);

    if (!pvPara && !poGuild)
    {
        GuildTransFrame::Instance().AddGetGuildTrans(0, rstSsPkgReq.m_ullGuildId, rstSsPkg);
    }
    else
    {
        _HandleGuildDissolveReq(rstSsPkg, poGuild);
    }

    return 0;
}

void DissolveGuildReq_SS::_HandleGuildDissolveReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_DISSOLVE_GUILD_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    SS_PKG_DISSOLVE_GUILD_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stDissolveGuildRsp;
    rstSsPkgRsp.m_nErrNo = ERR_NONE;

    do
    {
        if (poGuild == NULL)
        {
            rstSsPkgRsp.m_nErrNo = ERR_SYS;
            break;
        }

        //判断玩家是否有条件进行解散公会操作
        DT_ONE_GUILD_MEMBER* pstMemInfo = poGuild->FindMember(rstSsPkg.m_stHead.m_ullUin);
        if (pstMemInfo==NULL || pstMemInfo->m_bIdentity != GUILD_IDENTITY_LEADER)
        {
            rstSsPkgRsp.m_nErrNo = ERR_GUILD_PERMISSION_DENIED;
            break;
        }

        int iRet = poGuild->Dissolve();
        rstSsPkgRsp.m_nErrNo = iRet;
    }while(false);

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return;
}

int QuitGuildReq_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_QUIT_GUILD_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stQuitGuildReq;

    Player* poPlayer = PlayerMgr::Instance().GetPlayer(rstSsPkg.m_stHead.m_ullUin);
    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgReq.m_ullGuildId);

    if (!pvPara && (!poPlayer ||!poGuild))
    {
        GuildTransFrame::Instance().AddGetGuildTrans(rstSsPkg.m_stHead.m_ullUin, rstSsPkgReq.m_ullGuildId, rstSsPkg);
    }
    else
    {
        _HandleGuildQuitReq(rstSsPkg, poGuild, poPlayer);
    }

    return 0;
}

void QuitGuildReq_SS::_HandleGuildQuitReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild, Player* poPlayer)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_QUIT_GUILD_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    SS_PKG_QUIT_GUILD_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stQuitGuildRsp;
    rstSsPkgRsp.m_nErrNo = ERR_NONE;

    do
    {
        if (poGuild == NULL || poPlayer == NULL)
        {
            rstSsPkgRsp.m_nErrNo = ERR_SYS;
            break;
        }

        int iRet = poGuild->DelMember( rstSsPkg.m_stHead.m_ullUin);
        if (iRet != ERR_NONE)
        {
            rstSsPkgRsp.m_nErrNo = iRet;
            break;
        }

        //更新玩家信息
        poPlayer->SetGuildId(0);
    }while(false);

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return;
}

int RefreshMemInfoNtf_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_REFRESH_MEMBERINFO_NTF& rstSsPkgReq = rstSsPkg.m_stBody.m_stRefreshMemInfoNtf;

    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgReq.m_ullGuildId);

    if (!pvPara && !poGuild)
    {
        GuildTransFrame::Instance().AddGetGuildTrans(0, rstSsPkgReq.m_ullGuildId, rstSsPkg);
    }
    else
    {
        _HandleGuildRefreshMemNtf(rstSsPkg, poGuild);
    }

    return 0;
}

void RefreshMemInfoNtf_SS::_HandleGuildRefreshMemNtf(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
    SS_PKG_REFRESH_MEMBERINFO_NTF& rstSsRefreshMemInfoNtf = rstSsPkg.m_stBody.m_stRefreshMemInfoNtf;

    if (poGuild == NULL)
    {
        LOGERR_r("RefreshMemInfo failed, Guild(%lu) not found", rstSsRefreshMemInfoNtf.m_ullGuildId);
        return;
    }

    poGuild->RefreshMemberInfo(rstSsRefreshMemInfoNtf);
}

int GuildLvUpReq_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_GUILD_LEVELUP_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildLvUpReq;

    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgReq.m_ullGuildId);

    if (!pvPara && !poGuild)
    {
        GuildTransFrame::Instance().AddGetGuildTrans(0, rstSsPkgReq.m_ullGuildId, rstSsPkg);
    }
    else
    {
        _HandleGuildLvUpReq(rstSsPkg, poGuild);
    }

    return 0;
}

void GuildLvUpReq_SS::_HandleGuildLvUpReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_LEVELUP_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    SS_PKG_GUILD_LEVELUP_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stGuildLvUpRsp;

    do
    {
        if (poGuild == NULL)
        {
            rstSsPkgRsp.m_nErrNo = ERR_SYS;
            break;
        }

        DT_ONE_GUILD_MEMBER* pstMemInfo = poGuild->FindMember(rstSsPkg.m_stHead.m_ullUin);
        if (pstMemInfo == NULL || pstMemInfo->m_bIdentity != GUILD_IDENTITY_LEADER)
        {
            rstSsPkgRsp.m_nErrNo = ERR_GUILD_PERMISSION_DENIED;
            break;
        }

        int iRet = poGuild->LevelUp();
        rstSsPkgRsp.m_nErrNo = iRet;
    }while(false);

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);

    return;
}

int UpdateGulidNotice_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_UPDATE_GUILD_NOTICE_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stUpdateGuildNoticeReq;

    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgReq.m_ullGuildId);
    if (!pvPara && !poGuild)
    {
        GuildTransFrame::Instance().AddGetGuildTrans(0, rstSsPkgReq.m_ullGuildId, rstSsPkg);
    }
    else
    {
        _HandleGuildUptNoticeReq(rstSsPkg, poGuild);
    }

    return 0;
}

void UpdateGulidNotice_SS::_HandleGuildUptNoticeReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_UPDATE_GUILD_NOTICE_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;

    SS_PKG_UPDATE_GUILD_NOTICE_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stUpdateGuildNoticeReq;
    SS_PKG_UPDATE_GUILD_NOTICE_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stUpdateGuildNoticeRsp;
    rstSsPkgRsp.m_nErrNo = ERR_NONE;

    do
    {
        if (poGuild == NULL)
        {
            rstSsPkgRsp.m_nErrNo = ERR_SYS;
            break;
        }

        DT_ONE_GUILD_MEMBER* pstMemInfo = poGuild->FindMember(rstSsPkg.m_stHead.m_ullUin);
        if (pstMemInfo==NULL || pstMemInfo->m_bIdentity==GUILD_IDENTITY_MEMBER)
        {
            rstSsPkgRsp.m_nErrNo = ERR_GUILD_PERMISSION_DENIED;
            break;
        }

        int iRet = poGuild->UptGuildNotice(rstSsPkgReq.m_szGulidNotice);
        rstSsPkgRsp.m_nErrNo = iRet;
    }while(false);

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return;
}

int GulidUptFundNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_GUILD_UPDATE_FUND_NTF& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildUpdateFundNtf;

    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgReq.m_ullGuildId);

    if (!pvPara && !poGuild)
    {
        GuildTransFrame::Instance().AddGetGuildTrans(0, rstSsPkgReq.m_ullGuildId, rstSsPkg);
    }
    else
    {
        _HandleGuildUptFundNtf(rstSsPkg, poGuild);
    }

    return 0;
}

void GulidUptFundNtf_SS::_HandleGuildUptFundNtf(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
    SS_PKG_GUILD_UPDATE_FUND_NTF& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildUpdateFundNtf;

    if (poGuild == NULL)
    {
        LOGERR_r("GuildUptFundNtf failed, Guild(%lu) not found", rstSsPkgReq.m_ullGuildId);
        return;
    }

    DT_ONE_GUILD_MEMBER* pstMemInfo = poGuild->FindMember(rstSsPkg.m_stHead.m_ullUin);
    if (pstMemInfo==NULL)
    {
        LOGERR_r("GuildUptFundNtf failed, Player(%lu) is not in Guild(%lu)", rstSsPkg.m_stHead.m_ullUin, rstSsPkgReq.m_ullGuildId);
        return;
    }

	//发事件广播
	DT_GUILD_NTF_MSG stNtfMsg;
	DT_GUILD_PKG_GUILD_LEVEL_EVENT& rstLevelEvent = stNtfMsg.m_stLevelEventBraod;
	rstLevelEvent.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
	rstLevelEvent.m_bType = GUILD_EVENT_DONATE;
	DT_GUILD_PKG_EVENT_DONATE& rstEventDonate = rstLevelEvent.m_stGuildEvent.m_stGuildEventDonate;
	rstEventDonate.m_bScienceType = rstSsPkgReq.m_bType;
	rstEventDonate.m_bDonateType = rstSsPkgReq.m_bDonateType;
	rstEventDonate.m_dwFundValueCh = rstSsPkgReq.m_iGuildFund;
	poGuild->SendBroadcastMsg(stNtfMsg, DT_GUILD_MSG_GUILD_LEVEL_EVENT);


	//记录历史资金
	pstMemInfo->m_dwFundTotle += rstSsPkgReq.m_iGuildFund;
	pstMemInfo->m_dwFundToday += rstSsPkgReq.m_iGuildFund;
	poGuild->BroadMemberInfoChange(*pstMemInfo);

	DT_GUILD_DONATE_INFO stDonateInfo;
	stDonateInfo.m_iFundValue = rstSsPkgReq.m_iGuildFund;
	StrCpy(stDonateInfo.m_szRoleName, pstMemInfo->m_szName, PKGMETA::MAX_NAME_LENGTH);
	stDonateInfo.m_bDonateType = rstSsPkgReq.m_bDonateType;

	if (rstSsPkgReq.m_bType == 0)
	{
		poGuild->AddFund(rstSsPkgReq.m_iGuildFund, &stDonateInfo);
	}
	else
	{
		poGuild->AddSocietyExp(rstSsPkgReq.m_bType, rstSsPkgReq.m_iGuildFund, &stDonateInfo);
	}

}

int SearchGuildReq_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_SEARCH_GUILD_REQ& rstSsSearchReq = rstSsPkg.m_stBody.m_stSearchGuildReq;

    //获取公会信息
    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsSearchReq.m_szGuildName);
    if (!pvPara && !poGuild)
    {
        GuildTransFrame::Instance().AddGetGuildByNameTrans(rstSsSearchReq.m_szGuildName, rstSsPkg);
    }
    else
    {
        _HandleSearchGuildReq(rstSsPkg, poGuild);
    }

    return 0;
}

void SearchGuildReq_SS::_HandleSearchGuildReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_SEARCH_GUILD_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;

    SS_PKG_SEARCH_GUILD_RSP& rstSsSearchRsp = m_stSsPkg.m_stBody.m_stSearchGuildRsp;

    if (!poGuild)
    {
        rstSsSearchRsp.m_bIsExist = 0;
    }
    else
    {
        rstSsSearchRsp.m_bIsExist = 1;
        poGuild->GetBriefInfo(rstSsSearchRsp.m_stGuildBriefInfo);
    }

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
}

int SetGuildMemJob_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_SET_GUILD_MEM_JOB_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stSetGuildMemJobReq;

    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgReq.m_ullGuildId);

    if (!pvPara && !poGuild)
    {
        GuildTransFrame::Instance().AddGetGuildTrans(0, rstSsPkgReq.m_ullGuildId, rstSsPkg);
    }
    else
    {
        _HandleGuildSetMemJobReq(rstSsPkg, poGuild);
    }

    return 0;
}

void SetGuildMemJob_SS::_HandleGuildSetMemJobReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_SET_GUILD_MEM_JOB_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;

    SS_PKG_SET_GUILD_MEM_JOB_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stSetGuildMemJobReq;
    SS_PKG_SET_GUILD_MEM_JOB_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stSetGuildMemJobRsp;
    rstSsPkgRsp.m_nErrNo = ERR_NONE;

    do
    {
        if (poGuild == NULL)
        {
            rstSsPkgRsp.m_nErrNo = ERR_SYS;
            break;
        }

        DT_ONE_GUILD_MEMBER* pstMemInfo = poGuild->FindMember(rstSsPkg.m_stHead.m_ullUin);
        if (pstMemInfo==NULL)
        {
            rstSsPkgRsp.m_nErrNo = ERR_GUILD_PERMISSION_DENIED;
            break;
        }

        int iRet = poGuild->SetGuildMemJob(pstMemInfo, rstSsPkgReq.m_ullMemUin, rstSsPkgReq.m_bMemJob);
        rstSsPkgRsp.m_nErrNo = iRet;
    }while(false);

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return;
}

int GuildKickPlayerReq_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_GUILD_KICK_PLAYER_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildKickPlayerReq;

    Player* poPlayer = PlayerMgr::Instance().GetPlayer(rstSsPkgReq.m_ullPlayerId);
    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgReq.m_ullGuildId);

    if (!pvPara && (!poPlayer ||!poGuild))
    {
        GuildTransFrame::Instance().AddGetGuildTrans(rstSsPkgReq.m_ullPlayerId, rstSsPkgReq.m_ullGuildId, rstSsPkg);
    }
    else
    {
        _HandleGuildKickPlayerReq(rstSsPkg, poGuild, poPlayer);
    }

    return 0;
}

void GuildKickPlayerReq_SS::_HandleGuildKickPlayerReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild, Player* poPlayer)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_KICK_PLAYER_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;

    SS_PKG_GUILD_KICK_PLAYER_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildKickPlayerReq;
    SS_PKG_GUILD_KICK_PLAYER_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stGuildKickPlayerRsp;
    rstSsPkgRsp.m_ullPlayerId = rstSsPkgReq.m_ullPlayerId;
    rstSsPkgRsp.m_nErrNo = ERR_NONE;

    do
    {
        if (poGuild == NULL || poPlayer == NULL)
        {
            rstSsPkgRsp.m_nErrNo = ERR_SYS;
            break;
        }

        DT_ONE_GUILD_MEMBER* pstOfficer = poGuild->FindMember(rstSsPkg.m_stHead.m_ullUin);
        if (pstOfficer==NULL)
        {
            rstSsPkgRsp.m_nErrNo = ERR_GUILD_PERMISSION_DENIED;
            break;
        }

        int iRet = poGuild->KickGuildMem(pstOfficer, rstSsPkgReq.m_ullPlayerId);
        if (iRet != ERR_NONE)
        {
            rstSsPkgRsp.m_nErrNo = iRet;
            break;
        }

        poPlayer->SetGuildId(0);
    }while(false);

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return;
}

// 军团战报名
int GFightApplyReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    SS_PKG_GUILD_FIGHT_APPLY_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildFightApplyReq;

    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgReq.m_ullGuildId);

    if (!pvPara && !poGuild)
    {
        GuildTransFrame::Instance().AddGetGuildTrans(0, rstSsPkgReq.m_ullGuildId, rstSsPkg);
    }
    else
    {
        _HandleGuildFightApplyReq(rstSsPkg, poGuild);
    }

    return 0;
}

void GFightApplyReq_SS::_HandleGuildFightApplyReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_FIGHT_APPLY_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;

    SS_PKG_GUILD_FIGHT_APPLY_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildFightApplyReq;
    SS_PKG_GUILD_FIGHT_APPLY_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stGuildFightApplyRsp;
    rstSsPkgRsp.m_nErrNo = ERR_NONE;

    do
    {
        if (poGuild == NULL)
        {
            rstSsPkgRsp.m_nErrNo = ERR_SYS;
            break;
        }

        DT_ONE_GUILD_MEMBER* pstMemInfo = poGuild->FindMember(rstSsPkg.m_stHead.m_ullUin);
        if (pstMemInfo==NULL || pstMemInfo->m_bIdentity == GUILD_IDENTITY_MEMBER)
        {
            rstSsPkgRsp.m_nErrNo = ERR_GUILD_PERMISSION_DENIED;
            break;
        }

        int iRet = GuildFightMgr::Instance().FightApply(poGuild, rstSsPkgReq.m_dwApplyFund);
        rstSsPkgRsp.m_nErrNo = iRet;
    }while(false);

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return;
}

//发公会聊天
int GuildMessageSendReq_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_GUILD_MESSAGE_SEND_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildMessageSendReq;

    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgReq.m_ullGuildId);

    if (!pvPara && !poGuild)
    {
        GuildTransFrame::Instance().AddGetGuildTrans(0, rstSsPkgReq.m_ullGuildId, rstSsPkg);
    }
    else
    {
        _HandleMessageSendReq(rstSsPkg, poGuild);
    }

    return 0;
}

void GuildMessageSendReq_SS::_HandleMessageSendReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_MESSAGE_SEND_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;

    SS_PKG_GUILD_MESSAGE_SEND_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildMessageSendReq;
    SS_PKG_GUILD_MESSAGE_SEND_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stGuildMessageSendRsp;
    rstSsPkgRsp.m_nErrNo = ERR_NONE;

    do
    {
        if (poGuild == NULL)
        {
            rstSsPkgRsp.m_nErrNo = ERR_SYS;
            break;
        }

        DT_GUILD_NTF_MSG stNtfMsg;
        DT_GUILD_PKG_MESSAGE_SEND& rstMessageSend = stNtfMsg.m_stMessageSend; // 公会聊天
        rstMessageSend.m_stRecord = rstSsPkgReq.m_stRecord;

        poGuild->SendBroadcastMsg(stNtfMsg, DT_GUILD_MSG_MESSAGE_SEND);
        rstSsPkgRsp.m_nErrNo = ERR_NONE;
    }while(false);

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return;
}

//更新公会约战房间
int RefreshRoomNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_REFRESH_GUILD_PVP_ROOM_NTF& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildRoomRefreshNtf;

    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgReq.m_stRoomInfo.m_ullReserveId);

    if (!pvPara && !poGuild)
    {
        GuildTransFrame::Instance().AddGetGuildTrans(0, rstSsPkgReq.m_stRoomInfo.m_ullReserveId, rstSsPkg);
    }
    else
    {
        _HandleGuildRefreshRoomNtf(rstSsPkg, poGuild);
    }

    return 0;

}

void RefreshRoomNtf_SS::_HandleGuildRefreshRoomNtf(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
    SS_PKG_REFRESH_GUILD_PVP_ROOM_NTF& rstSsRefreshRoomNtf = rstSsPkg.m_stBody.m_stGuildRoomRefreshNtf;

    if (poGuild == NULL)
    {
        LOGERR_r("RefreshRoom failed, Guild(%lu) not found", rstSsRefreshRoomNtf.m_stRoomInfo.m_ullReserveId);
        return;
    }

    poGuild->RefreshRoomInfo(rstSsRefreshRoomNtf);
}

//GM修改公会信息
int GuildGmUpdateInfoReq_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    return 0;
}

//获取军团排行榜
int GetGuildRankReq_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    SS_PKG_GUILD_GET_GUILD_RANK_REQ & rstGetRankReq = rstSsPkg.m_stBody.m_stGuildGetGuildRankReq;
    SS_PKG_GUILD_GET_GUILD_RANK_RSP & rstGetRankRsp = m_stSsPkg.m_stBody.m_stGuildGetGuildRankRsp;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_GET_GUILD_RANK_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;

	uint64_t ullRetVersion = GuildRankMgr::Instance().GetVersion();

	do
	{
		rstGetRankRsp.m_ullVersion = ullRetVersion;
		rstGetRankRsp.m_dwSelfRank = GuildRankMgr::Instance().GetRank(rstSsPkg.m_stHead.m_ullUin);
		if (rstGetRankReq.m_ullVersion == ullRetVersion)
		{
			break;
		}
		rstGetRankRsp.m_nCount = GuildRankMgr::Instance().GetTopList(rstGetRankRsp.m_astRankList);
	} while (false);

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return 0;
}

//获取军团积分排行榜
int GetGFightRankReq_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    SS_PKG_GUILD_GET_GFIGHT_RANK_RSP & rstGetRankRsp = m_stSsPkg.m_stBody.m_stGuildGetGFightRankRsp;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_GET_GFIGHT_RANK_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    rstGetRankRsp.m_nCount = GFightRankMgr::Instance().GetTopList(rstGetRankRsp.m_astRankList);
    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return 0;
}

//上传录像请求
int GuildUploadReplayReq_SS::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    SS_PKG_UPLOAD_REPLAY_REQ & rstUploadReq = rstSsPkg.m_stBody.m_stUploadReplayReq;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_UPLOAD_REPLAY_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    SS_PKG_UPLOAD_REPLAY_RSP & rstUploadRsp = m_stSsPkg.m_stBody.m_stUploadReplayRsp;

    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkg.m_stHead.m_ullReservId);
    if (poGuild==NULL)
    {
        rstUploadRsp.m_nErrNo = ERR_SYS;
    }
    else
    {
        rstUploadRsp.m_nErrNo = poGuild->UploadReplay(&rstUploadReq.m_stFileHead, rstUploadRsp.m_szURL);
    }

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return 0;
}




int GulidBossGetInfoReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    return 0;
}

int GulidBossResetReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    SS_PKG_GUILD_BOSS_RESET_REQ& rstSsPkgNtf = rstSsPkg.m_stBody.m_stGuildBossResetReq;
    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgNtf.m_ullGuildId);

    if (!pvPara && !poGuild)
    {
        GuildTransFrame::Instance().AddGetGuildTrans(0, rstSsPkgNtf.m_ullGuildId, rstSsPkg);
    }
    else
    {
        _HandleBossResetReq(rstSsPkg, poGuild);
    }
    return 0;
}

void GulidBossResetReq_SS::_HandleBossResetReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
    SS_PKG_GUILD_BOSS_RESET_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildBossResetReq;

    if (poGuild == NULL)
    {
        LOGERR_r("RefreshMemInfo failed, Guild(%lu) not found", rstSsPkgReq.m_ullGuildId);
        return;
    }
    poGuild->ResetBoss(rstSsPkg.m_stHead.m_ullUin);

}


int GulidBossFightSettleNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    SS_PKG_GUILD_BOSS_FIGHT_SETTLE_NTF& rstSsPkgNtf = rstSsPkg.m_stBody.m_stGuildBossFightSettleNtf;

    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgNtf.m_ullGuildId);

    if (!pvPara && !poGuild)
    {
        GuildTransFrame::Instance().AddGetGuildTrans(0, rstSsPkgNtf.m_ullGuildId, rstSsPkg);
    }
    else
    {
        _HandleBossFightSettleNtf(rstSsPkg, poGuild);
    }
    return 0;
}


void GulidBossFightSettleNtf_SS::_HandleBossFightSettleNtf(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
    SS_PKG_GUILD_BOSS_FIGHT_SETTLE_NTF& rstSsPkgNtf = rstSsPkg.m_stBody.m_stGuildBossFightSettleNtf;

    if (poGuild == NULL)
    {
        LOGERR_r("_HandleBossFightSettleNtf failed, Guild(%lu) not found", rstSsPkgNtf.m_ullGuildId);
        return;
    }
    poGuild->SettleBoss(rstSsPkg.m_stHead.m_ullUin ,rstSsPkgNtf);
}


int GuildBossEnterFightReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
    SS_PKG_GUILD_BOSS_ENTER_FIGHT_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildBossEnterFightReq;

    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgReq.m_ullGuildId);

    if (!pvPara && !poGuild)
    {
        GuildTransFrame::Instance().AddGetGuildTrans(0, rstSsPkgReq.m_ullGuildId, rstSsPkg);
    }
    else
    {
        _HandleBossEnterFightReq(rstSsPkg, poGuild);
    }
    return 0;
}


void GuildBossEnterFightReq_SS::_HandleBossEnterFightReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
    bzero(&m_stSsPkg, sizeof(m_stSsPkg));
    SS_PKG_GUILD_BOSS_ENTER_FIGHT_REQ& rstReq = rstSsPkg.m_stBody.m_stGuildBossEnterFightReq;
    SS_PKG_GUILD_BOSS_ENTER_FIGHT_RSP& rstRsp = m_stSsPkg.m_stBody.m_stGuildBossEnterFightRsp;
    if (poGuild == NULL)
    {
        LOGERR_r("poGuild<%lu> is not found.", rstReq.m_ullGuildId);
        return;
    }
    poGuild->EnterBossFight(rstSsPkg.m_stHead.m_ullUin, rstReq.m_dwFLevelId, rstRsp);
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_BOSS_ENTER_FIGHT_RSP;
    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
}


int GuildSetLevelLimitReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
	SS_PKG_GUILD_SET_LEVEL_LIMIT_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildSetLevelLimitReq;

	Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgReq.m_ullGuildId);

	if (!pvPara && !poGuild)
	{
		GuildTransFrame::Instance().AddGetGuildTrans(rstSsPkg.m_stHead.m_ullUin, rstSsPkgReq.m_ullGuildId, rstSsPkg);
	}
	else
	{
		_HandleSetLevelLimitReq(rstSsPkg, poGuild);
	}

	return 0;
}

void GuildSetLevelLimitReq_SS::_HandleSetLevelLimitReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_SET_LEVEL_LIMIT_RSP;
	m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
	SS_PKG_GUILD_SET_LEVEL_LIMIT_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stGuildSetLevelLimitRsp;

	rstSsPkgRsp.m_nErrNo = ERR_NONE;
	rstSsPkgRsp.m_bLevelLimit = rstSsPkg.m_stBody.m_stGuildSetLevelLimitReq.m_bLevelLimit;

	do
	{
		if (poGuild == NULL)
		{
			rstSsPkgRsp.m_nErrNo = ERR_SYS;
			break;
		}

		DT_ONE_GUILD_MEMBER* pstGuildMember = poGuild->FindMember(rstSsPkg.m_stHead.m_ullUin);

		if(pstGuildMember==NULL ||
			(pstGuildMember->m_bIdentity != GUILD_IDENTITY_LEADER &&
			pstGuildMember->m_bIdentity != GUILD_IDENTITY_DEPUTY_LEADER))
		{
			rstSsPkgRsp.m_nErrNo = ERR_GUILD_PERMISSION_DENIED;
			break;
		}

		poGuild->SetJoinLevel(rstSsPkg.m_stBody.m_stGuildSetLevelLimitReq.m_bLevelLimit);

	}while(false);

	GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);

	return;
}


int GuildLevelEventNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
	SS_PKG_GUILD_LEVEL_EVENT_NTF& rstNtf = rstSsPkg.m_stBody.m_stGuildLevelEventNtf;

	Guild* poGuild = GuildMgr::Instance().GetGuild(rstNtf.m_ullGuildId);

	if (!pvPara && !poGuild)
	{
		GuildTransFrame::Instance().AddGetGuildTrans(rstSsPkg.m_stHead.m_ullUin, rstNtf.m_ullGuildId, rstSsPkg);
	}
	else
	{
		_HandleLevelEventNtf(rstSsPkg, poGuild);
	}

	return 0;
}

void GuildLevelEventNtf_SS::_HandleLevelEventNtf(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
	if (poGuild == NULL)
	{
		return;
	}

	SS_PKG_GUILD_LEVEL_EVENT_NTF& rstNtf = rstSsPkg.m_stBody.m_stGuildLevelEventNtf;

	//发事件广播
	DT_GUILD_NTF_MSG stNtfMsg;
	DT_GUILD_PKG_GUILD_LEVEL_EVENT& rstLevelEvent = stNtfMsg.m_stLevelEventBraod;

	rstLevelEvent.m_ullUin = rstSsPkg.m_stHead.m_ullUin;

	switch (rstNtf.m_bType)
	{
	case 4:/*CHAPTER_TYPE_GUILD*/
		{
			rstLevelEvent.m_bType = GUILD_EVENT_LEVEL;
			DT_GUILD_PKG_EVENT_LEVEL& rstEventLevel = rstLevelEvent.m_stGuildEvent.m_stGuildEventLevel;
			rstEventLevel.m_dwLevelId = rstNtf.m_dwLevelId;
			rstEventLevel.m_bScore = rstNtf.m_dwValue;
			break;
		}
	case 5:/*CHAPTER_TYPE_ACTIVITY*/
		{
			rstLevelEvent.m_bType = GUILD_EVENT_GOLD;
			DT_GUILD_PKG_EVENT_GOLD& rstEventGold = rstLevelEvent.m_stGuildEvent.m_stGuildEventGold;
			rstEventGold.m_dwLevelId = rstNtf.m_dwLevelId;
			rstEventGold.m_dwScore = rstNtf.m_dwValue;
			break;
		}
	default:
		return;
		break;
	}

	poGuild->SendBroadcastMsg(stNtfMsg, DT_GUILD_MSG_GUILD_LEVEL_EVENT);
}


int GuildDrawSalaryReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /*= NULL */)
{
	SS_PKG_GUILD_SET_LEVEL_LIMIT_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildSetLevelLimitReq;

	Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgReq.m_ullGuildId);

	if (!pvPara && !poGuild)
	{
		GuildTransFrame::Instance().AddGetGuildTrans(rstSsPkg.m_stHead.m_ullUin, rstSsPkgReq.m_ullGuildId, rstSsPkg);
	}
	else
	{
		_HandleDrawSalaryReq(rstSsPkg, poGuild);
	}

	return 0;
}

void GuildDrawSalaryReq_SS::_HandleDrawSalaryReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
	SS_PKG_GUILD_DRAW_SALARY_RSP& rstSsPkgBodyRsp = m_stSsPkg.m_stBody.m_stGuildDrawSalaryRsp;
	m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_DRAW_SALARY_RSP;

	do
	{
		if (poGuild == NULL)
		{
			rstSsPkgBodyRsp.m_nErrNo = ERR_SYS;
			break;
		}

		DT_ONE_GUILD_MEMBER* pstMemInfo = poGuild->FindMember(rstSsPkg.m_stHead.m_ullUin);
		if (pstMemInfo == NULL)
		{
			rstSsPkgBodyRsp.m_nErrNo = ERR_SYS;
			break;
		}

		rstSsPkgBodyRsp.m_bSalaryIdentityToday = pstMemInfo->m_bSalaryIdentityToday;
		rstSsPkgBodyRsp.m_nErrNo = ERR_NONE;

	} while (false);

	GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);

}

//int GuildCompetitorInfoReq_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
//{
//    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_BOSS_COMPETITOR_INFO_RSP;
//    SS_PKG_GUILD_BOSS_COMPETITOR_INFO_REQ& rstSsReq = rstSsPkg.m_stBody.m_stGuildBossCompetitorInfoReq;
//    SS_PKG_GUILD_BOSS_COMPETITOR_INFO_RSP& rstSsRsp = m_stSsPkg.m_stBody.m_stGuildBossCompetitorInfoRsp;
//    DT_GUILD_RANK_INFO rstGuildInfo = { 0 };
//    GuildBossMgr::Instance().GetCompetitorInfo(rstSsReq.m_ullMyGuildID, &rstGuildInfo);
//    if (rstGuildInfo.m_ullGuildId == 0)
//    {
//        rstSsRsp.m_bIsCompetitorExisted = 0;
//        GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
//        return 0;
//    }
//    Player* poPlayer = PlayerMgr::Instance().GetPlayer(rstSsPkg.m_stHead.m_ullUin);
//    if (poPlayer == NULL)
//    {
//        LOGERR_r("Cannot find player");
//        return -1;
//    }
//
//    Guild* poCompetitorGuild = GuildMgr::Instance().GetGuild(rstGuildInfo.m_ullGuildId);
//    //如果对象不在内存中
//    if (pvPara == NULL && poCompetitorGuild == NULL)
//    {
//        GuildTransFrame::Instance().AddGetGuildTrans(
//            rstSsPkg.m_stHead.m_ullUin,
//            rstSsRsp.m_stCompetitorGuildInfo.m_ullGuildID,
//            rstSsPkg);
//    }
//    else
//    {
//        _HandleServerMsg(rstSsPkg, poCompetitorGuild, poPlayer);
//    }
//    return 0;
//}

//void GuildCompetitorInfoReq_SS::_HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, Guild * poGuild, Player * poPlayer)
//{
//    SS_PKG_GUILD_BOSS_COMPETITOR_INFO_RSP& rstSsRsp = m_stSsPkg.m_stBody.m_stGuildBossCompetitorInfoRsp;
//    rstSsRsp.m_stCompetitorGuildInfo.m_dwBossID = poGuild->GetGuildBoss().GetBossInfo().m_dwCurBossId;
//    rstSsRsp.m_stCompetitorGuildInfo.m_dwHpRemaining = poGuild->GetGuildBoss().GetBossLeftHp(rstSsRsp.m_stCompetitorGuildInfo.m_dwBossID);
//    StrCpy(rstSsRsp.m_stCompetitorGuildInfo.m_szGuildName, poGuild->GetGuildName(), MAX_NAME_LENGTH);
//    rstSsRsp.m_stCompetitorGuildInfo.m_ullGuildID = poGuild->GetGuildId();
//    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
//}

int GuildBossDamageRankInGuildReq_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_BOSS_DAMAGE_RANK_IN_GUILD_RSP;
    SS_PKG_GUILD_BOSS_DAMAGE_RANK_IN_GUILD_RSP& rstSsRsp = m_stSsPkg.m_stBody.m_stGuildBossDamageRankInGuildRsp;
    SS_PKG_GUILD_BOSS_DAMAGE_RANK_IN_GUILD_REQ& rstSsReq = rstSsPkg.m_stBody.m_stGuildBossDamageRankInGuildReq;
    uint32_t dwBossId = rstSsReq.m_bBossId;
    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsReq.m_ullGuildId);
    if (poGuild == NULL)
    {
        LOGERR_r("poGuild<%lu> is NULL", rstSsReq.m_ullGuildId);
        return -1;
    }

    DT_GUILD_ONE_BOSS_INFO* pstOneBossInfo = poGuild->GetGuildBoss().GetOneBossInfo(dwBossId);
    rstSsRsp.m_wCount = pstOneBossInfo->m_wAttackedMemNum;
    memcpy(rstSsRsp.m_astGuildMemberDamageRank, pstOneBossInfo->m_astDamageOfBossList,
        sizeof(DT_GUILD_BOSS_DAMAGE_NODE) * pstOneBossInfo->m_wAttackedMemNum);

    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return 0;
}

//int GuildBossGetRankListRsp_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
//{
//    SS_PKG_GUILD_BOSS_GET_RANK_LIST_RSP& rstRsp = m_stSsPkg.m_stBody.m_stGuildBossGetRankListRsp;
//
//    GuildBossMgr::Instance().SetGuildRankList(rstRsp.m_astRankList, rstRsp.m_wCount);
//    return 0;
//}

int GuildGetMemberListReq_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_GET_MEMBER_LIST_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    SS_PKG_GUILD_GET_MEMBER_LIST_RSP& rstRsp = m_stSsPkg.m_stBody.m_stGuildGetGuildMemRsp;
    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkg.m_stBody.m_stGuildGetGuildMemReq.m_ullGuildId);
    if (poGuild == NULL)
    {
        LOGERR("poGuild is NULL");
        return -1;
    }
    GuildMember oGuildMember = poGuild->GetGuildMemInfo();
    rstRsp.m_wCount = oGuildMember.GetCurMemberNum();
    memcpy(rstRsp.m_UinList, oGuildMember.GetMemberList(), sizeof(rstRsp.m_UinList));
    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return 0;
}

int GuildBossGetPassedListReq_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_BOSS_PASSED_GUILD_LIST_RSP;
    uint64_t ullMyGuildId = rstSsPkg.m_stBody.m_stGuildBossPassedGuildListReq.m_ullGuildId;
    Guild* poGuild = GuildMgr::Instance().GetGuild(ullMyGuildId);
    if (poGuild == NULL)
    {
        LOGERR_r("poGuild is Null");
        m_stSsPkg.m_stBody.m_stGuildBossPassedGuildListRsp.m_nErrNo = ERR_DEFAULT;
        GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
        return 0;
    }
    uint32_t dwBossId = rstSsPkg.m_stBody.m_stGuildBossPassedGuildListReq.m_bBossId;
    int iBossIndex = poGuild->GetGuildBoss().FindBossIndex(dwBossId);
    if (iBossIndex >= 0)
    {
        m_stSsPkg.m_stBody.m_stGuildBossPassedGuildListRsp.m_nErrNo = ERR_NONE;
        DT_GUILD_BOSS_PASSED_GUILD* pstPassedGuildList =
            GuildBossMgr::Instance().GetGuildBossPassedGuildInfo(iBossIndex);
        if (pstPassedGuildList == NULL)
        {
            return -1;
        }
        SS_PKG_GUILD_BOSS_PASSED_GUILD_LIST_RSP& rstRsp = m_stSsPkg.m_stBody.m_stGuildBossPassedGuildListRsp;
        rstRsp.m_stPassedGuildList = *pstPassedGuildList;
        for (int i = 0; i < pstPassedGuildList->m_bCount; ++i)
        {
            if (pstPassedGuildList->m_GuildId[i] == ullMyGuildId)
            {
                rstRsp.m_bBeInFlag = 1;
                break;
            }
        }
        rstRsp.m_ullMyGuildPassedTime = poGuild->GetGuildBoss().GetOneBossInfo(dwBossId)->m_ullPassedTimeStamp;
        GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
        return 0;
    }
    else
    {
        return -1;
    }
}

int GuildSpeedPartnerReq_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
	SS_PKG_SPEED_PARTNER_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildSpeedPartnerReq;

	Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgReq.m_ullGuildId);

	if (!pvPara && !poGuild)
	{
		GuildTransFrame::Instance().AddGetGuildTrans(rstSsPkg.m_stHead.m_ullUin, rstSsPkgReq.m_ullGuildId, rstSsPkg);
	}
	else
	{
		_HandleSpeedPartnerReq(rstSsPkg, poGuild);
	}

	return 0;
}

void GuildSpeedPartnerReq_SS::_HandleSpeedPartnerReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
	SS_PKG_SPEED_PARTNER_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildSpeedPartnerReq;
	SS_PKG_SPEED_PARTNER_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stGuildSpeedPartnerRsp;

	int iErr = ERR_NONE;
	do
	{
		if (poGuild == NULL)
		{
			LOGERR("poGuild is null. GuildId<%lu>", rstSsPkgReq.m_ullGuildId);
			iErr = ERR_SYS;
			break;
		}

		iErr = poGuild->SpeedPartner(rstSsPkg.m_stHead.m_ullUin, rstSsPkgReq.m_ullTargetUin);

	} while (false);

	rstSsPkgRsp.m_nErrNo = iErr;
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_SPEED_PARTNER_RSP;
	m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;

	GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);

}

int GuildGetBeSpeededInfoReq_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
	SS_PKG_GET_BE_SPEED_INFO_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildGetBeSpeededInfoReq;

	Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgReq.m_ullGuildId);

	if (!pvPara && !poGuild)
	{
		GuildTransFrame::Instance().AddGetGuildTrans(rstSsPkg.m_stHead.m_ullUin, rstSsPkgReq.m_ullGuildId, rstSsPkg);
	}
	else
	{
		_HandleGetBeSpeededInfoReq(rstSsPkg, poGuild);
	}

	return 0;
}

void GuildGetBeSpeededInfoReq_SS::_HandleGetBeSpeededInfoReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
	SS_PKG_GET_BE_SPEED_INFO_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildGetBeSpeededInfoReq;
	SS_PKG_GET_BE_SPEED_INFO_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stGuildGetBeSpeededInfoRsp;

	int iErr = ERR_NONE;
	do
	{
		if (poGuild == NULL)
		{
			LOGERR("poGuild is null. GuildId<%lu>", rstSsPkgReq.m_ullGuildId);
			iErr = ERR_SYS;
			break;
		}

		iErr = poGuild->GetSpeedInfo(rstSsPkg.m_stHead.m_ullUin, rstSsPkgRsp.m_bBeSpeededCount);
	} while (false);

	rstSsPkgRsp.m_nErrNo = iErr;
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GET_BE_SPEED_INFO_RSP;
	m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;

	GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);

}

int GuildHangStarReq_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    SS_PKG_GUILD_HANG_STAR_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildHangStarReq;

    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkgReq.m_ullGuildId);

    if (!pvPara && !poGuild)
    {
        GuildTransFrame::Instance().AddGetGuildTrans(rstSsPkg.m_stHead.m_ullUin, rstSsPkgReq.m_ullGuildId, rstSsPkg);
    }
    else
    {
        _HandleGuildHangStarReq(rstSsPkg, poGuild);
    }

    return 0;
}

void GuildHangStarReq_SS::_HandleGuildHangStarReq(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
    SS_PKG_GUILD_HANG_STAR_REQ& rstSsPkgReq = rstSsPkg.m_stBody.m_stGuildHangStarReq;
    SS_PKG_GUILD_HANG_STAR_RSP& rstSsPkgRsp = m_stSsPkg.m_stBody.m_stGuildHangStarRsp;
    rstSsPkgRsp.m_stHangStarInfo.m_bCount = 0;

    int iErr = ERR_NONE;
    do 
    {
        if (poGuild == NULL)
        {
            LOGERR_r("poGuild<%lu> is null.", rstSsPkgReq.m_ullGuildId);
            iErr = ERR_SYS;
            break;
        }
        
        iErr = poGuild->ChangeStar(rstSsPkg.m_stHead.m_ullUin, rstSsPkgReq.m_ullTargetUin, rstSsPkgReq.m_bType, rstSsPkgRsp.m_stHangStarInfo);
        rstSsPkgRsp.m_bType = rstSsPkgReq.m_bType;
    } while (false);
    
    rstSsPkgRsp.m_nErrNo = iErr;
    
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_HANG_STAR_RSP;

    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);

}    

int GuildBossGetMemFightTimesReq_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    Guild* poGuild = GuildMgr::Instance().GetGuild(rstSsPkg.m_stBody.m_stGuildBossGetMemFightTimesReq.m_ullGuildId);
    if (poGuild == NULL)
    {
        LOGERR_r("poGuild<%lu> is NULL", rstSsPkg.m_stBody.m_stGuildBossGetMemFightTimesReq.m_ullGuildId);
        return 0;
    }

    SS_PKG_GUILD_BOSS_GET_MEM_FIGHT_TIMES_RSP& rstRsp = m_stSsPkg.m_stBody.m_stGuildBossGetMemFightTimesRsp;
    poGuild->GetGuildBoss().GetBossInfo().m_astMemFightBossTimes;
    rstRsp.m_wMemNum = poGuild->GetGuildBoss().GetBossInfo().m_wMemNum;
    memcpy(rstRsp.m_astMemFightBossTimes, poGuild->GetGuildBoss().GetBossInfo().m_astMemFightBossTimes, sizeof(DT_GUILD_BOSS_FIGHT_TIMES) * rstRsp.m_wMemNum);
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_BOSS_GET_MEM_FIGHT_TIMES_RSP;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    LOGRUN_r("There are <%d> players in this guild", rstRsp.m_wMemNum);
    LOGRUN_r("Player<%lu> fight <%d> times.", rstRsp.m_astMemFightBossTimes[0].m_ullUin, rstRsp.m_astMemFightBossTimes[0].m_bFightTimes);
    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return 0;
}

int GuildBossSetBossFightTimesToZeroNtf_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    uint64_t ullGuildId = rstSsPkg.m_stBody.m_stGuildBossSetBossFightTimesToZeroNtf.m_ullGuildId;
    Guild* poGuild = GuildMgr::Instance().GetGuild(ullGuildId);
    if (poGuild == NULL)
    {
        LOGERR_r("poGuild<%lu> is NULL.", ullGuildId);
        return -1;
    }

    uint16_t wMemNum = poGuild->GetGuildMemInfo().GetCurMemberNum();
    DT_GUILD_BOSS_FIGHT_TIMES *pstFightTimes = poGuild->GetGuildBoss().GetBossInfo().m_astMemFightBossTimes;
    DT_GUILD_BOSS_FIGHT_TIMES stFightTimes;
    stFightTimes.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    int iEqual = 0;
    int iIndex = MyBSearch(&stFightTimes, poGuild->GetGuildBoss().GetBossInfo().m_astMemFightBossTimes, wMemNum,
        sizeof(DT_GUILD_BOSS_FIGHT_TIMES), &iEqual, UinCmp);
    if (iEqual != 0)
    {
        pstFightTimes[iIndex].m_bFightTimes = 0;
    }
    else
    {
        LOGERR_r("player <%lu> isn't in this guild <%lu>.", rstSsPkg.m_stHead.m_ullUin, poGuild->GetGuildId());
    }

    return 0;
}

int GuildBossGetMemWhoGetSubRwdReq_SS::HandleServerMsg(PKGMETA::SSPKG & rstSsPkg, void * pvPara)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_BOSS_GET_MEM_WHO_GET_SUB_RWD_RSP;
    uint64_t ullGuildId = rstSsPkg.m_stBody.m_stGuildBossGetMemWhoGetSubRwdReq.m_ullGuildId;
    Guild* poGuild = GuildMgr::Instance().GetGuild(ullGuildId);
    if (poGuild == NULL)
    {
        LOGERR_r("poGuild<%lu> is NULL.", ullGuildId);
        return -1;
    }
    SS_PKG_GUILD_BOSS_GET_MEM_WHO_GET_SUB_RWD_RSP& rstSsRsp = m_stSsPkg.m_stBody.m_stGuildBossGetMemWhoGetSubRwdRsp;
    memcpy(rstSsRsp.m_astMemGetSubRwdList, poGuild->GetGuildBoss().GetBossInfo().m_astMemGetSubRwdList,
        sizeof(DT_GUILD_BOSS_GET_SUB_RWD_NODE) * poGuild->GetGuildBoss().GetBossInfo().m_wMemGetSubRwdNum);
    rstSsRsp.m_wMemGetSubRwdNum = poGuild->GetGuildBoss().GetBossInfo().m_wMemGetSubRwdNum;
    m_stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    GuildSvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
    return 0;
}

int GuildExpeditionCommonNtf_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara /* NULL */)
{

	SS_PKG_GUILD_EXPEDITION_COMMON_NTF& rstNtf = rstSsPkg.m_stBody.m_stGuildExpeditionCommonNtf;

	Guild* poGuild = GuildMgr::Instance().GetGuild(rstNtf.m_ullGuildId);
	if (!pvPara && !poGuild)
	{
		GuildTransFrame::Instance().AddGetGuildTrans(0, rstNtf.m_ullGuildId, rstSsPkg);
	}
	else
	{
		_HandlExpeditionCommonNtf(rstSsPkg, poGuild);
	}
	return 0;

}

void GuildExpeditionCommonNtf_SS::_HandlExpeditionCommonNtf(PKGMETA::SSPKG& rstSsPkg, Guild* poGuild)
{
	if (poGuild == NULL)
	{
		return;
	}
	SS_PKG_GUILD_EXPEDITION_COMMON_NTF& rstNtf = rstSsPkg.m_stBody.m_stGuildExpeditionCommonNtf;

	switch (rstNtf.m_wMsgId)
	{
	case DT_GUILD_MSG_EXPEDITION_AWARD:
		poGuild->HandleExpeditionAward(rstNtf.m_stNtfInfo.m_stExpeditionAward.m_bAwardType);
		break;
	case DT_GUILD_MSG_EXPEDITION_UPDATE_DEFEND_INFO:
		poGuild->UpdateExpeidtionDefendInfo(rstNtf.m_stNtfInfo.m_stExpeditionUpdateDefendInfo);
		poGuild->SendBroadcastMsg(rstNtf.m_stNtfInfo, rstNtf.m_wMsgId);
		break;
	case DT_GUILD_MSG_EXPEDITION_FIGHT_RESULT:
		poGuild->SendBroadcastMsg(rstNtf.m_stNtfInfo, rstNtf.m_wMsgId);
		break;
	default:
		LOGERR_r("Guild<%lu> Expedition Common ntf error.MsgId<%hu>", poGuild->GetGuildId(), rstNtf.m_wMsgId);
		break;
	}
}
