#pragma once

#include "singleton.h"
#include "common_proto.h"
#include <map>
#include <list>

#define MAX_CHOOSE_NODE_NUM 6

using namespace PKGMETA;
using namespace std;

class ChooseMgr : public TSingleton<ChooseMgr>
{
public:
	typedef list<uint32_t> ListSkin_t;
	typedef map<uint32_t, ListSkin_t*> MapId2SkinList_t;

	struct ChooseNode
	{
		int m_bTalent;
		int m_iChooseCnt;
	};

	struct GeneralNode
	{
		DT_RANK_GENERAL_INFO m_stGeneralInfo;
		uint8_t m_bTalent;
	};

public:
	static int GeneralTalentCmp(const void *pstFirst, const void *pstSecond);

public:
	ChooseMgr();
	~ChooseMgr();

	bool Init();
	void Update();

	int GetGeneralChooseList(uint8_t& bGeneralCnt, DT_RANK_GENERAL_INFO GeneralList[]);

	int GetMSkillChooseList(uint8_t& bMSkillCnt, DT_ITEM_MSKILL GeneralList[]);

	ListSkin_t* GetSkinListByGeneral(uint32_t dwGeneral);

private:
	bool _InitMSkillList();
	bool _InitGeneralList();
	bool _InitChooseNode();
	bool _InitSeason();
	bool _InitRuleList();
	bool _InitSkinList();

	void _AddGeneral(uint8_t& bGeneralCnt, DT_RANK_GENERAL_INFO GeneralList[], DT_RANK_GENERAL_INFO& rstGeneralInfo);

	void _RDChoose(uint8_t& bGeneralCnt, DT_RANK_GENERAL_INFO GeneralList[], int& iIndex, ChooseNode& rstChooseNode);

private:
	//当前的RuleId
	uint8_t m_bCurRuleId;

	//是否是RD模式
	uint8_t m_bIsRD;
	ChooseNode m_astChooseList[MAX_CHOOSE_NODE_NUM];

	uint32_t m_RandomNum[MAX_NUM_GCARD_FOR_CHOOSE];

	uint8_t m_bGeneralCnt;
	GeneralNode m_GeneralList[MAX_NUM_GCARD_FOR_CHOOSE];

	uint8_t m_bMSkillCnt;
	DT_ITEM_MSKILL m_MSkillList[MAX_NUM_ROLE_MSKILL];

	//赛季开始时间和结束时间
	uint64_t m_ullSeasonStartTime;
	uint64_t m_ullSeasonEndTime;
	//赛季持续月份
	uint8_t m_bSeasonLastMonth;

	//武将和皮肤对应map
	MapId2SkinList_t m_mapId2SkinList;
};

