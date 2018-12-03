#pragma once

#include "singleton.h"
#include "GameTimer.h"

class TimeSyncTimer : public GameTimer
{
public:
	TimeSyncTimer();
	virtual ~TimeSyncTimer(){};
};

