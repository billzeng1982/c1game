#pragma once

#include "singleton.h"
#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "../cfg/RankSvrCfgDesc.h"

class RankSvrMsgLayer: public CMsgLayerBase, public TSingleton<RankSvrMsgLayer>
{
public:
	RankSvrMsgLayer();
	virtual ~RankSvrMsgLayer(){}

	virtual bool Init();

	virtual int DealPkg();
	int SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg);
	int SendToMailSvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToGuildSvr(PKGMETA::SSPKG & rstSsPkg);

private:
	virtual void _RegistServerMsg();
};



