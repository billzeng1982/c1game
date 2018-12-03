#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "RankSys.h"


#define MAX_RANK_TOP_NUM 50
#define MAX_NAME_LENGTH 255
template<typename Key, typename RankNode>
class RankCompare
{
public:
    int operator() (const RankNode & stLeft, const RankNode & stRight) const
    {
        if (stRight < stLeft)
        {
            return 1;
        }
        else if (stLeft == stRight)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
    int operator() (const Key iLeft, const Key iRight) const
    {
        if (iRight < iLeft)
        {
            return 1;
        }
        else if (iLeft == iRight)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
};
struct Student
{
    uint64_t m_ullUin;
    uint32_t m_dwValue;
    void ShowMe()
    {
        printf("uin=%lu\tscore=%u\n", this->m_ullUin, this->m_dwValue);
    }
};
struct RankRoleNode
{
    Student m_stScoreRankInfo;

    bool operator < (const RankRoleNode & a) const
    {
        if (this->m_stScoreRankInfo.m_dwValue > a.m_stScoreRankInfo.m_dwValue)
        {
            return true;
        }
        return false;
    }

    bool operator==(const RankRoleNode& a) const
    {
        return (this->m_stScoreRankInfo.m_dwValue == a.m_stScoreRankInfo.m_dwValue);
    }
    uint64_t GetKey()
    {
        return m_stScoreRankInfo.m_ullUin;
    }
    void SetRankInfo(Student& rstRankInfo) { m_stScoreRankInfo = rstRankInfo; }

    Student& GetRankInfo()
    {
        return m_stScoreRankInfo;
    }

    int pack(char* buffer, size_t size, size_t* usedSize = NULL, unsigned cutVer = 0) const
    {
        *usedSize = MIN(sizeof(m_stScoreRankInfo), size);
        memcpy(buffer, &m_stScoreRankInfo, *usedSize);
        return 0;
    }

    int unpack(const char* buffer, size_t size, size_t* usedSize = NULL, unsigned cutVer = 0)
    {
        *usedSize = MIN(sizeof(m_stScoreRankInfo), size);
        memcpy(&m_stScoreRankInfo, buffer , *usedSize);
        return 0;
        
    }
    void ShowMe() { m_stScoreRankInfo.ShowMe(); }
};
template<typename Key, typename RankNode, typename RankInfo, typename RankCompare>
class RankBase
{
public:
    RankBase() { m_iTopMaxNum = 0; };
    virtual ~RankBase() { m_RankSys.OnExit(); }
    virtual bool Init(const char *pszName, int iMaxNum, int iVersion)
    {
        m_iTopMaxNum = iMaxNum;
        strncpy(m_szRankFileName, pszName, sizeof(m_szRankFileName));
        if (!m_RankSys.Init(m_szRankFileName, iVersion))
        {
            LOGERR("%s Init Failed", m_szRankFileName);
            return false;
        }
        return true;
    }
    void OnExit() { m_RankSys.OnExit(); }
    int UpdateRank(RankNode& rstRankNode)
    {
        Key key = rstRankNode.GetKey();
        int iRet = m_RankSys.Update(key, rstRankNode);
        if (iRet <= MAX_RANK_TOP_NUM)
        {
            m_ullVersion = CGameTime::Instance().GetCurrTimeMs();
        }

        return iRet;
    }
    int GetTopList(RankInfo* pstTopList)
    {
        m_TopVector.clear();
        int count = m_RankSys.GetTopObj(m_iTopMaxNum, m_TopVector);

        for (int i = 0; i < count; i++)
        {
            pstTopList[i] = m_TopVector[i].GetRankInfo();
            pstTopList[i].m_dwRank = i + 1;
        }
        return count;
    }
    int GetRank(Key key) { return m_RankSys.GetRank(key); }
    void ClearRank() { m_RankSys.ClearRank(); }
    bool DeleteFromRank(Key key) { return m_RankSys.Delete(key); }
    uint64_t GetVersion()
    {
        if (m_ullVersion == 0)
        {
            return 1;
        }
        else
        {
            return m_ullVersion;
        }
    }
    void PrintMe()
    {
        m_RankSys.PrintMe();
    }
public:
    int m_iTopMaxNum;
    RankNode m_stRankNode;
    std::vector<RankNode> m_TopVector;
    RankSys<Key, RankNode, RankCompare> m_RankSys;
    char m_szRankFileName[MAX_NAME_LENGTH];
    uint64_t m_ullVersion;
};

typedef RankBase<uint64_t, RankRoleNode, Student, RankCompare<uint64_t, RankRoleNode> > SRank;
void InsertRank(SRank& oSRank, uint32_t dwStart, uint32_t dwNum)
{
    RankRoleNode stNode;
    for (uint32_t i = 0; i < dwNum; i++)
    {
        stNode.m_stScoreRankInfo.m_ullUin = dwStart + i;
        stNode.m_stScoreRankInfo.m_dwValue = dwStart + i;
        oSRank.UpdateRank(stNode);
    }
}




int main()
{
    SRank oSRank;
    oSRank.Init("TestRankTable", 10000, 0);
    oSRank.PrintMe();
    InsertRank(oSRank, 0, 4);
    oSRank.PrintMe();
    oSRank.m_RankSys.Delete(1);
    InsertRank(oSRank, 5, 2);
    oSRank.PrintMe();


    oSRank.OnExit();
    return 0;
}