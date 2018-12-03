#pragma once

#include <map>

#include "comm/tlist.h"
#include "common_proto.h"




class Guild
{
public:
	Guild() {};
	~Guild() {};

	uint64_t GetId();

	//从数据库初始化
	bool InitFromDB(PKGMETA::DT_GUILD_EXPEDITION_GUILD_DATA& rstData);

	//打包
	bool PackToData(PKGMETA::DT_GUILD_EXPEDITION_GUILD_DATA& rstData);

	//重置
	void Reset();

	//获取公会信息
	void GetInfo(OUT PKGMETA::DT_GUILD_EXPEDITION_GUILD_INFO& rstInfo);

	//更新公会信息	
	void UptInfo(PKGMETA::DT_GUILD_EXPEDITION_GUILD_UPLOAD_INFO& rstInfo);

	//创建初始公会信息
	void CreateInit(uint64_t ullGuildId, PKGMETA::DT_GUILD_EXPEDITION_GUILD_UPLOAD_INFO& rstInfo);

	//获取玩家需要的所有信息
	void GetAllInfo(OUT PKGMETA::DT_GUILD_EXPEDITION_ALL_INFO& rstInfo);

	//获取匹配信息
	void GetMatchedInfo(OUT PKGMETA::DT_GUILD_EXPEDITION_MATCHED_GUILD_INFO& rstInfo);

	//匹配公会
	int MatchGuild();

	//处理战斗结果
	int HandleFightResult(uint64_t ullUin, uint8_t bIsWin, uint64_t ullFoeGuildId, uint64_t ullFoeUin);

	//每日刷新
	void DailyUpdate();

	//修改已设置防守阵容的成员数量
	int AddDefendMemberNum(uint64_t ullUin, uint32_t iDeltali);

	//能否与该玩家战斗
	int CanFight(uint64_t ullFoeUin, uint64_t ullFoeGuildId);

	//查找成员
	PKGMETA::DT_GUILD_EXPEDITION_ONE_MEMBER_INFO* FindMember(uint64_t ullUin);

	//增加成员
	PKGMETA::DT_GUILD_EXPEDITION_ONE_MEMBER_INFO* AddMember(uint64_t ullUin);

	//删除成员
	void DelMember(uint64_t ullUin);

	//更新成员信息
	void UptMember(PKGMETA::DT_GUILD_EXPEDITION_ONE_MEMBER_INFO& rstMemberInfo);

	//获取被匹配时需要用的简要信息
	void GetMatchedBriefInfo(PKGMETA::DT_GUILD_EXPEDITION_MATCHED_GUILD_CLIENT_INFO& rstInfo);

	//增加战斗记录
	PKGMETA::DT_GUILD_EXPEDITION_FIGHT_LOG* AddFightLog(uint64_t ullUin, uint8_t bIsWin, uint64_t ullFoeUin);

	//获取公会战力
	uint64_t GetLi() { return m_stGuildInfo.m_ullGuildLi; }

	//更新公会防守信息
	void SendDefendInfo(uint64_t ullUin);

	//发送奖励
	void CheckSendAward(PKGMETA::DT_GUILD_EXPEDITION_MATCHED_GUILD_INFO* pstMatchedGuildInfo);

	//公会解散
	void Dissolve();

	//成员退出
	void MemberQuit(uint64_t ullUin);

	//查找已匹配到的公会信息
	PKGMETA::DT_GUILD_EXPEDITION_MATCHED_GUILD_INFO* FindMatchedGuild(uint64_t ullGuildId);


	PKGMETA::DT_GUILD_EXPEDITION_MATCHED_ONE_MEMBER_INFO* FindMatchedGuildMember(PKGMETA::DT_GUILD_EXPEDITION_MATCHED_GUILD_INFO* pstMatchedGuildInfo, uint64_t ullMatchedUin);
private:
	PKGMETA::DT_GUILD_EXPEDITION_GUILD_INFO m_stGuildInfo;

	int m_iTotalDefendReadyNum;							//本公会设置防守的人数

	int m_iTotalDefeatNum;								//击败的总人数
	int m_iTotalMatchedGuildMemberNum;					//匹配到的公会总人数
	std::map<uint64_t, int>  m_oGuildId2DefeatNumMap;			//每个匹配公会的击破人数

	int m_iUpdateAllDefendInfoFreq;						//全量更新防守信息频率控制器, 玩家设置一次+1

public:
	TLISTNODE                                   m_stDirtyListNode;          //脏数据链表头,用于回写数据库
	TLISTNODE                                   m_stTimeListNode;           //时间链表头,用于LRU算法
};

