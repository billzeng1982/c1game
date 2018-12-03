#pragma once

#include "common_proto.h"
#include "comm/tlist.h"
#include "ov_res_public.h"
#include "strutil.h"
class MineOre
{
public:
    MineOre() {};
    ~MineOre() {};

	//是否为空旷
	bool IsEmptyOre() { return m_stOreInfo.m_ullOwnerUin == 0; }
    //获取矿唯一id
    uint64_t GetUid() { return m_stOreInfo.m_ullUid; }
    //获取拥有者
    uint64_t GetOwner() { return m_stOreInfo.m_ullOwnerUin; }
    void SetOwner(uint64_t ullUin) { m_stOreInfo.m_ullOwnerUin = ullUin; }

	//获取占领者的战力
	uint32_t GetOwnerLi() { return m_stOreInfo.m_dwOwnerLi; }

	//获取挑战者
    uint64_t GetChallenger() { return m_stOreInfo.m_ullChallengerUin; }
    void SetChallenger(uint64_t ullUin) { m_stOreInfo.m_ullChallengerUin = ullUin; }

	//更新名字
	void UpdateName(const char* szName) { StrCpy(m_stOreInfo.m_szOwnerName, szName, sizeof(m_stOreInfo.m_szOwnerName)); }

	//获取矿Id
    uint32_t GetId() { return m_stOreInfo.m_dwOreId; }

    //获取占有时间点
    uint64_t GetOccupyTime() { return m_stOreInfo.m_ullOccupyTime; }
    void SetOccupyTime(uint64_t ullTime) { m_stOreInfo.m_ullOccupyTime = ullTime; }

	//获取选择占领过期时间
	uint64_t GetSelectOccupyOutTime() { return m_stOreInfo.m_ullSelectOccupyOutTime; }
	void SetSelectOccupyOutTime(uint64_t ullTime) { m_stOreInfo.m_ullSelectOccupyOutTime = ullTime; }

    //获取矿类型
    uint8_t GetType() { return m_stOreInfo.m_bOreType; }
    //设置矿唯一id
    void SetUid(uint64_t ullUid) { m_stOreInfo.m_ullUid = ullUid; }

    //重置矿资源数据
    void Reset();

	//清理矿资源
	void Clear4Reuse();

    //从数据库初始化矿信息
    bool InitFromDB(PKGMETA::DT_MINE_ORE_DATA& rstData);

    bool PackToData(OUT PKGMETA::DT_MINE_ORE_DATA& rstData);

    //初始化矿信息
    void Init(uint64_t ullUid, uint8_t bOreType, uint32_t dwOreId);

    //获取矿信息
    void GetOreInfo(OUT PKGMETA::DT_MINE_ORE_INFO& rstOreInfo);

    PKGMETA::DT_MINE_ORE_INFO& GetOreInfo() { return m_stOreInfo; };

    //占领矿
    int Occupy(char szName[], uint32_t dwAddr, uint64_t ullOwnerUin, uint32_t dwLeaderValue,
		uint8_t bTroopCount, PKGMETA::DT_TROOP_INFO* pstTroopInfo, PKGMETA::DT_ITEM_MSKILL& rstMSkill);

    //放弃矿
    int Drop();

    //修改矿
    int Modify(uint64_t ullOwnerUin, PKGMETA::DT_ITEM_MSKILL& rstMSkill);

	//放弃抢占矿
	int GiveUpGrab();

	//获取产出周期个数
	int GetProduceCycleNum(RESMINE* pstResMine, uint8_t bType, OUT uint64_t* pstSettleTime = NULL);

	//增加被抢夺次数
	void AddBeLootedCount() { m_stOreInfo.m_bBeLootedCount++; }

	//获取被抢夺次数
	uint8_t GetBeLootedCount() { return m_stOreInfo.m_bBeLootedCount; }

	//清理被抢夺信息
	void ClearBeLootedInfo();

	//获取保护时间 战斗时设置
    uint64_t GetFightGuardTime() { return m_stOreInfo.m_ullFightGuardTime; }

	//获取真正的战斗保护时间 
	uint64_t GetRealFightGuardTime();

	//设置保护时间 战斗时设置
    void SetFightGuardTime(uint64_t ullTime) { m_stOreInfo.m_ullFightGuardTime = ullTime; }


public:
    TLISTNODE m_stDirtyListNode;
    TLISTNODE m_stTimeListNode;
	TLISTNODE m_stEmptyListNode;
private:
    PKGMETA::DT_MINE_ORE_INFO m_stOreInfo;
};

