#include "Gm.h"
#include "../player/PlayerMgr.h"
#include "LogMacros.h"
#include "../Item.h"
#include "../Majesty.h"
#include "../../framework/ZoneSvrMsgLayer.h"
#include "../../ZoneSvr.h"
#include "../AP.h"
#include "../Sdk/SdkDMMgr.h"
#include "../../module/Marquee.h"
#include "../player/Player.h"
#include "../../gamedata/GameDataMgr.h"
#include "../../module/FightPVE.h"
#include "../../module/Consume.h"
#include "../../module/GeneralCard.h"
#include "../../module/Props.h"
#include "../../module/Guild.h"
#include "../../module/MasterSkill.h"
#include "../../module/Mall.h"
#include "../../module/VIP.h"
#include "common_proto.h"
#include "cs_proto.h"
#include "../SkillPoint.h"
#include "Pay.h"
#include "../player/PlayerLogic.h"

using namespace PKGMETA;
using namespace DWLOG;

bool GmMgr::Init()
{
    return true;
}

void GmMgr::Fini()
{

}

int GmMgr::HandleCsMsg(PlayerData* pstData, CS_PKG_GM_REQ& rstGmReq, SC_PKG_GM_RSP& rstGmRsp)
{
    int iRet = ERR_NONE;

    switch (rstGmReq.m_bType)
    {
    case GM_HANDLE_TYPE_MAJESTY_LVUP:
        iRet = HandleMsgMajestyLvUp(pstData, rstGmReq, rstGmRsp);
        break;
    case GM_HANDLE_TYPE_DEBUG:
        iRet = HandleMsgDebug(pstData, rstGmReq, rstGmRsp);
        break;
    case GM_HANDLE_TYPE_PVE_DEBUG:
        iRet = HandleMsgPveDebug(pstData, rstGmReq, rstGmRsp);
        break;
	case GM_HANDLE_TYPE_ULTIMATE_SKILL_POINT:
		iRet = HandleMsgUltimateSkillPoint(pstData, rstGmReq, rstGmRsp);
        break;
    default:
        break;
    }

    return iRet;
}

int GmMgr::HandleSsMsg(SS_PKG_GM_REQ& rstGmReq, SS_PKG_GM_RSP& rstGmRsp)
{
    int iRet = ERR_NONE;

    switch (rstGmReq.m_bType)
    {
    case GM_HANDLE_TYPE_CMD:
        iRet = HandleMsgCmd(rstGmReq, rstGmRsp);
        break;
    case GM_HANDLE_TYPE_SEND_MAIL:
        iRet = HandleMsgSendMail(rstGmReq, rstGmRsp);
        break;
    case GM_HANDLE_TYPE_FUNCTION:
        iRet = HandleMsgFunction(rstGmReq, rstGmRsp);
        break;
    case GM_HANDLE_TYPE_MARQUEE_NOTIFY:
        iRet = HandleMsgMarqueeNotify(rstGmReq, rstGmRsp);
        break;
    case GM_HANDLE_TYPE_MARQUEE_ON_TIME_ADD:
        iRet = HandleMsgMarqueeOnTimeAdd(rstGmReq, rstGmRsp);
        break;
    case GM_HANDLE_TYPE_MARQUEE_ON_TIME_DEL:
        iRet = HandleMsgMarqueeOnTimeDel(rstGmReq, rstGmRsp);
        break;
    case GM_HANDLE_TYPE_GET_MARQUEE_LIST:
        iRet = HandleMsgGetMarqueeList(rstGmReq, rstGmRsp);
        break;
    case GM_HANDLE_TYPE_KICK_PLAYER:
        iRet = HandleMsgKickPlayer(rstGmReq, rstGmRsp);
        break;
    default:
        break;
    }

    return iRet;
}

int GmMgr::HandleMsgCmd(SS_PKG_GM_REQ& rstGmReq, SS_PKG_GM_RSP& rstGmRsp)
{
    rstGmReq.m_stHandle.m_stCmd.m_szCmd[MAX_GM_CMD_NUM-1] = '\0';	    //前台参数保护,防止访问越界
    StrCpy(m_CmdBuff, rstGmReq.m_stHandle.m_stCmd.m_szCmd, MAX_GM_CMD_NUM);
    m_ArgsMap.clear();
    int iRet = ParseCmd(rstGmReq.m_stHandle.m_stCmd.m_szCmd);
    if (ERR_NONE != iRet || m_ArgsMap.size() < 1)
    {
        return iRet;
    }
    if (strcmp(m_ArgsMap[0], CMD_ADD_ITEM) == 0)
    {
        return GmAddItem();
    }
    else if (strcmp(m_ArgsMap[0], CMD_SET_EXP) == 0)
    {
        return GmSetExp();
    }
    else if (strcmp(m_ArgsMap[0], CMD_SET_LV) == 0)
    {
        return GmSetLv();
    }

    else if (strcmp(m_ArgsMap[0], CMD_SET_AP) == 0)
    {
        return GmSetAp();
    }
    else if (strcmp(m_ArgsMap[0], CMD_BAN_ACCOUNT) == 0)
    {
        return GmBanAccount();
    }
    else if (strcmp(m_ArgsMap[0], CMD_SET_GUILD_LV) == 0)
    {
        return GmSetGuildLv();
    }
    else if (strcmp(m_ArgsMap[0], CMD_SET_GUILD_FUND) == 0)
    {
        return GmSetGuildFund();
    }
    else if (strcmp(m_ArgsMap[0], CMD_BLACK_ROOM) == 0)
    {
        return GmBlackRoom();
    }
    else if (strcmp(m_ArgsMap[0], CMD_GET_UIN) == 0)
    {
        return GmGetUin(rstGmRsp);
    }
    else if (strcmp(m_ArgsMap[0], CMDG_GET_ROLE_NAME) == 0)
    {
        return GmGetRoleName(rstGmRsp);
    }
    else if (strcmp(m_ArgsMap[0], "send_order") == 0)
    {
        return GmSendOrder();
    }

    return ERR_WRONG_PARA;
}

int GmMgr::ParseCmd(char* pszCmd, char const* pszDelims)
{
    if (NULL == pszCmd || '\0' == pszCmd[0])
    {
        return ERR_WRONG_PARA;
    }

    char* p = strtok(pszCmd, pszDelims);
    while(p != NULL)
    {
        m_ArgsMap.push_back(p);
        p = strtok(NULL, pszDelims);
    }
    return ERR_NONE;
}

int GmMgr::GmAddItem()
{
    uint64_t ullUin ;
	int32_t iType;
	int32_t iId ;
    int32_t iNum;
    sscanf(m_ArgsMap[1], "%lu", &ullUin);
	sscanf(m_ArgsMap[2], "%d", &iType);
    sscanf(m_ArgsMap[3], "%d", &iId);
    sscanf(m_ArgsMap[4], "%d", &iNum);
    DT_ITEM oItem;
	LOGWARN("调试功能");
	if (ullUin == 0)
	{
		ullUin = m_ullSenderUin;
	}
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(ullUin);
    if (NULL == poPlayer || poPlayer->GetState() != PLAYER_STATE_INGAME)
    {
        LOGERR("palyer<%lu> is not exist or not in game state", ullUin);
        return ERR_SYS;
    }
    return Item::Instance().AddItem(&poPlayer->GetPlayerData(), iType, iId, iNum, oItem, 1);
}



void GmMgr::HanldeMsgMultKick(SS_PKG_GM_MULT_KICK_REQ& rstGmReq, SS_PKG_GM_MULT_KICK_RSP& rstGmRsp)
{
	for (int i = 0; i < rstGmReq.m_dwCount; i++)
	{
		uint64_t Uin = rstGmReq.m_UinList[i];
		Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(Uin);
		if (NULL == poPlayer)
		{
			LOGERR("palyer<%lu> is not online", Uin);
		}
		PlayerLogic::Instance().PlayerLogout(poPlayer, PLAYER_LOGOUT_BY_LOGIC);
		rstGmRsp.m_szErrNo[i] = ERR_NONE;
	}
}
//*********************************************************************************************************//
int GmMgr::GmSetLv()
{
    uint64_t ullUin;
    int iLv = 0;

    if (m_ArgsMap.size() < 3)
    {
        return ERR_WRONG_PARA;
    }
    sscanf(m_ArgsMap[1], "%lu", &ullUin);
    sscanf(m_ArgsMap[2], "%d", &iLv);
	if (ullUin == 0)
	{
		ullUin = m_ullSenderUin;
	}
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(ullUin);
    if (NULL == poPlayer )
    {
        bzero(&m_stSsPkg, sizeof(m_stSsPkg));
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ROLE_GM_UPDATE_INFO_REQ;
        StrCpy(m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoReq.m_szCmd, m_CmdBuff, MAX_GM_CMD_NUM);
        m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoReq.m_ullUin = ullUin;
        ZoneSvrMsgLayer::Instance().SendToRoleSvr(m_stSsPkg);
        return ERR_NONE;
    }

    if ( poPlayer->GetState() != PLAYER_STATE_INGAME)
    {
        LOGERR("palyer<%lu> is not exist or not in game state", ullUin);
        return ERR_WRONG_STATE;
    }

    ResMajestyLvMgr_t& roResMgr = CGameDataMgr::Instance().GetResMajestyLvMgr();
    RESMAJESTYLV* pstResLv = roResMgr.Find((int)(iLv - 1));
    if ( NULL == pstResLv  )
    {
        return ERR_SYS;
    }
    PlayerData& oPlayerdata = poPlayer->GetPlayerData();
    int iCurLv = Majesty::Instance().GetLevel(&oPlayerdata);
    if ( iLv > iCurLv)
    {//如果等级是增加,就走正常流程,会触发相关任务等,方便测试
        Majesty::Instance().AddExp(&oPlayerdata, pstResLv->m_dwExp - Majesty::Instance().GetExp(&oPlayerdata));
        return ERR_NONE;
    }
    else if (iLv == iCurLv)
    {
        return ERR_NONE;
    }
    else
    {
        Majesty::Instance().GmSetLv(&oPlayerdata, iLv);
        return ERR_NONE;
    }

    return ERR_NONE;
}

int GmMgr::GmSetLv(uint64_t ullUin, char* pszRoleName, uint64_t ullLv)
{
    Player* poPlayer = NULL;
    int iLv = (int)ullLv;
    if (0 != ullUin)
    {
        poPlayer = PlayerMgr::Instance().GetPlayerByUin(ullUin);
        sprintf(m_CmdBuff, "%s %lu %d", CMD_SET_LV, ullUin, iLv);
    }
    else if (NULL != pszRoleName && '\0' != pszRoleName[0])
    {
        poPlayer = PlayerMgr::Instance().GetPlayerByRoleName(pszRoleName);
        sprintf(m_CmdBuff, "%s %s %d", CMD_SET_LV, pszRoleName, iLv);
    }
    else
    {
        return ERR_WRONG_PARA;
    }

    if (NULL == poPlayer)
    {
        bzero(&m_stSsPkg, sizeof(m_stSsPkg));
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ROLE_GM_UPDATE_INFO_REQ;
        StrCpy(m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoReq.m_szCmd, m_CmdBuff, MAX_GM_CMD_NUM);
        m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoReq.m_ullUin = ullUin;
        if (NULL != pszRoleName && '\0' != pszRoleName)
        {
            StrCpy(m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoReq.m_szRoleName, pszRoleName, PKGMETA::MAX_NAME_LENGTH);
        }
        ZoneSvrMsgLayer::Instance().SendToRoleSvr(m_stSsPkg);
        return ERR_NONE;
    }

    if ( poPlayer->GetState() != PLAYER_STATE_INGAME)
    {
        LOGERR("palyer<%lu> is not exist or not in game state", poPlayer->GetUin());
        return ERR_WRONG_STATE;
    }

    ResMajestyLvMgr_t& roResMgr = CGameDataMgr::Instance().GetResMajestyLvMgr();
    RESMAJESTYLV* pstResLv = roResMgr.Find((int)(iLv - 1));
    if ( NULL == pstResLv  )
    {
        return ERR_SYS;
    }
    PlayerData& oPlayerdata = poPlayer->GetPlayerData();
    int iCurLv = Majesty::Instance().GetLevel(&oPlayerdata);
    if ( iLv > iCurLv)
    {//如果等级是增加,就走正常流程,会触发相关任务等,方便测试
        Majesty::Instance().AddExp(&oPlayerdata, pstResLv->m_dwExp - Majesty::Instance().GetExp(&oPlayerdata));
        return ERR_NONE;
    }
    else if (iLv == iCurLv)
    {
        return ERR_NONE;
    }
    else
    {
        Majesty::Instance().GmSetLv(&oPlayerdata, iLv);
        return ERR_NONE;
    }

    return ERR_NONE;
}

int GmMgr::GmSetExp()
{
    uint64_t ullUin = 0;
    uint32_t iExp = 0;
    if (m_ArgsMap.size() < 3)
    {
        return ERR_WRONG_PARA;
    }
    sscanf(m_ArgsMap[1], "%lu", &ullUin);
    sscanf(m_ArgsMap[2], "%u", &iExp);
    if (iExp <= 0 || iExp > GM_MAX_EXP)
    {
        return ERR_WRONG_PARA;
    }
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(ullUin);
    if (NULL == poPlayer )
    {
        bzero(&m_stSsPkg, sizeof(m_stSsPkg));
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ROLE_GM_UPDATE_INFO_REQ;
        StrCpy(m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoReq.m_szCmd, m_CmdBuff, MAX_GM_CMD_NUM);
        m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoReq.m_ullUin = ullUin;
        ZoneSvrMsgLayer::Instance().SendToRoleSvr(m_stSsPkg);
        return ERR_NONE;
    }
    if ( poPlayer->GetState() != PLAYER_STATE_INGAME )
    {
        return ERR_WRONG_STATE;
    }

    poPlayer->GetPlayerData().GetMajestyInfo().m_dwExp = 0;
    poPlayer->GetPlayerData().GetMajestyInfo().m_wLevel = 1;
    Majesty::Instance().AddExp(&poPlayer->GetPlayerData(), iExp);
    return ERR_NONE;
}

int GmMgr::GmSetAp()
{
    uint64_t ullUin = 0;
    uint32_t iAp = 0;
    if (m_ArgsMap.size() < 3)
    {
        return ERR_WRONG_PARA;
    }
    sscanf(m_ArgsMap[1], "%lu", &ullUin);
    sscanf(m_ArgsMap[2], "%u", &iAp);

    if (iAp <= 0 || iAp > GM_MAX_AP)
    {
        return ERR_WRONG_PARA;
    }
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(ullUin);
    if (NULL == poPlayer)
    {
        bzero(&m_stSsPkg, sizeof(m_stSsPkg));
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ROLE_GM_UPDATE_INFO_REQ;
        StrCpy(m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoReq.m_szCmd, m_CmdBuff, MAX_GM_CMD_NUM);
        m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoReq.m_ullUin = ullUin;
        ZoneSvrMsgLayer::Instance().SendToRoleSvr(m_stSsPkg);

        return ERR_NONE;
    }
    if ( poPlayer->GetState() != PLAYER_STATE_INGAME )
    {
        return ERR_WRONG_STATE;
    }
    poPlayer->GetPlayerData().GetMajestyInfo().m_dwAP = iAp;

    return ERR_NONE;
}

int GmMgr::HandleMsgSendMail(SS_PKG_GM_REQ& rstGmReq, SS_PKG_GM_RSP& rstGmRsp)
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
    SS_PKG_MAIL_ADD_REQ& rstReq = m_stSsPkg.m_stBody.m_stMailAddReq;
    rstReq.m_ullStartTimeSec = CGameTime::Instance().GetCurrSecond();
    rstReq.m_ullEndTimeSec = rstReq.m_ullStartTimeSec + SECONDS_OF_WEEK;

    DT_MAIL_DATA& rstMailData = rstReq.m_stMailData;
    rstMailData.m_bType = rstGmReq.m_stHandle.m_stSendMail.m_bType;
    rstMailData.m_bState = MAIL_STATE_OPENED;
    StrCpy(rstMailData.m_szTitle, rstGmReq.m_stHandle.m_stSendMail.m_szTitle, MAX_MAIL_TITLE_LEN);
    StrCpy(rstMailData.m_szContent, rstGmReq.m_stHandle.m_stSendMail.m_szContent, MAX_MAIL_CONTENT_LEN);
    m_stSsPkg.m_stBody.m_stMailAddReq.m_nUinCount = MIN(MAX_MAIL_MULTI_USER_NUM, rstGmReq.m_stHandle.m_stSendMail.m_nUinCount);
    memcpy(m_stSsPkg.m_stBody.m_stMailAddReq.m_UinList, rstGmReq.m_stHandle.m_stSendMail.m_UinList , sizeof(rstGmReq.m_stHandle.m_stSendMail.m_UinList));
    rstMailData.m_bAttachmentCount = MIN(MAX_MAIL_ATTACHMENT_NUM, rstGmReq.m_stHandle.m_stSendMail.m_bAttachmentCount);
    for (int i = 0; i < rstMailData.m_bAttachmentCount; i++)
    {
        rstMailData.m_astAttachmentList[i].m_bItemType = rstGmReq.m_stHandle.m_stSendMail.m_astAttachmentList[i].m_bType;
        rstMailData.m_astAttachmentList[i].m_dwItemId = rstGmReq.m_stHandle.m_stSendMail.m_astAttachmentList[i].m_dwId;
        rstMailData.m_astAttachmentList[i].m_iValueChg = rstGmReq.m_stHandle.m_stSendMail.m_astAttachmentList[i].m_dwNum;
    }
    rstMailData.m_ullTimeStampMs = CGameTime::Instance().GetCurrSecond();

    ZoneSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
    return ERR_NONE;
}

int GmMgr::HandleMsgMarqueeNotify(SS_PKG_GM_REQ& rstGmReq, SS_PKG_GM_RSP& rstGmRsp)
{
    Marquee::Instance().AddGMMsg(rstGmReq.m_stHandle.m_stMarqueeNotify.m_stMarqueeInfo);

    return ERR_NONE;
}

int GmMgr::HandleMsgMajestyLvUp(PlayerData* pstData, CS_PKG_GM_REQ& rstGmReq, SC_PKG_GM_RSP& rstGmRsp)
{
#ifndef GM_ON
	return ERR_SYS;
#endif
	uint32_t dwExpCurr = Majesty::Instance().GetExp(pstData);
	uint32_t dwExpNextLevel = Majesty::Instance().GetExpNextLv(pstData);

    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PURCHASE_RSP;
    SC_PKG_PURCHASE_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stPurchaseRsp;
    rstScPkgBodyRsp.m_bItemType = ITEM_TYPE_EXP;
    rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    Item::Instance().RewardItem(pstData, ITEM_TYPE_EXP, 0, dwExpNextLevel - dwExpCurr, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_GM_DEBUG);

    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
	// 立马更新主公等级
	Majesty::Instance().CalMajestyLv(pstData);

    return ERR_NONE;
}

int GmMgr::HandleMsgDebug(PlayerData* pstData, CS_PKG_GM_REQ& rstGmReq, SC_PKG_GM_RSP& rstGmRsp)
{
#ifndef GM_ON
	return ERR_SYS;
#endif

    SC_PKG_GM_DEBUG_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stGMDebugRsp;
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GM_DEBUG_RSP;
    bzero(&rstScPkgBodyRsp, sizeof(rstScPkgBodyRsp));
    rstScPkgBodyRsp.m_nErrNo = ERR_NONE;
    int iRet = 0;

    switch(rstGmReq.m_stHandle.m_stDebug.m_chType)
    {
    case GM_DEBUG_GENERAL:
        iRet = GeneralCard::Instance().AddDataForDebug(pstData, rstScPkgBodyRsp.m_astItemList);
        if (iRet > 0)
        {
            rstScPkgBodyRsp.m_nCount = (int16_t)iRet;
        }
        else
        {
            rstScPkgBodyRsp.m_nErrNo = (int16_t)iRet;
        }
        break;
    case GM_DEBUG_MONEY:
        rstScPkgBodyRsp.m_dwCurGold = Consume::Instance().AddGold(pstData, 1000000, DWLOG::METHOD_GM_DEBUG);
        rstScPkgBodyRsp.m_dwCurDiamond = Consume::Instance().AddDiamond(pstData, 10000, DWLOG::METHOD_GM_DEBUG);
        Guild::Instance().AddGuildContribution(pstData, 1000, METHOD_GM_DEBUG);
		Consume::Instance().AddSyncPVPCoin(pstData, 10000, DWLOG::METHOD_GM_DEBUG);
		Consume::Instance().AddAsyncPVPCoin(pstData, 10000, DWLOG::METHOD_GM_DEBUG);
        Consume::Instance().AddPeakArenaCoin(pstData, 10000, DWLOG::METHOD_GM_DEBUG);
        //VIP::Instance().AddExp(pstData, 50);
        //Guild::Instance().AddGuidFund(&poPlayer->GetPlayerData(), 2000);
        break;
    case GM_DEBUG_EQUIP:
        rstScPkgBodyRsp.m_nErrNo = (int16_t)iRet;
        break;
    case GM_DEBUG_PROPS:
        iRet = Props::Instance().AddDataForDebug(pstData, rstScPkgBodyRsp.m_astItemList);
        if (iRet > 0)
        {
            rstScPkgBodyRsp.m_nCount = (int16_t)iRet;
        }
        else
        {
            rstScPkgBodyRsp.m_nErrNo = (int16_t)iRet;
        }
        break;
    case GM_DEBUG_MSKILL:
        iRet = MasterSkill::Instance().AddDataForDebug(pstData, rstScPkgBodyRsp.m_astItemList);
        if (iRet > 0)
        {
            rstScPkgBodyRsp.m_nCount = (int16_t)iRet;
        }
        else
        {
            rstScPkgBodyRsp.m_nErrNo = (int16_t)iRet;
        }
        break;
    case GM_DEBUG_BLANCE:
        // 由其他协议返回数据
        _AddDataForDebugAll(pstData);
        rstScPkgBodyRsp.m_nErrNo = ERR_NONE;
        rstScPkgBodyRsp.m_nCount = 0;
        break;
	case GM_DEBUG_ELO:
        {
            //增加段位
            uint8_t& bELOLvid = pstData->GetELOInfo().m_stPvp6v6Info.m_stBaseInfo.m_bELOLvId;
            uint32_t& dwScore = pstData->GetELOInfo().m_stPvp6v6Info.m_stBaseInfo.m_dwScore;
            int32_t iMaxNum = CGameDataMgr::Instance().GetResScoreMgr().GetResNum();
            if (bELOLvid < iMaxNum)
            {
                RESSCORE* poResScore = CGameDataMgr::Instance().GetResScoreMgr().Find(bELOLvid);
                RESSCORE* poNewResScore = CGameDataMgr::Instance().GetResScoreMgr().Find(bELOLvid+1);
                if (!poResScore || !poNewResScore)
                {
                    LOGERR("GM_DEBUG_ELO error");
                    rstScPkgBodyRsp.m_nErrNo = ERR_SYS;
                    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
                    return ERR_NONE;
                }
                bELOLvid ++;
                dwScore = poNewResScore->m_dwScore;
            }
            else
            {
                dwScore++;
            }
            rstScPkgBodyRsp.m_nErrNo = ERR_NONE;
        }
		break;
    default:
        rstScPkgBodyRsp.m_nErrNo = ERR_SYS;
        break;
    }

    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);

    return ERR_NONE;
}
void GmMgr::_AddDataForDebugAll(PlayerData* pstData)
{
    ResDebugMgr_t& rstResDebugMgr = CGameDataMgr::Instance().GetResDebugMgr();
    RESDEBUGCOND * pResDebug =  rstResDebugMgr.Find(1);
    if (!pResDebug)
    {
        LOGERR("pResDebug is null");
        return;
    }

    ResConsumeMgr_t & rstResConsumeMgr = CGameDataMgr::Instance().GetResConsumeMgr();
    RESCONSUME * pResConsume = rstResConsumeMgr.Find(401);
    if (!pResConsume)
    {
        LOGERR("pResConsume is null");
        return;
    }

    ResMajestyLvMgr_t& roResMgr = CGameDataMgr::Instance().GetResMajestyLvMgr();
    RESMAJESTYLV* pstResLv = roResMgr.Find(pResDebug->m_bMajestyLv -1);
    if (pstResLv==NULL)
    {
        LOGERR("pstResLv is null");
        return;
    }
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();

    rstMajestyInfo.m_wLevel = pResDebug->m_bMajestyLv;
    rstMajestyInfo.m_dwExp = pstResLv->m_dwExp;

    DT_ITEM astItemList[MAX_DEBUG_CNT_NUM];
    GeneralCard::Instance().AddDataForDebug(pstData, astItemList);
    DT_ROLE_GCARD_INFO& rstGCardInfo = pstData->GetGCardInfo();
    for (int i=0; i<rstGCardInfo.m_iCount; i++)
    {
        rstGCardInfo.m_astData[i].m_bStar = pResDebug->m_bGeneralStar;
        rstGCardInfo.m_astData[i].m_bPhase = pResDebug->m_bGeneralPhase;
        //rstGCardInfo.m_astData[i].m_dwSkillLevel = pResDebug->m_bGSkillLv;
        rstGCardInfo.m_astData[i].m_astCheatsList[0].m_bLevel = pResDebug->m_bGSkillStar;
        rstGCardInfo.m_astData[i].m_astCheatsList[1].m_bLevel = pResDebug->m_bGSkillStar;
        rstGCardInfo.m_astData[i].m_bCheatsType = 3;
    }

    MasterSkill::Instance().AddDataForDebug(pstData, astItemList);
    DT_ROLE_MSKILL_INFO& rstMSkillInfo = pstData->GetMSkillInfo();
    for (int i=0; i<rstMSkillInfo.m_iCount; i++)
    {
        rstMSkillInfo.m_astData[i].m_bLevel= pResDebug->m_bMSkillLv;
    }
}

int GmMgr::HandleMsgFunction(SS_PKG_GM_REQ& rstGmReq, SS_PKG_GM_RSP& rstGmRsp)
{
    int iRet = ERR_NONE;
    DT_GM_FUNCTION_LIST& rstFunctionList = rstGmReq.m_stHandle.m_stFunctionList;
    for (uint32_t i = 0; i < rstFunctionList.m_dwCount; i++)
    {
        if (iRet != ERR_NONE)
        {
            break;
        }
        switch (rstFunctionList.m_astFunctionInfo[i].m_bType)
        {
        case GM_RECHARGE:  //充值
            for (int j = 0; j < rstFunctionList.m_nUinCount; ++j)
            {
                iRet = GmRecharge(rstFunctionList.m_UinList[j], rstFunctionList.m_astFunctionInfo[i].m_ullValue);
            }
            break;
        case GM_SET_SILENCE:  //禁言
            for (int j = 0; j < rstFunctionList.m_nUinCount; ++j)
            {
                iRet = GmBlackRoom(rstFunctionList.m_UinList[j], NULL, rstFunctionList.m_astFunctionInfo[i].m_ullValue);
            }
            break;
        case GM_SET_LEVEL:   //设置等级
            for (int j = 0; j < rstFunctionList.m_nUinCount; ++j)
            {
                iRet = GmSetLv(rstFunctionList.m_UinList[j], NULL, rstFunctionList.m_astFunctionInfo[i].m_ullValue);
            }
            break;
        case GM_SET_FREEZEN:  //封号
            for (int j = 0; j < rstFunctionList.m_nUinCount; ++j)
            {
                iRet = GmBanAccount(rstFunctionList.m_UinList[j], rstFunctionList.m_astFunctionInfo[i].m_ullValue);
            }
            break;
        case GM_SET_STOREITEM_DOWN:  //商城商品下架
            break;
        case GM_SET_ITEMPAGE_ON:    //商城道具
            break;
        default:
            iRet = ERR_SYS;
            break;
        }
    }

    return iRet;
}

int GmMgr::HandleMsgPveDebug(PlayerData* pstData, CS_PKG_GM_REQ& rstGmReq, SC_PKG_GM_RSP& rstGmRsp)
{
#ifndef GM_ON
	return ERR_SYS;
#endif
    ResFightLevelMgr_t& rResFLevelMgr = CGameDataMgr::Instance().GetResFightLevelMgr();
    for (int i=0; i<rResFLevelMgr.GetResNum(); i++)
    {
        if (rResFLevelMgr.GetResByPos(i)->m_bChapterType == CHAPTER_TYPE_TUTORIAL ||    // 新手章节
            rResFLevelMgr.GetResByPos(i)->m_bChapterType == CHAPTER_TYPE_NORMAL ||      // 普通章节
            rResFLevelMgr.GetResByPos(i)->m_bChapterType == CHAPTER_TYPE_HERO           // 英雄章节
            )
        {
            FightPVE::Instance().UpdateRecord4Debug(pstData, rResFLevelMgr.GetResByPos(i)->m_dwId);
        }
    }
    m_stScPkg.m_stHead.m_wMsgId = SC_MSG_GM_PVE_DEBUG_RSP;

    SC_PKG_GM_PVE_DEBUG_RSP& rstScPkgBodyRsp =  m_stScPkg.m_stBody.m_stGMPveDebugRsp;
    rstScPkgBodyRsp.m_nErrNo = ERR_NONE;

    ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
    return ERR_NONE;
}


int GmMgr::GmBanAccount()
{
    uint64_t ullUin = 0;
    uint64_t tTime = 0;

    if (m_ArgsMap.size() < 3)
    {
        return ERR_WRONG_PARA;
    }

    sscanf(m_ArgsMap[1], "%lu", &ullUin);
    sscanf(m_ArgsMap[2], "%lu", &tTime);
    bzero(&m_stSsPkg, sizeof(m_stSsPkg));
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ACCOUNT_GM_UPDATE_BANTIME_REQ;
    m_stSsPkg.m_stBody.m_stAccountGmUpdateBanTimeReq.m_ullUin = ullUin;
    m_stSsPkg.m_stBody.m_stAccountGmUpdateBanTimeReq.m_ullBanTime = tTime;
    ZoneSvrMsgLayer::Instance().SendToAccountSvr(m_stSsPkg);

    return ERR_NONE;
}

int GmMgr::GmBanAccount(uint64_t ullUin, uint64_t tTime)
{
    tTime += (uint64_t)CGameTime::Instance().GetCurrSecond();
    bzero(&m_stSsPkg, sizeof(m_stSsPkg));
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ACCOUNT_GM_UPDATE_BANTIME_REQ;
    m_stSsPkg.m_stBody.m_stAccountGmUpdateBanTimeReq.m_ullUin = ullUin;
    m_stSsPkg.m_stBody.m_stAccountGmUpdateBanTimeReq.m_ullBanTime = tTime;
    ZoneSvrMsgLayer::Instance().SendToAccountSvr(m_stSsPkg);

    return ERR_NONE;
}

int GmMgr::GmBlackRoom()
{
    uint64_t ullUin = 0;
    uint64_t tTime = 0;

    if (m_ArgsMap.size() < 3)
    {
        return ERR_WRONG_PARA;
    }
    sscanf(m_ArgsMap[1], "%lu", &ullUin);
    sscanf(m_ArgsMap[2], "%lu", &tTime);

    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(ullUin);
    if (NULL == poPlayer)
    {
        bzero(&m_stSsPkg, sizeof(m_stSsPkg));
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ROLE_GM_UPDATE_INFO_REQ;
        StrCpy(m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoReq.m_szCmd, m_CmdBuff, MAX_GM_CMD_NUM);
        m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoReq.m_ullUin = ullUin;
        ZoneSvrMsgLayer::Instance().SendToRoleSvr(m_stSsPkg);
        return ERR_NONE;
    }

    if ( poPlayer->GetState() != PLAYER_STATE_INGAME )
    {
        return ERR_WRONG_STATE;
    }
    poPlayer->GetPlayerData().GetRoleBaseInfo().m_llBlackRoomTime = tTime;
    return ERR_NONE;
}

int GmMgr::GmBlackRoom(uint64_t ullUin, char* pszRoleName, uint64_t tTime)
{
    Player* poPlayer = NULL;
    tTime += (uint64_t)CGameTime::Instance().GetCurrSecond();
    if (0 != ullUin)
    {
        poPlayer = PlayerMgr::Instance().GetPlayerByUin(ullUin);
        sprintf(m_CmdBuff, "%s %lu %lu", CMD_BLACK_ROOM, ullUin, tTime);
    }
    else if (NULL != pszRoleName && '\0' != pszRoleName[0])
    {
        poPlayer = PlayerMgr::Instance().GetPlayerByRoleName(pszRoleName);
        sprintf(m_CmdBuff, "%s %s %lu", CMD_BLACK_ROOM, pszRoleName, tTime);
    }
    else
    {
        return ERR_WRONG_PARA;
    }

    if (NULL == poPlayer)
    {
        bzero(&m_stSsPkg, sizeof(m_stSsPkg));
        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ROLE_GM_UPDATE_INFO_REQ;
        StrCpy(m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoReq.m_szCmd, m_CmdBuff, MAX_GM_CMD_NUM);
        m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoReq.m_ullUin = ullUin;
        if (NULL != pszRoleName && '\0' != pszRoleName[0])
        {
            pszRoleName[PKGMETA::MAX_NAME_LENGTH - 1] = '\0';
            StrCpy(m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoReq.m_szRoleName, pszRoleName, PKGMETA::MAX_NAME_LENGTH);
        }
        ZoneSvrMsgLayer::Instance().SendToRoleSvr(m_stSsPkg);
        return ERR_NONE;
    }

    if ( poPlayer->GetState() != PLAYER_STATE_INGAME )
    {
        return ERR_WRONG_STATE;
    }

    poPlayer->GetPlayerData().GetRoleBaseInfo().m_llBlackRoomTime = tTime;

    return ERR_NONE;
}

int GmMgr::GmSetGuildLv()
{
    if (m_ArgsMap.size() < 3)
    {
        return ERR_WRONG_PARA;
    }
    bzero(&m_stSsPkg, sizeof(m_stSsPkg));
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_GM_UPDATE_INFO_REQ;
    SS_PKG_GUILD_GM_UPDATE_INFO_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stGuildGmUpdateInfoReq;
    sscanf(m_ArgsMap[1], "%lu", &rstSsReq.m_ullGuildId);
    sscanf(m_ArgsMap[2], "%hhu", &rstSsReq.m_bGuildLevel);
    if ( !(rstSsReq.m_ullGuildId > 0 && rstSsReq.m_bGuildLevel > 0) )
    {
        return ERR_WRONG_PARA;
    }
    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return ERR_NONE;
}

int GmMgr::GmSetGuildFund()
{
    if (m_ArgsMap.size() < 3)
    {
        return ERR_WRONG_PARA;
    }
    bzero(&m_stSsPkg, sizeof(m_stSsPkg));
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_GUILD_GM_UPDATE_INFO_REQ;
    SS_PKG_GUILD_GM_UPDATE_INFO_REQ& rstSsReq = m_stSsPkg.m_stBody.m_stGuildGmUpdateInfoReq;
    sscanf(m_ArgsMap[1], "%lu", &rstSsReq.m_ullGuildId);
    sscanf(m_ArgsMap[2], "%u", &rstSsReq.m_dwGuildFund);
    if ( !(rstSsReq.m_ullGuildId > 0 && rstSsReq.m_dwGuildFund > 0) )
    {
        return ERR_WRONG_PARA;
    }
    ZoneSvrMsgLayer::Instance().SendToGuildSvr(m_stSsPkg);
    return ERR_NONE;
}

 void GmMgr::GmCmdTest(PlayerData* pstData, char* pszCmd)
 {
    SS_PKG_GM_REQ rstGmReq = { 0 };
    SS_PKG_GM_RSP rstGmRsp ={ 0 };
    StrCpy(rstGmReq.m_stHandle.m_stCmd.m_szCmd, pszCmd, MIN(MAX_GM_CMD_NUM, MAX_MESSAGE_RECORD_LEN));
	SetSenderUin(pstData->m_ullUin);
    GmMgr::HandleMsgCmd(rstGmReq, rstGmRsp);
 }

 int GmMgr::GmSendOrder()
 {
     uint32_t dwTest = 1;
     sscanf(m_ArgsMap[1], "%u", &dwTest);
     if (dwTest > 3000)
     {
         dwTest = 3000;
     }
     SdkDMMgr::Instance().TestSend(dwTest);
     return 0;
 }

 int GmMgr::GmGetUin(SS_PKG_GM_RSP& rstGmRsp)
 {
     if (m_ArgsMap.size() < 2)
     {
         return ERR_WRONG_PARA;
     }
     char szName[PKGMETA::MAX_NAME_LENGTH];
     sscanf(m_ArgsMap[1], "%s", szName);
     Player* poPlayer = PlayerMgr::Instance().GetPlayerByRoleName(szName);
     if (NULL == poPlayer)
     {
         bzero(&m_stSsPkg, sizeof(m_stSsPkg));
         m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ROLE_GM_UPDATE_INFO_REQ;
         StrCpy(m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoReq.m_szCmd, m_CmdBuff, MAX_GM_CMD_NUM);
         //m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoReq.m_ullUin = ullUin;
         ZoneSvrMsgLayer::Instance().SendToRoleSvr(m_stSsPkg);
         return 1;
     }
     else
     {
         snprintf(rstGmRsp.m_szResult, 1024, "%lu::%s", poPlayer->GetUin(), poPlayer->GetRoleName());
         return ERR_NONE;
     }

 }

 int GmMgr::GmGetRoleName(SS_PKG_GM_RSP& rstGmRsp)
 {
     if (m_ArgsMap.size() < 2)
     {
         return ERR_WRONG_PARA;
     }
     uint64_t ullUin = 0;
     sscanf(m_ArgsMap[1], "%lu", &ullUin);
     Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(ullUin);
     if (NULL == poPlayer)
     {
         bzero(&m_stSsPkg, sizeof(m_stSsPkg));
         m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ROLE_GM_UPDATE_INFO_REQ;
         StrCpy(m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoReq.m_szCmd, m_CmdBuff, MAX_GM_CMD_NUM);
         //m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoReq.m_ullUin = ullUin;
         ZoneSvrMsgLayer::Instance().SendToRoleSvr(m_stSsPkg);
         return 1;
     }
     else
     {
         snprintf(rstGmRsp.m_szResult, 1024, "%lu::%s", poPlayer->GetUin(), poPlayer->GetRoleName());
         return ERR_NONE;
     }
 }


/*
GM手动给玩家补发充值未到的钻石
*/
int GmMgr::GmRecharge(uint64_t ullUin, uint64_t ullProductID)
{
    Pay* GmPay = NULL;
    GmPay->GmDoPayOk(ullUin, ullProductID);

    return 0;
}

 /*
增加技能点
 */
 int GmMgr::HandleMsgUltimateSkillPoint(PlayerData* pstData, CS_PKG_GM_REQ& rstGmReq, SC_PKG_GM_RSP& rstGmRsp)
 {
#ifndef GM_ON
	 return ERR_SYS;
#endif
	 m_stScPkg.m_stHead.m_wMsgId = SC_MSG_PURCHASE_RSP;
	 SC_PKG_PURCHASE_RSP& rstScPkgBodyRsp = m_stScPkg.m_stBody.m_stPurchaseRsp;
	 rstScPkgBodyRsp.m_bItemType = ITEM_TYPE_SKILL_POINT;
	 rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

	 Item::Instance().RewardItem(pstData, ITEM_TYPE_SKILL_POINT, 0, MAX_SKILL_POINT, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_GM_DEBUG);

	 ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);

	 return ERR_NONE;
 }

/*
添加定时跑马灯
*/
int GmMgr::HandleMsgMarqueeOnTimeAdd(SS_PKG_GM_REQ& rstGmReq, SS_PKG_GM_RSP& rstGmRsp)
{
    int iRet = Marquee::Instance().AddMarqueeOnTime(rstGmReq.m_stHandle.m_stMarqueeOnTimeAdd);

    return iRet;
}

/*
去除定时跑马灯
*/
int GmMgr::HandleMsgMarqueeOnTimeDel(SS_PKG_GM_REQ& rstGmReq, SS_PKG_GM_RSP& rstGmRsp)
{
    int iRet = Marquee::Instance().DelMarqueeOnTime(rstGmReq.m_stHandle.m_stMarqueeOnTimeDel.m_ullMarqueeMsgId);
    return iRet;
}

/*
获取定时跑马灯info
*/
int GmMgr::HandleMsgGetMarqueeList(SS_PKG_GM_REQ& rstGmReq, SS_PKG_GM_RSP& rstGmRsp)
{
    DT_GM_MARQUEE_LIST_INFO& rstGmMarqueeInfo = Marquee::Instance().GetMarqueeOnTimeInfo();
    rstGmRsp.m_stMarqueeOnTimeInfo = rstGmMarqueeInfo;
    return ERR_NONE;
}

/*
踢玩家下线
 */
int GmMgr::HandleMsgKickPlayer(SS_PKG_GM_REQ& rstGmReq, SS_PKG_GM_RSP& rstGmRsp)
{
    uint64_t ullUin = rstGmReq.m_stHandle.m_stKickPlayer.m_ullUin;
    Player* poPlayer = PlayerMgr::Instance().GetPlayerByUin(ullUin);
    if (NULL == poPlayer)
    {
        LOGERR("palyer<%lu> is not online", ullUin);
        return ERR_SYS;
    }
    PlayerLogic::Instance().PlayerLogout(poPlayer, PLAYER_LOGOUT_BY_LOGIC);
    return ERR_NONE;
}

