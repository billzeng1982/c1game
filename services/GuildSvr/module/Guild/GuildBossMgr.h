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

    //�鿴����Bossͨ�ؾ�������
    DT_GUILD_BOSS_PASSED_GUILD* GetGuildBossPassedGuildInfo(int iBossIndex);

    void SetPassedGuildInfo(uint64_t ullGuildId, int iBossIndex);

    void ResetPassedGuild(int iBossIndex) { m_oGuildBossFile.m_oData.m_astPassGuildRank[iBossIndex].m_bCount = 0; }

	uint32_t FindBossID( uint32_t dwFLevelID );

	uint32_t GetBossMaxHp(uint32_t dwBossID);

    void GetSubsectionAward(DT_GUILD_ONE_BOSS_INFO stGuildOneBossInfo, OUT int &iCount, OUT uint64_t *ullRet);

    int GetSubsectionRwdTypeNum(uint32_t dwDamageHp, uint32_t dwBossId);

    //����BossIndex��ȡ��ǰ�ж��پ���ͨ�������Boss
    int GetPassedGuildNum(uint8_t bBossIndex){ return m_oGuildBossFile.m_oData.m_PassedGuildNum[bBossIndex]; }

    //����BossIndex����ͨ����Boss�ľ�������
    void SetPassedGuildNum(uint8_t bBossIndex) { ++m_oGuildBossFile.m_oData.m_PassedGuildNum[bBossIndex]; }

    //��������Boss�ľ���ͨ������
    void ResetPassedGuildNum(uint8_t bBossNum);

    //int GetGuildRank(uint64_t ullGuildId);

    //void GetCompetitorInfo(uint64_t ullGuildId, DT_GUILD_RANK_INFO *pstCompetitorInfo);

    //int SetGuildRankList(DT_GUILD_RANK_INFO* pstGuildRankInfo, uint16_t wCount);
private:
	//��һ�γ�ʼ��������ʱ�ĳ�ʼ��
	bool _FirstInit();

	//���ļ���ʼ��
	bool _InitFromFile();

	//�����ȱ��浽�ļ���
	bool _SaveSchedule();

	// ��ʼ��һЩboss�Ĺ̶���Ϣ
	void _InitBossFixedInfo();

    //�����˺���BossId��ȡ�ֶν�������
    int _GetSubsectionRwdTypeNum(uint32_t dwDamageHp, uint32_t dwBossId);
	
private:
	//FILE* m_fp;

	uint64_t m_ullTimeSec;

	//�洢���ļ���
	char m_szFileName[MAX_NAME_LENGTH];

	//����boss����ʱ�䣬���ͻ�����ʾʹ�ã���λS
	uint64_t m_ullChgStateTime;

	bool bIsOpen;

    //�������а����ж��پ���
    //uint32_t m_wCount;

    //ͨ��Boss�ľ���������չʾ�ã����ֵΪ10��
    uint8_t m_bPassedGuildNum;

    bool m_bIsFileExist;

    DT_GUILD_BOSS_PASSED_GUILD m_szPassGuildInfo[MAX_GUILD_BOSS_NUM];
    //DT_GUILD_RANK_INFO m_szGuildRankInfo[MAX_LEN_GUILD_MEMBER];

    uint32_t m_astPassedGuildNum[MAX_GUILD_BOSS_NUM];

    //�����Ҫ����GuildBoss��ص����ݣ���FD_GUILD_BOSS��������
    FileData<FD_GUILD_BOSS> m_oGuildBossFile;

	std::map<uint32_t, uint32_t> m_FLevelToBossIdMap;   // <����ID, BossId>
	std::map<uint32_t, uint32_t>  m_BossId2MaxHpMap;
};
