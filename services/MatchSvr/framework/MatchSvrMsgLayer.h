#pragma once

#include "singleton.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "ss_proto.h"

class MatchSvrMsgLayer: public CMsgLayerBase, public TSingleton<MatchSvrMsgLayer> {
public:
	MatchSvrMsgLayer();
	~MatchSvrMsgLayer() {}

	virtual bool Init();
	virtual int DealPkg();
	int SendToClusterGate(PKGMETA::SSPKG& rstSsPkg);
	int SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg);
	int SendToFightSvr(PKGMETA::SSPKG& rstSsPkg);
protected:
	virtual void _RegistServerMsg();
};

