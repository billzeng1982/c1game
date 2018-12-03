#pragma once

#include "utils/singleton.h"

#include "Guild.h"
#include "Player.h"


class LogicMgr: public TSingleton<LogicMgr>
{

public:
	LogicMgr() {};
	~LogicMgr() {};
	bool Init();
	void Update();
	void Fini();

	//获取公会所有信息
	int GetGuildAllInfo(uint64_t ullGuildId, OUT PKGMETA::DT_GUILD_EXPEDITION_ALL_INFO& rstInfo);


	//上传更新公会信息
	void UploadGuildInfo(PKGMETA::DT_GUILD_EXPEDITION_GUILD_UPLOAD_INFO& rstInfo);

	//设置战斗信息
	void SetFightInfo(uint64_t ullUin, uint64_t ullGuildId, uint32_t dwAddr, 
		PKGMETA::DT_GUILD_EXPEDITION_FIGHT_INFO& m_stFightInfo, PKGMETA::DT_GUILD_EXPEDITION_PLAYER_SHOW_INFO& rstShowInfo);

	//战斗请求
	int FightRequest(uint64_t ullUin, uint64_t ullGuildId, uint64_t ullFoeUin, uint64_t ullFoeGuildId,
		OUT PKGMETA::DT_GUILD_EXPEDITION_FIGHT_INFO& rstFightInfo);

	//战斗结果处理
	int FightResult(uint64_t ullUin, uint64_t ullGuildId, uint8_t bIsWin, uint64_t ullFoeUin, uint64_t ullFoeGuildId);

	//公会解散
	void GuildDissovle(uint64_t ullGuildId);

	//公会成员退出
	void GuildMemberQuit(uint64_t ullGuildId, uint64_t ullUin);

public:
	uint64_t m_ullLastMatchTime;
	
	int m_iDefeatDamageHp;		//击破的血量
	uint8_t m_bWinDamageHp;		//胜利扣的血量
	uint8_t m_bLoseDamageHp;	//失败扣的血量

	int m_iLeastDefendNum;		//最少设置防御阵容的玩家数量 才能参加匹配
	int m_iPeriodDefeatRate;		//阶段性击破人数百分比



private:
	int		m_iDailyUpdateHour;	//每天更新时间
};

