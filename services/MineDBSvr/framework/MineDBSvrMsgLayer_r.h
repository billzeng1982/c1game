#pragma once

// 多线程版本
#include "MsgLayerBase_r.h"

class MineDBSvrMsgLayer_r : public MsgLayerBase_r
{
public:
    MineDBSvrMsgLayer_r(){}
	virtual ~MineDBSvrMsgLayer_r() {}

	virtual bool Init();

protected:
	virtual void _RegistServerMsg();
};


