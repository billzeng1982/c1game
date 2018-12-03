#pragma once

// 多线程版本

#include "MsgLayerBase_r.h"

class RoleSvrMsgLayer_r : public MsgLayerBase_r
{
public:
	RoleSvrMsgLayer_r(){}
	virtual ~RoleSvrMsgLayer_r() {}

	virtual bool Init();

protected:
	virtual void _RegistServerMsg();
};

