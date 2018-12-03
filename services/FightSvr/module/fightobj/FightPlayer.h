#pragma  once
#include <list>
#include "cs_proto.h"
#include "ov_res_public.h"
#include "FightObj.h"
#include "../skill/MasterSkill.h"
#include "../skill/Tactics.h"

using namespace PKGMETA;

class FightPlayer : public FightObj
{
public:
	FightPlayer();
	virtual ~FightPlayer();
	virtual void Clear();

private:
	void _Construct();

public:
	static FightPlayer* Get();
	static void Release(FightPlayer* pObj);

public:
	bool Init(Dungeon* poDungeon, DT_FIGHT_PLAYER_INFO& rPlayerInfo);

public:
	void Update(int iDeltaTime);
	float _GetMoraleIncSpeed();
	float GetMorale();
	float GetMoraleForPreUpdate();
	void ChgMorale(float fMorale);
    bool IsGeneralInTeam(uint32_t dwGeneralId);

    //选择武将
	DT_TROOP_INFO* ChooseGeneral(uint32_t dwGeneralId);

	//系统随机选择武将
	DT_TROOP_INFO* RandomChooseGenaral();

	//选择军师技
	MasterSkill* ChooseMasterSkill(uint32_t dwMSkillId);

	//系统随机选择军师技
	MasterSkill* RandomChooseMSkill();

public:
	uint64_t m_ullUin;		//战场初始化时赋值
	char m_szName[MAX_NAME_LENGTH]; // 玩家姓名
	Player* m_poPlayer;		// 用户登录成功后赋值
	bool m_bReady;			// 客户端场景加载成功时赋值
	int m_iZoneSvrProcId;

	DT_FIGHT_PLAYER_INFO  m_stPlayerInfo;
	DT_DUNGEON_TASK_INFO m_stTaskInfo;

	float m_fMoraleMax;			// 士气上限
	float m_fMorale;			// 当前士气
	float m_fMoraleIncSpeed;	// 士气增长速度 阶段0
	float m_fMoraleIncSpeed1;	// 士气增长速度 阶段1
	float m_fMoraleIncSpeed2;	// 士气增长速度 阶段2

	int m_iMoraleSpeedUpTime1;	// 士气增长阶段1 时间点
	int m_iMoraleSpeedUpTime2;	// 士气增长阶段2 时间点

	// 军师技
	MasterSkill* m_poMasterSkill;

	//栅栏数
	uint32_t m_dwBarrierCount;
};

