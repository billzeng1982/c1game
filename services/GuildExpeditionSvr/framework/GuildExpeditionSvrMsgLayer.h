#pragma once

#include "utils/singleton.h"
#include "msglayer/CommBusLayer.h"
#include "ss_proto.h"
#include "msglayer/MsgBase_c.h"
#include "msglayer/MsgLayerBase_c.h"
#include "../cfg/GuildExpeditionSvrCfgDesc.h"
#include "mng/DynMempool.h"

class GuildExpeditionSvrMsgLayer : public CMsgLayerBase_c, public TSingleton<GuildExpeditionSvrMsgLayer>
{
public:
    GuildExpeditionSvrMsgLayer();
	~GuildExpeditionSvrMsgLayer(){}

	bool Init();

	/*
		返回处理的包数量6
		<0: error
	*/
	int DealPkg();

	IMsgBase_c* GetMsgHandler( int iMsgID );
	int SendToOtherSvrByGate(PKGMETA::SSPKG* rstSsPkg);
    int SendToClusterDBSvr(PKGMETA::SSPKG* rstSsPkg);

	PKGMETA::SSPKG* GetSendPkg() { return m_SendSsPkgPool.Get(); };
private:
	void _RegistServerMsg();


private:
	GUILDEXPEDITIONSVRCFG*	m_pstConfig;
	DynMempool<PKGMETA::SSPKG> m_SendSsPkgPool;
};

