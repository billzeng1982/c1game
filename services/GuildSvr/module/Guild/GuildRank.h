#pragma once
#include "define.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "algo/RankSys.h"


struct GuildRankNode
{
	DT_GUILD_RANK_INFO stScoreRankInfo;
	bool operator < (const GuildRankNode & a ) const
	{
		if (this->stScoreRankInfo.m_dwStarNum > a.stScoreRankInfo.m_dwStarNum)
		{
			return true;
		}
		else if (this->stScoreRankInfo.m_dwStarNum == a.stScoreRankInfo.m_dwStarNum &&
			this->stScoreRankInfo.m_ullTimeStampMs < a.stScoreRankInfo.m_ullTimeStampMs)
		{
			return true;
		}
		return false;
	}

	bool operator==(const GuildRankNode& a) const
	{
		return this->stScoreRankInfo.m_dwStarNum == a.stScoreRankInfo.m_dwStarNum;
	}

    uint64_t& GetKey()
    {
        return stScoreRankInfo.m_ullGuildId;
    }
    void SetRankInfo(PKGMETA::DT_GUILD_RANK_INFO stRankInfo)
    {
        stScoreRankInfo = stRankInfo;
    }
    PKGMETA::DT_GUILD_RANK_INFO& GetRankInfo()
    {
        return stScoreRankInfo;
    }

    TdrError::ErrorType pack(char* buffer, size_t size, size_t* usedSize = NULL, unsigned cutVer = 0) const
    {
        return stScoreRankInfo.pack(buffer, size, usedSize, cutVer);
    }
    TdrError::ErrorType unpack(const char* buffer, size_t size, size_t* usedSize = NULL, unsigned cutVer = 0)
    {
        return stScoreRankInfo.unpack(buffer, size, usedSize, cutVer);
    }

};

struct ScoreCompare
{
	int operator() ( const GuildRankNode & stLeft, const GuildRankNode & stRight) const
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
	int operator() (const uint64_t iLeft, const uint64_t  iRight) const
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



class GuildRankMgr : public TSingleton<GuildRankMgr>
{
public:
	GuildRankMgr();
	virtual ~GuildRankMgr() { m_GuildRankSys.OnExit();}
	bool Init();
	void Fini(){ m_GuildRankSys.OnExit(); }
	int UpdateRank(DT_GUILD_RANK_INFO& rstRankInfo);
	int GetTopList(DT_GUILD_RANK_INFO* pstTopList);
	void Delete(uint64_t ullGuildId);
	void RewardRank();
	void Reset();
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
	int GetRank(uint64_t ullGuildId) { return m_GuildRankSys.GetRank(ullGuildId); }
    GuildRankNode* GetGuildByRank(uint16_t wRank) { return m_GuildRankSys.GetObjAtN(wRank); }
    int GetCurGuildRankNum() { return m_iCurGuildRankNum; }
	
	void UpdateGuildRankNtf(Guild* poGuild, DT_GUILD_RANK_INFO& rstGuildRankInfo);

private:
	PKGMETA::SSPKG m_stSsPkg;
	int m_iGuildRankMinScore;	//排行榜上的最低分数
	int m_iCurGuildRankNum;		//当前排行榜人数
	int m_iGuildTopMaxNum;		//排行榜最大数量
	GuildRankNode m_stGuildRankNode;
	vector<GuildRankNode> m_GuildTopVector;
	RankSys<uint64_t, GuildRankNode, ScoreCompare> m_GuildRankSys;
	uint64_t m_ullVersion;
};


struct GFightRankNode
{
	DT_GFIGHT_RANK_INFO stScoreRankInfo;
	bool operator < (const GFightRankNode & a ) const
	{
		if (this->stScoreRankInfo.m_wScore > a.stScoreRankInfo.m_wScore)
		{
			return true;
		}
		else if (this->stScoreRankInfo.m_wScore == a.stScoreRankInfo.m_wScore &&
			this->stScoreRankInfo.m_ullTimeStampMs < a.stScoreRankInfo.m_ullTimeStampMs)
		{
			return true;
		}
		return false;
	}

	bool operator==(const GFightRankNode& a) const
	{
		return this->stScoreRankInfo.m_wScore == a.stScoreRankInfo.m_wScore;
	}

    uint64_t& GetKey()
    {
        return stScoreRankInfo.m_ullGuildId;
    }
    void SetRankInfo(PKGMETA::DT_GFIGHT_RANK_INFO stRankInfo)
    {
        stScoreRankInfo = stRankInfo;
    }
    PKGMETA::DT_GFIGHT_RANK_INFO& GetRankInfo()
    {
        return stScoreRankInfo;
    }

    TdrError::ErrorType pack(char* buffer, size_t size, size_t* usedSize = NULL, unsigned cutVer = 0) const
    {
        return stScoreRankInfo.pack(buffer, size, usedSize, cutVer);
    }
    TdrError::ErrorType unpack(const char* buffer, size_t size, size_t* usedSize = NULL, unsigned cutVer = 0)
    {
        return stScoreRankInfo.unpack(buffer, size, usedSize, cutVer);
    }

};

struct GFightScoreCompare
{
	int operator() ( const GFightRankNode & stLeft, const GFightRankNode & stRight) const
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
	int operator() (const uint64_t iLeft, const uint64_t  iRight) const
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

class GFightRankMgr : public TSingleton<GFightRankMgr>
{
public:
	GFightRankMgr() {};
	virtual ~GFightRankMgr() { m_RankSys.OnExit();}
	bool Init();
	void Fini(){ m_RankSys.OnExit(); }
	int UpdateRank(DT_GFIGHT_RANK_INFO& rstRankInfo);
	int GetTopList(DT_GFIGHT_RANK_INFO* pstTopList);
 	void Delete(uint64_t ullGuildId);
// 	void RewardRank();
// 	void Reset();

	void UpdateGFightRankNtf(Guild* poGuild, DT_GFIGHT_RANK_INFO& rstGFightRankInfo);
private:
	PKGMETA::SSPKG m_stSsPkg;
	int m_iTopMaxNum;			//排行榜最大数量
	GFightRankNode m_stRankNode;
	vector<GFightRankNode> m_TopVector;
	RankSys<uint64_t, GFightRankNode, GFightScoreCompare> m_RankSys;
};

