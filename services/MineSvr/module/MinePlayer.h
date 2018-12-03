#pragma once

#include "common_proto.h"
#include "comm/tlist.h"
#include "MineOre.h"
#include "ss_proto.h"
using namespace PKGMETA;
class MinePlayer
{
public:
    MinePlayer() {};
    ~MinePlayer() {};

    void Reset();
    uint64_t GetUin() {  return m_stPlayerInfo.m_ullUin;  }

    void Init(uint64_t ullUin);

    //  从数据库初始化矿信息
    bool InitFromDB(DT_MINE_PLAYER_DATA& rstData);

    //  打包
    bool PackToData(OUT DT_MINE_PLAYER_DATA& rstData);

	const char* GetPlayerName() { return m_stPlayerInfo.m_szName; }

	uint32_t GetPlayerAddr() { return m_stPlayerInfo.m_dwSvrAddr; }

    //  更新名字和服务器地址
    void SetPlayerInfo(char* szName, uint32_t dwAddr, uint16_t wLv, uint16_t dwIconId, uint32_t dwLeaderValue);

    //  能否占领矿
    int CanOccupyOre(uint64_t ullOreUid);

    //  矿Uid是否在ExploreList里
    bool CheckOreUidInExploreList(uint64_t ullOreUid);

    //  矿Uid是否在OwnList里
    bool CheckOreUidInOwnList(uint64_t ullOreUid);

	//	调查矿
	int Investigate(uint64_t ullOreUid, OUT PKGMETA::DT_MINE_ORE_INFO& rstOreInfo);

	//  失效
	int SetOreUesedState(uint64_t ullOreUid);

    //  检查是否失效
    int IsSetOreUsedState(uint64_t ullOreUid);

    //  获取所有信息
    int GetAllInfo(SS_PKG_MINE_GET_INFO_RSP& rstRsp);

    int GetOwnOreInfo(OUT uint8_t& rbOreCount, OUT DT_MINE_ORE_INFO* pstOreList);



    uint8_t GetAwardLogCount() { return m_stPlayerInfo.m_bAwardCount; }
    //  放弃矿
    int DropOre(MineOre& rstOre);

    //  占有矿
    int OccupyOre(MineOre& rstOre, uint8_t bTroopCount, DT_TROOP_INFO* pstTroopInfo, DT_ITEM_MSKILL& rstMSkill);

	//	删除拥有矿列表
	int DelOwnOreUid(uint64_t ullOreUid);

    //  每日结算
    int SettleDaily();

	//	赛季结算
	int SeasonSettle();

	//	赛季重置
	void SeasonReset();

	//  发起挑战
	int ChallengeRequest(MineOre& rstOre);

	//	发起复仇
	int RevengeRequest(MineOre& rstOre);

    //  挑战成功
    int ChallengeWin(MineOre& rstOre);

	//	挑战失败
	int ChallengeLost(MineOre& rstOre);

    //  抢占矿
    int GrabOre(MineOre& rstOre, uint8_t bTroopCount, DT_TROOP_INFO* pstTroopInfo, DT_ITEM_MSKILL& rstMSkill);

    //  结算一个矿
    int SettleOneOre(uint8_t bType, MineOre& rstOre, OUT DT_MINE_AWARD& rstAward);

	//	抢夺资源/复仇资源
	int LootOre(RESMINE* pResMine, uint8_t bAwardLogType, MineOre& rstOre, DT_MINE_AWARD& rstAward, DT_MINE_COM_LOG& rstComLog, DT_MINE_COM_LOG& rstSelfComLog);

	//	复仇成功
	int RevengeWin(MineOre& rstOre, uint64_t ullObjUin, uint8_t bRevengeLogIndex);

	//	复仇失败
	int RevengeLost(MineOre& rstOre, uint64_t ullObjUin);

    //  DT_ITEM_SIMPLE 合并
    int _AddItemByMerge(uint8_t bItemType, uint32_t dwItemId, uint32_t dwItemNum, OUT DT_MINE_AWARD& rstAward);

    void SendNtf(SSPKG& rstPkgNew, uint8_t bType);
    //  Log通知
    void SentAddAwardNtf(DT_MINE_AWARD& rstAward);

    void SendRevengeLogNtf(DT_MINE_REVENGE_LOG& rstRevengeLog);

    void SendComLogNtf(DT_MINE_COM_LOG& rstComLog);

	//	是否在当前赛季
	bool IsInCurSeason();

    //  更新探索列表
    void UpdateExploreOreList(uint8_t bExploreOreCount, DT_MINE_ORE_INFO* pstExploreOreList);

    int GetAwardLog(uint8_t bIndex, OUT DT_MINE_AWARD& rstAward);

    void AddComLog(DT_MINE_COM_LOG& rstComLog);
    void AddRevengeLog(DT_MINE_REVENGE_LOG& rstRevengeLog);
	void AddAwardLog(DT_MINE_AWARD& rstAward);

	void UpdateAddr(uint32_t dwAddr);
public:
    TLISTNODE m_stDirtyListNode;
    TLISTNODE m_stTimeListNode;
private:
    DT_MINE_PLAYER_INFO m_stPlayerInfo;
};

