#pragma once

#include <map>
#include "common_proto.h"
#include "define.h"

using namespace std;
using namespace PKGMETA;

class GuildMember
{
public:
    GuildMember();
    virtual ~GuildMember(){};

    bool InitFromDB(DT_GUILD_MEMBER_BLOB& rstBlob, uint64_t ullGuildId, int iMaxNum, int* pDeputyNum, int* pEliteNum, uint16_t wVersion);
    bool PackGuildMemberInfo(DT_GUILD_MEMBER_BLOB& rstBlob, uint16_t wVersion);
    void Clear();

    int Add(DT_ONE_GUILD_MEMBER& rstOneMember);
    int Del(uint64_t ullUin);
    DT_ONE_GUILD_MEMBER* Find(uint64_t ullUin);

    int GetCurMemberNum() { return m_iCount; }
    void SetMaxMemberNum(int iMaxNum) { m_iMaxNum = iMaxNum; }
    bool IsDeputy( uint64_t ullUin );
    //获取成员列表
    uint64_t* GetMemberList();

    //获取公会中所有成员的星星数总和
    uint32_t GetStarSumNum();

    //获取公会中所有成员的星星数总和
    uint32_t GetLiSum();

	//清空活跃度
    void ClearVitality();

	//发送活跃度奖励
	void SendVitalityReward();

	//获取活跃度最高的成员
	DT_ONE_GUILD_MEMBER*  GetNextLeader();

	//获取军团远征成员信息
	void GetGuildExpeditionMemberInfo(OUT int8_t& chCount, OUT DT_GUILD_EXPEDITION_ONE_MEMBER_INFO* pstInfoList);

	//军团远征结算
	void SettleExpeditionAward(uint8_t bAwardType);

	//更新防守信息
	int UpdateDefendInfo(DT_GUILD_PKG_EXPEDITION_UPDATE_DEFEND_INFO& rstDefendInfo);
private:
    int _FindMember(uint64_t ullUin);


private:
    int m_iCount;
    int m_iMaxNum;
    uint64_t m_ullGuildId;
    uint64_t m_MemberList[MAX_NUM_MEMBER];
    map<uint64_t, DT_ONE_GUILD_MEMBER*> m_oUinToMemberMap;
    map<uint64_t, DT_ONE_GUILD_MEMBER*>::iterator m_oUinToMemberMapIter;
};

