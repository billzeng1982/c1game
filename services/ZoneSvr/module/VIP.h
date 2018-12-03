#pragma once
#include "define.h"
#include "singleton.h"
#include "cs_proto.h"
#include "player/Player.h"
#include "../gamedata/GameDataMgr.h"

using namespace PKGMETA;

class VIP : public TSingleton<VIP>
{
public:
    VIP();
    ~VIP();

    bool Init();

    //增加VIP经验,VIP经验达到一定值后,可以提升VIP等级
    uint32_t AddExp(PlayerData* pstData, uint32_t dwValue, uint32_t dwApproach);

    int RecvDailyGif(PlayerData* pstData, DT_SYNC_ITEM_INFO& rstRewardItemInfo, uint64_t* pTimestamp);

    int RecvLevelGif(PlayerData* pstData, uint8_t bLevel, DT_SYNC_ITEM_INFO& rstSyncItemInfo);

    int UpdateServer();

private:
    //发送升级通知(注意:需求变化,只要经验变化,即使没有升级也要发送通知)
    void SendLvUpNtf(PlayerData* pstData);

private:
    int m_iUptTime;
    bool m_bUptFlag;

    uint64_t m_ullUptTimeStamp;

    //VIP的经验等级对照表
    uint32_t m_VipExpToLevelList[MAX_VIP_LEVEL];

    //VIP
    RESVIP* m_ResVIPList[MAX_VIP_LEVEL];

    //VIP每日礼包
    RESGIFTPACKAGE* m_ResDailyGifList[MAX_VIP_LEVEL];

    //VIP等级礼包
    RESGIFTPACKAGE* m_ResLevelGifList[MAX_VIP_LEVEL];
};
