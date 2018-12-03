#pragma once

#include "singleton.h"
#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "../cfg/GuildSvrCfgDesc.h"

class GuildSvrMsgLayer : public CMsgLayerBase, public TSingleton<GuildSvrMsgLayer>
{
public:
    GuildSvrMsgLayer();
    ~GuildSvrMsgLayer(){}

    bool Init();

    /*
        返回处理的包数量
        <0: error
    */
    int DealPkg();
    int SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToMailSvr(PKGMETA::SSPKG& rstSsPkg);
	int SendToRankSvr(PKGMETA::SSPKG& rstSsPkg);
	int SendToGuildExpeditionSvr(PKGMETA::SSPKG& rstSsPkg);

	PKGMETA::SSPKG& GetNtfPkg() { return m_stSsNtfPkg; }
private:
    void _RegistServerMsg();

private:
    GUILDSVRCFG*	m_pstConfig;
    PKGMETA::SSPKG 	m_stSsRecvPkg;
	PKGMETA::SSPKG 	m_stSsNtfPkg;
};



