#pragma once

#include "define.h"
#include "cs_proto.h"
#include "common_proto.h"
#include "singleton.h"
#include "player/PlayerMgr.h"
#include "../gamedata/GameDataMgr.h"

#define MAX_MSG_LIST_LEN (MAX_NUM_MARQUEE_INFO_PUR_NTF * 2)

using namespace PKGMETA;

class Marquee : public TSingleton<Marquee>
{
public: 
    const static uint32_t GCARD_GAIN = 1; //武将获取
    const static uint32_t GCARD_OPT = 0; //武将培养
public:
	Marquee();
	virtual ~Marquee();

	bool Init();
	void AddGMMsg(DT_MARQUEE_INFO& stMarqueeInfo); //GM marquee info
	void AddLocalMsg(const char* pszName, uint8_t bType, uint32_t dwParam = 0 , uint32_t dwParam2 = 0 , const char* pszContent = NULL); //local marquee info

    void SaveMarqueeForGCard(PlayerData* pstData, uint8_t bType, uint32_t dwId, uint32_t dwStar);
    void SaveMarqueeForGCard(PlayerData* pstData, uint8_t bType, RESGENERAL* pResGCard, uint32_t dwStar);
	void UpdateServer();

	void SaveMarqueeForHorn(PlayerData* pstData, const char* pszContent);

    //定时跑马灯
    int AddMarqueeOnTime(DT_GM_MARQUEE_ON_TIME_ADD& rstMarqueeOnTimeAdd);
    int DelMarqueeOnTime(uint64_t ullMarqueeMsgId);
    DT_GM_MARQUEE_LIST_INFO& GetMarqueeOnTimeInfo() { return m_stMarqueeOnTimeInfo; }

private:
	void _SendMsgToClient(int iStart, int iEnd);
	bool _Check();
    bool _IsScreenGeneral(uint32_t dwId);
    //检查是否出发定时跑马灯
    void _UpdateMarqueeOnTime();
    //是否有效的定时跑马灯
    bool _CheckMarqueeOnTime(DT_GM_MARQUEE_ON_TIME_ADD& rstMarqueeAdd);

private:
	PKGMETA::SCPKG m_stScPkg;
	uint64_t m_ullSendInterval;  //消息发送的间隔
	uint64_t m_ullLastSendTime;  //上次发送的时间戳

	DT_MARQUEE_INFO m_stMarqueeMsgList[MAX_MSG_LIST_LEN];
	int m_iHead; //head
	int m_iTail; //tail

    //定时播放数据
    DT_GM_MARQUEE_LIST_INFO m_stMarqueeOnTimeInfo;
    DT_GM_MARQUEE_ON_TIME_ADD m_stOneMarqueeOnTime;
};

