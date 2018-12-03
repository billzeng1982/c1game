#pragma once

// 多线程版本
#include "MsgLayerBase_r.h"

class ClusterDBSvrMsgLayer_r : public MsgLayerBase_r
{
public:
    ClusterDBSvrMsgLayer_r(){}
	virtual ~ClusterDBSvrMsgLayer_r() {}

	virtual bool Init();

protected:
	virtual void _RegistServerMsg();
};


