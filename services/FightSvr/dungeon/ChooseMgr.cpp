#include "ChooseMgr.h"
#include "GameTime.h"
#include "GameDataMgr.h"
#include "LogMacros.h"
#include "FakeRandom.h"
#include "Dungeon.h"

using namespace PKGMETA;

int ChooseMgr::GeneralTalentCmp(const void *pstFirst, const void *pstSecond)
{
    GeneralNode* pstFirstNode = (GeneralNode*)pstFirst;
    GeneralNode* pstSecondNode = (GeneralNode*)pstSecond;

    int iFirstKey = (int)pstFirstNode->m_bTalent * 1000 + (int)pstFirstNode->m_stGeneralInfo.m_dwGeneralID;
    int iSecondKey = (int)pstSecondNode->m_bTalent * 1000 + (int)pstSecondNode->m_stGeneralInfo.m_dwGeneralID;

    return iSecondKey - iFirstKey;
}

ChooseMgr::ChooseMgr() : m_bCurRuleId(0), m_bGeneralCnt(0), m_bMSkillCnt(0)
{}

ChooseMgr::~ChooseMgr()
{}

bool ChooseMgr::Init()
{
    if (!_InitMSkillList())
    {
        return false;
    }

    if (!_InitGeneralList())
    {
        return false;
    }

    if (!_InitChooseNode())
    {
        return false;
    }

    if (!_InitSeason())
    {
        return false;
    }

    if (!_InitRuleList())
    {
        return false;
    }

    if (!_InitSkinList())
    {
        return false;
    }

    return true;
}

bool ChooseMgr::_InitSkinList()
{
    int iNum = CGameDataMgr::Instance().GetResGeneralSkinMgr().GetResNum();
    for (int i = 0; i < iNum; i++)
    {
        RESGENERALSKIN* pResSkin = CGameDataMgr::Instance().GetResGeneralSkinMgr().GetResByPos(i);
        MapId2SkinList_t::iterator it = m_mapId2SkinList.find(pResSkin->m_dwGeneralId);
        if (it == m_mapId2SkinList.end())
        {
            ListSkin_t* poSkinList = new ListSkin_t();
            poSkinList->push_back(pResSkin->m_dwId);
            m_mapId2SkinList.insert(MapId2SkinList_t::value_type(pResSkin->m_dwGeneralId, poSkinList));
        }
        else
        {
            ListSkin_t* poSkinList = it->second;
            poSkinList->push_back(pResSkin->m_dwId);
        }
    }

    return true;
}


bool ChooseMgr::_InitMSkillList()
{
    //生成军师技列表
    RESPEAKARENAPARA* pResPara = CGameDataMgr::Instance().GetResPeakArenaParaMgr().Find(4);
    if (!pResPara)
    {
        LOGERR("pResPara is null");
        return false;
    }

    RESMASTERSKILL* pResMSkill;
    m_bMSkillCnt = CGameDataMgr::Instance().GetResMasterSkillMgr().GetResNum();
    for (int i=0; i<m_bMSkillCnt; i++)
    {
        pResMSkill = CGameDataMgr::Instance().GetResMasterSkillMgr().GetResByPos(i);
        m_MSkillList[i].m_bId = pResMSkill->m_dwId;
        m_MSkillList[i].m_bLevel = pResPara->m_paramList[0];
        LOGRUN("Gen MSkillList, index(%d) MSkilId(%d)", i, m_MSkillList[i].m_bId);
    }

    return true;
}

bool ChooseMgr::_InitGeneralList()
{
    //生成武将列表
    RESPEAKARENAPARA* pResPara = CGameDataMgr::Instance().GetResPeakArenaParaMgr().Find(1);
    if (!pResPara)
    {
       LOGERR("pResPara is null");
       return false;
    }

    RESPEAKARENACHOOSERULE* pResRule = CGameDataMgr::Instance().GetResPeakArenaChooseRuleMgr().GetResByPos(m_bCurRuleId);
    if (!pResRule)
    {
       LOGERR("pResRule is null");
       return false;
    }

    //生成本周可选武将
    GeneralNode stNode;
    RESPEAKARENAGENERAL* pResGeneral;
    int iGeneralResNum = CGameDataMgr::Instance().GetResPeakArenaGeneralMgr().GetResNum();
    for (int i=0; i<iGeneralResNum; i++)
    {
       if (m_bGeneralCnt >= MAX_NUM_GCARD_FOR_CHOOSE)
       {
           break;
       }

       pResGeneral = CGameDataMgr::Instance().GetResPeakArenaGeneralMgr().GetResByPos(i);
       if (pResGeneral->m_bIsOpen != 1)
       {
           continue;
       }

       int iFlag = 0;
       for (int j = 0; j < pResRule->m_bGeneralBanCnt; j++)
       {
           if (pResRule->m_generalBanList[j] == pResGeneral->m_dwId)
           {
               iFlag = 1;
               break;
           }
       }
       if (iFlag)
       {
           continue;
       }

       stNode.m_bTalent = pResGeneral->m_bTalent;
       stNode.m_stGeneralInfo.m_dwGeneralID = pResGeneral->m_dwId;
       stNode.m_stGeneralInfo.m_bLv = pResPara->m_paramList[0];
       stNode.m_stGeneralInfo.m_bStar = pResPara->m_paramList[1];
       stNode.m_stGeneralInfo.m_bGrade = pResPara->m_paramList[2];

       size_t nmemb = (size_t)m_bGeneralCnt;
       nmemb = (nmemb >= MAX_NUM_GCARD_FOR_CHOOSE) ? nmemb -1 : nmemb;
       if (MyBInsert(&stNode, m_GeneralList, &nmemb, sizeof(GeneralNode), 1, ChooseMgr::GeneralTalentCmp))
       {
           m_bGeneralCnt = (int32_t)nmemb;
       }
    }

    for (uint8_t i=0; i<m_bGeneralCnt; i++)
    {
        GeneralNode& rstNode =  m_GeneralList[i];
        LOGRUN("GeneralList[%d], Talent(%d), GeneralId(%u)", i, rstNode.m_bTalent, rstNode.m_stGeneralInfo.m_dwGeneralID);
    }

    return true;
}

bool ChooseMgr::_InitChooseNode()
{
    //生成武将列表
    RESPEAKARENAPARA* pResPara1 = CGameDataMgr::Instance().GetResPeakArenaParaMgr().Find(11);
    if (!pResPara1)
    {
       LOGERR("pResPara1 is null");
       return false;
    }

    //生成武将列表
    RESPEAKARENAPARA* pResPara2 = CGameDataMgr::Instance().GetResPeakArenaParaMgr().Find(12);
    if (!pResPara2)
    {
       LOGERR("pResPara2 is null");
       return false;
    }

    for (int i=0; i<MAX_CHOOSE_NODE_NUM; i++)
    {
        m_astChooseList[i].m_iChooseCnt = pResPara1->m_paramList[i];
        m_astChooseList[i].m_bTalent = pResPara2->m_paramList[i];
    }

    return true;
}

bool ChooseMgr::_InitSeason()
{
    RESPEAKARENAPARA* pResPara = CGameDataMgr::Instance().GetResPeakArenaParaMgr().Find(9);
    if (!pResPara)
    {
        return false;
    }
    m_bSeasonLastMonth = pResPara->m_paramList[0];

    //计算赛季开始月和赛季结束月
    struct tm tTime;
    bzero(&tTime, sizeof(tTime));

    //计算赛季开始时间
    int iStartYear = CGameTime::Instance().GetCurrYear();
    int iStartMonth = CGameTime::Instance().GetCurrMonth();
    int iDeltaMonth = ((iStartYear + 1900 - pResPara->m_paramList[1]) * 12 + (iStartMonth + 1 - pResPara->m_paramList[2])) % m_bSeasonLastMonth;
    if (iStartMonth < iDeltaMonth)
    {
        iStartYear--;
        iStartMonth = iStartMonth + 12 - iDeltaMonth;
    }
    else
    {
        iStartMonth -= iDeltaMonth;
    }
    tTime.tm_year = iStartYear;
    tTime.tm_mon = iStartMonth;
    tTime.tm_mday = 1;
    m_ullSeasonStartTime = mktime(&tTime);

    //计算赛季结束时间
    int iEndYear = iStartYear;
    int iEndMonth = iStartMonth + m_bSeasonLastMonth;
    if (iEndMonth > 11)
    {
        iEndYear++;
        iEndMonth -= 12;
    }
    tTime.tm_year = iEndYear;
    tTime.tm_mon = iEndMonth;
    tTime.tm_mday = 1;
    m_ullSeasonEndTime = mktime(&tTime) - 1;

    LOGRUN("Init Season, Start Year(%d) Month(%d), Stop Year(%d) Month(%d)", iStartYear, iStartMonth, iEndYear, iEndMonth);

    return true;
}

bool ChooseMgr::_InitRuleList()
{
    m_bCurRuleId = 0;
    m_bIsRD = 1;
    return true;
}

void ChooseMgr::Update()
{


}

int ChooseMgr::GetGeneralChooseList(uint8_t& bGeneralCnt, DT_RANK_GENERAL_INFO GeneralList[])
{
    if (!m_bIsRD)
    {
        bGeneralCnt = m_bGeneralCnt;
        for (int i=0; i<m_bGeneralCnt; i++)
        {
            this->_AddGeneral(bGeneralCnt, GeneralList, m_GeneralList[i].m_stGeneralInfo);
        }
    }
    else
    {
        int iIndex = 0;
        for (int i=0; i<MAX_CHOOSE_NODE_NUM; i++)
        {
            this->_RDChoose(bGeneralCnt, GeneralList, iIndex, m_astChooseList[i]);
        }
    }

    return ERR_NONE;
}

void ChooseMgr::_AddGeneral(uint8_t& bGeneralCnt, DT_RANK_GENERAL_INFO GeneralList[], DT_RANK_GENERAL_INFO& rstGeneralInfo)
{
    size_t nmemb = (size_t)bGeneralCnt;
    nmemb = (nmemb >= MAX_NUM_GCARD_FOR_CHOOSE) ? nmemb -1 : nmemb;
    if (MyBInsert(&rstGeneralInfo, GeneralList, &nmemb, sizeof(DT_RANK_GENERAL_INFO), 1, Dungeon::GeneralCmp))
    {
       bGeneralCnt = (int32_t)nmemb;
    }
}

void ChooseMgr::_RDChoose(uint8_t& bGeneralCnt, DT_RANK_GENERAL_INFO GeneralList[], int& iIndex, ChooseNode& rstChooseNode)
{
    int iStart = iIndex;
    while (iIndex<m_bGeneralCnt && m_GeneralList[iIndex].m_bTalent>=rstChooseNode.m_bTalent)
    {
        iIndex++;
    }
    int iCnt = iIndex - iStart;

    int iChooseCnt = iCnt > rstChooseNode.m_iChooseCnt ? rstChooseNode.m_iChooseCnt : iCnt;

    CFakeRandom::Instance().Random(iCnt, iChooseCnt, m_RandomNum);

    for (uint8_t i=0; i<iChooseCnt; i++)
    {
        this->_AddGeneral(bGeneralCnt, GeneralList, m_GeneralList[m_RandomNum[i] + iStart].m_stGeneralInfo);
    }
}

int ChooseMgr::GetMSkillChooseList(uint8_t& bMSkillCnt, DT_ITEM_MSKILL MSkillList[])
{
    bMSkillCnt = m_bMSkillCnt;
    for (int i=0; i<m_bMSkillCnt; i++)
    {
        MSkillList[i] = m_MSkillList[i];
    }

    return ERR_NONE;
}

ChooseMgr::ListSkin_t* ChooseMgr::GetSkinListByGeneral(uint32_t dwGeneral)
{
    MapId2SkinList_t::iterator it = m_mapId2SkinList.find(dwGeneral);
    if (it != m_mapId2SkinList.end())
    {
        return it->second;
    }
    else
    {
        return NULL;
    }
}

