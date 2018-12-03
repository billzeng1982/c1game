#pragma once
#include "../../../common/define.h"
#include "../../../common/protocol/PKGMETA/common_proto.h"
#include "../../../common/protocol/PKGMETA/cs_proto.h"
#include "../../../common/utils/singleton.h"
#include "player/PlayerData.h"


using namespace PKGMETA;

#define NOT_SET_STAR -1

struct LiObj
{
    uint64_t m_ullUin;                  //玩家Uin
    uint32_t m_dwId;                    //武将Id
    // 武将属性
    /*
    float m_fInitHp;                      //血量
    float m_fGeneralInitStr;              //物攻
    float m_fGeneralInitWit;              //法攻
    float m_fGeneralInitStrDef;           //物防
    float m_fGeneralInitWitDef;           //法防
    float m_fInitSpeed;                      //速度
    float m_fInitChanceHit;             // 初始命中率
    float m_fInitChanceDodge;           // 初始闪避率
    float m_fInitChanceCritical;        // 初始暴击率
    float m_fInitChanceAntiCritical;    // 初始抗暴击率
    float m_fInitChanceBlock;           // 初始招架率
    float m_fInitParaCritical;          // 初始暴击系数
    float m_fInitParaAntiCritical;      // 初始抗暴击系数
    float m_fInitParaBlock;             // 初始招架系数
    float m_fInitParaDamageAdd;         // 初始附加真实伤害
    float m_fInitParaSuckBlood;         // 初始吸血系数
    float m_fBaseDamageAtkNormal;	        //白兵基础伤害
    */
    float m_afAttr[MAX_ATTR_ADD_NUM];
    void AddAttr(float * afAttr)
    {
        if (afAttr == NULL)
        {
            return;
        }
        for (size_t i = 0; i < MAX_ATTR_ADD_NUM; i++)
        {
            m_afAttr[i] += afAttr[i];
        }
    }
    void GetAttr(OUT float * afAttr)
    {
        if (afAttr == NULL)
        {
            return;
        }
        memcpy(afAttr, m_afAttr, sizeof(m_afAttr));
    }
};

class GeneralCard : public TSingleton<GeneralCard>
{
public:
    static const uint32_t CONST_GCARD_SKILL_DETA = 5;   //资质 间隔5
    static const float CONST_RATIO_PERMILLAGE = 0.001f;  //千分比

    static const uint64_t MODULE_MINE_USE = 1 << 0;          //武将是否在挖矿中用于挑战的标志

public:
    GeneralCard();
    virtual ~GeneralCard(){}
    bool Init();

private:
	PKGMETA::SCPKG m_stScPkg;
    DT_ITEM_GCARD m_stGCard;
    DT_ITEM m_stItem;

    //经验卡ID
    uint32_t m_ExpCardId[MAX_NUM_EQUIP_EXPCARD_TYPE];
    //经验卡提供的经验
    uint32_t m_ExpCardExp[MAX_NUM_EQUIP_EXPCARD_TYPE];

	//星级提供的统帅值
	uint32_t m_dwStarLeaderValue;
	//阶提供的统帅值
	uint32_t m_dwPhaseLeaderValue;
	//亲密度等级提供的统帅值
	uint32_t m_dwTrainLvLeaderValue;
	//亲密度阶提供的统帅值
	uint32_t m_dwTrainPhaseLeaderValue;
	//名将录提供的统帅值
	uint32_t m_dwFameHallLeaderValue;

public:
    //**武将结构 增/删/查 操作
    //返回 : <0#错误 | >= 0#表示增加的武将的数组索引
    int Add(PlayerData* pstData, DT_ITEM_GCARD* pstItem);
    int Del(PlayerData* pstData, uint32_t dwId);
    DT_ITEM_GCARD* Find(PlayerData* pstData, uint32_t dwId);

    //获取武将品阶
    int GetClassScore(uint32_t dwId);

	//初始化统帅值
	uint32_t InitLeaderValue(PlayerData* pstData);
	void UptLeaderValue(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);

	//获取武将统帅值
	uint32_t GetGCardLeaderValue(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);
    //获取武将战斗力
    //bIsRecount = true 表示强制重新计算战力
	uint32_t GetGCardLi(PlayerData* pstData, uint32_t dwId, bool bIsRecount = false);

    //获取阵容战斗力
    uint32_t GetTeamLi(PlayerData* pstData, uint8_t bBattleArrayType = BATTLE_ARRAY_TYPE_NORMAL);
    //武将战力更新触发器
    //  注意:如果是影响所有武将战力变化,特殊处理,不调用此接口
    void TriggerUpdateGCardLi(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral, bool bIsAdd = true);

	void TriggerAllGcardLi(PlayerData* pstData);


    //计算单个武将战力,最后会保存在武将身上
    uint32_t CalGcardLi(PlayerData* pstData, uint32_t dwId);
    uint32_t CalGcardLi(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);
    //对单个武将机选军师技战力
    uint32_t GetMSkillLi(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral);
    //军师技修改后武将战力的增加值
    uint32_t GetMSkillLi(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral, float* afOldAttr);
    //军师技升级计算所有战力
    void CalAllCardLiByMSkillUp(PlayerData* pstData);
    //计算军师技的属性
    void CalMSkillAttr(PlayerData* pstData);
	int HandleGroupCard(PlayerData* pstData, uint8_t bType, uint32_t dwParam);

    int AddDataForNewPlayer(PlayerData* pstData);
    int AddDataForDebug(PlayerData* pstData, DT_ITEM* pstItemList, int iIdx = 0);

    DT_ITEM_GCARD* AddWrapPrimary(PlayerData* pstData, uint32_t dwId, int32_t iStar=NOT_SET_STAR);
    //激活武将缘分
    void OpenGCardFate(PlayerData* pstData, uint32_t dwId);
    //武将重生
    int Reborn(PlayerData* pstData, uint32_t dwId, uint8_t bType, DT_SYNC_ITEM_INFO& rstSyncItemInfo);

    int LvUp(PlayerData* pstData, uint32_t dwId, DT_CONSUME_ITEM_INFO& rstConsumeItemInfo, DT_SYNC_ITEM_INFO& rstSyncItemInfo);
	int LvUp(PlayerData* pstData, uint32_t dwId, uint32_t dwIncExp);
    int LvPhaseUp(PlayerData* pstData, uint32_t dwId, DT_SYNC_ITEM_INFO& rstSyncItemInfo);
    int LvReborn(PlayerData* pstData, uint32_t dwId, DT_SYNC_ITEM_INFO& rstRewardItemInfo, uint32_t dwRatio);
    //武将升星
    int StarUp(PlayerData* pstData, uint32_t dwId, SC_PKG_GCARD_STAR_UP_RSP& rstScPkgBodyRsp);
    int StarReborn(PlayerData* pstData, uint32_t dwId, DT_SYNC_ITEM_INFO& rstRewardItemInfo, uint32_t* pGold, uint32_t dwRatio);
    //武将合成
    int Composite(PlayerData* pstData, uint32_t dwId, SC_PKG_GCARD_COMPOSITE_RSP& rstScPkgBodyRsp);
    //武将升阶(升品)
    int PhaseUp(PlayerData* pstData, uint32_t dwId, SC_PKG_GCARD_PHASE_UP_RSP& rstScPkgBodyRsp);
	int PhaseReborn(PlayerData* pstData, uint32_t dwId, DT_SYNC_ITEM_INFO& rstRewardItemInfo, uint32_t* pGold, uint32_t dwRatio);
    //武将技能升级
    int SkillLvUp(PlayerData* pstData, CS_PKG_GCARD_SKILL_LVUP_REQ& rstCsPkgBodyReq, SC_PKG_GCARD_SKILL_LVUP_RSP& rstScPkgBodyRsp);
    int SkillReborn(PlayerData* pstData, uint32_t dwId, DT_SYNC_ITEM_INFO& rstRewardItemInfo, uint32_t* pGold, uint32_t dwRatio);
    //秘笈升级
    int CheatsLvUp (PlayerData* pstData, uint32_t dwId, uint8_t bSeqId, SC_PKG_CHEATS_LVUP_RSP& rstScPkgBodyRsp);
    int CheatsReborn(PlayerData* pstData, uint32_t dwId, DT_SYNC_ITEM_INFO& rstRewardItemInfo, uint32_t dwRatio);

    //**兵种操作
    //兵种升级
    int ArmyLvUp(PlayerData* pstData, uint32_t dwGeneralId,  SC_PKG_GCARD_ARMY_LVUP_RSP& rstSCPkgBodyRsp);
    //兵种一键升级
    int ArmyLvUpTotal(PlayerData* pstData, uint32_t dwGeneralId,  SC_PKG_GCARD_ARMY_LVUP_RSP& rstSCPkgBodyRsp);
    //兵种升阶
    int ArmyPhaseUp(PlayerData* pstData, uint32_t dwGeneralId, SC_PKG_GCARD_ARMY_PHASEUP_RSP& rstSCPkgBodyRsp);
    int ArmyLvReborn(PlayerData* pstData, uint32_t dwId, DT_SYNC_ITEM_INFO& rstRewardItemInfo, uint32_t* pGold, uint32_t dwRatio);
    int ArmyPhaseReborn(PlayerData* pstData, uint32_t dwId, DT_SYNC_ITEM_INFO& rstRewardItemInfo, uint32_t dwRatio);

    int CheatsChg (PlayerData* pstData, uint32_t dwId, uint8_t bSeqId);
    int InitFateBuffIds( PlayerData* pstData, DT_ITEM_GCARD* m_stGCard);

	int InitMSkillAttrs(PlayerData* pstData, DT_ITEM_GCARD* pstGCardInfo, DT_TROOP_INFO &rstTroopInfo);
	int InitFeedTrainAttrs(PlayerData* pstData, DT_ITEM_GCARD* pstGCardInfo, DT_TROOP_INFO &rstTroopInfo);

	int GetFragmentNum(PlayerData* pstData, uint32_t dwId, int32_t iStar, OUT uint32_t* pdwFragmentId = NULL);

	int TransUniversalFrag(PlayerData* pstData, CS_PKG_TRANS_UNIVERSAL_FARG_REQ& rstCsPkgBodyReq, SC_PKG_TRANS_UNIVERSAL_FARG_RSP& rstScPkgBodyRsp);

	//给过关武将添加经验
	int AddGeneralExp(PlayerData* pstData, uint32_t dwPveLevelId, uint32_t* pGeneralList=NULL, uint8_t bBattleArrayType=BATTLE_ARRAY_TYPE_NORMAL);

    void TestGCardLiTime(PlayerData* pstData, int iCnt);
    //登录后初始化已激活武将缘分
    void InitGCardFate(PlayerData* pstData);

    bool IsGCardFateOpen(PlayerData* pstData, uint32_t dwFateId);

	void InitFeedTrain(PlayerData* pstData);

	//喂食培养亲密度
	int FeedTrain(PlayerData* pstData, CS_PKG_GCARD_TRAIN_REQ& rstCsReq, SC_PKG_GCARD_TRAIN_RSP& rstScRsp);

	//提高提高亲密度段位
	int TrainLvUp(PlayerData* pstData, CS_PKG_TRAIN_LVUP_REQ& rstCsReq, SC_PKG_TRAIN_LVUP_RSP& rstScRsp);

	void CalFeedTrainAttr(PlayerData * pstData);

	//更改武将技能标志
	int ChgSkillFlag(PlayerData * pstData, uint32_t dwId, uint32_t dwSkillFlag, uint32_t dwAIFlag);

    //某个标识是否有效, 具体自定义
    //ullModuleBitFlat 含义 需统一定义在本类的开头 方便查看
    bool GetModuleState(DT_ITEM_GCARD* pstGCardInfo, uint64_t ullModuleBitFlat);
    //设置标识
    void SetModuleState(DT_ITEM_GCARD* pstGCardInfo, uint64_t ullModuleBitFlat, bool bState);

    //获得武将经验物品的经验和ID
    inline uint32_t* GetExpPropsInfo() { return m_ExpCardExp; }
	void ClearMineAp(PlayerData* pstData);

private:
    //战力计算
    void _AddValue(float& rOriginalData, uint8_t bType,  float fIncValue, int iPer);
    //  属性计算:武将基础
    void _AddGeneralLevel(DT_ITEM_GCARD* pstGeneral, LiObj& rRtData);
    //  属性计算:装备升星
    void _AddGeneralStar(DT_ITEM_GCARD* pstGeneral, LiObj& rRtData);
    //  属性计算:宝石
    void _AddGeneralGem(DT_ITEM_GCARD* pstGeneral, LiObj& rRtData);
    //  属性计算:武将升阶
    void _AddGeneralPhase(DT_ITEM_GCARD* pstGeneral, LiObj& rRtData);
    //  属性计算:装备
    void _AddEquipAttr(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral, LiObj& rRtData);
    //  属性计算:装备基础
    void _AddEquipBase(DT_ITEM_GCARD* pstGeneral, LiObj& rRtData, DT_ITEM_EQUIP& rstEquipInfo);
    //  属性计算:装备升星
    void _AddEquipStar(DT_ITEM_GCARD* pstGeneral, LiObj& rRtData, DT_ITEM_EQUIP& rstEquipInfo);
    //  属性计算:缘分
    void _AddGeneralFateAttr(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral, const LiObj& rBaseRtData, OUT LiObj& rRtData);
	//	属性计算：装备缘分
	void _AddEquipFateAttr(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral, const LiObj& rBaseRtData, OUT LiObj& rRtData);
	//	属性计算：武将亲密度
	void _AddFeedTrainAtrr(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral, LiObj& rRtData);
    //  得到技能系数
    float _GetGCardSkillRatio(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral, uint8_t bSpecialEquipType, uint32_t dwTalent);
    void _GetFateLi(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral, int& iFateLi, int& iFateRatio);
    void  _GetGCardAttr(PlayerData* pstData, DT_ITEM_GCARD* pstGeneral, OUT LiObj& rRtData);
};

