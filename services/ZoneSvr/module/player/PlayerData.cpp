#include "PlayerData.h"
#include "PlayerMgr.h"
#include "Player.h"
#include "md5Util.h"
#include "../GeneralCard.h"
#include "../Equip.h"
#include "../Props.h"
#include "../MasterSkill.h"
#include "../Majesty.h"
#include "../../gamedata/GameDataMgr.h"
#include "../Task.h"
#include "../FightPVE.h"
#include "../AP.h"
#include "../Guild.h"
#include "../DailyChallenge.h"
#include "PKGMETA_metalib.h"

PlayerData::PlayerData()
{
    this->Clear();
}

void PlayerData::Init(Player* poPlayer)
{
    m_pOwner = poPlayer;
    return;
}

void PlayerData::Clear()
{
     // 放在数据库的信息最好清零，虽然正常情况下登录会用数据库数据覆盖，注意是"正常"情况下
    bzero(&m_stRoleBaseInfo, sizeof(m_stRoleBaseInfo));
    bzero(&m_stELOInfo, sizeof(m_stELOInfo));

    bzero(m_szRoleBaseInfo_md5, sizeof(m_szRoleBaseInfo_md5));
    bzero(m_szRoleMajestyInfo_md5, sizeof(m_szRoleMajestyInfo_md5));
    bzero(m_szRoleGCardInfo_md5, sizeof(m_szRoleGCardInfo_md5));
    bzero(m_szRolePropsInfo_md5, sizeof(m_szRolePropsInfo_md5));
	bzero(m_szRoleItemsInfo_md5, sizeof(m_szRoleItemsInfo_md5));
    bzero(m_szRoleELOInfo_md5, sizeof(m_szRoleELOInfo_md5));
    bzero(m_szRoleTaskInfo_md5, sizeof(m_szRoleTaskInfo_md5));
    bzero(m_szRoleMSkillInfo_md5, sizeof(m_szRoleMSkillInfo_md5));
    bzero(m_szPveInfo_md5, sizeof(m_szPveInfo_md5));
    bzero(m_szMiscInfo_md5, sizeof(m_szMiscInfo_md5));
    bzero(m_szGuildInfo_md5, sizeof(m_szGuildInfo_md5));
    bzero(m_szTacticsInfo_md5, sizeof(m_szTacticsInfo_md5));

    // 不放数据库的信息，内存信息
    bzero(&m_oSelfInfo, sizeof(m_oSelfInfo)); // 战斗相关信息
    bzero(&m_oOpponentInfo, sizeof(m_oOpponentInfo));
    bzero(&m_oFriendAgreeInfo, sizeof(m_oFriendAgreeInfo));

    //m_pOwner = NULL; // 不能清空, player和player data是一一对应的
    m_ullDungeonTimeMs = 0;
    m_bMatchState = 0;
    m_dwPubMailSeq = 0xFFFFFFFF;

    m_bIsJoinGuild= false;
    m_bIsNeedCalLv = false;
    m_stEquipInfo.Clear();
    m_ullRoomNo = 0;
    m_bBossResetNum = 0;
    m_bIsGardFateInit = false;
    m_bIsMSkillInit = false;
    m_bIsFeedTrainInit = false;

    bzero(m_afMSkillAttr, sizeof(m_afMSkillAttr));
	bzero(m_afWeiFeedTrainAttr, sizeof(m_afWeiFeedTrainAttr));
	bzero(m_afShuFeedTrainAttr, sizeof(m_afShuFeedTrainAttr));
	bzero(m_afWuFeedTrainAttr, sizeof(m_afWuFeedTrainAttr));
	bzero(m_afOtherFeedTrainAttr, sizeof(m_afOtherFeedTrainAttr));
	bzero(m_afAllFeedTrainAttr, sizeof(m_afAllFeedTrainAttr));
    m_setOpenGCardFate.clear();
    m_OpenedActIdVector.clear();
    m_PrivateActMap.clear();
    m_ullPrivateActLastUpdateTime = 0;
	m_bGuildExpeditionFightSceneNum = 0;
    m_ullCardLastUptTime = 0;
    return;
}

void PlayerData::NewInit()
{
    this->_NewInitRoleBaseInfo();
    this->_NewInitRoleMiscInfo();
    this->_NewInitRoleMajestyInfo();
    this->_NewInitRoleEquipInfo();

    this->_NewInitRoleGCardInfo();
    this->_NewInitRoleMSkillInfo();
    this->_NewInitRolePropsInfo();
	this->_NewInitRoleItemsInfo();
    this->_NewInitRoleELOInfo();
    this->_NewInitRoleTaskInfo();
    this->_NewInitRolePveInfo();
    this->_NewInitRoleGuildInfo();
    this->_NewInitRoleTacticsInfo();

    Task::Instance().InitPlayerData(this);
    MasterSkill::Instance().AddDataForNewPlayer(this);
    GeneralCard::Instance().AddDataForNewPlayer(this);
    Props::Instance().AddDataForNewPlayer(this);
    Majesty::Instance().SetName(this, m_pOwner->GetRoleName());
    FightPVE::Instance().Init(this);
    AP::Instance().CreatePlayerData(this);
    Guild::Instance().InitPlayerData(this);
    DailyChallenge::Instance().UpdatePlayerData(this);
    GeneralCard::Instance().InitLeaderValue(this);

    return;
}

bool PlayerData::InitFromDB(DT_ROLE_WHOLE_DATA& rstRoleWholeData)
{
    memcpy(&m_stRoleBaseInfo, &rstRoleWholeData.m_stBaseInfo, sizeof(m_stRoleBaseInfo));
    this->m_ullUin = m_stRoleBaseInfo.m_ullUin;
    uint16_t wVersion = m_stRoleBaseInfo.m_wVersion;
    size_t ulUseSize = 0;

    m_stRoleBaseInfo.m_wVersion = PKGMETA::MetaLib::getVersion();

    int iRet = m_stMajestyInfo.unpack((char*)rstRoleWholeData.m_stMajestyBlob.m_szData, sizeof(rstRoleWholeData.m_stMajestyBlob.m_szData), &ulUseSize, wVersion);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("unpack DT_ROLE_MAJESTY_BLOB failed!");
        return false;
    }
	//以RoleBase的那么为准
	//	合服需要
	this->SetMajestyName(m_stRoleBaseInfo.m_szRoleName);

    m_stEquipInfo.Init();
    if (!m_stEquipInfo.InitFromDB(rstRoleWholeData.m_stEquipBlob, wVersion))
    {
        return false;
    }

    iRet = m_stGCardInfo.unpack((char*)rstRoleWholeData.m_stGCardBlob.m_szData, sizeof(rstRoleWholeData.m_stGCardBlob.m_szData), &ulUseSize, wVersion);
    if (iRet != TdrError::TDR_NO_ERROR) {
        LOGERR("unpack DT_ROLE_GCARD_BLOB failed!");
        return false;
    }

    iRet = m_stPropsInfo.unpack((char*)rstRoleWholeData.m_stPropsBlob.m_szData, sizeof(rstRoleWholeData.m_stPropsBlob.m_szData), &ulUseSize, wVersion);
    if (iRet != TdrError::TDR_NO_ERROR) {
        LOGERR("unpack DT_ROLE_PROPS_BLOB failed!");
        return false;
    }

	iRet = m_stItemsInfo.unpack((char*)rstRoleWholeData.m_stItemsBlob.m_szData, sizeof(rstRoleWholeData.m_stItemsBlob.m_szData), &ulUseSize, wVersion);
	if (iRet != TdrError::TDR_NO_ERROR) {
		LOGERR("unpack DT_ROLE_ITEMS_BLOB failed!");
		return false;
	}

    iRet = m_stELOInfo.unpack((char*)rstRoleWholeData.m_stELOBlob.m_szData, sizeof(rstRoleWholeData.m_stELOBlob.m_szData), &ulUseSize, wVersion);
    if (iRet != TdrError::TDR_NO_ERROR) {
        LOGERR("unpack DT_ROLE_ELO_BLOB failed!");
        return false;
    }

    iRet = m_stTaskInfo.unpack((char*)rstRoleWholeData.m_stTaskBlob.m_szData, sizeof(rstRoleWholeData.m_stTaskBlob.m_szData), &ulUseSize, wVersion);
    if (iRet != TdrError::TDR_NO_ERROR) {
        LOGERR("unpack DT_ROLE_TASK_BLOB failed!");
        return false;
    }

    iRet = m_stMSkillInfo.unpack((char*)rstRoleWholeData.m_stMSkillBlob.m_szData, sizeof(rstRoleWholeData.m_stMSkillBlob.m_szData), &ulUseSize, wVersion);
    if (iRet != TdrError::TDR_NO_ERROR) {
        LOGERR("unpack DT_ROLE_MSKILL_BLOB failed!");
        return false;
    }

    iRet = m_stPveInfo.unpack((char*)rstRoleWholeData.m_stPveBlob.m_szData, sizeof(rstRoleWholeData.m_stPveBlob.m_szData), &ulUseSize, wVersion);
    if (iRet != TdrError::TDR_NO_ERROR) {
        LOGERR("unpack DT_ROLE_MSKILL_BLOB failed!");
        return false;
    }

    iRet = m_stMiscInfo.unpack((char*)rstRoleWholeData.m_stMiscBlob.m_szData, sizeof(rstRoleWholeData.m_stMiscBlob.m_szData), &ulUseSize, wVersion);
    if (iRet != TdrError::TDR_NO_ERROR)
	{
        this->_NewInitRoleMiscInfo();
    }
    if (wVersion < 77)
    {
        this->_NewInitRoleMiscInfo();
    }

    iRet = m_stGuildInfo.unpack((char*)rstRoleWholeData.m_stGuildBlob.m_szData, sizeof(rstRoleWholeData.m_stGuildBlob.m_szData), &ulUseSize, wVersion);
    if (iRet != TdrError::TDR_NO_ERROR) {
        LOGERR("unpack DT_ROLE_GUILD_BLOB failed!");
        return false;
    }

    iRet = m_stTacticsInfo.unpack((char*)rstRoleWholeData.m_stTacticsBlob.m_szData, sizeof(rstRoleWholeData.m_stTacticsBlob.m_szData), &ulUseSize, wVersion);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        this->_NewInitRoleTacticsInfo();
    }
    if (wVersion < 84)
    {
        this->_NewInitRoleTacticsInfo();
    }
    return true;
}

// check update data by md5
// TODO: 计算hashcode, 看效率能提升多少
bool PlayerData::PackRoleDataUpt(INOUT PKGMETA::SS_PKG_ROLE_UPDATE_REQ& rstRoleUptReq)
{
    int iCount = 0;
    rstRoleUptReq.m_ullUin = m_stRoleBaseInfo.m_ullUin;
    StrCpy(rstRoleUptReq.m_szRoleName, m_stRoleBaseInfo.m_szRoleName, sizeof(m_stRoleBaseInfo.m_szRoleName));

    // base info
    if (CMD5Util::CmpMD5(&m_stRoleBaseInfo, sizeof(m_stRoleBaseInfo), this->GetRoleBaseInfo_md5()))
    {
        rstRoleUptReq.m_astRoleUpdate[iCount].m_bType = ROLE_DATA_TYPE_BASE;
        this->_PackRoleBaseInfo(rstRoleUptReq.m_astRoleUpdate[iCount].m_stData.m_stBaseInfo);
        ++iCount;
    }

    // majesty info
    if (CMD5Util::CmpMD5(&m_stMajestyInfo, sizeof(m_stMajestyInfo), this->GetRoleMajestyInfo_md5()))
    {
        rstRoleUptReq.m_astRoleUpdate[iCount].m_bType = ROLE_DATA_TYPE_MAJESTY;

        if (!this->_PackRoleMajestyInfo(rstRoleUptReq.m_astRoleUpdate[iCount].m_stData.m_stMajestyInfoBlob))
        {
            return false;
        }
        ++iCount;
    }

    // equip info
    if (m_stEquipInfo.IsNeedUpdate())
    {
        rstRoleUptReq.m_astRoleUpdate[iCount].m_bType = ROLE_DATA_TYPE_EQUIP;
        if (!this->_PackRoleEquipInfo(rstRoleUptReq.m_astRoleUpdate[iCount].m_stData.m_stEquipInfoBlob))
        {
            return false;
        }
        m_stEquipInfo.ResetUptFlag();
        ++iCount;
    }

    // general card info
    if (CMD5Util::CmpMD5(&m_stGCardInfo, sizeof(m_stGCardInfo), this->GetRoleGCardInfo_md5()))
    {
        rstRoleUptReq.m_astRoleUpdate[iCount].m_bType = ROLE_DATA_TYPE_GCARD;
        if (!this->_PackRoleGCardInfo(rstRoleUptReq.m_astRoleUpdate[iCount].m_stData.m_stGCardInfoBlob))
        {
            return false;
        }
        ++iCount;
    }

    // master skill info
    if (CMD5Util::CmpMD5(&m_stMSkillInfo, sizeof(m_stMSkillInfo), this->GetRoleMSkillInfo_md5()))
    {
        rstRoleUptReq.m_astRoleUpdate[iCount].m_bType = ROLE_DATA_TYPE_MSKILL;
        if (!this->_PackRoleMSkillInfo(rstRoleUptReq.m_astRoleUpdate[iCount].m_stData.m_stMSkillInfoBlob))
        {
            return false;
        }
        ++iCount;
    }

    // props info
    if (CMD5Util::CmpMD5(&m_stPropsInfo, sizeof(m_stPropsInfo), this->GetRolePropsInfo_md5()))
    {
        rstRoleUptReq.m_astRoleUpdate[iCount].m_bType = ROLE_DATA_TYPE_PROPS;
        if (!this->_PackRolePropsInfo(rstRoleUptReq.m_astRoleUpdate[iCount].m_stData.m_stPropsInfoBlob))
        {
            return false;
        }
        ++iCount;
    }

	// Items info
	if (CMD5Util::CmpMD5(&m_stItemsInfo, sizeof(m_stItemsInfo), this->GetRoleItemsInfo_md5()))
	{
		rstRoleUptReq.m_astRoleUpdate[iCount].m_bType = ROLE_DATA_TYPE_ITEMS;
		if (!this->_PackRoleItemsInfo(rstRoleUptReq.m_astRoleUpdate[iCount].m_stData.m_stItemsInfoBlob))
		{
			return false;
		}
		++iCount;
	}

    // ELO info
    if (CMD5Util::CmpMD5(&m_stELOInfo, sizeof(m_stELOInfo), this->GetRoleELOInfo_md5()))
    {
        rstRoleUptReq.m_astRoleUpdate[iCount].m_bType = ROLE_DATA_TYPE_ELO;
        if (!this->_PackRoleELOInfo(rstRoleUptReq.m_astRoleUpdate[iCount].m_stData.m_stELOInfoBlob))
        {
            return false;
        }
        ++iCount;
    }

    // Task info
    if (CMD5Util::CmpMD5(&m_stTaskInfo, sizeof(m_stTaskInfo), this->GetRoleTaskInfo_md5()))
    {
        rstRoleUptReq.m_astRoleUpdate[iCount].m_bType = ROLE_DATA_TYPE_TASK;
        if (!this->_PackRoleTaskInfo(rstRoleUptReq.m_astRoleUpdate[iCount].m_stData.m_stTaskInfoBlob))
        {
            return false;
        }
        ++iCount;
    }

    if (CMD5Util::CmpMD5(&m_stPveInfo, sizeof(m_stPveInfo), this->GetRolePveInfo_md5()))
    {
        rstRoleUptReq.m_astRoleUpdate[iCount].m_bType = ROLE_DATA_TYPE_PVE;
        if (!this->_PackRolePveInfo(rstRoleUptReq.m_astRoleUpdate[iCount].m_stData.m_stPveBlob))
        {
            return false;
        }
        ++iCount;
    }

    if (CMD5Util::CmpMD5(&m_stMiscInfo, sizeof(m_stMiscInfo), this->GetRoleMiscInfo_md5()))
    {
        rstRoleUptReq.m_astRoleUpdate[iCount].m_bType = ROLE_DATA_TYPE_MISC;
        if (!this->_PackRoleMiscInfo(rstRoleUptReq.m_astRoleUpdate[iCount].m_stData.m_stMiscBlob))
        {
            return false;
        }
        ++iCount;
    }

    if (CMD5Util::CmpMD5(&m_stGuildInfo, sizeof(m_stGuildInfo), this->GetRoleGuildInfo_md5()))
    {
        rstRoleUptReq.m_astRoleUpdate[iCount].m_bType = ROLE_DATA_TYPE_GUILD;
        if (!this->_PackRoleGuildInfo(rstRoleUptReq.m_astRoleUpdate[iCount].m_stData.m_stGuildBlob))
        {
            return false;
        }
        ++iCount;
    }
    if (CMD5Util::CmpMD5(&m_stTacticsInfo, sizeof(m_stTacticsInfo), this->GetRoleTaskInfo_md5()))
    {
        rstRoleUptReq.m_astRoleUpdate[iCount].m_bType = ROLE_DATA_TYPE_TACTICS;
        if (!this->_PackRoleTacticsInfo(rstRoleUptReq.m_astRoleUpdate[iCount].m_stData.m_stTacticsBlob))
        {
            return false;
        }
        ++iCount;
    }
    assert(iCount <= MAX_ROLE_UPDATE_NUM);
    rstRoleUptReq.m_nCount = iCount;

    return true;
}

bool PlayerData::PackRoleWholeData(INOUT PKGMETA::DT_ROLE_WHOLE_DATA& rstRoleWholeData)
{
    // 发给玩家的数据，需要根据玩家协议版本号裁剪
    this->_PackRoleBaseInfo(rstRoleWholeData.m_stBaseInfo);
    if (!this->_PackRoleMajestyInfo(rstRoleWholeData.m_stMajestyBlob, m_pOwner->GetProtocolVersion()))
    {
        return false;
    }

    if (!this->_PackRoleEquipInfo(rstRoleWholeData.m_stEquipBlob, m_pOwner->GetProtocolVersion()))
    {
        return false;
    }

    if (!this->_PackRoleGCardInfo(rstRoleWholeData.m_stGCardBlob, m_pOwner->GetProtocolVersion()))
    {
        return false;
    }

    if (!this->_PackRoleMSkillInfo(rstRoleWholeData.m_stMSkillBlob, m_pOwner->GetProtocolVersion()))
    {
        return false;
    }

    if (!this->_PackRolePropsInfo(rstRoleWholeData.m_stPropsBlob, m_pOwner->GetProtocolVersion()))
    {
        return false;
    }

	if (!this->_PackRoleItemsInfo(rstRoleWholeData.m_stItemsBlob, m_pOwner->GetProtocolVersion()))
	{
		return false;
	}

    if (!this->_PackRoleELOInfo(rstRoleWholeData.m_stELOBlob, m_pOwner->GetProtocolVersion()))
    {
        return false;
    }

    if (!this->_PackRoleTaskInfo(rstRoleWholeData.m_stTaskBlob, m_pOwner->GetProtocolVersion()))
    {
        return false;
    }

    if (!this->_PackRolePveInfo(rstRoleWholeData.m_stPveBlob, m_pOwner->GetProtocolVersion()))
    {
        return false;
    }

    if (!this->_PackRoleMiscInfo(rstRoleWholeData.m_stMiscBlob, m_pOwner->GetProtocolVersion()))
    {
        return false;
    }

    if (!this->_PackRoleGuildInfo(rstRoleWholeData.m_stGuildBlob, m_pOwner->GetProtocolVersion()))
    {
        return false;
    }
    if (!this->_PackRoleTacticsInfo(rstRoleWholeData.m_stTacticsBlob, m_pOwner->GetProtocolVersion()))
    {
        return false;
    }
    return true;
}

void PlayerData::_NewInitRoleBaseInfo()
{
    m_stRoleBaseInfo.m_ullUin = m_pOwner->GetUin();
    m_stRoleBaseInfo.m_dwBagSeq = 1;
    m_stRoleBaseInfo.m_llFirstLoginTime = CGameTime::Instance().GetCurrSecond();
    m_stRoleBaseInfo.m_llLastLoginTime = CGameTime::Instance().GetCurrSecond();
    m_stRoleBaseInfo.m_szRoleName[0] = '\0';
    m_stRoleBaseInfo.m_wVersion = PKGMETA::MetaLib::getVersion();
    return;
}

void PlayerData::_NewInitRoleMajestyInfo()
{
    bzero((void*)&m_stMajestyInfo, sizeof(m_stMajestyInfo));
    m_stMajestyInfo.m_wLevel = 1;

    RESAP* pResAP = CGameDataMgr::Instance().GetResAPMgr().Find(m_stMajestyInfo.m_wLevel);
    assert(pResAP);
    m_stMajestyInfo.m_dwAP = (uint32_t)pResAP->m_wAPLimit;
    m_stMajestyInfo.m_ullAPResumeLastTime = CGameTime::Instance().GetCurrSecond();

    RESVIP* pResVip = CGameDataMgr::Instance().GetResVIPMgr().Find(1);
    assert(pResVip);
    m_stMajestyInfo.m_bGoldPurchaseTimesMax = pResVip->m_dwBuyGoldenTimes;
    m_stMajestyInfo.m_bAPPurchaseTimesMax = pResVip->m_dwBuyAPTimes;

    RESBASIC* pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find((int)PVP_REWARD_INFO);
    assert(pResBasic);
    m_stMajestyInfo.m_bPVPRewardTimesMax = (uint8_t)pResBasic->m_para[0];

    m_stMajestyInfo.m_wSkillPoint = MAX_SKILL_POINT;
    m_stMajestyInfo.m_ullSPResumeLastTime = CGameTime::Instance().GetCurrSecond();
    m_stMajestyInfo.m_dwHighestLi = 1000;

    return;
}

void PlayerData::_NewInitRoleEquipInfo()
{
    m_stEquipInfo.Init();
    return;
}

void PlayerData::_NewInitRoleGCardInfo()
{
    bzero((void*)&m_stGCardInfo, sizeof(m_stGCardInfo));
    return;
}

void PlayerData::_NewInitRoleMSkillInfo()
{
    bzero((void*)&m_stMSkillInfo, sizeof(m_stMSkillInfo));
    return;
}

void PlayerData::_NewInitRolePropsInfo()
{
    bzero((void*)&m_stPropsInfo, sizeof(m_stPropsInfo));
    return;
}

void PlayerData::_NewInitRoleItemsInfo()
{
	bzero((void*)&m_stItemsInfo, sizeof(m_stItemsInfo));
	Majesty::Instance().SetDefaultItems(m_stMajestyInfo, m_stItemsInfo);

	return;
}

void PlayerData::_NewInitRoleELOInfo()
{
    bzero((void*)&m_stELOInfo, sizeof(m_stELOInfo));
    m_stELOInfo.m_stPvp6v6Info.m_stBaseInfo.m_bELOLvId = 1;

    DT_ROLE_PEAK_ARENA_INFO& rstPeakArenainfo = m_stELOInfo.m_stPeakArenaInfo;
    rstPeakArenainfo.m_bELOLvId = 1;
    rstPeakArenainfo.m_ullOutputLastUptTime = CGameTime::Instance().GetCurrSecond();

    return;
}

void PlayerData::_NewInitRoleTaskInfo()
{
    bzero((void*)&m_stTaskInfo, sizeof(m_stTaskInfo));
    return;
}

void PlayerData::_NewInitRolePveInfo()
{
    bzero((void*)&m_stPveInfo, sizeof(m_stPveInfo));
    return;
}

void PlayerData::_NewInitRoleMiscInfo()
{
    bzero((void*)&m_stMiscInfo, sizeof(m_stMiscInfo));
    m_stMiscInfo.m_ullTutorialStep = 100;
	m_stMiscInfo.m_bGroupCardId = 7; //后3位1表示默认未开启
    return;
}

void PlayerData::_NewInitRoleGuildInfo()
{
    bzero((void*)&m_stGuildInfo, sizeof(m_stGuildInfo));
    m_stGuildInfo.m_stGuildHangInfo.m_stDefaultSlot.m_bState = GUILD_HANG_STATE_UNLOCKED;
    //LOGRUN("m_stGuildInfo.m_stGuildHangInfo.m_stDefaultSlot.m_dwGeneralId", m_stGuildInfo.m_stGuildHangInfo.m_stDefaultSlot.m_dwGeneralId);
    return;
}

void PlayerData::_NewInitRoleTacticsInfo()
{
    bzero((void*)&m_stTacticsInfo, sizeof(m_stTacticsInfo));
    //需要运行解锁函数
    return;
}
bool PlayerData::_PackRoleBaseInfo(DT_ROLE_BASE_INFO& rstInfo)
{
    memcpy(&rstInfo, &m_stRoleBaseInfo, sizeof(m_stRoleBaseInfo));
    return true;
}

bool PlayerData::_PackRoleMajestyInfo(DT_ROLE_MAJESTY_BLOB& rstBlob, uint16_t wVersion)
{
    size_t ulUseSize = 0;
    int iRet = m_stMajestyInfo.pack((char*)rstBlob.m_szData, sizeof(rstBlob.m_szData), &ulUseSize, wVersion);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("pack DT_ROLE_MAJESTY_BLOB failed!");
        return false;
    }

    rstBlob.m_iLen = (int)ulUseSize;
    return true;
}

bool PlayerData::_PackRoleEquipInfo(DT_ROLE_EQUIP_BLOB& rstBlob, uint16_t wVersion)
{
    return m_stEquipInfo.PackEquipInfo(rstBlob, wVersion);
}

bool PlayerData::_PackRoleGCardInfo(DT_ROLE_GCARD_BLOB& rstBlob, uint16_t wVersion)
{
    size_t ulUseSize = 0;
    int iRet = m_stGCardInfo.pack((char*)rstBlob.m_szData, sizeof(rstBlob.m_szData), &ulUseSize, wVersion);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("pack DT_ROLE_GCARD_BLOB failed!");
        return false;
    }

    rstBlob.m_iLen = (int)ulUseSize;
    return true;
}

bool PlayerData::_PackRoleMSkillInfo(DT_ROLE_MSKILL_BLOB& rstBlob, uint16_t wVersion)
{
    size_t ulUseSize = 0;
    int iRet = m_stMSkillInfo.pack((char*)rstBlob.m_szData, sizeof(rstBlob.m_szData), &ulUseSize, wVersion);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("pack DT_ROLE_MSKILL_BLOB failed!");
        return false;
    }

    rstBlob.m_iLen = (int)ulUseSize;
    return true;
}

bool PlayerData::_PackRolePropsInfo(DT_ROLE_PROPS_BLOB& rstBlob, uint16_t wVersion)
{
    size_t ulUseSize = 0;
    int iRet = m_stPropsInfo.pack((char*)rstBlob.m_szData, sizeof(rstBlob.m_szData), &ulUseSize, wVersion);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("pack DT_ROLE_PROPS_BLOB failed!");
        return false;
    }

    rstBlob.m_iLen = (int)ulUseSize;
    return true;
}

bool PlayerData::_PackRoleItemsInfo(DT_ROLE_ITEMS_BLOB& rstBlob, uint16_t wVersion)
{
	size_t ulUseSize = 0;
	int iRet = m_stItemsInfo.pack((char*)rstBlob.m_szData, sizeof(rstBlob.m_szData), &ulUseSize, wVersion);
	if( iRet != TdrError::TDR_NO_ERROR)
	{
		LOGERR("pack DT_ROLE_ITEMS_BLOB failed!");
		return false;
	}

	rstBlob.m_iLen = (int)ulUseSize;
	return true;
}

bool PlayerData::_PackRoleELOInfo(DT_ROLE_ELO_BLOB& rstBlob, uint16_t wVersion)
{
    size_t ulUseSize = 0;
    int iRet = m_stELOInfo.pack((char*)rstBlob.m_szData, sizeof(rstBlob.m_szData), &ulUseSize, wVersion);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("pack DT_ROLE_ELO_BLOB failed!");
        return false;
    }

    rstBlob.m_iLen = (int)ulUseSize;
    return true;
}

bool PlayerData::_PackRoleTaskInfo(DT_ROLE_TASK_BLOB& rstBlob, uint16_t wVersion)
{
    size_t ulUseSize = 0;
    int iRet = m_stTaskInfo.pack((char*)rstBlob.m_szData, sizeof(rstBlob.m_szData), &ulUseSize, wVersion);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("pack DT_ROLE_TASK_BLOB failed!");
        return false;
    }

    rstBlob.m_iLen = (int)ulUseSize;
    return true;
}

bool PlayerData::_PackRolePveInfo(DT_ROLE_PVE_BLOB& rstBlob, uint16_t wVersion)
{
    size_t ulUseSize = 0;
    int iRet = m_stPveInfo.pack((char*)rstBlob.m_szData, sizeof(rstBlob.m_szData), &ulUseSize, wVersion);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("pack DT_ROLE_PVE_BLOB failed!");
        return false;
    }

    rstBlob.m_iLen = (int)ulUseSize;
    return true;
}

bool PlayerData::_PackRoleMiscInfo(DT_ROLE_MISC_BLOB& rstBlob, uint16_t wVersion)
{
    size_t ulUseSize = 0;
    int iRet = m_stMiscInfo.pack((char*)rstBlob.m_szData, sizeof(rstBlob.m_szData), &ulUseSize, wVersion);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("pack DT_ROLE_MISC_BLOB failed!");
        return false;
    }

    rstBlob.m_iLen = (int)ulUseSize;
    return true;
}

bool PlayerData::_PackRoleGuildInfo(DT_ROLE_GUILD_BLOB& rstBlob, uint16_t wVersion)
{
    size_t ulUseSize = 0;
    int iRet = m_stGuildInfo.pack((char*)rstBlob.m_szData, sizeof(rstBlob.m_szData), &ulUseSize, wVersion);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("pack DT_ROLE_GUILD_BLOB failed!");
        return false;
    }

    rstBlob.m_iLen = (int)ulUseSize;
    return true;
}

bool PlayerData::_PackRoleTacticsInfo(DT_ROLE_TACTICS_BLOB& rstBlob, uint16_t wVersion)
{
    size_t ulUseSize = 0;
    int iRet = m_stTacticsInfo.pack((char*)rstBlob.m_szData, sizeof(rstBlob.m_szData), &ulUseSize, wVersion);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("pack DT_ROLE_TACTICS_BLOB failed!");
        return false;
    }

    rstBlob.m_iLen = (int)ulUseSize;
    return true;
}
