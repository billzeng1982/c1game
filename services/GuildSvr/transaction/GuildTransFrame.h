#pragma once

#include "TransactionFrame.h"
#include "singleton.h"
#include "ss_proto.h"

using namespace PKGMETA;

class GuildTransFrame : public TSingleton<GuildTransFrame>
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

    void AddCreateGuildTrans(uint64_t ullPlayerId, const char* pszGuildName, SSPKG& rstSsPkg);

    void AddGetGuildTrans(uint64_t ullPlayerId, uint64_t ullGuildId, SSPKG& rstSsPkg);

    void AddRstPlayerTrans(uint64_t ullPlayerId);

	void AddGetGuildByNameTrans(char* pszName, SSPKG& rstSsPkg);

private:
    TransactionFrame m_oFrame;
};
