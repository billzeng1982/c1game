#pragma once

// 多线程版本

#include "MsgLayerBase_r.h"

class XiYouSDKSvrMsgLayer_r : public MsgLayerBase_r
{
public:
	XiYouSDKSvrMsgLayer_r(){}
	virtual ~XiYouSDKSvrMsgLayer_r() {}

	virtual bool Init();

protected:
	virtual void _RegistServerMsg();
};



