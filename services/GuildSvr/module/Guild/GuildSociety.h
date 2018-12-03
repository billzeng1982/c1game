#pragma once

#include "common_proto.h"

using namespace std;
using namespace PKGMETA;

class GuildSociety
{
public:
	GuildSociety() {};
	~GuildSociety()	{};
	bool InitFromDB(DT_GUILD_SOCIETY_BLOB& rstBlob, uint64_t ullGuildId, uint16_t wVersion);
	bool PackGuildSocietyInfo(DT_GUILD_SOCIETY_BLOB& rstBlob, uint16_t wVersion);
	void Clear();
	//  ��ʼBoss�����Ϣ
	bool Init();
	bool LoadGameData();

	//�����Ƽ���֧�ȼ���typeΪGUILD_SOCIETY_TYPE
	int AddExp(uint8_t bType, uint32_t dwValueExp);

	int GetValue(uint8_t bType);

	//�Ƿ���ʱ��������������
	bool IsLvMax(uint8_t bType);

	int DonateExp();

	uint8_t GetLv(uint8_t bType)	{ return m_oSocietyInfo.m_astBranches[bType-1].m_bLv; }
	uint32_t GetExp(uint8_t bType)	{ return m_oSocietyInfo.m_astBranches[bType-1].m_dwExp; }

private:
	DT_GUILD_SOCIETY_INFO m_oSocietyInfo;
	uint64_t m_ullGuildId;


};
