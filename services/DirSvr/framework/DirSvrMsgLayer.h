#pragma once

#include "singleton.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "ss_proto.h"
#include "UdpRelay.h"

class DirSvrMsgLayer: public CMsgLayerBase, public TSingleton<DirSvrMsgLayer>
{
public:
	DirSvrMsgLayer();
	~DirSvrMsgLayer() {}

	virtual bool Init();
	virtual int DealPkg();

private:
    UdpRelaySvr m_oUdpRelaySvr;
    DT_SERVER_INFO m_stSvrInfo;
};

