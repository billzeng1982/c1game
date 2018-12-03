#pragma once
#include "../cfg/RankSvrCfgDesc.h"
#include "MsgBase.h"
#include "RankSys.h"
#include "../RankSvr.h"
#include <vector>
#include "ss_proto.h"
#include "common_proto.h"
#include "RankBase.h"
#include "../gamedata/GameDataMgr.h"


struct Rank6V6Node
{
    PKGMETA::DT_RANK_INFO m_stScoreRankInfo;

    bool operator < (const Rank6V6Node & a) const
    {
        if (this->m_stScoreRankInfo.m_dwScore > a.m_stScoreRankInfo.m_dwScore)
        {
            return true;
        }
        else if ((this->m_stScoreRankInfo.m_dwScore == a.m_stScoreRankInfo.m_dwScore)
            && this->m_stScoreRankInfo.m_dwEffiency > a.m_stScoreRankInfo.m_dwEffiency)
        {
            return true;
        }
        else if ((this->m_stScoreRankInfo.m_dwScore == a.m_stScoreRankInfo.m_dwScore)
            && (this->m_stScoreRankInfo.m_dwEffiency == a.m_stScoreRankInfo.m_dwEffiency)
            && (this->m_stScoreRankInfo.m_ullTimeStampMs < a.m_stScoreRankInfo.m_ullTimeStampMs))
        {
            return true;
        }

        return false;
    }

    bool operator==(const Rank6V6Node& a) const
    {
        return (this->m_stScoreRankInfo.m_dwScore == a.m_stScoreRankInfo.m_dwScore &&
            this->m_stScoreRankInfo.m_dwEffiency == a.m_stScoreRankInfo.m_dwEffiency &&
            this->m_stScoreRankInfo.m_ullTimeStampMs == a.m_stScoreRankInfo.m_ullTimeStampMs);
    }
    uint64_t GetKey()
    {
        return m_stScoreRankInfo.m_ullUin;
    }
    void SetRankInfo(PKGMETA::DT_RANK_INFO& rstRankInfo) { m_stScoreRankInfo = rstRankInfo; }

    PKGMETA::DT_RANK_INFO& GetRankInfo()
    {
        return m_stScoreRankInfo;
    }

    TdrError::ErrorType pack(char* buffer, size_t size, size_t* usedSize = NULL, unsigned cutVer = 0) const
    {
        return m_stScoreRankInfo.pack(buffer, size, usedSize, cutVer);
    }
    TdrError::ErrorType unpack(const char* buffer, size_t size, size_t* usedSize = NULL, unsigned cutVer = 0)
    {
        return m_stScoreRankInfo.unpack(buffer, size, usedSize, cutVer);
    }
};

struct RankRoleNode
{
    PKGMETA::DT_RANK_ROLE_INFO m_stScoreRankInfo;

    bool operator < (const RankRoleNode & a) const
    {
        if (this->m_stScoreRankInfo.m_dwValue > a.m_stScoreRankInfo.m_dwValue)
        {
            return true;
        }
        else if ((this->m_stScoreRankInfo.m_dwValue == a.m_stScoreRankInfo.m_dwValue)
            && (this->m_stScoreRankInfo.m_ullTimeStampMs < a.m_stScoreRankInfo.m_ullTimeStampMs))
        {
            return true;
        }

        return false;
    }

    bool operator==(const RankRoleNode& a) const
    {
        return (this->m_stScoreRankInfo.m_dwValue == a.m_stScoreRankInfo.m_dwValue &&
            this->m_stScoreRankInfo.m_ullTimeStampMs == a.m_stScoreRankInfo.m_ullTimeStampMs);
    }
    uint64_t GetKey()
    {
        return m_stScoreRankInfo.m_ullUin;
    }
    void SetRankInfo(PKGMETA::DT_RANK_ROLE_INFO& rstRankInfo) { m_stScoreRankInfo = rstRankInfo; }

    PKGMETA::DT_RANK_ROLE_INFO& GetRankInfo()
    {
        return m_stScoreRankInfo;
    }

    TdrError::ErrorType pack(char* buffer, size_t size, size_t* usedSize = NULL, unsigned cutVer = 0) const
    {
        return m_stScoreRankInfo.pack(buffer, size, usedSize, cutVer);
    }

    TdrError::ErrorType unpack(const char* buffer, size_t size, size_t* usedSize = NULL, unsigned cutVer = 0)
    {
        return m_stScoreRankInfo.unpack(buffer, size, usedSize, cutVer);
    }
};

struct RankDailyChallengeNode
{
    PKGMETA::DT_DAILY_CHALLENGE_RANK_INFO m_stScoreRankInfo;

    bool operator < (const RankDailyChallengeNode & a) const
    {
        if (this->m_stScoreRankInfo.m_dwScore > a.m_stScoreRankInfo.m_dwScore)
        {
            return true;
        }
        else if ((this->m_stScoreRankInfo.m_dwScore == a.m_stScoreRankInfo.m_dwScore)
            && this->m_stScoreRankInfo.m_ullTimeStampMs < a.m_stScoreRankInfo.m_ullTimeStampMs)
        {
            return true;
        }

        return false;
    }

    bool operator==(const RankDailyChallengeNode& a) const
    {
        return (this->m_stScoreRankInfo.m_dwScore == a.m_stScoreRankInfo.m_dwScore &&
            this->m_stScoreRankInfo.m_ullTimeStampMs == a.m_stScoreRankInfo.m_ullTimeStampMs);
    }

    uint64_t GetKey()
    {
        return m_stScoreRankInfo.m_ullUin;
    }
    void SetRankInfo(PKGMETA::DT_DAILY_CHALLENGE_RANK_INFO& rstRankInfo) { m_stScoreRankInfo = rstRankInfo; }
    PKGMETA::DT_DAILY_CHALLENGE_RANK_INFO& GetRankInfo()
    {
        return m_stScoreRankInfo;
    }

    TdrError::ErrorType pack(char* buffer, size_t size, size_t* usedSize = NULL, unsigned cutVer = 0) const
    {
        return m_stScoreRankInfo.pack(buffer, size, usedSize, cutVer);
    }
    TdrError::ErrorType unpack(const char* buffer, size_t size, size_t* usedSize = NULL, unsigned cutVer = 0)
    {
        return m_stScoreRankInfo.unpack(buffer, size, usedSize, cutVer);
    }

};

//  武将排行榜复合Key
struct RankGCardKey
{
    uint64_t m_ullUin;
    uint32_t m_dwSeq;
    bool operator < (const RankGCardKey & a) const
    {
        if (this->m_ullUin < a.m_ullUin)
        {
            return true;
        }
        else if ((this->m_ullUin == a.m_ullUin)
            && (this->m_dwSeq < a.m_dwSeq))
        {
            return true;
        }

        return false;
    }

    bool operator == (const RankGCardKey& a) const
    {
        return (this->m_ullUin == a.m_ullUin &&
            this->m_dwSeq == a.m_dwSeq);
    }
    RankGCardKey& operator = (const RankGCardKey& a)
    {
        this->m_ullUin = a.m_ullUin;
        this->m_dwSeq = a.m_dwSeq;
        return *this;
    }
};
struct RankGCardNode
{
    RankGCardKey m_stKey;
    PKGMETA::DT_RANK_ROLE_INFO m_stScoreRankInfo;
    bool operator < (const RankGCardNode & a) const
    {
        if (this->m_stScoreRankInfo.m_dwValue > a.m_stScoreRankInfo.m_dwValue)
        {
            return true;
        }
        else if ((this->m_stScoreRankInfo.m_dwValue == a.m_stScoreRankInfo.m_dwValue)
            && (this->m_stScoreRankInfo.m_ullTimeStampMs < a.m_stScoreRankInfo.m_ullTimeStampMs))
        {
            return true;
        }

        return false;
    }

    bool operator==(const RankGCardNode& a) const
    {
        return (this->m_stScoreRankInfo.m_dwValue == a.m_stScoreRankInfo.m_dwValue &&
            this->m_stScoreRankInfo.m_ullTimeStampMs == a.m_stScoreRankInfo.m_ullTimeStampMs);
    }
    RankGCardKey& GetKey()
    {
        return m_stKey;
    }
    void SetRankInfo(PKGMETA::DT_RANK_ROLE_INFO stRankInfo)
    {
        m_stScoreRankInfo = stRankInfo;
        m_stKey.m_ullUin = m_stScoreRankInfo.m_ullUin;
        m_stKey.m_dwSeq = m_stScoreRankInfo.m_astGeneralList[0].m_dwGeneralID;

    }
    PKGMETA::DT_RANK_ROLE_INFO& GetRankInfo()
    {
        return m_stScoreRankInfo;
    }

    TdrError::ErrorType pack(char* buffer, size_t size, size_t* usedSize = NULL, unsigned cutVer = 0) const
    {
        return m_stScoreRankInfo.pack(buffer, size, usedSize, cutVer);
    }
    TdrError::ErrorType unpack(const char* buffer, size_t size, size_t* usedSize = NULL, unsigned cutVer = 0)
    {
        TdrError::ErrorType iRet = m_stScoreRankInfo.unpack(buffer, size, usedSize, cutVer);
        m_stKey.m_ullUin = m_stScoreRankInfo.m_ullUin;
        m_stKey.m_dwSeq = m_stScoreRankInfo.m_astGeneralList[0].m_dwGeneralID;
        return iRet;
    }
};


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

struct PeakArenaRankNode
{
	DT_PEAK_ARENA_RANK_INFO stScoreRankInfo;
	bool operator < (const PeakArenaRankNode & a ) const
	{
		if (this->stScoreRankInfo.m_dwScore > a.stScoreRankInfo.m_dwScore)
		{
			return true;
		}
		else if (this->stScoreRankInfo.m_dwScore == a.stScoreRankInfo.m_dwScore &&
			this->stScoreRankInfo.m_wWinCount > a.stScoreRankInfo.m_wWinCount)
		{
			return true;
		}
		else if (this->stScoreRankInfo.m_dwScore == a.stScoreRankInfo.m_dwScore &&
			this->stScoreRankInfo.m_wWinCount == a.stScoreRankInfo.m_wWinCount &&
			this->stScoreRankInfo.m_ullTimeStampMs < a.stScoreRankInfo.m_ullTimeStampMs)
		{
			return true;
		}

		return false;
	}

	bool operator==(const PeakArenaRankNode& a) const
	{
		return this->stScoreRankInfo.m_dwScore == a.stScoreRankInfo.m_dwScore &&
			this->stScoreRankInfo.m_wWinCount == a.stScoreRankInfo.m_wWinCount &&
			this->stScoreRankInfo.m_ullTimeStampMs == a.stScoreRankInfo.m_ullTimeStampMs;
	}

	uint64_t& GetKey()
	{
		return stScoreRankInfo.m_ullUin;
	}
	void SetRankInfo(PKGMETA::DT_PEAK_ARENA_RANK_INFO stRankInfo)
	{
		stScoreRankInfo = stRankInfo;
	}
	PKGMETA::DT_PEAK_ARENA_RANK_INFO& GetRankInfo()
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


class RankMgr: public TSingleton<RankMgr>
{
    //PVP排行榜
    typedef RankBase<uint64_t, Rank6V6Node, PKGMETA::DT_RANK_INFO, RankCompare<uint64_t, Rank6V6Node> > CRank6V6;
    //队伍战力排行榜
    typedef RankBase<uint64_t, RankRoleNode, PKGMETA::DT_RANK_ROLE_INFO, RankCompare<uint64_t, RankRoleNode> > CRankLiBase;
    //关卡星星排行榜
    typedef RankBase<uint64_t, RankRoleNode, PKGMETA::DT_RANK_ROLE_INFO, RankCompare<uint64_t, RankRoleNode> > CRankPveStar;
    //武将获得数量排行榜
    typedef RankBase<uint64_t, RankRoleNode, PKGMETA::DT_RANK_ROLE_INFO, RankCompare<uint64_t, RankRoleNode> > CRankGCardCnt;
    //每日挑战排行榜
    typedef RankBase<uint64_t, RankDailyChallengeNode, PKGMETA::DT_DAILY_CHALLENGE_RANK_INFO, RankCompare<uint64_t, RankDailyChallengeNode> > CRankDailyChallengeBase;
    //武将战力排行榜
    typedef RankBase<RankGCardKey, RankGCardNode, PKGMETA::DT_RANK_ROLE_INFO, RankCompare<RankGCardKey, RankGCardNode> > CRankGCardLi;
	//公会榜
	typedef RankBase<uint64_t, GuildRankNode, PKGMETA::DT_GUILD_RANK_INFO, RankCompare<uint64_t, GuildRankNode> > CRankGuild;
	//公会战榜
	typedef RankBase<uint64_t, GFightRankNode, PKGMETA::DT_GFIGHT_RANK_INFO, RankCompare<uint64_t, GFightRankNode> > CRankGFight;
	//巅峰匹配排行榜
	typedef RankBase<uint64_t, PeakArenaRankNode, PKGMETA::DT_PEAK_ARENA_RANK_INFO, RankCompare<uint64_t, PeakArenaRankNode> > CRankPeakArenaBase;

    class CRankDailyChallenge : public CRankDailyChallengeBase
    {
    private:
        const static int DAILY_RANK_SETTLE_MAIL_ID = 10007;
        const static int DAILY_RANK_SETTLE_MAIL1_ID = 10032;
    public:
        void SettleRank(bool bUseBak = false);

	private:
		void _SettleRank(vector<uint64_t>& UinList);

    private:
        SSPKG m_stSsPkg;
    };

    class CRankLi : public CRankLiBase
    {
    private:
        const static int LI_RANK_SETTLE_MAIL_ID = 10009;
        const static int LI_RANK_CONSOLATION_MAIL_ID = 10010;

    public:
        void SettleRank(bool bUseBak = false);

    private:
		void _SettleRank(vector<uint64_t>& UinList);

    private:
        SSPKG m_stSsPkg;
    };

	class CRankPeakArena : public CRankPeakArenaBase
	{
	private:
        const static int PEAK_ARENA_SETTLE_MAIL_ID = 10033;

    public:
        void SettleRank(bool bUseBak = false);

    private:
		void _SettleRank(vector<uint64_t>& UinList);

    private:
        SSPKG m_stSsPkg;
	};

public:
    RankMgr() {};
    ~RankMgr() {};
    bool Init();
    void OnExit();

public:
    CRank6V6& GetRank6V6() { return m_oRank6V6; }
    CRankLi& GetRankLi() { return m_oRankLi; }
    CRankGCardLi& GetRankGCardLi() { return m_oRankGCardLi; }
    CRankPveStar& GetRankPveStar() { return m_oRankPveStar; }
    CRankGCardCnt& GetRankGCardCnt() { return m_oRankGCardCnt; }
    CRankDailyChallenge& GetRankDailyChallenge() { return m_oRankDailyChallenge; }
	CRankGuild& GetRankGuild()	{ return m_oRankGuild; }
	CRankGFight& GetRankGFight()	{ return m_oRankGFight; }
    CRankPeakArena& GetRankPeakArena() { return m_oRankPeakArena; }

	//备份排行榜
	void BackUpRankList(const char* szFileName, vector<uint64_t>& UinList);
	//从备份中读排行
	int ReadRankFromBackUp(const char* szFileName, vector<uint64_t>& UinList);

private:
    CRank6V6 m_oRank6V6;
    CRankLi m_oRankLi;
    CRankGCardLi m_oRankGCardLi;
    CRankPveStar m_oRankPveStar;
    CRankGCardCnt m_oRankGCardCnt;
    CRankDailyChallenge m_oRankDailyChallenge;
	CRankGuild m_oRankGuild;
	CRankGFight m_oRankGFight;
	CRankPeakArena m_oRankPeakArena;
};

