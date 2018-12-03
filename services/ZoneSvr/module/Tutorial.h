#pragma once
#include "define.h"
#include "cs_proto.h"
#include "singleton.h"
#include "player/PlayerData.h"
#include <list>
#include <map>

using namespace PKGMETA;

class Tutorial : public TSingleton<Tutorial>
{
public:
	Tutorial();
	virtual ~Tutorial();

	bool Init();

	int DrawBonus(PlayerData* pstData, uint8_t& rbBonusReq, DT_SYNC_ITEM_INFO& rstSyncItemInfo);
    int RecordTutorialData(PlayerData* pstData, uint8_t bFlag, uint64_t ullDataValue);

public:
    void _SendTutorialGift(PlayerData* pstData); //新手礼包，通过邮件发放，完成新手某个节点后发放

private:
    static const int TUTORIAL_GIFT_MAIL_ID = 10012;
    static const int BASIC_AP_COMPENSATE_ID = 9504;

	typedef std::map<uint32_t/*领取次数*/, std::list<uint32_t>/*武将id*/> TutorialGeneralMap_t;

	TutorialGeneralMap_t m_GeneralMap;

    SSPKG m_stSsPkg;
};

