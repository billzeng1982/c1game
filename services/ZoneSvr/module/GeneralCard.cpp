#include "GeneralCard.h"
#include "LogMacros.h"
#include "../gamedata/GameDataMgr.h"
#include "Props.h"
#include "Majesty.h"
#include "common_proto.h"
#include "Consume.h"
#include "Item.h"
#include "Equip.h"
#include "Task.h"
#include "dwlog_svr.h"
#include "strutil.h"
#include "ZoneLog.h"
#include "player/Player.h"
#include "Marquee.h"
#include "Guild.h"
#include "MasterSkill.h"
#include "GloryItemsMgr.h"
#include "SkillPoint.h"
#include "RankMgr.h"
#include "Message.h"
#include "AsyncPvp.h"
#include "CpuSampleStats.h"

using namespace PKGMETA;
using namespace DWLOG;

#define LI_FACTOR_MILLI 0.001
#define GCARD_STAR_MIN (0)
#define GCARD_STAR_MAX (5)


int GeneralCardCmp(const void *pstFirst, const void *pstSecond)
{
    DT_ITEM_GCARD* pstItemFirst = (DT_ITEM_GCARD*)pstFirst;
    DT_ITEM_GCARD* pstItemSecond = (DT_ITEM_GCARD*)pstSecond;

    int iResult = (int)pstItemFirst->m_dwId - (int)pstItemSecond->m_dwId;
    return iResult;
}

GeneralCard::GeneralCard()
{
    bzero((char*)&m_stGCard, sizeof(m_stGCard));
}

bool GeneralCard::Init()
{
    ResBasicMgr_t& rstResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
    RESBASIC* pstResBasic = rstResBasicMgr.Find(GENERAL_LV_UP_EXPERIENCE);
    if (pstResBasic == NULL)
    {
        LOGERR("Init GeneralCard failed, pstResbasic is null");
        return false;
    }

    ResPropsMgr_t& rstResPropsMgr = CGameDataMgr::Instance().GetResPropsMgr();
    for (int i=0; i < MAX_NUM_EQUIP_EXPCARD_TYPE && i < 6; i++)
    {
        m_ExpCardId[i] = (uint32_t)pstResBasic->m_para[i];
        RESPROPS* pResprops = rstResPropsMgr.Find(m_ExpCardId[i]);
        if (pResprops == NULL)
        {
            LOGERR("Init GeneralCard failed, pResprops is null");
            return false;
        }
        m_ExpCardExp[i] = pResprops->m_dwExp;
    }

    pstResBasic = rstResBasicMgr.Find(9401);
    if (pstResBasic == NULL)
    {
        LOGERR("Init GeneralCard failed, pstResbasic is null");
        return false;
    }

    m_dwStarLeaderValue = (uint32_t)pstResBasic->m_para[0];
    m_dwPhaseLeaderValue = (uint32_t)pstResBasic->m_para[1];
    m_dwTrainLvLeaderValue = (uint32_t)pstResBasic->m_para[2];
    m_dwTrainPhaseLeaderValue = (uint32_t)pstResBasic->m_para[3];
    m_dwFameHallLeaderValue = (uint32_t)pstResBasic->m_para[4];

    return true;
}

int GeneralCard::Add(PlayerData* pstData, DT_ITEM_GCARD* pstItem)
{
    int iEqual = 0;
    DT_ROLE_GCARD_INFO& rstInfo = pstData->GetGCardInfo();

    if (rstInfo.m_iCount >= MAX_NUM_ROLE_GCARD)
    {
        return ERR_SYS;
    }

    MyBSearch(pstItem, rstInfo.m_astData, rstInfo.m_iCount, sizeof(DT_ITEM_GCARD), &iEqual, GeneralCardCmp);
    if (iEqual)
    {
        return ERR_ALREADY_EXISTED;
    }
    size_t nmemb = (size_t)rstInfo.m_iCount;
    if(nmemb >= MAX_NUM_ROLE_GCARD)
    {
        LOGERR("rstInfo.m_iCount<%d> reached MAX_NUM_ROLE_GCARD", rstInfo.m_iCount);
        return ERR_SYS;
    }
    int iIndex = 0;
    if (MyBInsertIndex(pstItem, rstInfo.m_astData, &nmemb, sizeof(DT_ITEM_GCARD), 1, &iIndex, GeneralCardCmp))
    {
        rstInfo.m_iCount = (int32_t)nmemb;
        Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_GENERAL, 1/*value*/);
        Guild::Instance().RefreshMemberInfo(pstData);
        //激活缘分
        OpenGCardFate(pstData, pstItem->m_dwId);
        //计算战力
        CalGcardLi(pstData, &rstInfo.m_astData[iIndex]);
        //增加武将头像
        GloryItemsMgr::Instance().AddMajestyItems(pstData, MAJESTY_ITEM_ACCESS_GENERAL, pstItem->m_dwId);
        //计算统帅值变化
        UptLeaderValue(pstData, &rstInfo.m_astData[iIndex]);
        //更新武将数量排行榜
        RankMgr::Instance().UpdateGCardCnt(pstData);
    }

    return iIndex;
}

int GeneralCard::Del(PlayerData* pstData, uint32_t dwId)
{
    m_stGCard.m_dwId = dwId;
    DT_ROLE_GCARD_INFO& rstInfo = pstData->GetGCardInfo();
    size_t nmemb = (size_t)rstInfo.m_iCount;
    MyBDelete(&m_stGCard, rstInfo.m_astData, &nmemb, sizeof(DT_ITEM_GCARD), GeneralCardCmp);
    rstInfo.m_iCount = (int)nmemb;

    return ERR_NONE;
}

DT_ITEM_GCARD* GeneralCard::Find(PlayerData* pstData, uint32_t dwId)
{
    m_stGCard.m_dwId = dwId;
    DT_ROLE_GCARD_INFO& rstInfo = pstData->GetGCardInfo();
    int iEqual = 0;
    int iIndex = MyBSearch(&m_stGCard, rstInfo.m_astData, rstInfo.m_iCount, sizeof(DT_ITEM_GCARD), &iEqual, GeneralCardCmp);
    if (!iEqual)
    {
        return NULL;
    }

    return &rstInfo.m_astData[iIndex];
}

int GeneralCard::Reborn(PlayerData* pstData, uint32_t dwId, uint8_t bType, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
    DT_ITEM_GCARD* pstGeneral = Find(pstData, dwId);
    if (!pstGeneral)
    {
        LOGERR("Player(%lu) Reborn failed, General(%u) not found", pstData->m_ullUin, dwId);
        return ERR_NOT_FOUND;
    }

    //武将重生日志
    ZoneLog::Instance().WriteRebornLog(pstData->m_pOwner, dwId, pstGeneral->m_bLevel, bType);

    int iRet = ERR_NONE;
    uint32_t dwRatio = (bType==1) ? 100 : 80;
    uint32_t dwGold = 0;
    uint32_t dwSumGold = 0;

    LvReborn(pstData, dwId, rstSyncItemInfo, dwRatio);
    StarReborn(pstData, dwId, rstSyncItemInfo, &dwGold, dwRatio);
    dwSumGold += dwGold;
    SkillReborn(pstData, dwId, rstSyncItemInfo, &dwGold, dwRatio);
    dwSumGold += dwGold;
    CheatsReborn(pstData, dwId, rstSyncItemInfo, dwRatio);
    ArmyLvReborn(pstData, dwId, rstSyncItemInfo, &dwGold, dwRatio);
    dwSumGold += dwGold;
    ArmyPhaseReborn(pstData, dwId, rstSyncItemInfo, dwRatio);
	PhaseReborn(pstData, dwId, rstSyncItemInfo, &dwGold, dwRatio);
	dwSumGold += dwGold;

    Equip::Instance().Recycle(pstData, dwId, rstSyncItemInfo, &dwGold, dwRatio);
    dwSumGold += dwGold;

    //增加金币
    Item::Instance().RewardItem(pstData, ITEM_TYPE_GOLD, 0, dwSumGold, rstSyncItemInfo, METHOD_GENERAL_OP_REBORN);

    if (bType == 1)
    {
        Item::Instance().ConsumeItem(pstData, ITEM_TYPE_DIAMOND, 0, -100, rstSyncItemInfo, METHOD_GENERAL_OP_REBORN);
    }
    else
    {
        Item::Instance().ConsumeItem(pstData, ITEM_TYPE_GOLD, 0, -100, rstSyncItemInfo, METHOD_GENERAL_OP_REBORN);
    }
    CalGcardLi(pstData, dwId);

    return iRet;
}

int GeneralCard::GetClassScore(uint32_t dwId)
{
    ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
    RESGENERAL* pstResGeneral = rstResGeneralMgr.Find(dwId);


    if (pstResGeneral == NULL)
    {
        LOGERR("dwId-%u is not found", dwId);
        return -1;
    }

    return (int)pstResGeneral->m_bPhase;
}

int GeneralCard::AddDataForNewPlayer(PlayerData* pstData)
{
    RESGENERAL* pstResGeneral = NULL;
    ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
    int iNum = rstResGeneralMgr.GetResNum();
    //uint32_t dwGeneralList[MAX_TROOP_NUM] = {0};
    int index = 0;
    for (int i=0; i<iNum; i++)
    {
        pstResGeneral = rstResGeneralMgr.GetResByPos(i);
        if (!pstResGeneral)
        {
            LOGERR("pEquip is null.");
            return -1;
        }

        if (pstResGeneral->m_bIsFree)
        {
            this->AddWrapPrimary(pstData, pstResGeneral->m_dwId);
            if(index < MAX_TROOP_FIGHT_NUM)
            {
                //dwGeneralList[index] = pstResGeneral->m_dwId;
                ++index;
            }
        }
    }
    //Majesty::Instance().SetDefaultGeneral(pstData, (uint8_t)PKGMETA::MATCH_TYPE_3V3, index, dwGeneralList);

    return ERR_NONE;
}

int GeneralCard::AddDataForDebug(PlayerData* pstData, DT_ITEM* pstItemList, int iIdx /*= 0*/)
{
    RESGENERAL* pstResGeneral = NULL;
    ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
    int iNum = rstResGeneralMgr.GetResNum();
    if (iNum > MAX_DEBUG_CNT_NUM)
    {
        iNum = MAX_DEBUG_CNT_NUM;
        LOGERR("GeneralCard : ResNum is larger than max num");
    }

    DT_ITEM_GCARD * pstGCard = NULL;
    int iIndex = iIdx;

    for (int i=0; i<iNum; i++)
    {
        pstResGeneral = rstResGeneralMgr.GetResByPos(i);
        if (!pstResGeneral)
        {
            LOGERR("pEquip is null.");
            return -1;
        }

        if ((pstResGeneral->m_bIsComposite == 0) || (pstResGeneral->m_bIsDebugTest == 0))
        {
            continue;
        }
        pstGCard = this->AddWrapPrimary(pstData, pstResGeneral->m_dwId);
        if (pstGCard != NULL)
        {
            DT_ITEM& rstItem = pstItemList[iIndex];
            rstItem.m_bItemType = ITEM_TYPE_GCARD;
            rstItem.m_dwItemId = pstResGeneral->m_dwId;
            rstItem.m_stItemData.m_stGCard = *pstGCard;
            iIndex++;
        }
    }
    return iIndex;
}

DT_ITEM_GCARD* GeneralCard::AddWrapPrimary(PlayerData* pstData, uint32_t dwId, int32_t iStar)
{
    // init
    DT_ITEM_GCARD stGCard;
    memset(&stGCard, 0, sizeof(stGCard));
    stGCard.m_dwId = dwId;
    stGCard.m_bLevel = 1;
    stGCard.m_bLvPhase= 1;
	stGCard.m_dwSkillFlagList = 0xffffffff;
	stGCard.m_dwAIFlagList = 0xffffffff;


    ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
    RESGENERAL* pResGeneral = rstResGeneralMgr.Find(dwId);
    if (!pResGeneral)
    {
        LOGERR("Uin<%lu> AddWrapPrimary fail, pResGeneral<%u> is null", pstData->m_ullUin, dwId);
        return NULL;
    }
    stGCard.m_bPhase = pResGeneral->m_bPhase; // 初始品阶

	if (NOT_SET_STAR == iStar)
	{
        stGCard.m_bStar = pResGeneral->m_bBornStar;  //初识星级
	}
	else
	{
        stGCard.m_bStar = iStar;
	}

	//初始亲密度
    stGCard.m_wTrainValue = 1;
    stGCard.m_bTrainPhase = 1;

    //初始技能等级，所有技能初始为1级，但是星级不够时不会解锁
    for (uint8_t i=0; i<MAX_NUM_GENERAL_SKILL; i++)
    {
        stGCard.m_szSkillLevel[i] = 1;
    }

    //初始兵种设置
    stGCard.m_bArmyLv = 1;
    stGCard.m_bArmyPhase = 1;

    ResGeneralSkillMgr_t& rstResGeneralSkillMgr = CGameDataMgr::Instance().GetResGeneralSkillMgr();
    RESGENERALSKILL* pResGeneralSkill = rstResGeneralSkillMgr.Find(pResGeneral->m_dwActiveSkillId);
    if (!pResGeneralSkill)
    {
        LOGERR("Uin<%lu> AddWrapPrimary fail, GCard<%u> pResGeneralSkill<%u> is null",
            pstData->m_ullUin, dwId, pResGeneral->m_dwActiveSkillId);
        return NULL;
    }
    for (int i=0; i<MAX_SKILL_CHEATS_NUM; i++)
    {
        stGCard.m_astCheatsList[i].m_bLevel = 0;
    }
    stGCard.m_astCheatsList[0].m_dwId = pResGeneralSkill->m_wCheats1;
    stGCard.m_astCheatsList[1].m_dwId = pResGeneralSkill->m_wCheats2;


    //初始装备
    ResEquipsListMgr_t& rstResEquipsListMgr = CGameDataMgr::Instance().GetResEquipsListMgr();
    RESEQUIPSLIST* pResEquipsList = NULL;

    DT_ITEM_EQUIP stItem;
    bzero(&stItem, sizeof(DT_ITEM_EQUIP));
    stItem.m_bLevel = 1;
    stItem.m_dwGCardId = dwId;
    stItem.m_bPhase = 1;
    int iSeq = 0;
    for (int i=0; i<EQUIP_TYPE_MAX_NUM; i++)
    {
        pResEquipsList = rstResEquipsListMgr.Find(pResGeneral->m_equipList[i]);
        stItem.m_dwId = pResEquipsList->m_grade[0];
        stItem.m_dwEquipListId = pResGeneral->m_equipList[i];
        stItem.m_bType = i + 1;
        iSeq = Equip::Instance().Add(pstData, &stItem);
        if (iSeq < 0)
        {
            return NULL;
        }
        stGCard.m_astEquipList[i].m_bType = i+1;
        stGCard.m_astEquipList[i].m_dwEquipSeq= iSeq;
    }

	//亲密度增加的属性buff需要重新计算
	pstData->m_bIsFeedTrainInit = false;
    int iIndex = this->Add(pstData, &stGCard);
    if (iIndex < 0 || iIndex > (MAX_NUM_ROLE_GCARD - 1))
    {
        return NULL;
    }

    return &pstData->GetGCardInfo().m_astData[iIndex];
}

//dwId为新添加的武将ID,只有在新添加武将时才调用此接口
void GeneralCard::OpenGCardFate(PlayerData* pstData, uint32_t dwId)
{
    ResGeneralFateMgr_t& rstResGeneralFateMgr = CGameDataMgr::Instance().GetResGeneralFateMgr();
    RESGENERALFATE* poResGeneralFate = NULL;
    DT_ITEM_GCARD* pstGeneral = NULL;
    RESGENERAL* pResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(dwId);
    if (NULL == pResGeneral)
    {
        LOGERR("Uin<%lu> pResGeneral<%u> is NULL", pstData->m_ullUin, dwId);
        return;
    }
    bool bIsOpen = true;
    vector<DT_ITEM_GCARD*> vpGCard;
    for (int i = 0; i < RES_MAX_GCARD_FATEBUFFIDS_NUM; i++)
    {
        if (0 == pResGeneral->m_fateId[i])
        {
            continue;
        }
        poResGeneralFate = rstResGeneralFateMgr.Find(pResGeneral->m_fateId[i]);
        if (NULL == poResGeneralFate)
        {
            LOGERR("Uin<%lu> poResGeneralFate<%u> is NULL", pstData->m_ullUin, pResGeneral->m_fateId[i]);
            continue;
        }
        //查找是否拥有相关缘分武将
        bIsOpen = true;
        vpGCard.clear();

        for (int j = 0 ; j < RES_MAX_GCARD_FATE_RELEVANCE_NUM; j++)
        {
            if (0 == poResGeneralFate->m_generalIds[j])
            {
                continue;
            }
            pstGeneral = Find(pstData, poResGeneralFate->m_generalIds[j]);
            if (pstGeneral == NULL)
            {
                bIsOpen = false;
                break;
            }
            vpGCard.push_back(pstGeneral);
        }
        //缘分开启
        if (bIsOpen)
        {
            pstData->m_setOpenGCardFate.insert(pResGeneral->m_fateId[i]);
            for (size_t j = 0; j < vpGCard.size(); j++)
            {
                pstGeneral = vpGCard[j];
                if (dwId != pstGeneral->m_dwId)
                {
                    //新增加的武将不调用此接口
                    TriggerUpdateGCardLi(pstData, pstGeneral);
                }
                /*
                    如果保存在武将身上存入数据库,当策划增加一个缘分时,就要做版本更新处理,暂时不这么做
                //对每个缘分武将写入缘分id
                int iFateIndex = 0;
                for (; iFateIndex < MAX_GCARD_FATEBUFFIDS_NUM; iFateIndex++)
                {
                    //判断身上是否已有,没有则在后面插入
                    if (pstGeneral->m_FateBuffIds[iFateIndex] == poResGeneralFate->m_dwId)
                    {
                        //已有,退出
                        iFateIndex = MAX_GCARD_FATEBUFFIDS_NUM;
                        break;
                    }
                    if (pstGeneral->m_FateBuffIds[iFateIndex] == 0)
                    {
                        break;
                    }
                }

                //插入
                if (iFateIndex != MAX_GCARD_FATEBUFFIDS_NUM)
                {
                    pstGeneral->m_FateBuffIds[iFateIndex] = poResGeneralFate->m_dwId;
                    if (dwId!= pstGeneral->m_dwId)
                    {
                        TriggerUpdateGCardLi(pstData, pstGeneral);
                    }
                }
                */
            }
        }

    }
}

uint32_t GeneralCard::InitLeaderValue(PlayerData* pstData)
{
    uint32_t dwLeaderValue = 0;
    DT_ROLE_GCARD_INFO& rstInfo = pstData->GetGCardInfo();
    for (int k = 0; k < rstInfo.m_iCount; k++)
    {
		dwLeaderValue = GetGCardLeaderValue(pstData, &rstInfo.m_astData[k]);
    }

    ResFameHallTeamMgr_t& rstFameHallTeamMgr = CGameDataMgr::Instance().GetResFameHallTeamMgr();
    int iResNum = rstFameHallTeamMgr.GetResNum();
    for (int i = 0; i < iResNum; i++)
    {
        RESFAMEHALLTEAM* pResFameHallTeam = rstFameHallTeamMgr.GetResByPos(i);
        bool bFlag = true;
        for (int j = 0; j < 3; j++)
        {
            if (pResFameHallTeam->m_generalsId[j] == 0 || !Find(pstData, pResFameHallTeam->m_generalsId[j]))
            {
                bFlag = false;
                break;
            }
        }

        if (bFlag)
        {
            dwLeaderValue += m_dwFameHallLeaderValue;
        }
    }

    pstData->m_dwLeaderValue = dwLeaderValue;

    return dwLeaderValue;
}

void GeneralCard::UptLeaderValue(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral)
{
	uint32_t dwLeaderValue = GetGCardLeaderValue(pstData, pstGeneral);

    ResFameHallTeamMgr_t& rstFameHallTeamMgr = CGameDataMgr::Instance().GetResFameHallTeamMgr();
    int iResNum = rstFameHallTeamMgr.GetResNum();
    bool bFlag = false;
    for (int i = 0; i < iResNum; i++)
    {
        RESFAMEHALLTEAM * pResFameHallTeam = rstFameHallTeamMgr.GetResByPos(i);
        for (int j = 0; j < 3; j++)
        {
            if (pResFameHallTeam->m_generalsId[j] == 0)
            {
                continue;
            }

            if (pResFameHallTeam->m_generalsId[j] == pstGeneral->m_dwId)
            {
                bool bExist = true;
                for (int k = 0; k < 3; k++)
                {
                    if (pResFameHallTeam->m_generalsId[k] == 0 || !Find(pstData, pResFameHallTeam->m_generalsId[k]))
                    {
                        bExist = false;
                        break;
                    }
                }
                bFlag = bExist;
                break;
            }
        }
    }

    if (bFlag)
    {
        dwLeaderValue += m_dwFameHallLeaderValue;
    }

    pstData->m_dwLeaderValue += dwLeaderValue;
}

uint32_t GeneralCard::GetGCardLeaderValue(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral)
{
	uint32_t dwLeaderValue = 0;
	dwLeaderValue += pstGeneral->m_bStar * m_dwStarLeaderValue;
	dwLeaderValue += pstGeneral->m_bPhase * m_dwPhaseLeaderValue;
	dwLeaderValue += pstGeneral->m_wTrainValue * m_dwTrainLvLeaderValue;
	dwLeaderValue += pstGeneral->m_bTrainPhase * m_dwTrainPhaseLeaderValue;
	return dwLeaderValue;
}

uint32_t GeneralCard::CalGcardLi(PlayerData * pstData, uint32_t dwId)
{
    DT_ITEM_GCARD* pstGeneral = Find(pstData, dwId);
    if (pstGeneral == NULL)
    {
        LOGERR("Uin<%lu> cant find the GCard<%u>", pstData->m_ullUin, dwId);
        return 0;
    }
    return this->CalGcardLi(pstData, pstGeneral);
}

uint32_t GeneralCard::CalGcardLi(PlayerData * pstData, DT_ITEM_GCARD * pstGeneral)
{
    CpuSampleStats::Instance().BeginSample("GeneralCard::CalGcardLi()");

    RESGENERAL* poResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(pstGeneral->m_dwId);
    RESLIGCARDBASE* poResLiGCardBase = CGameDataMgr::Instance().GetResLiGCardBaseMgr().Find(pstGeneral->m_dwId);
    if (poResGeneral == NULL || poResLiGCardBase == NULL)
    {
        LOGERR("Uin<%lu> GCard<%u> cant find the poResGeneral or poResLiGCardBase", pstData->m_ullUin, pstGeneral->m_dwId);
        return 0;
    }
    LiObj rRtData = { 0 };
    _GetGCardAttr(pstData, pstGeneral, rRtData);

    float fPermillage = 0.001;
    float fFiveBaseAttLi = (rRtData.m_afAttr[ATTR_HP] * poResLiGCardBase->m_wHpFactor +
        rRtData.m_afAttr[ATTR_STR] * poResLiGCardBase->m_wAtkFactor +
        rRtData.m_afAttr[ATTR_WIT] * poResLiGCardBase->m_wWitFactor +
        rRtData.m_afAttr[ATTR_STRDEF] * poResLiGCardBase->m_wAtkDefFactor +
        rRtData.m_afAttr[ATTR_WITDEF] * poResLiGCardBase->m_wWitDefFactor) * fPermillage;

    //  暴击
    float fSpecialCritical = rRtData.m_afAttr[ATTR_CHANCE_CRITICAL] * fPermillage * (rRtData.m_afAttr[ATTR_CHANCE_CRITICAL_VALUE] * fPermillage - 1);
    //  抗暴击
    float fSpecialAntiCritical = rRtData.m_afAttr[ATTR_CHANCE_ANTICRITICAL] * fPermillage * rRtData.m_afAttr[ATTR_CHANCE_ANTICRITICAL_VALUE] * fPermillage;
    //  闪避
    float fSepecialDoge = (1 == rRtData.m_afAttr[ATTR_CHANCE_DODGE] * fPermillage ? 0 : rRtData.m_afAttr[ATTR_CHANCE_DODGE]) * fPermillage / (1 - rRtData.m_afAttr[ATTR_CHANCE_DODGE] * fPermillage);
    //  命中
    float fSepecialHit = (1 == rRtData.m_afAttr[ATTR_CHANCE_HIT] * fPermillage ? 0 : rRtData.m_afAttr[ATTR_CHANCE_HIT]) * fPermillage / (1 - rRtData.m_afAttr[ATTR_CHANCE_HIT] * fPermillage);
    //  吸血
    float fSpecialiSuck = rRtData.m_afAttr[ATTR_SUCKBLOOD] * fPermillage;


    float fSpecialRatio = (fSpecialCritical + fSpecialAntiCritical + fSepecialDoge + fSepecialHit + fSpecialiSuck) / 3;
    float fSkillRatio = _GetGCardSkillRatio(pstData, pstGeneral, poResGeneral->m_bSpecialEquipType, poResGeneral->m_dwTalent);
    float fTotalLi = fFiveBaseAttLi * (1 + fSpecialRatio + fSkillRatio) + poResLiGCardBase->m_dwInitLi;

    uint32_t dwMSkillLi = GetMSkillLi(pstData, pstGeneral);
    //保存战力
    pstGeneral->m_dwBase5Li = (uint32_t)fFiveBaseAttLi;
    pstGeneral->m_dwLi = (uint32_t)fTotalLi + dwMSkillLi;

    /*
    LOGRUN("Uin<%lu> GCard<%u> All Attr  Hp=%f, Str=%f, Wit=%f, StrDef=%f, WitDef=%f, Speed=%f ChHit=%f, ChDodge=%f ,ChCritical=%f, ChAntiCritical=%f, ChBlock=%f, ChParaCritical=%f, ChParaAntiCritical=%f, ParaBlock=%f, ParaDamageAdd=%f, ParaSuckBlood=%f, BaseDam=%f",
    rRtData.m_ullUin, rRtData.m_dwId,
    rRtData.m_afAttr[ATTR_HP], rRtData.m_afAttr[ATTR_STR], rRtData.m_afAttr[ATTR_WIT],
    rRtData.m_afAttr[ATTR_STRDEF], rRtData.m_afAttr[ATTR_WITDEF], rRtData.m_afAttr[ATTR_SPEED], rRtData.m_afAttr[ATTR_CHANCE_HIT], rRtData.m_afAttr[ATTR_CHANCE_DODGE],
    rRtData.m_afAttr[ATTR_CHANCE_CRITICAL], rRtData.m_afAttr[ATTR_CHANCE_ANTICRITICAL], rRtData.m_afAttr[ATTR_CHANCE_BLOCK], rRtData.m_afAttr[ATTR_CHANCE_CRITICAL_VALUE],
    rRtData.m_afAttr[ATTR_CHANCE_ANTICRITICAL_VALUE], rRtData.m_afAttr[ATTR_CHANCE_BLOCK_VALUE], rRtData.m_afAttr[ATTR_DAMAGEADD], rRtData.m_afAttr[ATTR_SUCKBLOOD] , rRtData.m_afAttr[ATTR_BASE_NORMALATK]);

    LOGRUN("Uin<%lu> GCard<%u> GCardToTalLi<%u> fTotalLi<%f> dwMSkillLi<%u>, fFiveBaseAttLi<%f> fSpecialRatio<%f> fSkillRatio<%f>",
    pstData->m_ullUin, pstGeneral->m_dwId, pstGeneral->m_dwLi, fTotalLi, dwMSkillLi, fFiveBaseAttLi, fSpecialRatio, fSkillRatio);
    //*/

    CpuSampleStats::Instance().EndSample();
    if (pstData->GetMajestyInfo().m_dwHighGCardLi < pstGeneral->m_dwLi)
    {
        pstData->GetMajestyInfo().m_dwHighGCardLi = pstGeneral->m_dwLi;
        pstData->GetMajestyInfo().m_dwHighGCardLiId = pstGeneral->m_dwId;
    }

    return pstGeneral->m_dwLi;
}

uint32_t GeneralCard::GetMSkillLi(PlayerData * pstData, DT_ITEM_GCARD* pstGeneral)
{
    if (!pstData->m_bIsMSkillInit)
    {
        //军师技属性未初始化
        CalMSkillAttr(pstData);
    }
    return GetMSkillLi(pstData, pstGeneral, pstData->m_afMSkillAttr);
}

uint32_t GeneralCard::GetMSkillLi(PlayerData * pstData, DT_ITEM_GCARD * pstGeneral, float * afTempAttr)
{
    LiObj rRtData = { 0 };
    rRtData.AddAttr(afTempAttr);
    RESGENERAL* poResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(pstGeneral->m_dwId);
    RESLIGCARDBASE* poResLiGCardBase = CGameDataMgr::Instance().GetResLiGCardBaseMgr().Find(pstGeneral->m_dwId);
    if (poResGeneral == NULL || poResLiGCardBase == NULL)
    {
        LOGERR("Uin<%lu> GCard<%u> cant find the poResGeneral or poResLiGCardBase", pstData->m_ullUin, pstGeneral->m_dwId);
        return 0;
    }
    // 基础属性

    rRtData.m_afAttr[ATTR_CHANCE_CRITICAL_VALUE] += poResGeneral->m_fInitParaCritical;
    rRtData.m_afAttr[ATTR_CHANCE_ANTICRITICAL_VALUE] += poResGeneral->m_fInitParaAntiCritical;
    float fPermillage = 0.001;
    float fFiveBaseAttLi = (rRtData.m_afAttr[ATTR_HP] * poResLiGCardBase->m_wHpFactor +
        rRtData.m_afAttr[ATTR_STR] * poResLiGCardBase->m_wAtkFactor +
        rRtData.m_afAttr[ATTR_WIT] * poResLiGCardBase->m_wWitFactor +
        rRtData.m_afAttr[ATTR_STRDEF] * poResLiGCardBase->m_wAtkDefFactor +
        rRtData.m_afAttr[ATTR_WITDEF] * poResLiGCardBase->m_wWitDefFactor) * fPermillage;



    //  暴击
    float fSpecialCritical = rRtData.m_afAttr[ATTR_CHANCE_CRITICAL] * fPermillage * (rRtData.m_afAttr[ATTR_CHANCE_CRITICAL_VALUE] * fPermillage - 1);
    //  抗暴击
    float fSpecialAntiCritical = rRtData.m_afAttr[ATTR_CHANCE_ANTICRITICAL] * fPermillage * rRtData.m_afAttr[ATTR_CHANCE_ANTICRITICAL_VALUE] * fPermillage;
    //  闪避
    float fSepecialDoge = (1 == rRtData.m_afAttr[ATTR_CHANCE_DODGE] * fPermillage ? 0 : rRtData.m_afAttr[ATTR_CHANCE_DODGE]) * fPermillage / (1 - rRtData.m_afAttr[ATTR_CHANCE_DODGE] * fPermillage);
    //  命中
    float fSepecialHit = (1 == rRtData.m_afAttr[ATTR_CHANCE_HIT] * fPermillage ? 0 : rRtData.m_afAttr[ATTR_CHANCE_HIT]) * fPermillage / (1 - rRtData.m_afAttr[ATTR_CHANCE_HIT] * fPermillage);
    //  吸血
    float fSpecialiSuck = rRtData.m_afAttr[ATTR_SUCKBLOOD] * fPermillage;


    float fSpecialRatio = (fSpecialCritical + fSpecialAntiCritical + fSepecialDoge + fSepecialHit + fSpecialiSuck) / 3;

    float fTotal = pstGeneral->m_dwBase5Li * fSpecialRatio + fFiveBaseAttLi;
    /*
    LOGRUN("Uin<%lu> MasterSkill:All Attr  Hp=%f, Str=%f, Wit=%f, StrDef=%f, WitDef=%f, Speed=%f ChHit=%f, ChDodge=%f ,ChCritical=%f, ChAntiCritical=%f, ChBlock=%f, ChParaCritical=%f, ChParaAntiCritical=%f, ParaBlock=%f, ParaDamageAdd=%f, ParaSuckBlood=%f, BaseDam=%f",
        rRtData.m_ullUin,
        rRtData.m_afAttr[ATTR_HP], rRtData.m_afAttr[ATTR_STR], rRtData.m_afAttr[ATTR_WIT],
        rRtData.m_afAttr[ATTR_STRDEF], rRtData.m_afAttr[ATTR_WITDEF], rRtData.m_afAttr[ATTR_SPEED], rRtData.m_afAttr[ATTR_CHANCE_HIT], rRtData.m_afAttr[ATTR_CHANCE_DODGE],
        rRtData.m_afAttr[ATTR_CHANCE_CRITICAL], rRtData.m_afAttr[ATTR_CHANCE_ANTICRITICAL], rRtData.m_afAttr[ATTR_CHANCE_BLOCK], rRtData.m_afAttr[ATTR_CHANCE_CRITICAL_VALUE],
        rRtData.m_afAttr[ATTR_CHANCE_ANTICRITICAL_VALUE], rRtData.m_afAttr[ATTR_CHANCE_BLOCK_VALUE], rRtData.m_afAttr[ATTR_DAMAGEADD], rRtData.m_afAttr[ATTR_SUCKBLOOD] , rRtData.m_afAttr[ATTR_BASE_NORMALATK]);

    LOGRUN("Uin<%lu> MasterSkill fTotalLi<%f>  fFiveBaseAttLi<%f> fSpecialRatio<%f> GCard5Base<%u>",
        pstData->m_ullUin, fTotal, fFiveBaseAttLi, fSpecialRatio, pstGeneral->m_dwBase5Li);
    //*/
    return uint32_t(fTotal);
}

void GeneralCard::CalAllCardLiByMSkillUp(PlayerData * pstData)
{
    DT_ROLE_GCARD_INFO& rstInfo = pstData->GetGCardInfo();
    float fOldAttr[MAX_ATTR_ADD_NUM] = { 0 };
    memcpy(fOldAttr, pstData->m_afMSkillAttr, sizeof(fOldAttr));
    CalMSkillAttr(pstData);
    uint32_t dwOldLi = 0;
    uint32_t dwNewLi = 0;
    for (int i = 0 ; i < rstInfo.m_iCount; i++)
    {
        dwOldLi = GetMSkillLi(pstData, &rstInfo.m_astData[i], fOldAttr);
        dwNewLi = GetMSkillLi(pstData, &rstInfo.m_astData[i], pstData->m_afMSkillAttr);
        rstInfo.m_astData[i].m_dwLi += dwNewLi - dwOldLi;
        RankMgr::Instance().UpdateGCardLi(pstData, &rstInfo.m_astData[i]);
    }

}

void GeneralCard::CalMSkillAttr(PlayerData * pstData)
{
    float afTempAttr[MAX_ATTR_ADD_NUM] = { 0 };
    DT_ROLE_MSKILL_INFO& rstInfo = pstData->GetMSkillInfo();
    RESCONSUME *poConsume = NULL;
    RESMASTERSKILL *pResMSkill = NULL;


    for (int i = 0; i < rstInfo.m_iCount; i++)
    {

        pResMSkill = CGameDataMgr::Instance().GetResMasterSkillMgr().Find(rstInfo.m_astData[i].m_bId);
        if (NULL == pResMSkill)
        {
            continue;
        }
        uint8_t bLv = rstInfo.m_astData[i].m_bLevel;
        if (bLv < 1 || bLv > RES_MAX_MS_LEVEL)
        {
            continue;
        }
        for (uint32_t j = 0; j < pResMSkill->m_bMSAttrAddNum; j++)
        {
            poConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(pResMSkill->m_mSAttrAddBaseGrowId[j]);
            if (NULL == poConsume)
            {
                continue;
            }
            afTempAttr[pResMSkill->m_szMSAttrAddType[j]] += poConsume->m_lvList[bLv - 1];
        }
    }
    //保存值
    memcpy(pstData->m_afMSkillAttr, afTempAttr, sizeof(pstData->m_afMSkillAttr));
    pstData->m_bIsMSkillInit = true;
}

int GeneralCard::HandleGroupCard(PlayerData* pstData, uint8_t bType, uint32_t dwParam)
{
	DT_ROLE_MISC_INFO& rstMiscInfo = pstData->GetMiscInfo();

	ResMajestyGroupCardMgr_t& rResMajestyGroupCardMgr = CGameDataMgr::Instance().GetResMajestyGroupCardMgr();
	bool bNotifyClient = false;
	for (int i = 0; i < 6; i++)
	{
		uint8_t bOn = (1 << i) & rstMiscInfo.m_bGroupCardId;

		if (bOn > 0)
		{
			continue;
		}

		RESMAJESTYGROUPCARD* poResMajestyGroupCard = rResMajestyGroupCardMgr.GetResByPos(i);

		if (poResMajestyGroupCard == NULL)
		{
			continue;
		}

		switch (bType)
		{
		case 1:
			//升级类型组卡开孔
			if (1 == poResMajestyGroupCard->m_bType && dwParam >= poResMajestyGroupCard->m_dwParam)
			{
				rstMiscInfo.m_bGroupCardId |= (1 << i);
				bNotifyClient = true;
			}
			break;
		case 2:
			//关卡类型组卡开孔
			if (bType == poResMajestyGroupCard->m_bType && dwParam == poResMajestyGroupCard->m_dwParam)
			{
				rstMiscInfo.m_bGroupCardId |= (1 << i);
				bNotifyClient = true;
			}
			break;
		default:
			break;
		}
	}
	if (bNotifyClient)
	{
		m_stScPkg.m_stHead.m_wMsgId = SC_MSG_MAJESTY_GROUP_CARD_SYN;
		m_stScPkg.m_stBody.m_stGroupCardSyn.m_bGroupCardId = rstMiscInfo.m_bGroupCardId;
		ZoneSvrMsgLayer::Instance().SendToClient(pstData->m_pOwner, &m_stScPkg);
	}

	return 0;
}

// 返回所升等级
int GeneralCard::LvUp(PlayerData* pstData, uint32_t dwId, DT_CONSUME_ITEM_INFO& rstConsumeItemInfo, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
    DT_ITEM_GCARD* pstGeneral = Find(pstData, dwId);
    if (!pstGeneral)
    {
        LOGERR("pstGeneral is null");
        return ERR_NOT_FOUND;
    }

    ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
    RESGENERAL* pResGeneral = rstResGeneralMgr.Find(dwId);
    if (!pResGeneral)
    {
        LOGERR("pResGeneral is null");
        return ERR_NOT_FOUND;
    }

    //计算总经验
    uint32_t dwIncExp = 0;
    ResPropsMgr_t& rstResPropsMgr = CGameDataMgr::Instance().GetResPropsMgr();
    for (int i=0; i<rstConsumeItemInfo.m_bConsumeCount; i++)
    {
        DT_ITEM_CONSUME & rstItemConsume = rstConsumeItemInfo.m_astConsumeList[i];
        if (rstItemConsume.m_bItemType != ITEM_TYPE_PROPS)
        {
            LOGERR("Item Type is not Props");
            return ERR_DEFAULT;
        }

        RESPROPS* pResProps = rstResPropsMgr.Find(rstItemConsume.m_dwItemId);
        if (!pResProps)
        {
            LOGERR("pResProps is null");
            return ERR_NOT_FOUND;
        }

        if (!Props::Instance().IsEnough(pstData, rstItemConsume.m_dwItemId, rstItemConsume.m_dwItemNum))
        {
            LOGERR("Props is not Enough");
            return ERR_NOT_ENOUGH_PROPS;
        }

        dwIncExp += pResProps->m_dwExp * rstItemConsume.m_dwItemNum;
    }

    //计算增加经验后的等级
    uint32_t dwExp = pstGeneral->m_dwExp + dwIncExp;
    uint8_t bLevel = pstGeneral->m_bLevel;
    uint8_t bOldLevel = pstGeneral->m_bLevel;
    ResConsumeMgr_t& rstResConsumeMgr = CGameDataMgr::Instance().GetResConsumeMgr();
    RESCONSUME* pResConsume = rstResConsumeMgr.Find(pResGeneral->m_dwLevelExpConsume);
    if (!pResConsume )
    {
        LOGERR("pResConsume is null");
        return ERR_SYS;
    }
    while (bLevel <= pResConsume->m_dwLvCount)
    {
        if (dwExp >= pResConsume->m_lvList[bLevel-1])
        {
            bLevel++;
        }
        else
        {
            break;
        }
    }
    bLevel--;

    //武将等级不能超过主公等级
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
    if (bLevel > rstMajestyInfo.m_wLevel)
    {
        bLevel = rstMajestyInfo.m_wLevel;
        dwExp = pResConsume->m_lvList[bLevel] -1;
    }

    if (bLevel == RES_MAX_GENERAL_LV_LEN)
    {
        dwExp = pResConsume->m_lvList[RES_MAX_GENERAL_LV_LEN - 1];
    }

    //等级不能超过当前阶的最大值
//    if (bLevel > pstGeneral->m_bLvPhase * 10)
//    {
//        bLevel = pstGeneral->m_bLvPhase * 10;
//        dwExp = pResConsume->m_lvList[bLevel] -1;
//    }
    pstGeneral->m_dwExp = dwExp;
    int ibLevelUp = bLevel - pstGeneral->m_bLevel;
    pstGeneral->m_bLevel = bLevel;

    //物品同步
    for (int i=0; i<rstConsumeItemInfo.m_bConsumeCount; i++)
    {
        DT_ITEM_CONSUME & rstItemConsume = rstConsumeItemInfo.m_astConsumeList[i];
        Item::Instance().ConsumeItem(pstData, rstItemConsume.m_bItemType, rstItemConsume.m_dwItemId, -rstItemConsume.m_dwItemNum, rstSyncItemInfo, METHOD_GENERAL_OP_LVUP);
    }

    ZoneLog::Instance().WriteGeneralLog(pstData, dwId, pstData->GetMajestyInfo().m_wLevel, METHOD_GENERAL_OP_LVUP, bOldLevel, pstGeneral->m_bLevel, 0,
										pstGeneral->m_bPhase, pstGeneral->m_wTrainValue, pstGeneral->m_bStar);
    if (ibLevelUp != 0)
    {
        TriggerUpdateGCardLi(pstData, pstGeneral);
    }

    return ibLevelUp;
}

// 直接增加武将经验，副本战斗用
int GeneralCard::LvUp(PlayerData* pstData, uint32_t dwId, uint32_t dwIncExp)
{
	DT_ITEM_GCARD* pstGeneral = Find(pstData, dwId);
	if (!pstGeneral)
	{
		LOGERR("pstGeneral is null");
		return ERR_NOT_FOUND;
	}

	ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
	RESGENERAL* pResGeneral = rstResGeneralMgr.Find(dwId);
	if (!pResGeneral)
	{
		LOGERR("pResGeneral is null");
		return ERR_NOT_FOUND;
	}

	//计算增加经验后的等级
	uint32_t dwExp = pstGeneral->m_dwExp + dwIncExp;
	uint8_t bLevel = pstGeneral->m_bLevel;
	//uint8_t bOldLevel = pstGeneral->m_bLevel;
	ResConsumeMgr_t& rstResConsumeMgr = CGameDataMgr::Instance().GetResConsumeMgr();
	RESCONSUME* pResConsume = rstResConsumeMgr.Find(pResGeneral->m_dwLevelExpConsume);
	if (!pResConsume )
	{
		LOGERR("pResConsume is null");
		return ERR_SYS;
	}
	while (bLevel <= pResConsume->m_dwLvCount)
	{
		if (dwExp >= pResConsume->m_lvList[bLevel-1])
		{
			bLevel++;
		}
		else
		{
			break;
		}
	}
	bLevel--;

	//武将等级不能超过主公等级
	DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
	if (bLevel > rstMajestyInfo.m_wLevel)
	{
		bLevel = rstMajestyInfo.m_wLevel;
		dwExp = pResConsume->m_lvList[bLevel] -1;
	}

	pstGeneral->m_dwExp = dwExp;
	int ibLevelUp = bLevel - pstGeneral->m_bLevel;
	pstGeneral->m_bLevel = bLevel;
    if (ibLevelUp != 0)
    {
        TriggerUpdateGCardLi(pstData, pstGeneral);
    }
	return ibLevelUp;
}

// 返回，当前阶数
int GeneralCard::LvPhaseUp(PlayerData* pstData, uint32_t dwId, DT_SYNC_ITEM_INFO& rstSyncItemInfo)
{
    DT_ITEM_GCARD* pstGeneral = Find(pstData, dwId);
    if (!pstGeneral)
    {
        LOGERR("pstGeneral is null");
        return ERR_NOT_FOUND;
    }
    ResGeneralLevelCapMgr_t& rstResGeneralLevelCapMgr = CGameDataMgr::Instance().GetResGeneralLevelCapMgr();
    RESGENERALLEVELCAP* pResGeneralLevelCap = rstResGeneralLevelCapMgr.Find(pstGeneral->m_bLvPhase + 1);
    if (!pResGeneralLevelCap)
    {
        LOGERR("pResGeneralLevelCap is null");
        return ERR_SYS;
    }

    if (pstGeneral->m_bLevel < pResGeneralLevelCap->m_wStartLv)
    {
        return ERR_DEFAULT;
    }

    //检查材料是否够
    for (int i=0; i < pResGeneralLevelCap->m_wMaterialTypesCount; i++)
    {
        if (!Props::Instance().IsEnough(pstData, pResGeneralLevelCap->m_materialIds[i], pResGeneralLevelCap->m_materialCount[i]))
        {
            LOGERR("Props is not Enough");
            return ERR_NOT_ENOUGH_PROPS;
        }
    }

    //物品同步
    for (int j=0; j < pResGeneralLevelCap->m_wMaterialTypesCount; j++)
    {
        Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, pResGeneralLevelCap->m_materialIds[j], -pResGeneralLevelCap->m_materialCount[j], rstSyncItemInfo, METHOD_GENERAL_OP_PHASEUP);
    }

    pstGeneral->m_bLvPhase++;

    TriggerUpdateGCardLi(pstData, pstGeneral);

    return pstGeneral->m_bLvPhase;
}

int GeneralCard::LvReborn(PlayerData* pstData, uint32_t dwId, DT_SYNC_ITEM_INFO& rstRewardItemInfo, uint32_t dwRatio)
{
    DT_ITEM_GCARD* pstGeneral = Find(pstData, dwId);
    if (!pstGeneral)
    {
        LOGERR("Player(%lu) LvReborn failed, pstGeneral is null", pstData->m_ullUin);
        return ERR_NOT_FOUND;
    }

    uint32_t dwExp = pstGeneral->m_dwExp;
    dwExp = (dwExp * dwRatio) / 100;
    for (int i=MAX_NUM_EQUIP_EXPCARD_TYPE-1; i>=0; i--)
    {
        uint32_t dwNum = dwExp /m_ExpCardExp[i];
        Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, m_ExpCardId[i], dwNum, rstRewardItemInfo, METHOD_GENERAL_OP_REBORN);
        dwExp -= dwNum * m_ExpCardExp[i];
    }

    pstGeneral->m_bLevel = 1;
    pstGeneral->m_dwExp = 0;
    pstGeneral->m_bLvPhase = 1;
    //  脱掉宝石
    for (uint8_t i = 0; i < MAX_GEM_SLOT_NUM; i++)
    {
        if ( pstGeneral->m_GemSlot[i] > 1)
        {// 0是锁住, 1是空,其他就是宝石ID
            Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, pstGeneral->m_GemSlot[i], 1, rstRewardItemInfo, METHOD_GENERAL_OP_REBORN);
        }
        pstGeneral->m_GemSlot[i]  = 0; //锁住
    }
    return ERR_NONE;
}

// 返回当前的星级
int GeneralCard::StarUp(PlayerData* pstData, uint32_t dwId, SC_PKG_GCARD_STAR_UP_RSP& rstScPkgBodyRsp)
{
    DT_ITEM_GCARD* pstGeneral = Find(pstData, dwId);
    if (!pstGeneral)
    {
        LOGERR("pstGeneral is null");
        return ERR_NOT_FOUND;
    }

    ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
    RESGENERAL* pResGeneral = rstResGeneralMgr.Find(pstGeneral->m_dwId);
    if (!pResGeneral)
    {
        LOGERR("pResGeneral is null");
        return ERR_NOT_FOUND;
    }

    // find consume
    ResConsumeMgr_t& rstResConsumeMgr = CGameDataMgr::Instance().GetResConsumeMgr();
    RESCONSUME* pResConsume = rstResConsumeMgr.Find(pResGeneral->m_dwUpStarConsume);
    if (!pResConsume )
    {
        LOGERR("pResConsume is null");
        return ERR_NOT_FOUND;
    }

    if (pstGeneral->m_bStar >= RES_MAX_STAR_LEVEL -1)
    {
        return ERR_TOP_LEVEL;
    }

    uint32_t dwNeedNum = pResConsume->m_lvList[pstGeneral->m_bStar];

    // 道具是否足够
    if (!Props::Instance().IsEnough(pstData, pResGeneral->m_dwUpStarProp, dwNeedNum))
    {
        return ERR_NOT_ENOUGH_PROPS;
    }

    //判断金币是否够
    RESCONSUME *pConsumeGold = rstResConsumeMgr.Find(pResGeneral->m_dwGoldgeneralstaruse);
    if(NULL == pConsumeGold)
    {
        LOGERR("pConsumeGold(%u) is not found", pResGeneral->m_dwGoldgeneralstaruse);
        return ERR_SYS;
    }

    if (!Consume::Instance().IsEnoughGold(pstData, pConsumeGold->m_lvList[pstGeneral->m_bStar]))
    {
        return ERR_NOT_ENOUGH_GOLD;
    }

    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, pResGeneral->m_dwUpStarProp, -dwNeedNum, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_STAR_UP);
    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_GOLD, 0, -pConsumeGold->m_lvList[pstGeneral->m_bStar], rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_STAR_UP);

    pstGeneral->m_bStar++;
    rstScPkgBodyRsp.m_bDestStar = pstGeneral->m_bStar;
    Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_GCARD_STAR, 1, 1, dwId, pstGeneral->m_bStar);

    //增加统帅值
    pstData->m_dwLeaderValue += m_dwStarLeaderValue;
    AsyncPvp::Instance().UptToAsyncSvr(pstData);

    //记武将培养日志
    ZoneLog::Instance().WriteGeneralLog(pstData, dwId, pstData->GetMajestyInfo().m_wLevel, METHOD_GENERAL_OP_STAR_UP, pstGeneral->m_bStar -1, pstGeneral->m_bStar, 0,
										pstGeneral->m_bPhase, pstGeneral->m_wTrainValue, pstGeneral->m_bStar);

    if (pstGeneral->m_bStar == 5)
    {
        Marquee::Instance().SaveMarqueeForGCard(pstData, Marquee::GCARD_OPT, pResGeneral, pstGeneral->m_bStar);
        Message::Instance().AutoSendWorldMessage(pstData, 1103, "GCardId=%u", pResGeneral->m_dwId);
        Message::Instance().AutoSendSysMessage(1101, "Name=%s|GCardId=%u", pstData->GetRoleName(), pResGeneral->m_dwId);
    }
    else if (pstGeneral->m_bStar == 6)
    {
        Message::Instance().AutoSendWorldMessage(pstData, 1104, "GCardId=%u", pResGeneral->m_dwId);
        Message::Instance().AutoSendSysMessage(1102, "Name=%s|GCardId=%u", pstData->GetRoleName(), pResGeneral->m_dwId);
    }
    else if (pstGeneral->m_bStar == 4)
    {
        Message::Instance().AutoSendWorldMessage(pstData, 1106, "GCardId=%u", pResGeneral->m_dwId);
        Message::Instance().AutoSendSysMessage(1105, "Name=%s|GCardId=%u", pstData->GetRoleName(), pResGeneral->m_dwId);
    }
    TriggerUpdateGCardLi(pstData, pstGeneral);
    return (int)pstGeneral->m_bStar;
}

int GeneralCard::StarReborn(PlayerData* pstData, uint32_t dwId, DT_SYNC_ITEM_INFO& rstRewardItemInfo, uint32_t* pGold, uint32_t dwRatio)
{
    DT_ITEM_GCARD* pstGeneral = Find(pstData, dwId);
    if (!pstGeneral)
    {
        LOGERR("Player(%lu) LvReborn failed, pstGeneral is null", pstData->m_ullUin);
        return ERR_NOT_FOUND;
    }

    RESGENERAL* pResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(pstGeneral->m_dwId);
    assert(pResGeneral);

    ResConsumeMgr_t& rstResConsumeMgr = CGameDataMgr::Instance().GetResConsumeMgr();
    RESCONSUME* pResPropsConsume = rstResConsumeMgr.Find(pResGeneral->m_dwUpStarConsume);
    RESCONSUME* pResGoldConsume = rstResConsumeMgr.Find(pResGeneral->m_dwGoldgeneralstaruse);
    assert(pResPropsConsume);
    assert(pResGoldConsume);

    uint32_t dwGold = 0;
    uint32_t dwNum = 0;
    for (int i=pstGeneral->m_bStar; i>0; i--)
    {
        dwGold += pResGoldConsume->m_lvList[i-1];
        dwNum += pResPropsConsume->m_lvList[i -1];
    }

    dwGold = (dwGold * dwRatio) / 100;
    dwNum = (dwNum * dwRatio) / 100;

    *pGold = dwGold;
    Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, pResGeneral->m_dwUpStarProp, dwNum, rstRewardItemInfo, METHOD_GENERAL_OP_REBORN);

    pstGeneral->m_bStar = 0;

    return ERR_NONE;
}

int GeneralCard::Composite(PlayerData* pstData, uint32_t dwId, SC_PKG_GCARD_COMPOSITE_RSP& rstScPkgBodyRsp)
{
    ResPropsMgr_t& rstResPropsMgr = CGameDataMgr::Instance().GetResPropsMgr();
    RESPROPS* pResProps = rstResPropsMgr.Find(dwId);
    if (!pResProps)
    {
        LOGERR("Uin<%lu> pResProps<id=%u> is null", pstData->m_ullUin, dwId);
        return ERR_NOT_FOUND;
    }
    // check type
    if (pResProps->m_dwType != PROPS_TYPE_GCARD)
    {
        LOGERR("Uin<%lu> pResProps->m_dwType is err, PropId<%u> type<%u>", pstData->m_ullUin, dwId, pResProps->m_dwType);
        return ERR_SYS;
    }

    // find composite fragments
    //DT_ITEM_PROPS* pstProps = Props::Instance().Find(pstData, dwId);
    if (!Item::Instance().IsEnough(pstData, ITEM_TYPE_PROPS, dwId, pResProps->m_dwItemNum))
    {
        LOGERR("Uin<%lu> Composite general card error:have no enough props<id=%u>", pstData->m_ullUin, dwId);
        return ERR_NOT_ENOUGH_PROPS;
    }
    // find dest general
    uint32_t dwGCardId = pResProps->m_dwItemFull;
    DT_ITEM_GCARD* pstGeneral = Find(pstData, dwGCardId);
    if (pstGeneral)
    {
        LOGERR("Uin<%lu> pstGeneral<%u> is exist", pstData->m_ullUin, dwGCardId);
        return ERR_EXIST_GENERAL_TYPE;
    }

    RESGENERAL* pResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(dwGCardId);
    if (!pResGeneral)
    {
        LOGERR("Uin<%lu> pResGeneral<id=%u> is NULL", pstData->m_ullUin, dwGCardId);
        return ERR_NOT_FOUND;
    }

    Item::Instance().RewardItem(pstData, ITEM_TYPE_GCARD, dwGCardId, pResGeneral->m_bBornStar, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_COMPOSITE);
    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, dwId, -pResProps->m_dwItemNum, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_COMPOSITE);

    return ERR_NONE;
}

int GeneralCard::PhaseUp(PlayerData* pstData, uint32_t dwId, SC_PKG_GCARD_PHASE_UP_RSP& rstScPkgBodyRsp)
{

    DT_ITEM_GCARD* pstGeneral = Find(pstData, dwId);
    if (!pstGeneral)
    {
        LOGERR("PhaseUp : pstGeneral is null");
        return ERR_NOT_FOUND;
    }

    // 已升满阶
    if ( (int)pstGeneral->m_bPhase >= RES_MAX_GENERAL_PHASE )
    {
        return ERR_TOP_LEVEL;
    }

    //uint8_t bDstGrade = pstGeneral->m_bPhase + 1;

    //// 升阶等级检查 暂时去掉，靠前台 改表后的查询方式太难受了，不要了
    //ResGeneralPhaseUpMgr_t& roResGenrPhaseUpMgr = CGameDataMgr::Instance().GetResGeneralPhaseUpMgr();
    //RESGENERALPHASEUP* pResGenrPhaseUp = roResGenrPhaseUpMgr.Find( pstGeneral->m_bPhase );
    //if( !pResGenrPhaseUp )
    //{
    //    LOGERR("pResGenrPhaseUp is null. phase: %d", pstGeneral->m_bPhase);
    //    return ERR_NOT_FOUND;
    //}
    //if( pstGeneral->m_bLevel < pResGenrPhaseUp->m_bPhaseLevel )
    //{
    //    LOGERR("General level is not enough");
    //    return ERR_LEVEL_LIMIT;
    //}

    // 升阶材料检查
    ResGeneralPhaseMgr_t& roResGenrPhaseMgr = CGameDataMgr::Instance().GetResGeneralPhaseMgr();
    RESGENERALPHASE * pResPhase = roResGenrPhaseMgr.Find(pstGeneral->m_dwId);
    if (!pResPhase )
    {
        LOGERR("pResPhase is null");
        return ERR_NOT_FOUND;
    }
    assert( pstGeneral->m_bPhase > 0 );
    uint32_t dwPhasePropsID = pResPhase->m_phaseUpProps[ pstGeneral->m_bPhase-1 ];

    ResGeneralPhasePropsMgr_t& roResGenrPhasePropsMgr = CGameDataMgr::Instance().GetResGeneralPhasePropsMgr();
    RESGENERALPHASEPROPS* pResGenrPhaseProps = roResGenrPhasePropsMgr.Find( dwPhasePropsID );
    if( !pResGenrPhaseProps )
    {
        LOGERR("pResGenrPhaseProps is null, id: %u", dwPhasePropsID);
        return ERR_NOT_FOUND;
    }

    for( int i = 0; i < (int)pResGenrPhaseProps->m_dwMeterialsNum; i++ )
    {
        DT_ITEM_PROPS* pstProps = Props::Instance().Find(pstData, pResGenrPhaseProps->m_propId[i]);
        if( !pstProps || pstProps->m_dwNum < pResGenrPhaseProps->m_propNum[i] )
        {
            return ERR_NOT_ENOUGH_PROPS;
        }
    }

    // 金币检查
    if( pstData->GetMajestyInfo().m_dwGold < pResGenrPhaseProps->m_dwGoldenNum )
    {
        return ERR_NOT_ENOUGH_GOLD;
    }

    // 完成升阶
    pstGeneral->m_bPhase += 1;
    rstScPkgBodyRsp.m_bDestPhase = pstGeneral->m_bPhase;
    AsyncPvp::Instance().UptToAsyncSvr(pstData);

    // 增加统帅值
    pstData->m_dwLeaderValue += m_dwPhaseLeaderValue;

    // 扣除消耗
    for( int i = 0; i < (int)pResGenrPhaseProps->m_dwMeterialsNum; i++ )
    {
        Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, pResGenrPhaseProps->m_propId[i], \
            -1*(int)pResGenrPhaseProps->m_propNum[i], rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_PHASEUP );
    }

    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_GOLD, 0, -1*(int)pResGenrPhaseProps->m_dwGoldenNum, \
        rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_PHASEUP);
    //  升阶
    Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_GENERAL_LEVEL, 1, 3, pstGeneral->m_bPhase);
	Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_LVUP, 1, 1, 3);
    TriggerUpdateGCardLi(pstData, pstGeneral);
    ZoneLog::Instance().WriteGeneralLog(pstData, pstGeneral->m_dwId, pstData->GetMajestyInfo().m_wLevel, METHOD_GENERAL_OP_PHASEUP, pstGeneral->m_bPhase - 1, pstGeneral->m_bPhase, 0,
										pstGeneral->m_bPhase, pstGeneral->m_wTrainValue, pstGeneral->m_bStar);
    return ERR_NONE;
}

int GeneralCard::PhaseReborn(PlayerData* pstData, uint32_t dwId, DT_SYNC_ITEM_INFO& rstRewardItemInfo, uint32_t* pGold, uint32_t dwRatio)
{
	DT_ITEM_GCARD* pstGeneral = Find(pstData, dwId);
	if (!pstGeneral)
	{
		LOGERR("Player(%lu) LvReborn failed, pstGeneral is null", pstData->m_ullUin);
		return ERR_NOT_FOUND;
	}

	ResGeneralPhaseMgr_t& roResGenrPhaseMgr = CGameDataMgr::Instance().GetResGeneralPhaseMgr();
	RESGENERALPHASE * pResPhase = roResGenrPhaseMgr.Find(pstGeneral->m_dwId);
	if (!pResPhase )
	{
		LOGERR("pResPhase is null");
		return ERR_NOT_FOUND;
	}
	assert(pstGeneral->m_bPhase > 0);

	ResGeneralPhasePropsMgr_t& roResGenrPhasePropsMgr = CGameDataMgr::Instance().GetResGeneralPhasePropsMgr();

	uint32_t dwGold = 0;

	for(int i=pstGeneral->m_bPhase-1; i>0; i--)
	{
		uint32_t dwPhasePropsID = pResPhase->m_phaseUpProps[i-1];
		RESGENERALPHASEPROPS* pResGenrPhaseProps = roResGenrPhasePropsMgr.Find( dwPhasePropsID );
		if( !pResGenrPhaseProps )
		{
			LOGERR("pResGenrPhaseProps is null, id: %u", dwPhasePropsID);
			return ERR_NOT_FOUND;
		}
		dwGold += pResGenrPhaseProps->m_dwGoldenNum;
		for(size_t j=0; j< pResGenrPhaseProps->m_dwMeterialsNum; j++)
		{
			uint32_t dwNum = (pResGenrPhaseProps->m_propNum[j] * dwRatio) / 100;
			Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, pResGenrPhaseProps->m_propId[j], dwNum, rstRewardItemInfo, METHOD_GENERAL_OP_REBORN);
		}
	}

	dwGold = (dwGold * dwRatio) / 100;

	*pGold = dwGold;

	pstGeneral->m_bPhase = 1;

	return ERR_NONE;
}


int GeneralCard::SkillLvUp(PlayerData* pstData, CS_PKG_GCARD_SKILL_LVUP_REQ& rstCsPkgBodyReq, SC_PKG_GCARD_SKILL_LVUP_RSP& rstScPkgBodyRsp)
{
    //是否开启
	int iErrNo = Majesty::Instance().IsArriveLevel(pstData, LEVEL_LIMIT_SKILL_LVUP);
	if (ERR_NONE != iErrNo)
	{
		return iErrNo;
	}

    //获取相应的武将卡
    DT_ITEM_GCARD* pstGeneral = Find(pstData, rstCsPkgBodyReq.m_dwGeneralId);
    if (!pstGeneral)
    {
        LOGERR("pstGeneral(%u) is null", rstCsPkgBodyReq.m_dwGeneralId);
        return ERR_NOT_FOUND;
    }

    //校验是否开启此技能
    if (rstCsPkgBodyReq.m_bSkillId >= pstGeneral->m_bStar)
    {
        return ERR_NOT_SATISFY_COND;
    }

    //校验等级是否超过上限
    uint8_t bDstSkillLevel = pstGeneral->m_szSkillLevel[rstCsPkgBodyReq.m_bSkillId]+ 1;
    if (bDstSkillLevel > MAX_GCRAD_SKILL_LEVEL)
    {
        return ERR_TOP_LEVEL;
    }

    //获取武将升星数据档
    RESGENERALSTAR* pResGeneralStar = CGameDataMgr::Instance().GetResGeneralStarMgr().Find(pstGeneral->m_dwId);
    if (!pResGeneralStar)
    {
        LOGERR("pResGeneralStar is null");
        return ERR_SYS;
    }

    //获取金币消耗数据档
    RESCONSUME* pResGoldConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(pResGeneralStar->m_goldConsume[rstCsPkgBodyReq.m_bSkillId]);
    if (!pResGoldConsume )
    {
        LOGERR("pResGoldConsume is null");
        return ERR_SYS;
    }

    //获取技能点消耗数据档
    RESCONSUME* pResSPConsume = CGameDataMgr::Instance().GetResConsumeMgr().Find(pResGeneralStar->m_skillPointConsume[rstCsPkgBodyReq.m_bSkillId]);
    if (!pResSPConsume )
    {
        LOGERR("pResSPConsume is null");
        return ERR_SYS;
    }

    //校验金钱是否够
    int32_t iGoldConsume = pResGoldConsume->m_lvList[bDstSkillLevel-1];
    if (!Consume::Instance().IsEnoughGold(pstData, iGoldConsume))
    {
         return ERR_NOT_ENOUGH_GOLD;
    }

    //校验技能点是否够
    int32_t iSPConsume = pResSPConsume->m_lvList[bDstSkillLevel-1];
    if (!SkillPoint::Instance().IsEnough(pstData, iSPConsume))
    {
        //当技能点不够时，同步服务器上保存的技能点信息给客户端，防止客户端出现技能点有但是点击提示材料不够的情况
        DT_SYNC_ITEM_INFO& rstSyncItemInfo = rstScPkgBodyRsp.m_stSyncItemInfo;
        DT_ITEM& rstItem = rstSyncItemInfo.m_astSyncItemList[rstSyncItemInfo.m_bSyncItemCount++];
        rstItem.m_bItemType = ITEM_TYPE_SKILL_POINT;
        rstItem.m_dwValueAfterChg = pstData->GetMajestyInfo().m_wSkillPoint;
        DT_ITEM_DATA& rstItemData = rstItem.m_stItemData;
        rstItemData.m_stSkillPoint.m_ullLastResumeTime = pstData->GetMajestyInfo().m_ullSPResumeLastTime;
        return ERR_NOT_ENOUGH_PROPS;
    }

    //扣钱并升级
    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_GOLD, 0, -iGoldConsume, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_SKILL_LVUP);
    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_SKILL_POINT, 0, -iSPConsume, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_SKILL_LVUP);

    pstGeneral->m_szSkillLevel[rstCsPkgBodyReq.m_bSkillId] = bDstSkillLevel;
    rstScPkgBodyRsp.m_dwSkillLevel = bDstSkillLevel;

    //记武将培养日志
    ZoneLog::Instance().WriteGeneralLog(pstData, rstCsPkgBodyReq.m_dwGeneralId, pstData->GetMajestyInfo().m_wLevel, METHOD_GENERAL_OP_SKILL_LVUP, bDstSkillLevel - 1, bDstSkillLevel, 0,
										pstGeneral->m_bPhase, pstGeneral->m_wTrainValue, pstGeneral->m_bStar);
    TriggerUpdateGCardLi(pstData, pstGeneral);
    return ERR_NONE;
}


int GeneralCard::SkillReborn(PlayerData* pstData, uint32_t dwId, DT_SYNC_ITEM_INFO& rstRewardItemInfo, uint32_t* pGold, uint32_t dwRatio)
{
    DT_ITEM_GCARD* pstGeneral = Find(pstData, dwId);
    if (!pstGeneral)
    {
        LOGERR("Player(%lu) SkillReborn failed, pstGeneral is null", pstData->m_ullUin);
        return ERR_NOT_FOUND;
    }

    uint8_t bDstSkillLevel = 0;

    RESGENERAL* pResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(pstGeneral->m_dwId);
    assert(pResGeneral);

    RESGENERALSTAR* pResGeneralStar = CGameDataMgr::Instance().GetResGeneralStarMgr().Find(pstGeneral->m_dwId);
    assert(pResGeneralStar);

    ResConsumeMgr_t& rstResConsumeMgr = CGameDataMgr::Instance().GetResConsumeMgr();

    uint32_t dwGoldConsume = 0;
    for (size_t i = 0; i < MAX_NUM_GENERAL_SKILL; i++)
    {
        bDstSkillLevel = pstGeneral->m_szSkillLevel[i];
        RESCONSUME* pResGoldConsume = rstResConsumeMgr.Find(pResGeneralStar->m_goldConsume[i]);
        for (size_t j = 0; j < bDstSkillLevel; j++)
        {
            dwGoldConsume += pResGoldConsume->m_lvList[j];
        }
        pstGeneral->m_szSkillLevel[i] = 1;
    }
    *pGold = dwGoldConsume * dwRatio / 100;

    return ERR_NONE;
}

int GeneralCard::CheatsLvUp (PlayerData* pstData, uint32_t dwId, uint8_t bSeqId, SC_PKG_CHEATS_LVUP_RSP& rstScPkgBodyRsp)
{
    //获取相应的武将卡
    DT_ITEM_GCARD* pstGeneral = Find(pstData, dwId);
    if (!pstGeneral)
    {
        LOGERR("CheatsLvUp : pstGeneral is null");
        return ERR_NOT_FOUND;
    }
    if ((bSeqId > MAX_SKILL_CHEATS_NUM) || (bSeqId == 0))
    {
        LOGERR("CheatsLvUp : bSeqId=%d", bSeqId);
        return ERR_SYS;
    }

    //获取武将数据档
    ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
    RESGENERAL* pResGeneral = rstResGeneralMgr.Find(pstGeneral->m_dwId);
    if (!pResGeneral)
    {
        LOGERR("pResGeneral is null");
        return ERR_SYS;
    }

    //获取技能数据档
    ResGeneralSkillMgr_t & rstResSkillMgr = CGameDataMgr::Instance().GetResGeneralSkillMgr();
    RESGENERALSKILL * pResSkill = rstResSkillMgr.Find(pResGeneral->m_dwActiveSkillId);
    if (!pResSkill)
    {
        LOGERR("pResSkill is null");
        return ERR_SYS;
    }

    //校验升阶材料
    uint32_t dwConsume = pResSkill->m_gradeUpPropUse[bSeqId -1];
    uint32_t dwPropId = bSeqId -1 == 0 ?pResSkill->m_dwGradeUpPropId1 : pResSkill->m_dwGradeUpPropId2;
    if (!Props::Instance().IsEnough(pstData, dwPropId, dwConsume))
    {
        return ERR_NOT_ENOUGH_PROPS;
    }

    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, dwPropId, -dwConsume, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_CHEATSLV);

    pstGeneral->m_astCheatsList[bSeqId-1].m_bLevel = 1;
    pstGeneral->m_bCheatsType = pstGeneral->m_bCheatsType | bSeqId;
    TriggerUpdateGCardLi(pstData, pstGeneral);
    return ERR_NONE;
}

int GeneralCard::CheatsReborn(PlayerData* pstData, uint32_t dwId, DT_SYNC_ITEM_INFO& rstRewardItemInfo, uint32_t dwRatio)
{
    DT_ITEM_GCARD* pstGeneral = Find(pstData, dwId);
    if (!pstGeneral)
    {
        LOGERR("Player(%lu) CheatReborn failed, pstGeneral is null", pstData->m_ullUin);
        return ERR_NOT_FOUND;
    }

    //获取武将数据档
    RESGENERAL* pResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(pstGeneral->m_dwId);
    assert(pResGeneral);

    //获取技能数据档
    RESGENERALSKILL * pResSkill = CGameDataMgr::Instance().GetResGeneralSkillMgr().Find(pResGeneral->m_dwActiveSkillId);
    assert(pResSkill);

    for (int i=0; i<MAX_SKILL_CHEATS_NUM; i++)
    {
       uint32_t propId = i == 0 ? pResSkill->m_dwGradeUpPropId1:pResSkill->m_dwGradeUpPropId2;
        if ( pstGeneral->m_astCheatsList[i].m_bLevel > 0)
        {
            uint32_t dwNum = pResSkill->m_gradeUpPropUse[i];
            dwNum = (dwNum * dwRatio + 50) /100;
            Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, propId, dwNum, rstRewardItemInfo, METHOD_GENERAL_OP_REBORN);
            pstGeneral->m_astCheatsList[i].m_bLevel = 0;
        }
    }

    pstGeneral->m_bCheatsType = 0;
    return ERR_NONE;
}

int GeneralCard::CheatsChg (PlayerData* pstData, uint32_t dwId, uint8_t bSeqId)
{
    //获取相应的武将卡
    DT_ITEM_GCARD* pstGeneral = Find(pstData, dwId);
    if (!pstGeneral)
    {
        LOGERR("CheatsLvUp : pstGeneral is null");
        return ERR_NOT_FOUND;
    }

    if (bSeqId > MAX_SKILL_CHEATS_NUM)
    {
        LOGERR("CheatsLvUp : dwCheatsId=%d", bSeqId);
        return ERR_SYS;
    }

    if ((bSeqId != 0) && (pstGeneral->m_astCheatsList[bSeqId-1].m_bLevel == 0))
    {
        return ERR_SYS;
    }

    pstGeneral->m_bCheatsType = pstGeneral->m_bCheatsType | bSeqId;

    return ERR_NONE;
}


// 更改武将技能标志
int GeneralCard::ChgSkillFlag(PlayerData * pstData, uint32_t dwId, uint32_t dwSkillFlag, uint32_t dwAIFlag)
{
	DT_ITEM_GCARD* pstGeneral = Find(pstData, dwId);
	if (!pstGeneral)
	{
		LOGERR("ChgSkillFlag : pstGeneral is null");
		return ERR_NOT_FOUND;
	}

	pstGeneral->m_dwSkillFlagList = dwSkillFlag;
	pstGeneral->m_dwAIFlagList = dwAIFlag;

	return ERR_NONE;
}


bool GeneralCard::GetModuleState( DT_ITEM_GCARD* pstGCardInfo, uint64_t ullModuleBitFlat)
{

    return (pstGCardInfo->m_ullModuleFlag & ullModuleBitFlat) == ullModuleBitFlat;
}

void GeneralCard::SetModuleState(DT_ITEM_GCARD* pstGCardInfo, uint64_t ullModuleBitFlat, bool bState)
{

    if (bState)
    {
        pstGCardInfo->m_ullModuleFlag |= ullModuleBitFlat;    //置1
    }
    else
    {
        pstGCardInfo->m_ullModuleFlag &= ~ullModuleBitFlat;   //置0
    }
}

void GeneralCard::ClearMineAp(PlayerData* pstData)
{
	DT_ROLE_GCARD_INFO& rstGCardInfo = pstData->GetGCardInfo();
	for (int i = 0; i< rstGCardInfo.m_iCount; i++)
	{
		rstGCardInfo.m_astData[i].m_bMineAP = 0;
	}

}

//兵种升级，返回当前等级
int GeneralCard::ArmyLvUp(PlayerData* pstData, uint32_t dwGeneralId, SC_PKG_GCARD_ARMY_LVUP_RSP& rstSCPkgBodyRsp)
{
	int iErrNo = Majesty::Instance().IsArriveLevel(pstData, LEVEL_LIMIT_AREM_LVUP);

	if (ERR_NONE != iErrNo)
	{
		return iErrNo;
	}

    //获取相应的武将卡
    DT_ITEM_GCARD* pstGeneral = Find(pstData, dwGeneralId);
    if (!pstGeneral)
    {
        LOGERR("ArmyLvUp : pstGeneral is null");
        return ERR_NOT_FOUND;
    }
    uint8_t& bArmyLv = pstGeneral->m_bArmyLv;

    //获取武将数据档
    ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
    RESGENERAL* pResGeneral = rstResGeneralMgr.Find(pstGeneral->m_dwId);
    if (!pResGeneral)
    {
        LOGERR("pResGeneral is null");
        return ERR_SYS;
    }
    ResBasicMgr_t& rstResBasiclMgr = CGameDataMgr::Instance().GetResBasicMgr();
    RESBASIC* pResBasic = rstResBasiclMgr.Find(ARMY_UP_PHASE_LVLIMIT);
    if (!pResBasic)
    {
        LOGERR("ArmyLvUp fail, pResBasic is null");
        return ERR_SYS;
    }
    uint8_t bCurMaxlv = (uint8_t)pResBasic->m_para[pstGeneral->m_bArmyPhase - 1];    //当前品阶能达到的最大等级

    if (bArmyLv >= bCurMaxlv)
    {
        LOGERR("ArmyLvUp fail, CurPhase max lv");
        return ERR_GCARD_ARMY_CUR_PHASE_MAX_LV;
    }
    //道具消耗检测
    ResConsumeMgr_t& rstResConsumeMgr = CGameDataMgr::Instance().GetResConsumeMgr();
    RESCONSUME* pResConsumeProp = NULL ;
    int iPropTypeNum = sizeof(pResGeneral->m_armyUpMaterialIDs) / sizeof(pResGeneral->m_armyUpMaterialIDs[0]);
    uint32_t dwConsumePropNum = 0;
    for (int i = 0; i< iPropTypeNum; i++)
    {
        pResConsumeProp = rstResConsumeMgr.Find(pResGeneral->m_armyUpMaterialNums[i]);
        if (NULL == pResConsumeProp)
        {
            LOGERR("ArmyLvUP failed, pResConsume is NULL");
            return ERR_SYS;
        }
        dwConsumePropNum = pResConsumeProp->m_lvList[bArmyLv];
        if (0 == dwConsumePropNum)
        {//配置有可能是0,跳过
            continue;
        }

        if (!Props::Instance().IsEnough(pstData, pResGeneral->m_armyUpMaterialIDs[i], dwConsumePropNum))
        {//这种情况是前台没有限制有问题
            LOGERR("ArmyLvUp : ERR_NOT_ENOUGH_PROPS");
            return ERR_NOT_ENOUGH_PROPS;
        }
    }
   //金币消耗检测
    RESCONSUME* pResConsumeGold = rstResConsumeMgr.Find(pResGeneral->m_dwArmyUpGold);
    if (!pResConsumeGold)
    {
        LOGERR("ArmyLvUp: pResConsumeProp or pResConsumeProp is null");
        return ERR_SYS;
    }
    uint32_t dwConsumeGoldNum = pResConsumeGold->m_lvList[bArmyLv];
    if (!Consume::Instance().IsEnoughGold(pstData, dwConsumeGoldNum))
    {
        return ERR_NOT_ENOUGH_GOLD;
    }

    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_GOLD, 0, -dwConsumeGoldNum, rstSCPkgBodyRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_ARMY_LVUP);
    for (int i = 0; i < iPropTypeNum; i++)
    {
        pResConsumeProp = rstResConsumeMgr.Find(pResGeneral->m_armyUpMaterialNums[i]);
        if (NULL == pResConsumeProp)
        {
            LOGERR("ArmyLvUP failed, pResConsume is NULL");
            return ERR_SYS;
        }
        dwConsumePropNum = pResConsumeProp->m_lvList[bArmyLv];
        if (0 == dwConsumePropNum)
        {//配置有可能是0,跳过
            continue;
        }

        Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, pResGeneral->m_armyUpMaterialIDs[i], -dwConsumePropNum, rstSCPkgBodyRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_ARMY_LVUP);
    }

    rstSCPkgBodyRsp.m_bCurLv = ++bArmyLv;

    //记武将培养日志
    ZoneLog::Instance().WriteGeneralLog(pstData, dwGeneralId, pstGeneral->m_bLevel, METHOD_GENERAL_OP_ARMY_LVUP, bArmyLv - 1, bArmyLv, 0,
										pstGeneral->m_bPhase, pstGeneral->m_wTrainValue, pstGeneral->m_bStar);
    TriggerUpdateGCardLi(pstData, pstGeneral);
    return (int)bArmyLv;
}

int GeneralCard::ArmyLvUpTotal(PlayerData* pstData, uint32_t dwGeneralId,  SC_PKG_GCARD_ARMY_LVUP_RSP& rstSCPkgBodyRsp)
{
	int iErrNo = Majesty::Instance().IsArriveLevel(pstData, LEVEL_LIMIT_AREM_LVUP);

	if (ERR_NONE != iErrNo)
	{
		return iErrNo;
	}

    //获取相应的武将卡
    DT_ITEM_GCARD* pstGeneral = Find(pstData, dwGeneralId);
    if (!pstGeneral)
    {
        LOGERR("ArmyLvUp : pstGeneral is null");
        return ERR_NOT_FOUND;
    }

    uint8_t bArmyLv = pstGeneral->m_bArmyLv;

    //获取武将数据档
    ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
    RESGENERAL* pResGeneral = rstResGeneralMgr.Find(pstGeneral->m_dwId);

    if (!pResGeneral)
    {
        LOGERR("pResGeneral is null");
        return ERR_SYS;
    }

    ResBasicMgr_t& rstResBasiclMgr = CGameDataMgr::Instance().GetResBasicMgr();
    RESBASIC* pResBasic = rstResBasiclMgr.Find(ARMY_UP_PHASE_LVLIMIT);

    if (!pResBasic)
    {
        LOGERR("ArmyLvUp fail, pResBasic is null");
        return ERR_SYS;
    }

    uint8_t bCurMaxlv = (uint8_t)pResBasic->m_para[pstGeneral->m_bArmyPhase - 1];    //当前品阶能达到的最大等级

    int iRet = ERR_NONE;
    uint32_t dwGoldCost = 0;
    int iPropTypeNum = sizeof(pResGeneral->m_armyUpMaterialIDs) / sizeof(pResGeneral->m_armyUpMaterialIDs[0]);
    uint32_t dwPropCostNum[iPropTypeNum + 1];

    for (int i = 0; i < iPropTypeNum; i++)
    {
        dwPropCostNum[i] = 0;
    }

    //道具消耗检测
    ResConsumeMgr_t& rstResConsumeMgr = CGameDataMgr::Instance().GetResConsumeMgr();
    RESCONSUME* pResConsumeProp = NULL ;
    RESCONSUME* pResConsumeGold = NULL ;

    while (true)
    {
        if (bArmyLv >= bCurMaxlv)
        {
            LOGERR("ArmyLvUp fail, CurPhase max lv");
            iRet = ERR_GCARD_ARMY_CUR_PHASE_MAX_LV;
            break;
        }
        if (bArmyLv >= pstGeneral->m_bLevel)
        {
            LOGERR_r("Can not excell majesty level");
            iRet = ERR_TOP_LEVEL;
            break;
        }
        uint32_t dwConsumePropNum = 0;
        uint8_t bEnoughProps = 1;
        for (int i = 0; i< iPropTypeNum; i++)
        {
            pResConsumeProp = rstResConsumeMgr.Find(pResGeneral->m_armyUpMaterialNums[i]);
            if (NULL == pResConsumeProp)
            {
                LOGERR("ArmyLvUP failed, pResConsume is NULL");
                return ERR_SYS;
            }
            dwConsumePropNum = pResConsumeProp->m_lvList[bArmyLv];
            if (0 == dwConsumePropNum)
            {
                //配置有可能是0,跳过
                continue;
            }
            if (!Props::Instance().IsEnough(pstData, pResGeneral->m_armyUpMaterialIDs[i], dwConsumePropNum + dwPropCostNum[i]))
            {
                //这种情况是前台没有限制有问题
                LOGERR("ArmyLvUp : ERR_NOT_ENOUGH_PROPS");
                iRet = ERR_NOT_ENOUGH_PROPS;
                bEnoughProps = 0;
                break;
            }
        }
        if (!bEnoughProps)
        {
            break;
        }

        //金币消耗检测
        pResConsumeGold = rstResConsumeMgr.Find(pResGeneral->m_dwArmyUpGold);
        if (!pResConsumeGold)
        {
            LOGERR("ArmyLvUp: pResConsumeGold is null");
            return ERR_SYS;
        }
        uint32_t dwConsumeGoldNum = pResConsumeGold->m_lvList[bArmyLv];
        if (!Consume::Instance().IsEnoughGold(pstData, dwGoldCost + dwConsumeGoldNum))
        {
            iRet = ERR_NOT_ENOUGH_GOLD;
            break;
        }

        //消耗
        //Item::Instance().AddSyncItem(pstData, ITEM_TYPE_GOLD, 0, -dwConsumeGoldNum, rstSCPkgBodyRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_ARMY_LVUP);
        for (int i = 0; i < iPropTypeNum; i++)
        {
            pResConsumeProp = rstResConsumeMgr.Find(pResGeneral->m_armyUpMaterialNums[i]);
            if (NULL == pResConsumeProp)
            {
                LOGERR("ArmyLvUP failed, pResConsume is NULL");
                return ERR_SYS;
            }
            dwConsumePropNum = pResConsumeProp->m_lvList[bArmyLv];
            if (0 == dwConsumePropNum)
            {
                //配置有可能是0,跳过
                continue;
            }
            dwPropCostNum[i] += dwConsumePropNum;
        }

        dwGoldCost += dwConsumeGoldNum;
        bArmyLv++;
    }

    //最后进行校验
    rstSCPkgBodyRsp.m_bCurLv = bArmyLv;
    if (bArmyLv == pstGeneral->m_bArmyLv)
    {
        return iRet;
    }
    else
    {
        pstGeneral->m_bArmyLv = bArmyLv;
        //消耗
        Item::Instance().ConsumeItem(pstData, ITEM_TYPE_GOLD, 0, -dwGoldCost, rstSCPkgBodyRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_ARMY_LVUP);
        for (int i = 0; i < iPropTypeNum; i++)
        {
            pResConsumeProp = rstResConsumeMgr.Find(pResGeneral->m_armyUpMaterialNums[i]);
            if (NULL == pResConsumeProp)
            {
                LOGERR("ArmyLvUP failed, pResConsumeProp is NULL");
                return ERR_SYS;
            }

            Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, pResGeneral->m_armyUpMaterialIDs[i], -dwPropCostNum[i], rstSCPkgBodyRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_ARMY_LVUP);
        }
    }

    //记武将培养日志
    //ZoneLog::Instance().WriteGeneralLog(pstData, dwGeneralId, pstGeneral->m_bLevel, METHOD_GENERAL_OP_ARMY_LVUP, bArmyLv - 1, bArmyLv, 0);
    TriggerUpdateGCardLi(pstData, pstGeneral);
    return (int)bArmyLv;
}

int GeneralCard::ArmyLvReborn(PlayerData* pstData, uint32_t dwId, DT_SYNC_ITEM_INFO& rstRewardItemInfo, uint32_t* pGold, uint32_t dwRatio)
{
    DT_ITEM_GCARD* pstGeneral = Find(pstData, dwId);
    if (!pstGeneral)
    {
        LOGERR("Player(%lu) ArmyLvReborn failed, pstGeneral is null", pstData->m_ullUin);
        return ERR_NOT_FOUND;
    }

    RESGENERAL* pResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(pstGeneral->m_dwId);
    assert(pResGeneral);

    //金币消耗数据档
    ResConsumeMgr_t& rstResConsumeMgr = CGameDataMgr::Instance().GetResConsumeMgr();
    RESCONSUME* pResGoldConsume = rstResConsumeMgr.Find(pResGeneral->m_dwArmyUpGold);
    assert(pResGoldConsume);

    //返还金币
    uint32_t dwGold = 0;
    for (uint32_t j=pstGeneral->m_bArmyLv; j>1; j--)
    {
       dwGold += pResGoldConsume->m_lvList[j -1];
    }
    dwGold = (dwGold * dwRatio) / 100;
    *pGold = dwGold;

    //返还道具
    for (uint32_t i=0; i<2; i++)
    {
        //道具消耗数据档
        RESCONSUME* pResPropsConsume = rstResConsumeMgr.Find(pResGeneral->m_armyUpMaterialNums[i]);
        assert(pResPropsConsume);

        //计算数量
        uint32_t dwNum = 0;
        for (uint32_t j=pstGeneral->m_bArmyLv; j>1; j--)
        {
            dwNum += pResPropsConsume->m_lvList[j -1];
        }
        dwNum = (dwNum * dwRatio + 50) /100;

        Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, pResGeneral->m_armyUpMaterialIDs[i], dwNum, rstRewardItemInfo, METHOD_GENERAL_OP_REBORN);
    }

    pstGeneral->m_bArmyLv = 1;

    return ERR_NONE;
}

//兵种升阶
int GeneralCard::ArmyPhaseUp(PlayerData* pstData, uint32_t dwGeneralId, SC_PKG_GCARD_ARMY_PHASEUP_RSP& rstSCPkgBodyRsp)
{
    //获取相应的武将卡
    DT_ITEM_GCARD* pstGeneral = Find(pstData, dwGeneralId);
    if (!pstGeneral)
    {
        LOGERR("ArmyPhaseUp : pstGeneral is null");
        return ERR_NOT_FOUND;
    }
    //获取武将数据档
    ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
    RESGENERAL* pResGeneral = rstResGeneralMgr.Find(pstGeneral->m_dwId);
    if (!pResGeneral)
    {
        LOGERR("pResGeneral is null");
        return ERR_SYS;
    }

    ResArmyMgr_t& rstResArmylMgr = CGameDataMgr::Instance().GetResArmyMgr();    //兵种配置当
    RESARMY* pResArmy = rstResArmylMgr.Find(pstGeneral->m_bArmyPhase * 100 + pResGeneral->m_bArmyType);
    if (!pResArmy)
    {
        LOGERR("ArmyPhaseUp fail, pResArmy is null");
        return ERR_NOT_FOUND;
    }
    ResBasicMgr_t& rstResBasiclMgr = CGameDataMgr::Instance().GetResBasicMgr();     //基础配置档 包含进阶等级限制条件
    RESBASIC* pResBasic = rstResBasiclMgr.Find(ARMY_UP_PHASE_LVLIMIT);
    if (!pResBasic)
    {
        LOGERR("ArmyPhaseUp fail, pResBasic is null");
        return ERR_SYS;
    }
    if (pstGeneral->m_bArmyLv != pResBasic->m_para[pstGeneral->m_bArmyPhase-1])         //进阶等级限制
    {
        LOGERR("ArmyPhaseUp, lv not enough");
        return ERR_GCARD_ARMY_LV_LIMIT;
    }
    if (0 == pResArmy->m_dwNextPhaseID)
    {
        return ERR_GCARD_ARMY_PHASE_MAX;
    }
    //检查消耗
    int iIndex = pstGeneral->m_bArmyPhase - 1;
    if (!Props::Instance().IsEnough(pstData, pResGeneral->m_armyGradeMaterialIDs[iIndex], pResGeneral->m_armyGradeNums[iIndex]))
    {//这种情况是前台没有限制
        LOGERR("ArmyPhaseUp : ERR_NOT_ENOUGH_PROPS");
        return ERR_NOT_ENOUGH_PROPS;
    }
    //扣除消耗
    rstSCPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;
    Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, pResGeneral->m_armyGradeMaterialIDs[iIndex], -pResGeneral->m_armyGradeNums[iIndex], rstSCPkgBodyRsp.m_stSyncItemInfo, METHOD_GENERAL_OP_EQUIP_PHASEUP);
    //升阶处理
    pstGeneral->m_bArmyPhase++;
    rstSCPkgBodyRsp.m_bCurPhase = pstGeneral->m_bArmyPhase;
    rstSCPkgBodyRsp.m_dwCurArmyId = pstGeneral->m_bArmyPhase * 100 + pResGeneral->m_bArmyType;
    TriggerUpdateGCardLi(pstData, pstGeneral);
    return ERR_NONE;
}

int GeneralCard::ArmyPhaseReborn(PlayerData* pstData, uint32_t dwId, DT_SYNC_ITEM_INFO& rstRewardItemInfo, uint32_t dwRatio)
{
    DT_ITEM_GCARD* pstGeneral = Find(pstData, dwId);
    if (!pstGeneral)
    {
        LOGERR("Player(%lu) ArmyLvReborn failed, pstGeneral is null", pstData->m_ullUin);
        return ERR_NOT_FOUND;
    }

    RESGENERAL* pResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(pstGeneral->m_dwId);
    assert(pResGeneral);

    //金币消耗数据档
    for (int i=pstGeneral->m_bArmyPhase-1; i>=1; i--)
    {
        uint32_t dwNum = pResGeneral->m_armyGradeNums[i-1];
        dwNum = (dwNum * dwRatio + 50) / 100;
        Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, pResGeneral->m_armyGradeMaterialIDs[i-1], dwNum, rstRewardItemInfo, METHOD_GENERAL_OP_REBORN);
    }
    pstGeneral->m_bArmyPhase = 1;

    return ERR_NONE;
}

//初始化武将缘分的BUFFID,只在匹配时用,不存数据库
int GeneralCard::InitFateBuffIds( PlayerData* pstData, DT_ITEM_GCARD* pstGeneral)
{

    if (!pstGeneral)
    {
        LOGERR("Uin<%lu> pstGeneral is null", pstData->m_ullUin);
        return ERR_SYS;
    }
    //获取武将数据档
    RESGENERAL* pResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(pstGeneral->m_dwId);
    if (!pResGeneral)
    {
        LOGERR("Uin<%lu> pResGeneral is null", pstData->m_ullUin);
        return ERR_SYS;
    }
    uint32_t dwFateId = 0;
    for (int i = 0; i < RES_MAX_GCARD_FATEBUFFIDS_NUM; i++)
    {
        dwFateId = pResGeneral->m_fateId[i];
        if ( 0 == dwFateId || IsGCardFateOpen(pstData, dwFateId) )
        {
            pstGeneral->m_FateBuffIds[i] = 0;	//置0
            continue;
        }
        pstGeneral->m_FateBuffIds[i] = dwFateId;
    }

    return ERR_NONE;

}


int GeneralCard::InitMSkillAttrs(PlayerData* pstData, DT_ITEM_GCARD* pstGCardInfo, DT_TROOP_INFO &rstTroopInfo)
{

	if (!pstData->m_bIsMSkillInit)
	{
		//军师技属性未初始化
		CalMSkillAttr(pstData);
	}

	for (int i=0;i<MAX_ATTR_ADD_NUM;i++)
	{
		rstTroopInfo.m_AttrAddValue[i] += pstData->m_afMSkillAttr[i];
	}
	return ERR_SYS;
}


int GeneralCard::InitFeedTrainAttrs(PlayerData* pstData, DT_ITEM_GCARD* pstGCardInfo, DT_TROOP_INFO &rstTroopInfo)
{
	RESGENERAL* pResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(pstGCardInfo->m_dwId);
	if (!pResGeneral)
	{
		LOGERR("Uin<%lu> pResGeneral<%u> is null", pstData->m_ullUin, pstGCardInfo->m_dwId);
		return ERR_SYS;
	}

	if (!pstData->m_bIsFeedTrainInit)
	{
		this->CalFeedTrainAttr(pstData);
	}


	//亲密度提供的阵营buf（加属性）
	switch (pResGeneral->m_bCountryId)
	{
	case COUNTRY_WEI:
		for (int i=0;i<MAX_ATTR_ADD_NUM;i++)
		{
			rstTroopInfo.m_AttrAddValue[i] += pstData->m_afWeiFeedTrainAttr[i];
		}
		break;
	case COUNTRY_SHU:
		for (int i=0;i<MAX_ATTR_ADD_NUM;i++)
		{
			rstTroopInfo.m_AttrAddValue[i] += pstData->m_afShuFeedTrainAttr[i];
		}
		break;
	case COUNTRY_WU:
		for (int i=0;i<MAX_ATTR_ADD_NUM;i++)
		{
			rstTroopInfo.m_AttrAddValue[i] += pstData->m_afWuFeedTrainAttr[i];
		}
		break;
	case COUNTRY_MISC:
		for (int i=0;i<MAX_ATTR_ADD_NUM;i++)
		{
			rstTroopInfo.m_AttrAddValue[i] += pstData->m_afOtherFeedTrainAttr[i];
		}
		break;
	default:
		break;
	}

	//亲密度提供的全体buf（加属性）
	for (int i=0;i<MAX_ATTR_ADD_NUM;i++)
	{
		rstTroopInfo.m_AttrAddValue[i] += pstData->m_afAllFeedTrainAttr[i];
	}



	ResFameHallGeneralMgr_t& rFameHallGeneralMgr = CGameDataMgr::Instance().GetResFameHallGeneralMgr();
	ResFameHallIncreaseAttrMgr_t& rFameHallIncreaseAttrMgr = CGameDataMgr::Instance().GetResFameHallIncreaseAttrMgr();

	RESFAMEHALLGENERAL* pResFameHallGeneral = rFameHallGeneralMgr.Find(pstGCardInfo->m_dwId);
	if (!pResFameHallGeneral)
	{
		LOGERR("pResFameHallGeneral is null");
		return -1;
	}

	uint16_t wIndex = pResFameHallGeneral->m_dwAttrIncrStartId + pstGCardInfo->m_wTrainValue - 1;

	RESFAMEHALLINCREASEATTR* pResFameHallIncreaseAttr = rFameHallIncreaseAttrMgr.Find(wIndex);
	if (!pResFameHallIncreaseAttr)
	{
		LOGERR("Uin<%lu> pResFameHallIncreaseAttr<Index=%hu> is null:GCardId<%u> StartId<%u>, TrainValue<%hu>",
			pstData->m_ullUin, wIndex, pstGCardInfo->m_dwId, pResFameHallGeneral->m_dwAttrIncrStartId, pstGCardInfo->m_wTrainValue);

		return -1;
	}


	for (int i = 0; i < MAX_ATTR_ADD_NUM; i++)
	{
		rstTroopInfo.m_AttrAddValue[i] += pResFameHallIncreaseAttr->m_selfAttrAdd[i];
	}
	return 0;
}

uint32_t GeneralCard::GetGCardLi(PlayerData* pstData, uint32_t dwId,  bool bIsRecount)
{
    //获取相应的武将卡
    DT_ITEM_GCARD* pstGeneral = Find(pstData, dwId);
    if (pstGeneral == NULL)
    {
        LOGERR("Uin<%lu> cant find the GCard<%u>", pstData->m_ullUin, dwId);
        return 0;
    }
    //bIsRecount = true;
    if (!bIsRecount && pstGeneral->m_dwLi != 0)
    {
        //不要重新计算战力
        return pstGeneral->m_dwLi;
    }
    return CalGcardLi(pstData, pstGeneral);
}

uint32_t GeneralCard::GetTeamLi(PlayerData* pstData, uint8_t bBattleArrayType)
{
    DT_ROLE_MAJESTY_INFO& rstMajestyInfo = pstData->GetMajestyInfo();
	uint32_t dwTotalLi = 0;
	DT_BATTLE_ARRAY_INFO* pstBattleArrayInfo = Majesty::Instance().GetBattleArrayInfo(pstData, bBattleArrayType);
    if (!pstBattleArrayInfo)
    {
        LOGERR("Uin<%lu> get the pstBattleArrayInfo<%hhu> error!", pstData->m_ullUin, bBattleArrayType);
        return 0;
    }
    for (int i = 0; i < pstBattleArrayInfo->m_bGeneralCnt; i++)
    {
        if (0 == pstBattleArrayInfo->m_GeneralList[i])
        {
            continue;
        }

        dwTotalLi += GetGCardLi(pstData, pstBattleArrayInfo->m_GeneralList[i]);
    }

    if (rstMajestyInfo.m_dwHighestLi < dwTotalLi)
    {
        //  历史最高战力
        rstMajestyInfo.m_dwHighestLi = dwTotalLi;
    }
    if (bBattleArrayType == BATTLE_ARRAY_TYPE_NORMAL)
    {
        //  默认队伍才保存
        rstMajestyInfo.m_dwCurrentLi = dwTotalLi;
    }
    LOGRUN("Uin<%lu> TeamTotalLi=<%u>", pstData->m_ullUin, dwTotalLi);
    return dwTotalLi;
}

void GeneralCard::TriggerUpdateGCardLi(PlayerData * pstData, DT_ITEM_GCARD * pstGeneral, bool bIsAdd)
{
    uint32_t dwOldLi = pstGeneral->m_dwLi;
    if (dwOldLi != CalGcardLi(pstData, pstGeneral))
    {
        //有变化,更新武将战力排行榜
        RankMgr::Instance().UpdateGCardLi(pstData, pstGeneral, bIsAdd);
        dwOldLi = pstData->GetMajestyInfo().m_dwCurrentLi;
        if (dwOldLi != GetTeamLi(pstData))
        {
            //有变化,更新玩家战力排行榜
            RankMgr::Instance().UpdateLi(pstData);
        }
    }


}


//计算属性战力
void  GeneralCard::_GetGCardAttr(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral, OUT LiObj& rRtData)
{



    rRtData.m_dwId = pstGeneral->m_dwId;
    rRtData.m_ullUin = pstData->m_ullUin;

    //  顺序有讲究,不要随意变化

    // 武将等级加成
    this->_AddGeneralLevel(pstGeneral, rRtData);

    // 装备加成
    this->_AddEquipAttr(pstData, pstGeneral, rRtData);

    // 武将升阶加成
    this->_AddGeneralPhase(pstGeneral, rRtData);

    // 缘分加成
    LiObj stTempRtData = { 0 };
    this->_AddGeneralFateAttr(pstData, pstGeneral, rRtData, stTempRtData);

	// 装备缘分加成
	LiObj stTempEquipRtData = { 0 };
	this->_AddEquipFateAttr(pstData, pstGeneral, rRtData, stTempEquipRtData);



    // 武将星级加成
    this->_AddGeneralStar(pstGeneral, rRtData);

    rRtData.AddAttr(stTempRtData.m_afAttr);
    rRtData.AddAttr(stTempEquipRtData.m_afAttr);

    //  计算宝石加成
    this->_AddGeneralGem(pstGeneral, rRtData);

	// 亲密度加成
	this->_AddFeedTrainAtrr(pstData, pstGeneral, rRtData);
}


void GeneralCard::_AddValue(float& rOriginalData, uint8_t bType,  float fIncValue, int iPer)
{
    float fChgValue = 0;
    //iPer = 1000; //pve计算战力,不衰减
    if (bType == VALUE_INC_TYPE_PERMILLAGE)
    {
        fChgValue = rOriginalData * fIncValue * CONST_RATIO_PERMILLAGE;;
    }
    else if (bType == VALUE_INC_TYPE_DIRECT_ADD)
    {
        fChgValue = fIncValue;
    }
    else
    {
        LOGERR("AddValue failed, Type(%d) is invalid", bType);
    }

    //LOGRUN(" _AddValue, OriginalData(%f), Type(%d), IncValue(%f), Per(%d), ChgVal(%f)", rOriginalData, bType, fIncValue, iPer, fChgValue);

    rOriginalData += fChgValue;
}

void GeneralCard::_AddGeneralLevel(DT_ITEM_GCARD* pstGeneral, LiObj& rRtData)
{

    RESGENERAL* poResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(pstGeneral->m_dwId);
    if (!poResGeneral)
    {
        LOGERR("Uin<%lu> GCard<%u> cant fand the poResGeneral", rRtData.m_ullUin, rRtData.m_dwId);
        return ;
    }
    RESARMY* poResArmy = CGameDataMgr::Instance().GetResArmyMgr().Find(pstGeneral->m_bArmyPhase * 100 + poResGeneral->m_bArmyType);
    if (poResArmy == NULL)
    {
        LOGERR("Uin<%lu> cant find the ArmyID<%u>", rRtData.m_ullUin, pstGeneral->m_bArmyPhase * 100 + poResGeneral->m_bArmyType);
        return ;
    }
    // 基础属性
    rRtData.m_afAttr[ATTR_HP] = (float)poResGeneral->m_dwInitHP;
    rRtData.m_afAttr[ATTR_STR] = (float)poResGeneral->m_dwInitStr;
    rRtData.m_afAttr[ATTR_WIT] = (float)poResGeneral->m_dwInitWit;
    rRtData.m_afAttr[ATTR_STRDEF] = (float)poResGeneral->m_dwInitStrDef;
    rRtData.m_afAttr[ATTR_WITDEF] = (float)poResGeneral->m_dwInitWitDef;
    rRtData.m_afAttr[ATTR_SPEED] = poResArmy->m_fMoveSpeed*1000;
    rRtData.m_afAttr[ATTR_CHANCE_HIT] = poResGeneral->m_fInitChanceHit;
    rRtData.m_afAttr[ATTR_CHANCE_DODGE] = poResGeneral->m_fInitChanceDodge;
    rRtData.m_afAttr[ATTR_CHANCE_CRITICAL] = poResGeneral->m_fInitChanceCritical;
    rRtData.m_afAttr[ATTR_CHANCE_ANTICRITICAL] = poResGeneral->m_fInitChanceAntiCritical;
    rRtData.m_afAttr[ATTR_CHANCE_BLOCK] = poResGeneral->m_fInitChanceBlock;
    rRtData.m_afAttr[ATTR_CHANCE_CRITICAL_VALUE] = poResGeneral->m_fInitParaCritical;
    rRtData.m_afAttr[ATTR_CHANCE_ANTICRITICAL_VALUE] = poResGeneral->m_fInitParaAntiCritical;
    rRtData.m_afAttr[ATTR_CHANCE_BLOCK_VALUE] = poResGeneral->m_fInitParaBlock;
    rRtData.m_afAttr[ATTR_DAMAGEADD] = poResGeneral->m_fInitParaDamageAdd;
    rRtData.m_afAttr[ATTR_SUCKBLOOD] = poResGeneral->m_fInitParaSuckBlood;
    rRtData.m_afAttr[ATTR_BASE_NORMALATK] = (float)poResGeneral->m_dwBaseDamageAtkNormal;



    uint8_t m_bGeneralLevel = pstGeneral->m_bLevel;
    if (m_bGeneralLevel > RES_MAX_GENERAL_LV_LEN || m_bGeneralLevel == 0)
    {
        LOGERR("Uin<%lu> General Level(%d) is invalid", rRtData.m_ullUin, m_bGeneralLevel);
        return;
    }
    RESGENERALLEVEL* m_poResGeneralLevel = CGameDataMgr::Instance().GetResGeneralLevelMgr().Find(rRtData.m_dwId);
    if (!m_poResGeneralLevel)
    {
        LOGERR("Uin<%lu> m_poResGeneralLevel <%u> not find ", rRtData.m_ullUin, rRtData.m_dwId);
        return;
    }

    ResGeneralLevelGrowMgr_t& rstResGeneralLvGrowMgr = CGameDataMgr::Instance().GetResGeneralLevelGrowMgr();

    //Hp
    RESGENERALLEVELGROW * poResGrow = rstResGeneralLvGrowMgr.Find(m_poResGeneralLevel->m_dwHpGrowId);
    if (!poResGrow)
    {
        LOGERR("Uin<%lu> poResGrow(%u) is not found", rRtData.m_ullUin, m_poResGeneralLevel->m_dwHpGrowId);
        return;
    }
    _AddValue(rRtData.m_afAttr[ATTR_HP], m_poResGeneralLevel->m_dwHpGrowType,
        poResGrow->m_growLvList[m_bGeneralLevel-1], m_poResGeneralLevel->m_dwHpGrowPVPRatio);

    //Str
    poResGrow = rstResGeneralLvGrowMgr.Find(m_poResGeneralLevel->m_dwAtkGrowId);
    if (!poResGrow)
    {
        LOGERR("Uin<%lu> poResGrow(%u) is not found", rRtData.m_ullUin, m_poResGeneralLevel->m_dwAtkGrowId);
        return;
    }
    _AddValue(rRtData.m_afAttr[ATTR_STR], m_poResGeneralLevel->m_dwAtkGrowType,
        poResGrow->m_growLvList[m_bGeneralLevel-1], m_poResGeneralLevel->m_dwAtkGrowPVPRatio);

    //Wit
    poResGrow = rstResGeneralLvGrowMgr.Find(m_poResGeneralLevel->m_dwWitGrowId);
    if (!poResGrow)
    {
        LOGERR("Uin<%lu> poResGrow(%u) is not found", rRtData.m_ullUin, m_poResGeneralLevel->m_dwWitGrowId);
        return;
    }
    _AddValue(rRtData.m_afAttr[ATTR_WIT], m_poResGeneralLevel->m_dwWitGrowType,
        poResGrow->m_growLvList[m_bGeneralLevel-1], m_poResGeneralLevel->m_dwWitGrowPVPRatio);

    //StrDef
    poResGrow = rstResGeneralLvGrowMgr.Find(m_poResGeneralLevel->m_dwAtkDefGrowId);
    if (!poResGrow)
    {
        LOGERR("Uin<%lu> poResGrow(%u) is not found", rRtData.m_ullUin, m_poResGeneralLevel->m_dwAtkDefGrowId);
        return;
    }
    _AddValue(rRtData.m_afAttr[ATTR_STRDEF], m_poResGeneralLevel->m_dwAtkDefGrowType,
        poResGrow->m_growLvList[m_bGeneralLevel-1], m_poResGeneralLevel->m_dwAtkDefGrowPVPRatio);

    //WitDef
    poResGrow = rstResGeneralLvGrowMgr.Find(m_poResGeneralLevel->m_dwWitDefGrowId);
    if (!poResGrow)
    {
        LOGERR("Uin<%lu> poResGrow(%u) is not found", rRtData.m_ullUin, m_poResGeneralLevel->m_dwWitDefGrowId);
        return;
    }
    _AddValue(rRtData.m_afAttr[ATTR_WITDEF], m_poResGeneralLevel->m_dwWitDefGrowType,
        poResGrow->m_growLvList[m_bGeneralLevel-1], m_poResGeneralLevel->m_dwWitDefGrowPVPRatio);

    //BaseDamage
    poResGrow = rstResGeneralLvGrowMgr.Find(m_poResGeneralLevel->m_dwBaseDamageGrowId);
    if (!poResGrow)
    {
        LOGERR("Uin<%lu> poResGrow(%u) is not found", rRtData.m_ullUin, m_poResGeneralLevel->m_dwBaseDamageGrowId);
        return;
    }
    _AddValue(rRtData.m_afAttr[ATTR_BASE_NORMALATK], m_poResGeneralLevel->m_dwBaseDamageGrowType,
        poResGrow->m_growLvList[m_bGeneralLevel-1], m_poResGeneralLevel->m_dwBaseDamageGrowPVPRatio);

    /*
    LOGRUN("Uin<%lu> GCardId<%u> AddGeneralLevel Hp=%f, Str=%f, Wit=%f, StrDef=%f, WitDef=%f, BaseDam=%f",  rRtData.m_ullUin, rRtData.m_dwId, rRtData.m_afAttr[ATTR_HP], rRtData.m_afAttr[ATTR_STR],
        rRtData.m_afAttr[ATTR_WIT], rRtData.m_afAttr[ATTR_STRDEF], rRtData.m_afAttr[ATTR_WITDEF], rRtData.m_afAttr[ATTR_BASE_NORMALATK]);
        //*/
    return;

}

void GeneralCard::_AddGeneralStar(DT_ITEM_GCARD* pstGeneral, LiObj& rRtData)
{
    uint8_t m_bGeneralStar = pstGeneral->m_bStar;
    if (m_bGeneralStar >= RES_MAX_STAR_LEVEL)
    {
        LOGERR("Uin<%lu> General Star(%d) is invalid", rRtData.m_ullUin, m_bGeneralStar);
        return;
    }
    RESGENERALSTAR* m_poResGeneralStar = CGameDataMgr::Instance().GetResGeneralStarMgr().Find(rRtData.m_dwId);
    if (!m_poResGeneralStar)
    {
        LOGERR("Uin<%lu> cant find m_poResGeneralStar<%u>", rRtData.m_ullUin, rRtData.m_dwId);
        return;
    }

    _AddValue(rRtData.m_afAttr[ATTR_HP], m_poResGeneralStar->m_bHpGrowType,
        m_poResGeneralStar->m_hpGrow[m_bGeneralStar], m_poResGeneralStar->m_dwHpGrowPVPRatio);

    _AddValue(rRtData.m_afAttr[ATTR_STR], m_poResGeneralStar->m_bStrGrowType,
        m_poResGeneralStar->m_strGrow[m_bGeneralStar], m_poResGeneralStar->m_dwStrGrowPVPRatio);

    _AddValue(rRtData.m_afAttr[ATTR_WIT], m_poResGeneralStar->m_bWitGrowType,
        m_poResGeneralStar->m_witGrow[m_bGeneralStar], m_poResGeneralStar->m_dwWitGrowPVPRatio);

    _AddValue(rRtData.m_afAttr[ATTR_STRDEF], m_poResGeneralStar->m_bStrDefGrowType,
        m_poResGeneralStar->m_strDefGrow[m_bGeneralStar], m_poResGeneralStar->m_dwStrDefGrowPVPRatio);

    _AddValue(rRtData.m_afAttr[ATTR_WITDEF], m_poResGeneralStar->m_bWitDefGrowType,
        m_poResGeneralStar->m_witDefGrow[m_bGeneralStar], m_poResGeneralStar->m_dwWitDefGrowPVPRatio);

    _AddValue(rRtData.m_afAttr[ATTR_BASE_NORMALATK], m_poResGeneralStar->m_bBaseDamageGrowType,
        m_poResGeneralStar->m_baseDamageGrow[m_bGeneralStar], m_poResGeneralStar->m_dwBaseDamageGrowPVPRatio);
    /*
    LOGRUN("Uin<%lu> GCard<%u> AddGeneralStar Hp=%f, Str=%f, Wit=%f, StrDef=%f, WitDef=%f, BaseDam=%f",  rRtData.m_ullUin, rRtData.m_dwId, rRtData.m_afAttr[ATTR_HP], rRtData.m_afAttr[ATTR_STR],
        rRtData.m_afAttr[ATTR_WIT], rRtData.m_afAttr[ATTR_STRDEF], rRtData.m_afAttr[ATTR_WITDEF], rRtData.m_afAttr[ATTR_BASE_NORMALATK]);
        //*/
    return;
}

void GeneralCard::_AddEquipAttr(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral, LiObj& rRtData)
{
    LiObj stRtData = {0};
    for (int i=0; i<EQUIP_TYPE_MAX_NUM; i++)
    {
        DT_ITEM_EQUIP* pstEquipInfo = Equip::Instance().Find(pstData, pstGeneral->m_astEquipList[i].m_dwEquipSeq);
        if (pstEquipInfo == NULL)
        {
            LOGERR("Uin<%lu> GCard<%u> Equip<%u> pstEquipInfo is null",pstData->m_ullUin, rRtData.m_dwId, pstGeneral->m_astEquipList[i].m_dwEquipSeq);
            return ;
        }
        bzero(&stRtData, sizeof(stRtData));
        stRtData.m_ullUin = rRtData.m_ullUin;
        stRtData.m_dwId = rRtData.m_dwId;
        //计算装备等级基础属性和等级加成
        _AddEquipBase(pstGeneral, stRtData, *pstEquipInfo);

        /*
        LOGRUN("Uin<%lu> GCard<%u> Equip<%u> Add EquipStar Hp=%f, Str=%f, Wit=%f, StrDef=%f, WitDef=%f, BaseDam=%f", stRtData.m_ullUin, rRtData.m_dwId, pstEquipInfo->m_dwId,
        stRtData.m_afAttr[ATTR_HP], stRtData.m_afAttr[ATTR_STR], stRtData.m_afAttr[ATTR_WIT],
        stRtData.m_afAttr[ATTR_STRDEF], stRtData.m_afAttr[ATTR_WITDEF], stRtData.m_afAttr[ATTR_BASE_NORMALATK]);
        //*/
        //计算星级加成
        _AddEquipStar(pstGeneral, stRtData, *pstEquipInfo);

        rRtData.AddAttr(stRtData.m_afAttr);

        /*
        LOGRUN("Uin<%lu> GCard<%u> Equip<%u> Add EquipAttr Hp=%f, Str=%f, Wit=%f, StrDef=%f, WitDef=%f, BaseDam=%f, Speed=%f",
            stRtData.m_ullUin, stRtData.m_dwId, pstEquipInfo->m_dwId,
            stRtData.m_afAttr[ATTR_HP], stRtData.m_afAttr[ATTR_STR], stRtData.m_afAttr[ATTR_WIT],
            stRtData.m_afAttr[ATTR_STRDEF], stRtData.m_afAttr[ATTR_WITDEF], stRtData.m_afAttr[ATTR_BASE_NORMALATK], stRtData.m_afAttr[ATTR_SPEED]);
            //*/
    }
}

void GeneralCard::_AddEquipBase(DT_ITEM_GCARD* pstGeneral, LiObj& rRtData, DT_ITEM_EQUIP& rstEquipInfo)
{
    RESEQUIP* poResEquip = CGameDataMgr::Instance().GetResEquipMgr().Find(rstEquipInfo.m_dwId);
    if (poResEquip == NULL)
    {
        LOGERR("Uin<%lu> GCardId<%u> ResEquip(%u) not found.", rRtData.m_ullUin, rRtData.m_dwId, rstEquipInfo.m_dwId);
        return;
    }
    uint8_t bLevel = rstEquipInfo.m_bLevel;
    if (bLevel==0 ||bLevel > RES_MAX_GENERAL_LV_LEN)
    {
        LOGERR("Uin<%lu> GCardId<%u Equip<%u> Level(%d) is invalid", rRtData.m_ullUin, rRtData.m_dwId, rstEquipInfo.m_dwId, bLevel);
        return;
    }
    _AddValue(rRtData.m_afAttr[ATTR_BASE_NORMALATK], poResEquip->m_dwBaseDamageType, poResEquip->m_baseDamageList[bLevel -1], poResEquip->m_dwBaseDamageType);
    _AddValue(rRtData.m_afAttr[ATTR_HP], poResEquip->m_dwHpType, poResEquip->m_hpList[bLevel -1], poResEquip->m_dwHpPVPRatio);
    _AddValue(rRtData.m_afAttr[ATTR_STR], poResEquip->m_dwStrType, poResEquip->m_strList[bLevel -1], poResEquip->m_dwStrPVPRatio);
    _AddValue(rRtData.m_afAttr[ATTR_WIT], poResEquip->m_dwWitType, poResEquip->m_witList[bLevel -1], poResEquip->m_dwWitPVPRatio);
    _AddValue(rRtData.m_afAttr[ATTR_STRDEF], poResEquip->m_dwStrDefType, poResEquip->m_strDefList[bLevel -1], poResEquip->m_dwStrDefPVPRatio);
    _AddValue(rRtData.m_afAttr[ATTR_WITDEF], poResEquip->m_dwWitDefType, poResEquip->m_witDefList[bLevel -1], poResEquip->m_dwWitDefPVPRatio);
    _AddValue(rRtData.m_afAttr[ATTR_SPEED], poResEquip->m_dwSpeedType, poResEquip->m_speedList[bLevel -1], poResEquip->m_dwSpeedPVPRatio);

    _AddValue(rRtData.m_afAttr[ATTR_CHANCE_HIT], poResEquip->m_dwChanceHitType, poResEquip->m_fChanceHit, poResEquip->m_dwChanceHitPVPRatio);
    _AddValue(rRtData.m_afAttr[ATTR_CHANCE_DODGE], poResEquip->m_dwChanceDodgeType, poResEquip->m_fChanceDodge, poResEquip->m_dwChanceDodgePVPRatio);
    _AddValue(rRtData.m_afAttr[ATTR_CHANCE_CRITICAL], poResEquip->m_dwChanceCriticalType, poResEquip->m_fChanceCritical, poResEquip->m_dwChanceCriticalPVPRatio);
    _AddValue(rRtData.m_afAttr[ATTR_CHANCE_ANTICRITICAL], poResEquip->m_dwChanceAntiCriticalType, poResEquip->m_fChanceAntiCritical, poResEquip->m_dwChanceAntiCriticalPVPRatio);
    _AddValue(rRtData.m_afAttr[ATTR_CHANCE_BLOCK], poResEquip->m_dwChanceBlockType, poResEquip->m_fChanceBlock, poResEquip->m_dwChanceBlockPVPRatio);
    _AddValue(rRtData.m_afAttr[ATTR_CHANCE_CRITICAL_VALUE], poResEquip->m_dwParaCriticalType, poResEquip->m_fParaCritical, poResEquip->m_dwParaCriticalPVPRatio);
    _AddValue(rRtData.m_afAttr[ATTR_CHANCE_ANTICRITICAL_VALUE], poResEquip->m_dwParaAntiCriticalType, poResEquip->m_fParaAntiCritical, poResEquip->m_dwParaAntiCriticalPVPRatio);
    _AddValue(rRtData.m_afAttr[ATTR_CHANCE_BLOCK_VALUE], poResEquip->m_dwParaBlockType, poResEquip->m_fParaBlock, poResEquip->m_dwParaBlockPVPRatio);
    _AddValue(rRtData.m_afAttr[ATTR_DAMAGEADD], poResEquip->m_dwParaDamageAddType, poResEquip->m_fParaDamageAdd, poResEquip->m_dwParaDamageAddPVPRatio);
    _AddValue(rRtData.m_afAttr[ATTR_SUCKBLOOD], poResEquip->m_dwParaSuckBloodType, poResEquip->m_fParaSuckBlood, poResEquip->m_dwParaSuckBloodPVPRatio);

    /*
    LOGRUN("Uin<%lu> GCard<%u> Equip<%u> Add EquipBase Hp=%f, Str=%f, Wit=%f, StrDef=%f, WitDef=%f, BaseDam=%f, Speed=%f", rRtData.m_ullUin, rRtData.m_dwId, rstEquipInfo.m_dwId,
        rRtData.m_afAttr[ATTR_HP], rRtData.m_afAttr[ATTR_STR],rRtData.m_afAttr[ATTR_WIT],
        rRtData.m_afAttr[ATTR_STRDEF], rRtData.m_afAttr[ATTR_WITDEF], rRtData.m_afAttr[ATTR_BASE_NORMALATK], rRtData.m_afAttr[ATTR_SPEED]);
        //*/
    return;
}

void GeneralCard::_AddEquipStar(DT_ITEM_GCARD* pstGeneral, LiObj& rRtData, DT_ITEM_EQUIP& rstEquipInfo)
{
    RESEQUIP* poResEquip = CGameDataMgr::Instance().GetResEquipMgr().Find(rstEquipInfo.m_dwId);
    if (poResEquip == NULL)
    {
        LOGERR("Uin<%lu> GCardId<%u> ResEquip(%u) not found.", rRtData.m_ullUin, rRtData.m_dwId, rstEquipInfo.m_dwId);
        return;
    }
    uint8_t bStar = rstEquipInfo.m_wStar;
    if (bStar >= RES_MAX_STAR_LEVEL)
    {
        LOGERR("Uin<%lu> GCardId<%u> Equip<%u> Star(%d) is invalid", rRtData.m_ullUin, rRtData.m_dwId, rstEquipInfo.m_dwId, bStar);
        return;
    }

    //修改装备星级获取途径，首先获取武将id然后获取装备相关数据
    ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
    RESGENERAL* pResGeneral = rstResGeneralMgr.Find(rstEquipInfo.m_dwGCardId);
    if (!pResGeneral)
    {
        LOGERR("pResGeneral not found id=%u", rstEquipInfo.m_dwGCardId);
        return;
    }
    uint16_t wGeneralEquipStarId = pResGeneral->m_equipStarList[rstEquipInfo.m_bType - 1];//当前星级对应在generalequipstar表中的id
    ResGeneralEquipStarMgr_t& rstResGeneralEquipStarMgr = CGameDataMgr::Instance().GetResGeneralEquipStarMgr();
    RESGENERALEQUIPSTAR* pResGeneralEquipStar = rstResGeneralEquipStarMgr.Find(wGeneralEquipStarId);
    if (!pResGeneralEquipStar)
    {
        LOGERR("pResGeneralEquipStar not found id=%u", wGeneralEquipStarId);
        return;
    }

    RESEQUIPSTAR* poResEquipStar = CGameDataMgr::Instance().GetResEquipStarMgr().Find(pResGeneralEquipStar->m_starList[bStar]);
    if (poResEquipStar == NULL)
    {
        LOGERR("Uin<%lu> GCardId<%u> Equip<%u> ResEquipStar(%d) not found", rRtData.m_ullUin, rRtData.m_dwId, rstEquipInfo.m_dwId, poResEquip->m_starGrow[bStar]);
        return;
    }

    _AddValue(rRtData.m_afAttr[ATTR_HP], poResEquipStar->m_dwStarAttributeType, poResEquipStar->m_fStarAttribute, poResEquipStar->m_dwStarAttributePVPRatio);
    _AddValue(rRtData.m_afAttr[ATTR_STR], poResEquipStar->m_dwStarAttributeType, poResEquipStar->m_fStarAttribute, poResEquipStar->m_dwStarAttributePVPRatio);
    _AddValue(rRtData.m_afAttr[ATTR_WIT], poResEquipStar->m_dwStarAttributeType, poResEquipStar->m_fStarAttribute, poResEquipStar->m_dwStarAttributePVPRatio);
    _AddValue(rRtData.m_afAttr[ATTR_STRDEF], poResEquipStar->m_dwStarAttributeType, poResEquipStar->m_fStarAttribute, poResEquipStar->m_dwStarAttributePVPRatio);
    _AddValue(rRtData.m_afAttr[ATTR_WITDEF], poResEquipStar->m_dwStarAttributeType, poResEquipStar->m_fStarAttribute, poResEquipStar->m_dwStarAttributePVPRatio);
    _AddValue(rRtData.m_afAttr[ATTR_SPEED], poResEquipStar->m_dwSpeedStarAttributeType, poResEquipStar->m_fSpeedStarAttribute, poResEquipStar->m_dwSpeedStarAttributePVPRatio);
    _AddValue(rRtData.m_afAttr[ATTR_BASE_NORMALATK], poResEquipStar->m_dwStarAttributeType, poResEquipStar->m_fStarAttribute, poResEquipStar->m_dwStarAttributePVPRatio);
    /*
    LOGRUN("Uin<%lu> GCard<%u> Equip<%u> Add EquipStar Hp=%f, Str=%f, Wit=%f, StrDef=%f, WitDef=%f, BaseDam=%f, Speed=%f", rRtData.m_ullUin, rRtData.m_dwId, rstEquipInfo.m_dwId,
        rRtData.m_afAttr[ATTR_HP], rRtData.m_afAttr[ATTR_STR],rRtData.m_afAttr[ATTR_WIT],
        rRtData.m_afAttr[ATTR_STRDEF], rRtData.m_afAttr[ATTR_WITDEF], rRtData.m_afAttr[ATTR_BASE_NORMALATK], rRtData.m_afAttr[ATTR_SPEED]);
    //*/
    return;
}

void GeneralCard::_AddGeneralFateAttr(PlayerData * pstData, DT_ITEM_GCARD * pstGeneral, const LiObj & rBaseRtData, OUT LiObj & rRtData)
{

    RESGENERAL* pstResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(pstGeneral->m_dwId);
    if (NULL == pstResGeneral)
    {
        LOGERR("Uin<%lu> the pstGeneral<%u> is NULL", pstData->m_ullUin, pstGeneral->m_dwId);
        return ;
    }
    RESGENERALFATE* poResGeneralFate = NULL;
    uint32_t dwFateId = 0;
    if (!pstData->m_bIsGardFateInit)
    {
        InitGCardFate(pstData);
    }
    for (uint32_t i = 0; i < MAX_GCARD_FATEBUFFIDS_NUM; i++)
    {
        dwFateId = pstResGeneral->m_fateId[i];
        if (dwFateId == 0 || !IsGCardFateOpen(pstData, dwFateId))
        {
            continue;
        }
        poResGeneralFate = CGameDataMgr::Instance().GetResGeneralFateMgr().Find(dwFateId);
        if (NULL == poResGeneralFate)
        {
            LOGERR("Uin<%lu> can't  the poResGeneralFate<%u> is NULL", pstData->m_ullUin, dwFateId);
            continue;
        }
        for (uint32_t j = 0; j < MAX_ATTR_ADD_NUM; j++)
        {
            rRtData.m_afAttr[j] += rBaseRtData.m_afAttr[j] * poResGeneralFate->m_addAttribute[j] * CONST_RATIO_PERMILLAGE;
        }
    }
    /*
    LOGRUN("Uin<%lu> GCard<%u> GeneralFateAttr Hp=%f, Str=%f, Wit=%f, StrDef=%f, WitDef=%f, BaseDam=%f, Speed=%f", rBaseRtData.m_ullUin, rBaseRtData.m_dwId,
    rRtData.m_afAttr[ATTR_HP], rRtData.m_afAttr[ATTR_STR],rRtData.m_afAttr[ATTR_WIT],
    rRtData.m_afAttr[ATTR_STRDEF], rRtData.m_afAttr[ATTR_WITDEF], rRtData.m_afAttr[ATTR_BASE_NORMALATK], rRtData.m_afAttr[ATTR_SPEED]);
    //*/
}

void GeneralCard::_AddEquipFateAttr(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral, const LiObj& rBaseRtData, OUT LiObj& rRtData)
{
	RESGENERAL* pstResGeneral = CGameDataMgr::Instance().GetResGeneralMgr().Find(pstGeneral->m_dwId);
	if (NULL == pstResGeneral)
	{
		LOGERR("Uin<%lu> the pstGeneral<%u> is NULL", pstData->m_ullUin, pstGeneral->m_dwId);
		return ;
	}

	ResGeneralEquipFateMgr_t& rResGeneralEquipFateMgr = CGameDataMgr::Instance().GetResGeneralEquipFateMgr();
	RESGENERALEQUIPFATE* poResGeneralEquipFate = NULL;

	for (int i = 0; i < RES_MAX_EQUIP_FATEBUFFIDS_NUM; i++)
	{
		if ( 0 == pstResGeneral->m_equipFateId[i] )
		{
			continue;
		}

		poResGeneralEquipFate = rResGeneralEquipFateMgr.Find(pstResGeneral->m_equipFateId[i]);
		if (NULL == poResGeneralEquipFate || poResGeneralEquipFate->m_bEquipType < 1)
		{
			LOGERR("Uin<%lu> the poResGeneralEquipFate<%u> is NULL or pEquipType is 0", pstData->m_ullUin, pstResGeneral->m_equipFateId[i]);
			return ;
		}

		uint8_t bIndex = poResGeneralEquipFate->m_bEquipType - 1;
		DT_ITEM_EQUIP* pstEquipInfo = Equip::Instance().Find(pstData, pstGeneral->m_astEquipList[bIndex].m_dwEquipSeq);

		if (!pstEquipInfo || 0 == pstEquipInfo->m_wStar )
		{
			continue;
		}
		else
		{
			for (uint32_t j = 0; j < MAX_ATTR_ADD_NUM; j++)
			{
				rRtData.m_afAttr[j] += rBaseRtData.m_afAttr[j] * poResGeneralEquipFate->m_addAttribute[j] * CONST_RATIO_PERMILLAGE;
			}
		}
	}

	/*
    LOGRUN("Uin<%lu> GCard<%u> GeneralFateAttr Hp=%f, Str=%f, Wit=%f, StrDef=%f, WitDef=%f, BaseDam=%f, Speed=%f", rBaseRtData.m_ullUin, rBaseRtData.m_dwId,
    rRtData.m_afAttr[ATTR_HP], rRtData.m_afAttr[ATTR_STR],rRtData.m_afAttr[ATTR_WIT],
    rRtData.m_afAttr[ATTR_STRDEF], rRtData.m_afAttr[ATTR_WITDEF], rRtData.m_afAttr[ATTR_BASE_NORMALATK], rRtData.m_afAttr[ATTR_SPEED]);
	*/
}

void GeneralCard::_AddFeedTrainAtrr(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral, LiObj& rRtData)
{
	ResGeneralMgr_t& rResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
	RESGENERAL* pResGeneral = rResGeneralMgr.Find(pstGeneral->m_dwId);
	if (!pResGeneral)
	{
		LOGERR("pResGeneral is null");
	}

	if (!pstData->m_bIsFeedTrainInit)
	{
		this->CalFeedTrainAttr(pstData);
	}


	//亲密度提供的阵营buf（加属性）
	switch (pResGeneral->m_bCountryId)
	{
	case COUNTRY_WEI:
		rRtData.AddAttr(pstData->m_afWeiFeedTrainAttr);
		break;
	case COUNTRY_SHU:
		rRtData.AddAttr(pstData->m_afShuFeedTrainAttr);
		break;
	case COUNTRY_WU:
		rRtData.AddAttr(pstData->m_afWuFeedTrainAttr);
		break;
	case COUNTRY_MISC:
		rRtData.AddAttr(pstData->m_afOtherFeedTrainAttr);
		break;
	default:
		break;
	}

	//亲密度提供的全体buf（加属性）
	rRtData.AddAttr(pstData->m_afAllFeedTrainAttr);


	ResFameHallGeneralMgr_t& rFameHallGeneralMgr = CGameDataMgr::Instance().GetResFameHallGeneralMgr();
	ResFameHallIncreaseAttrMgr_t& rFameHallIncreaseAttrMgr = CGameDataMgr::Instance().GetResFameHallIncreaseAttrMgr();

	RESFAMEHALLGENERAL* pResFameHallGeneral = rFameHallGeneralMgr.Find(pstGeneral->m_dwId);
	if (!pResFameHallGeneral)
	{
		LOGERR("pResFameHallGeneral is null");
		return;
	}

	uint16_t wIndex = pResFameHallGeneral->m_dwAttrIncrStartId + pstGeneral->m_wTrainValue - 1;

	RESFAMEHALLINCREASEATTR* pResFameHallIncreaseAttr = rFameHallIncreaseAttrMgr.Find(wIndex);
	if (!pResFameHallIncreaseAttr)
	{
		LOGERR("pResFameHallIncreaseAttr is null");
		return;
	}


	for (int i = 0; i < MAX_ATTR_ADD_NUM; i++)
	{
		rRtData.m_afAttr[i] += pResFameHallIncreaseAttr->m_selfAttrAdd[i];
	}
	/*
	LOGRUN("Uin<%lu> GCard<%u>  Before Add EquipGem Hp=%f, Str=%f, Wit=%f, StrDef=%f, WitDef=%f, BaseDam=%f", rRtData.m_ullUin, rRtData.m_dwId,
		rRtData.m_afAttr[ATTR_HP], rRtData.m_afAttr[ATTR_STR],rRtData.m_afAttr[ATTR_WIT],
		rRtData.m_afAttr[ATTR_STRDEF], rRtData.m_afAttr[ATTR_WITDEF], rRtData.m_afAttr[ATTR_BASE_NORMALATK]);
	*/
}

float GeneralCard::_GetGCardSkillRatio(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral, uint8_t bSpecialEquipType, uint32_t dwTalent)
{
    float fRatio = 0;
    float fCardSkillWakeRatio = 0;
    if (bSpecialEquipType != 0)
    {
        DT_ITEM_EQUIP* pstEquipInfo = Equip::Instance().Find(pstData, pstGeneral->m_astEquipList[bSpecialEquipType - 1].m_dwEquipSeq);
        RESLIGCARDSKILL* poResGCardSkillWake = CGameDataMgr::Instance().GetResLiGCardSkillMgr().Find(dwTalent - 10);
        if (NULL == pstEquipInfo || NULL == poResGCardSkillWake)
        {
            LOGERR("Uin<%lu> can't get the equip info or the poResGCardSkillWake is NULL", pstData->m_ullUin);
            return 0;
        }
        fCardSkillWakeRatio = poResGCardSkillWake->m_value[pstEquipInfo->m_wStar];
    }
    else
    {
        //该武将没有觉醒技能
        fCardSkillWakeRatio = 1;
    }

    RESLIGCARDSKILL* poResGCardSkill = NULL;
    uint32_t dwId = 0;
    for (uint8_t i = 0 ; i < MAX_NUM_GENERAL_SKILL && i < pstGeneral->m_bStar; i++)
    {
        dwId = CONST_GCARD_SKILL_DETA * (i + 1) + dwTalent - 10;
        poResGCardSkill = CGameDataMgr::Instance().GetResLiGCardSkillMgr().Find(dwId);
        if (NULL == poResGCardSkill)
        {
            continue;
        }
        if (i == 1 )
        {   //主动技 要乘以技能觉醒参数
            fRatio += poResGCardSkill->m_value[pstGeneral->m_szSkillLevel[i] - 1] * fCardSkillWakeRatio;
        }
        else
        {
            fRatio += poResGCardSkill->m_value[ pstGeneral->m_szSkillLevel[i] - 1 ];
        }
    }
    //兵种技对应ID
    dwId = CONST_GCARD_SKILL_DETA * (MAX_NUM_GENERAL_SKILL + 1) + dwTalent - 10;
    poResGCardSkill = CGameDataMgr::Instance().GetResLiGCardSkillMgr().Find(dwId);
    if (poResGCardSkill == NULL)
    {
        LOGERR("Uin<%lu>  the poResGCardSkill<id=%u> is NULL", pstData->m_ullUin, dwId);
        return (fRatio /= 3);
    }
    fRatio += poResGCardSkill->m_value[pstGeneral->m_bArmyLv - 1];
    return (fRatio /= 3);

}

void GeneralCard::_AddGeneralGem(DT_ITEM_GCARD* pstGeneral, LiObj& rRtData)
{
    RESGEMLIST* poResGem = NULL;
    /*
        LOGRUN("Uin<%lu> GCard<%u>  Before Add EquipGem Hp=%f, Str=%f, Wit=%f, StrDef=%f, WitDef=%f, BaseDam=%f", rRtData.m_ullUin, rRtData.m_dwId,
            rRtData.m_afAttr[ATTR_HP], rRtData.m_afAttr[ATTR_STR],rRtData.m_afAttr[ATTR_WIT],
            rRtData.m_afAttr[ATTR_STRDEF], rRtData.m_afAttr[ATTR_WITDEF], rRtData.m_afAttr[ATTR_BASE_NORMALATK]);
    //*/
    for (int i = 0; i < MAX_GEM_SLOT_NUM; i++)
    {
        if (0 == pstGeneral->m_GemSlot[i] || 1 == pstGeneral->m_GemSlot[i])
        {//没有镶嵌宝石
            continue;
        }
        poResGem = CGameDataMgr::Instance().GetResGemListMgr().Find(pstGeneral->m_GemSlot[i]);
        if (NULL == poResGem)
        {
            continue;
        }
        for (int i = 0; i < 16; i++)
        {
            rRtData.m_afAttr[i + 1] += poResGem->m_attr[i];
            //rRtData.m_afAttr[ATTR_HP] += poResGem->m_attr[EQ_ATTR_HP-1] ;
        }


    }
    /*
    LOGRUN("Uin<%lu> GCard<%u>  After  Add EquipGem Hp=%f, Str=%f, Wit=%f, StrDef=%f, WitDef=%f, BaseDam=%f", rRtData.m_ullUin, rRtData.m_dwId,
        rRtData.m_afAttr[ATTR_HP], rRtData.m_afAttr[ATTR_STR],rRtData.m_afAttr[ATTR_WIT],
        rRtData.m_afAttr[ATTR_STRDEF], rRtData.m_afAttr[ATTR_WITDEF], rRtData.m_afAttr[ATTR_BASE_NORMALATK]);
    //*/

    return;
}

void GeneralCard::_AddGeneralPhase(DT_ITEM_GCARD * pstGeneral, LiObj & rRtData)
{
    uint8_t bPhase = pstGeneral->m_bPhase;
    if (bPhase < 1 || bPhase > RES_MAX_GENERAL_PHASE)
    {
        return;
    }

    RESGENERALPHASE* poResGeneralPhase = CGameDataMgr::Instance().GetResGeneralPhaseMgr().Find(pstGeneral->m_dwId);
    if (poResGeneralPhase == NULL)
    {
        LOGERR("ResGeneralPhase %u not found.", pstGeneral->m_dwId);
        return ;
    }

    int iInitHp = poResGeneralPhase->m_hpGrow[bPhase - 1];
    int iGeneralInitStr = poResGeneralPhase->m_strGrow[bPhase - 1];
    int iGeneralInitWit = poResGeneralPhase->m_witGrow[bPhase - 1];
    int iGeneralInitStrDef = poResGeneralPhase->m_strDefGrow[bPhase - 1];
    int iGeneralInitWitDef = poResGeneralPhase->m_witDefGrow[bPhase - 1];


    int iGeneralInitBaseDamageAtkNormal = poResGeneralPhase->m_baseDamageGrow[bPhase - 1];


    float fPer = 1;

    rRtData.m_afAttr[ATTR_HP] += iInitHp * fPer;
    rRtData.m_afAttr[ATTR_STR] += iGeneralInitStr * fPer;
    rRtData.m_afAttr[ATTR_WIT] += iGeneralInitWit * fPer;
    rRtData.m_afAttr[ATTR_STRDEF] += iGeneralInitStrDef * fPer;
    rRtData.m_afAttr[ATTR_WITDEF]+= iGeneralInitWitDef * fPer;
    rRtData.m_afAttr[ATTR_BASE_NORMALATK] += iGeneralInitBaseDamageAtkNormal * fPer;
}

int GeneralCard::GetFragmentNum(PlayerData* pstData, uint32_t dwId, int32_t iStar, OUT uint32_t* pdwFragmentId)
{

	if(iStar<GCARD_STAR_MIN || iStar>GCARD_STAR_MAX)
	{
		return ERR_WRONG_PARA;
	}

	ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
	RESGENERAL* pstGeneral = rstResGeneralMgr.Find(dwId);
	if(NULL==pstGeneral)
	{
		LOGERR("GetFragmentNum failed, pstGeneral is null");
		return ERR_SYS;
	}
    if (pdwFragmentId != NULL)
    {
        //碎片ID
        *pdwFragmentId = pstGeneral->m_dwExchangeId;
    }
	ResConsumeMgr_t& rstConsumeMgr = CGameDataMgr::Instance().GetResConsumeMgr();
	RESCONSUME* pstConsume = rstConsumeMgr.Find(pstGeneral->m_dwUpStarConsume);
	if(NULL==pstConsume)
	{
		LOGERR("GetFragmentNum failed, pstConsume is null");
		return ERR_SYS;
	}

	int iRet = 0;
	for(int i=0; i<iStar; i++)
	{
		iRet += pstConsume->m_lvList[i];
	}
	return iRet;
}

int GeneralCard::TransUniversalFrag(PlayerData* pstData, CS_PKG_TRANS_UNIVERSAL_FARG_REQ& rstCsPkgBodyReq, SC_PKG_TRANS_UNIVERSAL_FARG_RSP& rstScPkgBodyRsp)
{
	rstScPkgBodyRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

	ResBasicMgr_t& rResBasicMgr = CGameDataMgr::Instance().GetResBasicMgr();
	RESBASIC* poBasic = rResBasicMgr.Find(COMMON_PROP_ID);
	if (!poBasic)
	{
		LOGERR("poBasic is null");
		return ERR_SYS;
	}
	uint32_t dwUniversalFragId = poBasic->m_para[0];

	ResPropsMgr_t& rResPropsMgr = CGameDataMgr::Instance().GetResPropsMgr();
	RESPROPS* poProps = rResPropsMgr.Find(rstCsPkgBodyReq.m_dwGeneralId);
	if (!poProps)
	{
		LOGERR("poProps is null");
		return ERR_SYS;
	}
	if (!GeneralCard::Instance().Find(pstData, poProps->m_dwItemFull))
	{
		LOGERR("the player has no the general");
		return ERR_SYS;
	}

	ResGeneralMgr_t& rResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
	RESGENERAL* poGeneral = rResGeneralMgr.Find(poProps->m_dwItemFull);
	if (!poGeneral)
	{
		LOGERR("poGeneral is null");
		return ERR_SYS;
	}

	poBasic = rResBasicMgr.Find(COMMON_PROP_EXCHANGE_RATIO);
	if (!poBasic)
	{
		LOGERR("poBasic is null");
		return ERR_SYS;
	}

	//兑换比例根据资质硬编码，武将最差11
	if(poGeneral->m_dwTalent<11 || poGeneral->m_dwTalent>16)
	{
		LOGERR("the general's talent isn't in[11,16]");
		return ERR_SYS;
	}
	uint8_t bExchangeRatio = poBasic->m_para[poGeneral->m_dwTalent - 11];
	if(0==bExchangeRatio)
	{
		LOGERR("there's no the general's ExchangeRatio in resbasic");
		return ERR_SYS;
	}

	//消耗万能碎片数目
	uint16_t wConsumeNum = rstCsPkgBodyReq.m_bConsumeNum * bExchangeRatio;

	if (!Props::Instance().IsEnough(pstData, dwUniversalFragId, wConsumeNum))
	{
		return ERR_NOT_ENOUGH_PROPS;
	}

	Item::Instance().ConsumeItem(pstData, ITEM_TYPE_PROPS, dwUniversalFragId, -wConsumeNum, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_TRANS_UNIVERSAL_FRAG);
	Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, rstCsPkgBodyReq.m_dwGeneralId, rstCsPkgBodyReq.m_bConsumeNum, rstScPkgBodyRsp.m_stSyncItemInfo, METHOD_TRANS_UNIVERSAL_FRAG);

	return ERR_NONE;
}

int GeneralCard::AddGeneralExp(PlayerData* pstData, uint32_t dwPveLevelId, uint32_t* pGeneralList/*=NULL*/, uint8_t bBattleArrayType /*= BATTLE_ARRAY_TYPE_NORMAL*/)
{
	int iRet = ERR_NONE;
	RESFIGHTLEVELPL *pLevelPL = CGameDataMgr::Instance().GetResFightLevelPLMgr().Find(dwPveLevelId);

	if(NULL == pLevelPL)
	{
		LOGERR("dwPveLevelId:%d -> not exist.", dwPveLevelId);
		return ERR_SYS;
	}

	DT_BATTLE_ARRAY_INFO* pstBattleArray = Majesty::Instance().GetBattleArrayInfo(pstData, bBattleArrayType);
    if (!pstBattleArray)
    {
        LOGERR("Uin<%lu> get pstBattleArray<%hhu> error", pstData->m_ullUin, bBattleArrayType);
        return ERR_SYS;
    }
	for (int i=0; i<pstBattleArray->m_bGeneralCnt; i++)
	{
		uint16_t wGeneralId = pstBattleArray->m_GeneralList[i];

		pGeneralList[i] = wGeneralId;

		if (wGeneralId == 0)
		{
			continue;
		}

		iRet = GeneralCard::Instance().LvUp(pstData, wGeneralId, pLevelPL->m_wGeneralExpGet);
		if (iRet<0)
		{
			return iRet;
		}
	}

	return iRet;
}


void GeneralCard::TestGCardLiTime(PlayerData* pstData, int iCnt)
{
    clock_t start = 0, stop = 0;
    start = clock();
    for (int i = 0 ; i < iCnt; i++)
    {
        GetTeamLi(pstData);
    }

    stop = clock();
    float fClkPerSec = (float)CLOCKS_PER_SEC;
    float fTotal = (stop - start) / fClkPerSec;
    float fAverage = (stop - start) * 1000 / (fClkPerSec * iCnt);
    LOGERR( "**** Cnt=%d, TotalTime = %f second\n"
            "**** Average time = %f millisecond\n"
            "**** Average get Attr time = %f millisecond", iCnt, fTotal, fAverage, fAverage/6);
}

void GeneralCard::InitGCardFate(PlayerData * pstData)
{
    //获取武将数据档
    //ResGeneralMgr_t& rstResGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();
    ResGeneralFateMgr_t& rstResGeneralFateMgr = CGameDataMgr::Instance().GetResGeneralFateMgr();
    RESGENERALFATE* poResGeneralFate = NULL;
    DT_ITEM_GCARD* pstGeneral = NULL;
    bool bIsOpen = false;
    for (int i = 0; i < rstResGeneralFateMgr.GetResNum(); i++)
    {
        poResGeneralFate = rstResGeneralFateMgr.GetResByPos(i);
        if (NULL == poResGeneralFate)
        {
            LOGERR("Uin<%lu>  the poResGeneralFate NULL<pos %u>", pstData->m_ullUin, i);
            continue;
        }
        bIsOpen = true;
        for (int j = 0; j < 4; j++)
        {
            if (0 == poResGeneralFate->m_generalIds[j])
            {
                continue;
            }
            pstGeneral = Find(pstData, poResGeneralFate->m_generalIds[j]);
            if (pstGeneral == NULL)
            {
                bIsOpen = false;
                break;
            }
        }
        if (bIsOpen)
        {
            //激活,加入列表中
            pstData->m_setOpenGCardFate.insert(poResGeneralFate->m_dwId);
        }
    }
    pstData->m_bIsGardFateInit = true;
}

bool GeneralCard::IsGCardFateOpen(PlayerData * pstData, uint32_t dwFateId)
{
    return pstData->m_setOpenGCardFate.end() != pstData->m_setOpenGCardFate.find(dwFateId);
}

#define MAX_TRAIN_GRADE (5)

int GeneralCard::FeedTrain(PlayerData* pstData, CS_PKG_GCARD_TRAIN_REQ& rstCsReq, SC_PKG_GCARD_TRAIN_RSP& rstScRsp)
{
	//是否拥有该武将卡
	uint32_t dwId = rstCsReq.m_dwGeneralId;
	DT_ITEM_GCARD* pstGeneral = Find(pstData, dwId);
	if (!pstGeneral)
	{
		LOGERR("Player(%lu) LvReborn failed, pstGeneral is null", pstData->m_ullUin);
		return ERR_NOT_FOUND;
	}

	//检查道具是否足够，是否对应正确的角色

	//计算经验值
	uint32_t dwIncExp = 0;
	ResPropsMgr_t& rstResPropsMgr = CGameDataMgr::Instance().GetResPropsMgr();
	for (int i=0; i<rstCsReq.m_stConsumeList.m_bConsumeCount; i++)
	{
		DT_ITEM_CONSUME& rstItemConsume = rstCsReq.m_stConsumeList.m_astConsumeList[i];

		RESPROPS* pResProps = rstResPropsMgr.Find(rstItemConsume.m_dwItemId);
		if (!pResProps)
		{
			LOGERR("Player(%s) Uin(%lu) Equip LvUp failed, Props id(%d) not found", pstData->GetRoleBaseInfo().m_szRoleName,
				pstData->m_ullUin, rstItemConsume.m_dwItemId);
			return ERR_NOT_FOUND;
		}
		if (!Props::Instance().IsEnough(pstData, rstItemConsume.m_dwItemId, rstItemConsume.m_dwItemNum))
		{
			LOGERR("Player(%s) Uin(%lu) Equip LvUp failed, Props id(%d) is not enough", pstData->GetRoleBaseInfo().m_szRoleName,
				pstData->m_ullUin, rstItemConsume.m_dwItemId);
			return ERR_NOT_ENOUGH_PROPS;
		}

		dwIncExp += pResProps->m_dwExp * rstItemConsume.m_dwItemNum;
	}

	//物品消耗
	for (int i=0; i<rstCsReq.m_stConsumeList.m_bConsumeCount; i++)
	{
		DT_ITEM_CONSUME& rstItemConsume = rstCsReq.m_stConsumeList.m_astConsumeList[i];
		Item::Instance().RewardItem(pstData, rstItemConsume.m_bItemType, rstItemConsume.m_dwItemId, -rstItemConsume.m_dwItemNum,
			rstScRsp.m_stSyncItemInfo, METHOD_FEED_TRAIN_LVUP);
	}

	ResFameHallGeneralMgr_t& rFameHallGeneralMgr = CGameDataMgr::Instance().GetResFameHallGeneralMgr();
	RESFAMEHALLGENERAL* pFameHallGeneral = rFameHallGeneralMgr.Find(pstGeneral->m_dwId);
	if (!pFameHallGeneral)
	{
		LOGERR("Player(%s) Uin(%lu) General Train failed, pFameHallGeneral id(%d) not found", pstData->GetRoleBaseInfo().m_szRoleName,
			pstData->m_ullUin, pstGeneral->m_dwId);
		return ERR_SYS;
	}

	ResFameHallFriendShipRuleMgr_t& rFameHallFriendShipRuleMgr = CGameDataMgr::Instance().GetResFameHallFriendShipRuleMgr();
	RESFAMEHALLFRIENDSHIPRULE* pResFameHallFriendShipRule = rFameHallFriendShipRuleMgr.Find(pFameHallGeneral->m_dwFriendshipRuleId + pstGeneral->m_wTrainValue - 1);
	if (!pResFameHallFriendShipRule)
	{
		LOGERR("Player(%s) Uin(%lu) General Train failed, pResFameHallFriendShipRule id(%d) not found", pstData->GetRoleBaseInfo().m_szRoleName,
			pstData->m_ullUin, pstGeneral->m_dwTrainExp);
		return ERR_SYS;
	}

	//计算增加经验后的等级
	uint16_t wOldLv = pstGeneral->m_wTrainValue;
	pstGeneral->m_dwTrainExp += dwIncExp;
	while (pstGeneral->m_dwTrainExp >=  pResFameHallFriendShipRule->m_dwFriendshipExp
			&& pResFameHallFriendShipRule->m_bIsValidLevelup)
	{
		pstGeneral->m_wTrainValue++;
		pResFameHallFriendShipRule = rFameHallFriendShipRuleMgr.Find(pFameHallGeneral->m_dwFriendshipRuleId + pstGeneral->m_wTrainValue - 1);
		if (!pResFameHallFriendShipRule)
		{
			LOGERR("Player(%s) Uin(%lu) General Train failed, pResFameHallFriendShipRule id(%d) not found", pstData->GetRoleBaseInfo().m_szRoleName,
				pstData->m_ullUin, pstGeneral->m_dwTrainExp);
			return ERR_SYS;
		}

		if (pstGeneral->m_wTrainValue >= RES_MAX_FAME_HALL_FRIEND_SHIP_LV)
		{
			break;
		}
	}

	rstScRsp.m_wCurLv = pstGeneral->m_wTrainValue;
	rstScRsp.m_dwCurExp = pstGeneral->m_dwTrainExp;
	rstScRsp.m_dwIncExp = dwIncExp;

	if (wOldLv != pstGeneral->m_wTrainValue)
	{
        pstData->m_dwLeaderValue += m_dwTrainLvLeaderValue * (pstGeneral->m_wTrainValue - wOldLv);
        AsyncPvp::Instance().UptToAsyncSvr(pstData);
		pstData->m_bIsFeedTrainInit = false;
	}

	TriggerAllGcardLi(pstData);

	return ERR_NONE;
}

int GeneralCard::TrainLvUp(PlayerData* pstData, CS_PKG_TRAIN_LVUP_REQ& rstCsReq, SC_PKG_TRAIN_LVUP_RSP& rstScRsp)
{
	//是否拥有该武将卡
	uint32_t dwId = rstCsReq.m_dwGeneralId;
	DT_ITEM_GCARD* pstGeneral = Find(pstData, dwId);
	if (!pstGeneral)
	{
		LOGERR("Player(%lu) LvReborn failed, pstGeneral is null", pstData->m_ullUin);
		return ERR_NOT_FOUND;
	}

	ResFameHallGeneralMgr_t& rFameHallGeneralMgr = CGameDataMgr::Instance().GetResFameHallGeneralMgr();
	RESFAMEHALLGENERAL* pFameHallGeneral = rFameHallGeneralMgr.Find(pstGeneral->m_dwId);
	if (!pFameHallGeneral)
	{
		LOGERR("Player(%s) Uin(%lu) General Train failed, pFameHallGeneral id(%d) not found", pstData->GetRoleBaseInfo().m_szRoleName,
			pstData->m_ullUin, pstGeneral->m_dwId);
		return ERR_SYS;
	}

	ResFameHallFriendShipRuleMgr_t& rFameHallFriendShipRuleMgr = CGameDataMgr::Instance().GetResFameHallFriendShipRuleMgr();
	RESFAMEHALLFRIENDSHIPRULE* pResFameHallFriendShipRule = rFameHallFriendShipRuleMgr.Find(pFameHallGeneral->m_dwFriendshipRuleId + pstGeneral->m_wTrainValue - 1);
	if (!pResFameHallFriendShipRule)
	{
		LOGERR("Player(%s) Uin(%lu) General Train failed, pResFameHallFriendShipRule id(%d) not found", pstData->GetRoleBaseInfo().m_szRoleName,
			pstData->m_ullUin, pstGeneral->m_dwTrainExp);
		return ERR_SYS;
	}

	if ( pResFameHallFriendShipRule->m_bIsValidLevelup != 0 )
	{
		return ERR_LEVEL_LIMIT;
	}

	if ( pstGeneral->m_dwTrainExp < pResFameHallFriendShipRule->m_dwFriendshipExp )
	{
		return ERR_NOT_SATISFY_COND;
	}

	if ( pstGeneral->m_wTrainValue >= RES_MAX_FAME_HALL_FRIEND_SHIP_LV )
	{
		return ERR_TOP_LEVEL;
	}


	/*pstGeneral->m_dwTrainExp -= pResFameHallFriendShipRule->m_dwFriendshipExp;*/

    uint16_t wOldLv = pstGeneral->m_wTrainValue;
	pstGeneral->m_wTrainValue++;


	//给与奖励物品
	for ( int i=0; i<pResFameHallFriendShipRule->m_bBonusNum; i++ )
	{
		Item::Instance().RewardItem(pstData, pResFameHallFriendShipRule->m_szRewardItemTypeList[i],
            pResFameHallFriendShipRule->m_rewardItemIDList[i], pResFameHallFriendShipRule->m_rewardNumList[i], rstScRsp.m_stRewardItemInfo, METHOD_FEED_TRAIN_LVUP);
	}

	//继续升级
	pResFameHallFriendShipRule = rFameHallFriendShipRuleMgr.Find(pFameHallGeneral->m_dwFriendshipRuleId + pstGeneral->m_wTrainValue - 1);
	if (!pResFameHallFriendShipRule)
	{
		LOGERR("Player(%s) Uin(%lu) General Train failed, pResFameHallFriendShipRule id(%d) not found", pstData->GetRoleBaseInfo().m_szRoleName,
			pstData->m_ullUin, pstGeneral->m_dwTrainExp);
		return ERR_SYS;
	}
    pstGeneral->m_bTrainPhase = pResFameHallFriendShipRule->m_bGradLevel;

	while ( pstGeneral->m_dwTrainExp >=  pResFameHallFriendShipRule->m_dwFriendshipExp
		&& pResFameHallFriendShipRule->m_bIsValidLevelup)
	{
		pstGeneral->m_wTrainValue++;
		pResFameHallFriendShipRule = rFameHallFriendShipRuleMgr.Find(pFameHallGeneral->m_dwFriendshipRuleId + pstGeneral->m_wTrainValue - 1);
		if (!pResFameHallFriendShipRule)
		{
			LOGERR("Player(%s) Uin(%lu) General Train failed, pResFameHallFriendShipRule id(%d) not found", pstData->GetRoleBaseInfo().m_szRoleName,
				pstData->m_ullUin, pstGeneral->m_dwTrainExp);
			return ERR_SYS;
		}
	}

	rstScRsp.m_wCurLv = pstGeneral->m_wTrainValue;
	rstScRsp.m_dwCurExp = pstGeneral->m_dwTrainExp;

    pstData->m_dwLeaderValue += m_dwTrainLvLeaderValue * (pstGeneral->m_wTrainValue - wOldLv);
    pstData->m_dwLeaderValue += m_dwTrainPhaseLeaderValue;
    AsyncPvp::Instance().UptToAsyncSvr(pstData);

	pstData->m_bIsFeedTrainInit = false;
	TriggerAllGcardLi(pstData);

	return ERR_NONE;
}

void GeneralCard::TriggerAllGcardLi(PlayerData* pstData)
{
	DT_ROLE_GCARD_INFO& rstGcardInfo = pstData->GetGCardInfo();

	for (int i=0; i<rstGcardInfo.m_iCount; i++)
	{
		TriggerUpdateGCardLi(pstData, &rstGcardInfo.m_astData[i]);
	}
}

//void GeneralCard::InitFeedTrain(PlayerData* pstData)
//{
//	//获取武将数据档
//	DT_ITEM_GCARD* pstGeneral = NULL;
//
//	DT_ROLE_GCARD_INFO& rGCardInfo = pstData->GetGCardInfo();
//
//	ResFameHallGeneralMgr_t& rFameHallGeneralMgr = CGameDataMgr::Instance().GetResFameHallGeneralMgr();
//	ResFameHallIncreaseAttrMgr_t& rFameHallIncreaseAttrMgr = CGameDataMgr::Instance().GetResFameHallIncreaseAttrMgr();
//
//	for (int i=0; i<rGCardInfo.m_iCount; i++)
//	{
//		pstGeneral = &rGCardInfo.m_astData[i];
//
//	}
//
//	pstData->m_bIsGardFateInit = true;
//}


void GeneralCard::CalFeedTrainAttr(PlayerData * pstData)
{
	//获取武将数据档
	DT_ITEM_GCARD* pstGeneral = NULL;

	DT_ROLE_GCARD_INFO& rGCardInfo = pstData->GetGCardInfo();

	ResFameHallGeneralMgr_t& rFameHallGeneralMgr = CGameDataMgr::Instance().GetResFameHallGeneralMgr();
	ResFameHallIncreaseAttrMgr_t& rFameHallIncreaseAttrMgr = CGameDataMgr::Instance().GetResFameHallIncreaseAttrMgr();
	ResGeneralMgr_t& rGeneralMgr = CGameDataMgr::Instance().GetResGeneralMgr();

	RESGENERAL* pResGeneral = NULL;

	bzero(pstData->m_afWeiFeedTrainAttr, sizeof(float)*MAX_ATTR_ADD_NUM);
	bzero(pstData->m_afShuFeedTrainAttr, sizeof(float)*MAX_ATTR_ADD_NUM);
	bzero(pstData->m_afWuFeedTrainAttr, sizeof(float)*MAX_ATTR_ADD_NUM);
	bzero(pstData->m_afOtherFeedTrainAttr, sizeof(float)*MAX_ATTR_ADD_NUM);
	bzero(pstData->m_afAllFeedTrainAttr, sizeof(float)*MAX_ATTR_ADD_NUM);

	for (int i=0; i<rGCardInfo.m_iCount; i++)
	{

		pstGeneral = &rGCardInfo.m_astData[i];

		pResGeneral = rGeneralMgr.Find(pstGeneral->m_dwId);
		if (!pResGeneral)
		{
			LOGERR("pResGeneral is null");
			return;
		}

		RESFAMEHALLGENERAL* pResFameHallGeneral = rFameHallGeneralMgr.Find(pstGeneral->m_dwId);
		if (!pResFameHallGeneral)
		{
			LOGERR("pResFameHallGeneral is null");
			return;
		}

		uint16_t wIndex = pResFameHallGeneral->m_dwAttrIncrStartId + pstGeneral->m_wTrainValue - 1;

		RESFAMEHALLINCREASEATTR* pResFameHallIncreaseAttr = rFameHallIncreaseAttrMgr.Find(wIndex);
		if (!pResFameHallIncreaseAttr)
		{
			LOGERR("Uin<%lu> pResFameHallIncreaseAttr<Index=%hu> is null:GCardId<%u> StartId<%u>, TrainValue<%hu>",
				pstData->m_ullUin, wIndex, pstGeneral->m_dwId, pResFameHallGeneral->m_dwAttrIncrStartId, pstGeneral->m_wTrainValue);
			return;
		}

		for (int j=0; j<RES_MAX_BUFF_COUNTRY; j++)
		{
			uint8_t bCountry = pResFameHallGeneral->m_szCityzenShip[j];
			switch (bCountry)
			{
			case COUNTRY_WEI:
				for (int ii=0; ii<RES_MAX_ATTRIBUTE_NUM; ii++)
				{
					pstData->m_afWeiFeedTrainAttr[ii] += pResFameHallIncreaseAttr->m_groupAttrAdd[ii+j*RES_MAX_ATTRIBUTE_NUM];
				}
				break;
			case COUNTRY_SHU:
				for (int ii=0; ii<RES_MAX_ATTRIBUTE_NUM; ii++)
				{
					pstData->m_afShuFeedTrainAttr[ii] += pResFameHallIncreaseAttr->m_groupAttrAdd[ii+j*RES_MAX_ATTRIBUTE_NUM];
				}
				break;
			case COUNTRY_WU:
				for (int ii=0; ii<RES_MAX_ATTRIBUTE_NUM; ii++)
				{
					pstData->m_afWuFeedTrainAttr[ii] += pResFameHallIncreaseAttr->m_groupAttrAdd[ii+j*RES_MAX_ATTRIBUTE_NUM];
				}
				break;
			case COUNTRY_MISC:
				for (int ii=0; ii<RES_MAX_ATTRIBUTE_NUM; ii++)
				{
					pstData->m_afOtherFeedTrainAttr[ii] += pResFameHallIncreaseAttr->m_groupAttrAdd[ii+j*RES_MAX_ATTRIBUTE_NUM];
				}
				break;
			case COUNTRY_WHOLE:
				for (int ii=0; ii<RES_MAX_ATTRIBUTE_NUM; ii++)
				{
					pstData->m_afAllFeedTrainAttr[ii] += pResFameHallIncreaseAttr->m_groupAttrAdd[ii+j*RES_MAX_ATTRIBUTE_NUM];
				}
				break;
			default:
				break;
			}
		}

	}

	pstData->m_bIsFeedTrainInit = true;
}


