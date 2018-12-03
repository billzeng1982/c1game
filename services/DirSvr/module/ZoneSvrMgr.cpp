#include "ZoneSvrMgr.h"
#include "LogMacros.h"

using namespace PKGMETA;

int SvrCmp(const void *pstFirst, const void *pstSecond)
{
    int iResult = (int)((DT_SERVER_INFO_FOR_CLIENT*)pstFirst)->m_iIp -
                    (int)((DT_SERVER_INFO_FOR_CLIENT*)pstSecond)->m_iIp;
    return iResult;
}

ZoneSvrMgr::ZoneSvrMgr()
{
    m_stSvrList.m_bCount = 0;
}

int ZoneSvrMgr::HandleSvrStatNtf(DT_SERVER_INFO& rstSvrInfo)
{
    DT_SERVER_INFO_FOR_CLIENT stSvrInfo;
    stSvrInfo.m_dwSvrId = rstSvrInfo.m_dwSvrId;
    stSvrInfo.m_iIp = rstSvrInfo.m_iIp;
    stSvrInfo.m_wPort = rstSvrInfo.m_wPort;
    stSvrInfo.m_dwState = _GetSvrState(rstSvrInfo);
    memcpy(stSvrInfo.m_szSvrName, rstSvrInfo.m_szSvrName, MAX_NAME_LENGTH);

    size_t nmemb = (size_t)m_stSvrList.m_bCount;
    int iEqual = 0;
    int iIndex = MyBSearch(&stSvrInfo, m_stSvrList.m_astSvrList, m_stSvrList.m_bCount, sizeof(DT_SERVER_INFO_FOR_CLIENT), &iEqual, SvrCmp);
    if (iEqual)
    {
        m_stSvrList.m_astSvrList[iIndex] = stSvrInfo;
    }
    else
    {
        nmemb = (nmemb >= MAX_NUM_ZONESVR) ? nmemb -1 : nmemb;
        MyBInsert(&stSvrInfo, m_stSvrList.m_astSvrList, &nmemb, sizeof(DT_SERVER_INFO_FOR_CLIENT), 1, SvrCmp);
        m_stSvrList.m_bCount = nmemb;
    }

    return 0;
}


DT_SERVER_LIST& ZoneSvrMgr::GetSvrList()
{
    return m_stSvrList;
}


uint32_t ZoneSvrMgr::_GetSvrState(DT_SERVER_INFO& rstSvrInfo)
{
    uint32_t dwState;
    if (rstSvrInfo.m_bOnline == 0)
    {
        dwState = SERVER_STATE_CLOSE;
    }
    else
    {
        if (rstSvrInfo.m_dwLoad < IDLE_STATE_PLAYER_NUM_LIMIT)
        {
            dwState = SERVER_STATE_IDLE;
        }
        else if (rstSvrInfo.m_dwLoad < BUSY_STATE_PLAYER_NUM_LIMIT)
        {
            dwState = SERVER_STATE_BUSY;
        }
        else
        {
            dwState = SERVER_STATE_FULL;
        }
    }

    return dwState;
}

