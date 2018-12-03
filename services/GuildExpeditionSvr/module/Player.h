#pragma once


#include "comm/tlist.h"
#include "common_proto.h"

class Player
{
public:
	Player() {};
	~Player() {};
	bool InitFromDB(PKGMETA::DT_GUILD_EXPEDITION_PLAYER_DATA& rstData);
	bool PackToData(PKGMETA::DT_GUILD_EXPEDITION_PLAYER_DATA& rstData);

	//更新信息
	void UptInfo(uint32_t dwAddr, uint64_t ullGuildId, PKGMETA::DT_GUILD_EXPEDITION_FIGHT_INFO& rstInfo, PKGMETA::DT_GUILD_EXPEDITION_PLAYER_SHOW_INFO& rstShowInfo);

	//创建初始化
	void CreateInit(uint64_t ullUin, uint32_t dwAddr, uint64_t ullGuildId, PKGMETA::DT_GUILD_EXPEDITION_FIGHT_INFO& rstInfo, PKGMETA::DT_GUILD_EXPEDITION_PLAYER_SHOW_INFO& rstShowInfo);

	//重置
	void Reset();

	uint32_t GetHighestLi() { return m_stPlayerInfo.m_stShowInfo.m_dwHigheastLi; }

	//获取Uin
	uint64_t GetUin() { return m_stPlayerInfo.m_ullUin; };

	//获取展示信息
	void GetShowInfo(OUT PKGMETA::DT_GUILD_EXPEDITION_PLAYER_SHOW_INFO& rstInfo);

	//获取战斗信息
	void GetFightInfo(OUT PKGMETA::DT_GUILD_EXPEDITION_FIGHT_INFO& rstInfo);

	//获取昵称
	const char* GetName() { return m_stPlayerInfo.m_stShowInfo.m_szName; }
private:
	PKGMETA::DT_GUILD_EXPEDITION_PLAYER_INFO m_stPlayerInfo;
public:
	TLISTNODE                                   m_stDirtyListNode;          //脏数据链表头,用于回写数据库
	TLISTNODE                                   m_stTimeListNode;           //时间链表头,用于LRU算法
};


