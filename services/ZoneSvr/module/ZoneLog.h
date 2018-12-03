#pragma once
#include "define.h"
#include "singleton.h"
#include "NetLog.h"
#include "./player/Player.h"
#include "../gamedata/GameDataMgr.h"

class ZoneLog : public TSingleton<ZoneLog>
{
public:
	ZoneLog(){};
	virtual ~ZoneLog(){};
    bool Init(char *pszConfPath, char *pszCltMetaFile, char *pszSvrMetaFile);
    int WriteCltLog(const char* pszTableName, const char* pstLog, int iLen);
    int WriteSvrLog(const char* pszTableName, const char* pstLog, int iLen);

	//统一在这里记录具体的日志
    void WriteLoginLog(Player* poPlayer);
	void WriteLogoutLog(Player* poPlayer, int iLogoutReason);
	void WriteCreateNewLog(Player* poPlayer);
	void WriteGoldLog(PlayerData* pstData, uint32_t dwChgValue, uint8_t bChgType, uint64_t dwCurValue, uint32_t dwApproach);
	void WriteDiamondLog(PlayerData* pstData, uint32_t dwChgValue, uint8_t bChgType, uint64_t dwCurValue, uint32_t dwApproach);
	void WriteYuanLog(PlayerData* pstData, uint32_t dwChgValue, uint8_t bChgType, uint64_t dwCurValue, uint32_t dwApproach);
	void WriteRebornLog(Player* poPlayer, uint32_t dwId, uint8_t bLevel, uint8_t bType);
	void WriteGeneralLog(PlayerData* pstData, uint32_t dwId, uint16_t wLevel, uint16_t wOptType, uint32_t dwOpPara1, uint32_t dwOpPara2, uint32_t dwOpPara3, uint8_t bGeneralPhase, uint16_t wFameHallLvl, uint16_t wGeneralStar);
	void WriteGuildLog(PlayerData* pstData, uint16_t wOptType, uint64_t ullGuildId, const char* pszPara1, uint64_t ullPara2, uint32_t dwPara3);
	void WriteMatchLog(PlayerData* pstData, uint8_t bResult, uint16_t wMatchType = 0);
	void WriteLevelPassLog(PlayerData* pstData, const CS_PKG_FIGHT_PVE_SETTLE_REQ& rstCsPkgBodyReq, int iIsFirstPass);
	void WritePubMailLog(PlayerData* pstData, uint32_t dwMailId);
	void WriteFriendLog(PlayerData* pstData, uint8_t bType, uint64_t ullUinReceiver);
	void WriteGeneralGetLog(PlayerData* pstData, uint32_t dwId, uint16_t wMethod);
	void WriteItemPurchaseLog(PlayerData* pstData, const DT_ITEM_SHOW& rstItem, uint16_t wApproach);
	void WriteMSKillLog(PlayerData* pstData, uint8_t bMSId, uint8_t bMSLevel);
	void WritePropLog(PlayerData* pstData, uint32_t dwId, uint16_t wPropType, uint32_t dwChgValue, uint8_t bChgType, uint32_t dwCurValue, uint32_t dwApproach);
	void WriteAwardLog(PlayerData* pstData, uint16_t wMethode, const DT_SYNC_ITEM_INFO& rstRewardItemInfo);
	void WriteTaskLog(PlayerData* pstData, uint32_t dwTaskId, const char* pszTaskType);
	void WriteLevelSweepLog(PlayerData* pstData, uint32_t dwLevelId, uint8_t bSweepCnt);
	void WriteTutorialStepLog(PlayerData* pstData, uint8_t bFlag, uint64_t ullStep);
	void WriteLotteryLog(PlayerData* pstData, uint8_t bDrawType, const DT_SYNC_ITEM_INFO& rstRewardItemInfo, uint8_t bCount);
	void WriteFightPvPLog(PlayerData* pstData, uint32_t dwOldScore, uint32_t dwNewScore, uint16_t wOldRank, uint16_t wNewRank);
	void WriteSerialNumLog(PlayerData* pstData, const char* pszSerialNum);
	void WriteActivityLog(PlayerData* pstData, const CS_PKG_FIGHT_PVE_SETTLE_REQ& rstCsPkgReq, uint32_t dwPara1, uint32_t dwPara2, uint32_t dwPara3, uint8_t bEvaluateLvl, uint8_t bHardLvl);
	void WritePayLog(Player* poPlayer, const DT_SDK_PAY_CB& rstSdkPayCb, uint32_t dwMoneyPurchased);
	void WriteCoinLog(PlayerData* pstData, uint32_t dwChgValue, uint8_t bCoinType, uint8_t bChgType, uint64_t dwCurValue, uint32_t dwApproach);
	void WriteMajestyLog(PlayerData* pstData);
    void WritePvPLog(PlayerData* pstData, const char* pszPvPType, uint16_t wWinCount);
    void WriteArenaLog(const DT_ASYNC_PVP_PLAYER_SHOW_DATA* pstData, time_t CurTime);
    void WriteMarketRefreshLog(PlayerData* pstData, const char* pszShopType);
    void WritePromotionalActivityLog(PlayerData* pstData, uint8_t bConsumeAward, uint8_t bCombatEffectivenessAward, uint16_t wCarnivalAward, uint16_t wNew7DayAward, uint16_t wNew7DayProgress);
    void WriteGuildBossLog(PlayerData * pstData, uint32_t dwDamage, uint16_t wBossId);
    void WriteCloneBattleLog(Player* poPlayer, int8_t chMemCount, uint32_t dwBossId);
    void WriteDailyChallengeLog(PlayerData* pstData, const DT_ROLE_DAILY_CHALLENGE_INFO& stDailyChallengeInfo);
    void WriteClientCheatLog(PlayerData* pstData);
    void WriteMineOpLog(PlayerData* pstData, const SS_PKG_MINE_DEAL_ORE_RSP& rstMineRsp);
    void WriteMineExploreLog(PlayerData* pstData, const SS_PKG_MINE_EXPLORE_RSP& rstMineRsp);

private:
    CNetLog CltLog;
    CNetLog SvrLog;
    const int static BUFFER_SIZE = 1024;

private:
    void _GetDaySinceReg(const DT_ROLE_BASE_INFO& stRoleBaseInfo, uint16_t * pwDaySinceReg);
    /*
    日志中有如下字段的可以调用这个函数来减少代码量
    <entry name="PlayerID"                  type="uint64"           desc="玩家ID" />
    <entry name="AccountName"               type="string"           desc="玩家帐号"                   size="MAX_NAME_LENGTH" />
    <entry name="RoleName"                  type="string"           desc="角色名"                     size="MAX_NAME_LENGTH" />
    <entry name="DateTime"                  type="string"           desc="时间日期"                   size="MAX_TIMEINFO_LENGTH"/>
    <entry name="MajestyLvl"                type="uint16"           desc="主公等级" />
    <entry name="VIPLvl"                    type="int8"             desc="玩家VIP等级" />
    */
    template <typename LOG_TYPE>
    void _WritePlayerInfo(PlayerData* pstData, LOG_TYPE &stLog);
};

