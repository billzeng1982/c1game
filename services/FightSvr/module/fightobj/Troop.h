#pragma  once
#include <list>
#include "cs_proto.h"
#include "common_proto.h"
#include "ov_res_public.h"
#include "FightObj.h"
#include "General.h"
#include "../skill/Tactics.h"

// 除了网络协议数据会放大
// 客户端、服务端的数据都采用非放大的数据，方便统一处理

class Troop : public FightObj
{
public:
    static const float CONST_RATIO_PERMILLAGE = 0.001;
public:
    Troop();
    virtual ~Troop();
    virtual void Clear();

private:
    void _Construct();

public:
    static Troop* Get();
    static void Release(Troop* pObj);

public:
	static int GeneralInfoCmp(const void *pstFirst, const void *pstSecond);
	static int SkinCmp(const void *pstFirst, const void *pstSecond);

public:
    bool Init(Dungeon* poDungeon, FightPlayer* poFightPlayer, PKGMETA::DT_TROOP_INFO& rstTroopInfo);
    virtual void Update(int dt);

public:
    virtual void ChgHp(int iValueChgType, int iValueChgPara, FightObj* poSource, int iDamageRef, int& iHpChgBefore, int& iHpChgAfter, int& iDamageFxSrc, int& iDamageFxTar);
    virtual void AfterChgHp(int iValueChgType, int iValueChgPara, FightObj* poSource, int iHpChgBefore, int iHpChgAfter);

    bool HasSkill(uint32_t dwSkillId);

public:
	float GetActiveSkillBaseValue(int iParId);
	float GetActiveSkillRateValue(int iParId);

	float GetArmyBaseValueBase();
	float GetArmyBaseValueGrow();
	float GetArmyBaseValue();

private:
	bool _InitPeakArenaTroopData(PKGMETA::DT_TROOP_INFO& rstTroopInfo);
	bool _InitNormalTroopData(PKGMETA::DT_TROOP_INFO& rstTroopInfo);

	bool _InitPeakArenaTroopSkill(PKGMETA::DT_TROOP_INFO& rstTroopInfo);
	bool _InitNormalTroopSkill(PKGMETA::DT_TROOP_INFO& rstTroopInfo);

	bool _InitTactics(FightPlayer* poPlayer);

    void _InitRtData(FightObjRuntimeData& rRtData);
    void _InitPeakArenaRtData(FightObjRuntimeData& rRtData);

    void _AddValue(int& rOriginalData, uint8_t bType,  int iIncValue, int iPer);
    void _AddValue(float& rOriginalData, uint8_t bType,  float fIncValue, int iPer);

	void _AddSkinAttr(FightObjRuntimeData& rRtData, uint32_t dwSkinId);
    void _AddGeneralLevel(FightObjRuntimeData& rRtData); //武将的升级属性加成
    void _AddGeneralStar(FightObjRuntimeData& rRtData); //武将的星级属性加成
    void _AddGeneralPhase(FightObjRuntimeData& rRtData); //武将的阶属性加成
    void _AddGeneralGem(FightObjRuntimeData& rRtData, PKGMETA::DT_TROOP_INFO& rstTroopInfo);  //宝石和前台一致的计算流程
    void _AddGeneralFateAttr(FightObjRuntimeData& rRtData, OUT float attrValue[], PKGMETA::DT_TROOP_INFO& rstTroopInfo);
	void _AddEquipFateAttr(FightObjRuntimeData& rRtData, OUT float attrValue[], PKGMETA::DT_TROOP_INFO& rstTroopInfo);
	void _AddFeedTrainAtrr(FightObjRuntimeData& rRtData, PKGMETA::DT_TROOP_INFO& rstTroopInfo);
    void _AddEquipAttr(FightObjRuntimeData& rRtData, PKGMETA::DT_TROOP_INFO& rstTroopInfo);//武将的装备属性加成
    void _AddEquipBase(FightObjRuntimeData& rRtData, PKGMETA::DT_ITEM_EQUIP& rstEquipInfo);
    void _AddEquipStar(FightObjRuntimeData& rRtData, PKGMETA::DT_ITEM_EQUIP& rstEquipInfo);
	void _AddPeakArenaAttr(FightObjRuntimeData& rRtData, PKGMETA::DT_TROOP_INFO& rstTroopInfo);

    void _InitEquip(PKGMETA::DT_TROOP_INFO& rstTroopInfo);
    uint8_t _SetDelayDeadLastTime(int iValueChgType);

public:
    uint32_t m_dwGeneralId;		// 武将Id
    uint8_t m_bGeneralLevel;	// 武将等级，影响部队属性
    uint8_t m_bGeneralStar;		// 武将星级，影响被动技能开放
    uint8_t m_bGeneralPhase;	// 武将品阶，影响部队属性

    // 兵种属性
    int m_iArmyType;
    uint32_t m_iArmyId;					// 兵种ID，（同种类兵种，等级不同ID不同，图标不同,模型不同）
    uint8_t m_iArmyLevel;				// 兵种等级
    uint8_t m_iArmyPhase;               // 兵种品阶

    uint16_t m_bActiveSkillCheatsType;								//主动计能秘籍类型
    uint32_t m_dwActiveCheatsLevel[MAX_SKILL_CHEATS_NUM];			//主动技能秘籍等级

	uint32_t m_dwActiveSkillId;										//主动技能ID
	uint8_t m_bActiveSkillLevel;									//主动技能等级

	uint8_t m_bPassiveSkillCnt;										//被动技个数
	uint32_t m_dwPassiveSkillId[MAX_NUM_GENERAL_SKILL];				//被动技能ID
    uint8_t m_szPassiveSkillLevel[MAX_NUM_GENERAL_SKILL];			//被动技能等级

    //阵法buff,用于获取阵法buff的值
    Tactics* m_poTactics;

    DT_POSITION m_stKillPos;		// 自己将别人致死时，传给对面客户端

    RESGENERAL * m_poResGeneral;
    RESARMY * m_poResArmy;
    RESGENERALSTAR * m_poResGeneralStar;
    RESGENERALPHASE * m_poResGeneralPhase;
    RESGENERALLEVEL * m_poResGeneralLevel;
    RESPEAKARENAGENERAL * m_poResPeakArenaGeneral;
    General m_oGeneral;

    bool m_bIsRetreat;	// 已回城
    bool m_bIsAtkCity;	// 在攻城
};

