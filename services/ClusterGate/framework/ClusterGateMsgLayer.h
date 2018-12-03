#pragma once

#include "singleton.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "ss_proto.h"

class ClusterGateMsgLayer: public CMsgLayerBase, public TSingleton<ClusterGateMsgLayer>
{
public:
    ClusterGateMsgLayer();
    ~ClusterGateMsgLayer() {}

    virtual bool Init();
    virtual int DealPkg();
    int SendToMatchSvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToMatchSvr(MyTdrBuf* pstTdrBuf);
    int SendToSerialNumSvr(MyTdrBuf* pstTdrBuf);
	int SendToMineSvr(MyTdrBuf* pstTdrBuf);
	int SendToGuildExpeditionSvr(MyTdrBuf* pstTdrBuf);
protected:
    virtual void _RegistServerMsg();
    void _DealSvrPkg();
    bool _ForwardPkg(MyTdrBuf* pstTdrBuf);
};

