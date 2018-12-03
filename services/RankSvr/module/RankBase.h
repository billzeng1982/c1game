#pragma once
#include <vector>
#include "RankSys.h"
#include "../RankSvr.h"


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
    //获得当前排行榜上数量
    int GetCurRankNum() { return m_RankSys.Size(); }
    RankNode* GetRankNodeAtN(int n) { return m_RankSys.GetObjAtN(n); }
	uint64_t GetVersion()
	{
		if (m_ullVersion==0)
		{
			return 1;
		}
		else
		{
			return m_ullVersion;
		}	
	}
	void UpdateVersion()
	{
		m_ullVersion = CGameTime::Instance().GetCurrTimeMs();
	}
protected:
    int m_iTopMaxNum;
    RankNode m_stRankNode;
    std::vector<RankNode> m_TopVector;
    RankSys<Key, RankNode, RankCompare> m_RankSys;
    char m_szRankFileName[MAX_NAME_LENGTH];
	uint64_t m_ullVersion;
};


