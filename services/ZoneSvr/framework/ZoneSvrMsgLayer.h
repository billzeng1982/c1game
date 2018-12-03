#pragma once

#include "singleton.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "cs_proto.h"

class Player;
class ZoneSvrMsgLayer: public CMsgLayerBase, public TSingleton<ZoneSvrMsgLayer> {
public:
    ZoneSvrMsgLayer();
    ~ZoneSvrMsgLayer() {
    }

    virtual bool Init();
    virtual int DealPkg();

    int SendToClient(Player* poPlayer, PKGMETA::SCPKG* pstScPkg, char cSessCmd = PKGMETA::CONNSESSION_CMD_INPROC);
    int SendToClient(const PKGMETA::CONNSESSION* pstConnSess, PKGMETA::SCPKG* pstScPkg, char cSessCmd = PKGMETA::CONNSESSION_CMD_INPROC, uint16_t wVersion = 0);
    int MultiCastToClient(uint64_t UinList[], int iUinCnt, PKGMETA::SCPKG* pstScPkg, char cSessCmd = PKGMETA::CONNSESSION_CMD_INPROC);
    int BroadcastToClient(PKGMETA::SCPKG* pstScPkg);

    int SendToAccountSvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToRoleSvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToRankSvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToGuildSvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToFriendSvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToMessageSvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToMailSvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToClusterGate(PKGMETA::SSPKG& rstSsPkg);
    int SendToReplaySvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToXiYouSDKSvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToSdkDMSvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToSerialNumSvr(PKGMETA::SSPKG& rstSsPkg);
	int SendToMiscSvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToAsyncPvpSvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToCloneBattleSvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToIdipAgentSvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToMineSvr(PKGMETA::SSPKG& rstSsPkg);
	int SendToMatchSvr(PKGMETA::SSPKG& rstSsPkg);
	int SendToFightSvr(PKGMETA::SSPKG& rstSsPkg);
	int SendToClusterAccSvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToGuildExpeditionSvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToZoneHttpConn(PKGMETA::SSPKG& rstSsPkg);
		
    int SendCachePkgToClient(Player* poPlayer, char* pszBuffer,  uint32_t dwLen);
protected:
    virtual void _RegistClientMsg();
    virtual void _RegistServerMsg();

    void _DealConnPkg();

private:
    int m_iConnID;
};

