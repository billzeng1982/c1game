#pragma once

// 多线程版本

#include "MsgLayerBase_r.h"

class AccountSvrMsgLayer_r : public MsgLayerBase_r
{
public:
	AccountSvrMsgLayer_r(){}
	virtual ~AccountSvrMsgLayer_r() {}

	virtual bool Init();

protected:
	virtual void _RegistServerMsg();
};
