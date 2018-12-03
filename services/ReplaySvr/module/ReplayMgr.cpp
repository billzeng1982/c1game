 #include "ReplayMgr.h"
#include "LogMacros.h"
#include "common_proto.h"
#include "../framework/GameObjectPool.h"
#include "GameTime.h"
#include "unistd.h"
#include "strutil.h"
#include "../framework/ReplaySvrMsgLayer.h"

using namespace PKGMETA;

int ReplayCompare(DT_REPLAY_INFO* pstReplay1, DT_REPLAY_INFO* pstReplay2)
{
    uint32_t dwEfficiency1 = pstReplay1->m_stFileHead.m_astPlayerStaList[0].m_dwEfficiency
                        + pstReplay1->m_stFileHead.m_astPlayerStaList[1].m_dwEfficiency;

    uint32_t dwEfficiency2 = pstReplay2->m_stFileHead.m_astPlayerStaList[0].m_dwEfficiency
                         + pstReplay2->m_stFileHead.m_astPlayerStaList[1].m_dwEfficiency;

    if (dwEfficiency1 >= dwEfficiency2)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

ReplayMgr::ReplayMgr()
{
    m_dwCheckInterval = 0;
    m_dwUpdateInterval = 0;
    m_ullLastUpdateTime = 0;
    bzero(m_szURLRoot, URLROOT_MAXLEN);
    bzero(m_szRootDir, ROOTDIR_MAXLEN);
    bzero(m_ReplayList,sizeof(m_ReplayList));
    bzero(&m_stWholeReplayList, sizeof(m_stWholeReplayList));
    m_ToCheckedList.clear();
}

ReplayMgr::~ReplayMgr()
{

}

bool ReplayMgr::Init(char* pszURLRoot, char* pszRootDir, uint32_t dwCheckInterval, uint32_t dwUpdateInterval, const char* pszFileName)
{
    assert(pszURLRoot);
    StrCpy(m_szURLRoot, pszURLRoot, URLROOT_MAXLEN);

    assert(pszRootDir);
    StrCpy(m_szRootDir, pszRootDir, ROOTDIR_MAXLEN);

    assert(pszFileName);
    StrCpy(m_szFileName, pszFileName, MAX_LEN_FILEPATH);

    m_dwCheckInterval = dwCheckInterval * 1000;
    m_dwUpdateInterval = dwUpdateInterval * 1000;

    if (!m_oReplayPool.Init(REPLAY_POOL_INIT_NUM, REPLAY_POOL_DELTA_NUM))
    {
        return false;
    }

    if (!this->_InitFile())
    {
        LOGERR("init file failed");
        return false;
    }

    LOGRUN("Init ReplayMgr. URLRoot(%s), RootDir(%s), CheckInterval(%u), UpdateInterval(%u)",
            m_szURLRoot, m_szRootDir, m_dwCheckInterval, m_dwUpdateInterval);
    return true;
}

bool ReplayMgr::_InitFile()
{
    if (access(m_szFileName, F_OK))
    {
        if (!this->_CreatFile())
        {
            LOGERR("replay list file create failed.");
            return false;
        }
    }
    else
    {
        if (!this->_InitFromFile())
        {
            LOGERR("replay list file get head failed.");
            return false;
        }
    }

    return true;
}


bool ReplayMgr::_CreatFile()
{
    m_fp = fopen(m_szFileName, "wb+");
    if (m_fp == NULL)
    {
        LOGERR("ReplayMgr create file failed");
        return false;
    }

    bzero(&m_stWholeReplayList, sizeof(m_stWholeReplayList));


    LOGRUN("CreatFile (%s) success", m_szFileName);
    return true;
}

bool ReplayMgr:: _InitFromFile()
{
    m_fp = fopen(m_szFileName, "rb+");
    if (!m_fp)
    {
        LOGERR("ReplayMgr get filehead failed");
        return false;
    }

    fseek(m_fp, 0, SEEK_SET);
    if(fread(&m_stWholeReplayList, sizeof(m_stWholeReplayList), 1, m_fp) != 1)
    {
        LOGERR("ReplayMgr get filehead failed");
        return false;
    }

    LOGRUN("ReplayMgr InitFromFile success");

    return true;
}

void ReplayMgr::_WriteToFile()
{
    if (!m_fp)
    {
        LOGERR("write list to file failed, m_fp is null");
        return;
    }


    fseek(m_fp, 0, SEEK_SET);
    if(fwrite(&m_stWholeReplayList, sizeof(m_stWholeReplayList), 1, m_fp) != 1)
    {
        LOGERR("ReplayMgr write file failed");
        return;
    }

    fflush(m_fp);
    return;
}

void ReplayMgr::AppFini()
{
    this->_WriteToFile();

    if (m_fp)
    {
        fclose(m_fp);
        m_fp = NULL;
    }

    return;
}

void ReplayMgr::Update()
{
    _UpdateToCheckedList();
    _UpdateToPushReplay();
}

void ReplayMgr::_UpdateToCheckedList()
{
    int iCounter = 0;
    uint64_t ullCurTime = CGameTime::Instance().GetCurrTimeMs();
    while(!m_ToCheckedList.empty() && iCounter++ < MAX_DEAL_NUM_PER_FRAME)
    {
        DT_REPLAY_INFO * pstReplayInfo = m_ToCheckedList.front();
        assert(pstReplayInfo);

        if ((ullCurTime - pstReplayInfo->m_ullUpLoadTimeStamp) < m_dwCheckInterval)
        {
            break;
        }

        m_ToCheckedList.pop_front();

        //录像文件不存在，可能是上传失败，不加入ReplayMap
        if (!CheckReplayExist(pstReplayInfo->m_stFileHead.m_dwRaceNumber))
        {
            LOGERR("Replay(%u) Upload failed", pstReplayInfo->m_stFileHead.m_dwRaceNumber);
            m_oReplayPool.Release(pstReplayInfo);
            continue;
        }

        //加入m_ToBeWriteList，等待写入文件
        this->_CheckReplay(pstReplayInfo);
    }
}


void ReplayMgr::_CheckReplay(DT_REPLAY_INFO * pstReplayInfo)
{
    uint8_t bGrade = pstReplayInfo->m_stFileHead.m_bGrade;
    if (bGrade >= MAX_NUM_GRADE)
    {
        LOGERR("Replay(%u) Grade(%d) is error", pstReplayInfo->m_stFileHead.m_dwRaceNumber, bGrade);
        this->RemoveReplay(pstReplayInfo->m_stFileHead.m_dwRaceNumber);
        return;
    }

    if (m_ReplayList[bGrade] == NULL)
    {
        m_ReplayList[bGrade] = pstReplayInfo;
        return;
    }

    //检查的录像比当前已保存的录像更符合要求
    if (1 == ReplayCompare(pstReplayInfo, m_ReplayList[bGrade]))
    {
        this->RemoveReplay(m_ReplayList[bGrade]->m_stFileHead.m_dwRaceNumber);
        m_oReplayPool.Release(m_ReplayList[bGrade]);
        m_ReplayList[bGrade] = pstReplayInfo;
    }

    return;
}


void ReplayMgr::_UpdateToPushReplay()
{
    uint64_t ullCurTime = CGameTime::Instance().GetCurrTimeMs();
    if ((ullCurTime -m_ullLastUpdateTime) < m_dwUpdateInterval)
    {
        return;
    }

    m_ullLastUpdateTime = ullCurTime;

    //根据规则生成新的Queue
    for(int i=0; i<MAX_NUM_GRADE; i++)
    {
        if (m_ReplayList[i] == NULL)
        {
            continue;
        }

        DT_ONE_GRADE_REPLAY_LIST& rstOneGradeList = m_stWholeReplayList.m_astWholeReplay[i];
        if (rstOneGradeList.m_wReplayCount < MAX_NUM_REPLAY)
        {
            rstOneGradeList.m_astReplayList[rstOneGradeList.m_wReplayCount++] = *(m_ReplayList[i]);
        }
        else
        {
            DT_ONE_GRADE_REPLAY_LIST    stTemp;
            memcpy(stTemp.m_astReplayList, rstOneGradeList.m_astReplayList, sizeof(stTemp.m_astReplayList));
            memcpy(rstOneGradeList.m_astReplayList, &stTemp.m_astReplayList[1], sizeof(DT_REPLAY_INFO)* (MAX_NUM_REPLAY-1));
            rstOneGradeList.m_astReplayList[MAX_NUM_REPLAY -1] = *(m_ReplayList[i]);
        }

        m_oReplayPool.Release(m_ReplayList[i]);
        m_ReplayList[i] = NULL;
    }

    //将新的列表写入文件
    this->_WriteToFile();

    //将新的列表通知到ZoneSvr
    this->SendSynNtf();
}


void ReplayMgr::SendSynNtf()
{
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_REFRESH_REPLAYLIST_NTF;
    SS_PKG_REFRESH_REPLAYLIST_NTF & rstRefreshReplay= m_stSsPkg.m_stBody.m_stRefreshReplayNtf;

    rstRefreshReplay.m_ullTimeStamp = CGameTime::Instance().GetCurrTimeMs();;
    rstRefreshReplay.m_stReplayList = m_stWholeReplayList;

    ReplaySvrMsgLayer::Instance().SendToZoneSvr(m_stSsPkg);
}

int ReplayMgr::UploadReplay(DT_REPLAY_RECORD_FILE_HEADER* pReplayFileHead, char* pszURL)
{
    DT_REPLAY_INFO * pstReplayInfo = m_oReplayPool.Get();

    uint32_t dwRaceNumber = pReplayFileHead->m_dwRaceNumber;
    memcpy(&pstReplayInfo->m_stFileHead, pReplayFileHead, sizeof(DT_REPLAY_RECORD_FILE_HEADER));
    sprintf(pstReplayInfo->m_szURL, "%s/%u", m_szURLRoot, dwRaceNumber);
    sprintf(pszURL, "%s/%u", m_szURLRoot, dwRaceNumber);
    pstReplayInfo->m_ullUpLoadTimeStamp = CGameTime::Instance().GetCurrTimeMs();

    //允许上传，录像信息 加入ToBeChecked列表
    m_ToCheckedList.push_back(pstReplayInfo);

    return ERR_NONE;
}

bool ReplayMgr::CheckReplayExist(uint32_t dwRaceNumber)
{
    char szFilePath[MAX_LEN_FILEPATH];
    sprintf(szFilePath, "%s/%u", m_szRootDir, dwRaceNumber);
    return (access(szFilePath, F_OK) == 0);
}

void ReplayMgr::RemoveReplay(uint32_t dwRaceNumber)
{
    char szFilePath[MAX_LEN_FILEPATH];
    sprintf(szFilePath, "%s/%u", m_szRootDir, dwRaceNumber);
    int iRet = remove(szFilePath);
    if (iRet != 0)
    {
        LOGERR("remove Replay(%u) failed, Ret=%d", dwRaceNumber, iRet);
    }
}

