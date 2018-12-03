#pragma once

#include <stdio.h>
#include <map>
#include "singleton.h"
#include "ss_proto.h"
#include "tsec/taes.h"
#include "FileData.h"

using namespace PKGMETA;
using namespace std;

class GuildBossMgr : public TSingleton<GuildBossMgr>
{
public:
	GuildBossMgr() {}
	~GuildBossMgr()	{}

	bool Init(const char* pszFileName);

	void Update();

	void Fini();

	void GetState(DT_GUILD_BOSS_MODULE_STATE& rstStateInfo);

	bool IsOpen();

    //查看军团Boss通关军团排行
    DT_GUILD_BOSS_PASSED_GUILD* GetGuildBossPassedGuildInfo(int iBossIndex);

    void SetPassedGuildInfo(uint64_t ullGuildId, int iBossIndex);

    void ResetPassedGuild(int iBossIndex) { m_oGuildBossFile.m_oData.m_astPassGuildRank[iBossIndex].m_bCount = 0; }

	uint32_t FindBossID( uint32_t dwFLevelID );

	uint32_t GetBossMaxHp(uint32_t dwBossID);

    void GetSubsectionAward(DT_GUILD_ONE_BOSS_INFO stGuildOneBossInfo, OUT int &iCount, OUT uint64_t *ullRet);

    int GetSubsectionRwdTypeNum(uint32_t dwDamageHp, uint32_t dwBossId);

    //根据BossIndex获取当前有多少军团通过了这个Boss
    int GetPassedGuildNum(uint8_t bBossIndex){ return m_oGuildBossFile.m_oData.m_PassedGuildNum[bBossIndex]; }

    //根据BossIndex设置通过该Boss的军团数量
    void SetPassedGuildNum(uint8_t bBossIndex) { ++m_oGuildBossFile.m_oData.m_PassedGuildNum[bBossIndex]; }

    //重置所有Boss的军团通过数量
    void ResetPassedGuildNum(uint8_t bBossNum);

    //int GetGuildRank(uint64_t ullGuildId);

    //void GetCompetitorInfo(uint64_t ullGuildId, DT_GUILD_RANK_INFO *pstCompetitorInfo);

    //int SetGuildRankList(DT_GUILD_RANK_INFO* pstGuildRankInfo, uint16_t wCount);
private:
	//第一次初始化，开服时的初始化
	bool _FirstInit();

	//从文件初始化
	bool _InitFromFile();

	//将进度保存到文件中
	bool _SaveSchedule();

	// 初始化一些boss的固定信息
	void _InitBossFixedInfo();

    //根据伤害和BossId获取分段奖励数量
    int _GetSubsectionRwdTypeNum(uint32_t dwDamageHp, uint32_t dwBossId);
	
private:
	//FILE* m_fp;

	uint64_t m_ullTimeSec;

	//存储的文件名
	char m_szFileName[MAX_NAME_LENGTH];

	//军团boss开放时间，供客户端显示使用，单位S
	uint64_t m_ullChgStateTime;

	bool bIsOpen;

    //军团排行榜中有多少军团
    //uint32_t m_wCount;

    //通关Boss的军团数量（展示用，最大值为10）
    uint8_t m_bPassedGuildNum;

    bool m_bIsFileExist;

    DT_GUILD_BOSS_PASSED_GUILD m_szPassGuildInfo[MAX_GUILD_BOSS_NUM];
    //DT_GUILD_RANK_INFO m_szGuildRankInfo[MAX_LEN_GUILD_MEMBER];

    uint32_t m_astPassedGuildNum[MAX_GUILD_BOSS_NUM];

    //如果需要增加GuildBoss相关的数据，在FD_GUILD_BOSS里面增加
    FileData<FD_GUILD_BOSS> m_oGuildBossFile;

	std::map<uint32_t, uint32_t> m_FLevelToBossIdMap;   // <副本ID, BossId>
	std::map<uint32_t, uint32_t>  m_BossId2MaxHpMap;
};
