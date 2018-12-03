#pragma once
#include "define.h"
#include "player/PlayerData.h"
#include "ss_proto.h"
#include "cs_proto.h"
#include "common_proto.h"
#include "singleton.h"

using namespace PKGMETA;

class Mail : public TSingleton<Mail>
{
public:
	Mail();
	virtual ~Mail();

private:
	SCPKG m_stScPkg;
	SSPKG m_stSsPkg;

public:
    void UpdatePlayerData(PlayerData* pstData);
	void HandleSyncServerDataFromMailSvr(uint32_t dwMailId);
	void HandleDrawMsgFromMailSvr(uint64_t ullUin, PKGMETA::DT_MAIL_DATA& rstMailData);
	int SyncPlayerDataToClient(Player* poPlayer, SS_PKG_MAIL_SYNC_PLAYER_DATA_NTF& rstSsPkgBodyNtf);
	int SendPlayerStatToMailSvr(uint64_t ullUin, uint8_t bState);
	int SendMailStatToMailSvr(uint64_t ullUin, uint8_t bState, uint8_t bMailType, uint32_t dwMailId);
	void SyncPlayerPubMail(PlayerData* pstData);

	static int TestSendPriMail(uint64_t ullUin1, uint64_t ullUin2);
	static int TestSendPubMail();

public:
    uint32_t m_dwPubMailSeq;
};

