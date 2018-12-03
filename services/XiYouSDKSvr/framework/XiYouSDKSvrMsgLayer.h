#pragma once

#include "singleton.h"
#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase.h"
#include "MsgLayerBase.h"
#include "../cfg/XiYouSDKSvrCfgDesc.h"

class XiYouSDKSvrMsgLayer: public CMsgLayerBase, public TSingleton<XiYouSDKSvrMsgLayer>
{
public:
	XiYouSDKSvrMsgLayer();
	virtual ~XiYouSDKSvrMsgLayer(){}

	virtual bool Init();

	virtual int DealPkg();

	int SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg);

    IMsgBase* GetMsgHandler(int iMsgID);

private:
	virtual void _RegistServerMsg();
    void _ForwardToWorkThread(MyTdrBuf* pstTdrBuf);
    int _HandleWorkThreadRsp();

private:
    XIYOUSDKSVRCFG* m_pstConfig;
    int m_iThreadIdx;
};



