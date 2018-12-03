#pragma once

#include "TransactionFrame.h"
#include "singleton.h"

class MessageTransFrame : public TSingleton<MessageTransFrame>
{
private:
	static const uint32_t ACTION_TIMEOUT_TIME = 10000;

public:
	bool Init(uint32_t dwMaxTransaction, uint32_t dwMaxCompositeAction);

	void ScheduleTransaction(Transaction* poTransaction)
	{
		m_oFrame.ScheduleTransaction(poTransaction, ACTION_TIMEOUT_TIME);
	}

	void Update()
	{
		m_oFrame.Update();
	}

	void AsyncActionDone(TActionToken ullToken, void* pvData, uint32_t dwDataLen)
	{
		m_oFrame.AsyncActionDone(ullToken, pvData, dwDataLen);
	}

    void Fini() 
    {
        m_oFrame.Fini();
    }
private:
	TransactionFrame m_oFrame;
};

