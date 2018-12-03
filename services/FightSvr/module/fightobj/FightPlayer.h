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

    //ѡ���佫
	DT_TROOP_INFO* ChooseGeneral(uint32_t dwGeneralId);

	//ϵͳ���ѡ���佫
	DT_TROOP_INFO* RandomChooseGenaral();

	//ѡ���ʦ��
	MasterSkill* ChooseMasterSkill(uint32_t dwMSkillId);

	//ϵͳ���ѡ���ʦ��
	MasterSkill* RandomChooseMSkill();

public:
	uint64_t m_ullUin;		//ս����ʼ��ʱ��ֵ
	char m_szName[MAX_NAME_LENGTH]; // �������
	Player* m_poPlayer;		// �û���¼�ɹ���ֵ
	bool m_bReady;			// �ͻ��˳������سɹ�ʱ��ֵ
	int m_iZoneSvrProcId;

	DT_FIGHT_PLAYER_INFO  m_stPlayerInfo;
	DT_DUNGEON_TASK_INFO m_stTaskInfo;

	float m_fMoraleMax;			// ʿ������
	float m_fMorale;			// ��ǰʿ��
	float m_fMoraleIncSpeed;	// ʿ�������ٶ� �׶�0
	float m_fMoraleIncSpeed1;	// ʿ�������ٶ� �׶�1
	float m_fMoraleIncSpeed2;	// ʿ�������ٶ� �׶�2

	int m_iMoraleSpeedUpTime1;	// ʿ�������׶�1 ʱ���
	int m_iMoraleSpeedUpTime2;	// ʿ�������׶�2 ʱ���

	// ��ʦ��
	MasterSkill* m_poMasterSkill;

	//դ����
	uint32_t m_dwBarrierCount;
};

