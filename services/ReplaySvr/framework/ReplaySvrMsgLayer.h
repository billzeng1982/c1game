#pragma once

#include "singleton.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "ss_proto.h"

class ReplaySvrMsgLayer: public CMsgLayerBase, public TSingleton<ReplaySvrMsgLayer>
{
public:
	ReplaySvrMsgLayer();
	~ReplaySvrMsgLayer() {}

	virtual bool Init();
	virtual int DealPkg();
	int SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg);
	
protected:
	virtual void _RegistServerMsg();
};

