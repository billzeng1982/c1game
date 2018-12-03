#pragma once

// 多线程版本
#include "MsgLayerBase_r.h"

class MiscSvrMsgLayer_r : public MsgLayerBase_r
{
public:
    MiscSvrMsgLayer_r(){}
	virtual ~MiscSvrMsgLayer_r() {}

	virtual bool Init();

protected:
	virtual void _RegistServerMsg();
};


