#pragma once

#include "object.h"
#include "TransactionFrame.h"
#include "../module/Guild/Guild.h"
#include "../module/Player/Player.h"
#include "ss_proto.h"
#include "strutil.h"

using namespace PKGMETA;

class GuildTransaction : public Transaction
{
public:
    GuildTransaction() {}
    virtual ~GuildTransaction() {}

    virtual void Reset();

    void SetGuild(Guild* poGuild) { m_poGuild = poGuild; }
    Guild* GetGuild() { return m_poGuild; }

    void SetPlayer(Player* poPlayer) { m_poPlayer = poPlayer; }
    Player* GetPlayer() { return m_poPlayer; }

    void SetErrNo(int iErrNo) { m_iErrNo = iErrNo; }

    void SetGuildId(uint64_t ullGuildId) { m_ullGuildId = ullGuildId; }
    uint64_t GetGuildId() { return m_ullGuildId; }

    void SaveReqPkg(SSPKG& rstSsPkg) { memcpy(&m_stSsReqPkg, &rstSsPkg, sizeof(m_stSsReqPkg)); }
    void SetMsgId(uint16_t wMsgId) { m_stSsReqPkg.m_stHead.m_wMsgId = wMsgId; }

    virtual void OnFinished(int iErrCode);

protected:
    void _HandleGuildCreateReq();

private:
    SSPKG m_stSsReqPkg;
    Guild* m_poGuild;
    Player* m_poPlayer;
    uint64_t m_ullGuildId;
    int m_iErrNo;
};


class GetGuildAction : public IAction
{
public:
    GetGuildAction() { }
    virtual ~GetGuildAction() { }

    void SetGuildId(uint64_t ullGuildId) { m_ullGuildId = ullGuildId; }

    virtual void Reset();

    virtual int Execute(Transaction* pObjTrans);

    virtual void OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen);

private:
    uint64_t m_ullGuildId;
    GuildTransaction* m_pObjTrans;
};


class GetGuildByNameAction : public IAction
{
public:
    GetGuildByNameAction() { }
    virtual ~GetGuildByNameAction() { }

    void SetGuildName(char* pszName) { m_pszName = pszName; }

    virtual void Reset();

    virtual int Execute(Transaction* pObjTrans);

    virtual void OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen);

private:
    char* m_pszName;
    GuildTransaction* m_pObjTrans;
};


class CreateGuildAction : public IAction
{
public:
    CreateGuildAction() { }
    virtual ~CreateGuildAction() { }

    void SetGuildName(const char* pszName)
    {
        if (pszName!=NULL)
        {
            StrCpy(m_szGuildName, pszName, MAX_NAME_LENGTH);
        }
    }

    virtual void Reset();

    virtual int Execute(Transaction* pObjTrans);

    virtual void OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen);

private:
    char m_szGuildName[MAX_NAME_LENGTH];
    GuildTransaction* m_pObjTrans;
};


class GetPlayerAction : public IAction
{
public:
    GetPlayerAction() { }
    virtual ~GetPlayerAction() { }

    void SetPlayerId(uint64_t ullPlayerId) { m_ullPlayerId = ullPlayerId; }

    virtual void Reset();

    virtual int Execute(Transaction* pObjTrans);

    virtual void OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen);

private:
    uint64_t m_ullPlayerId;
    GuildTransaction* m_pObjTrans;
};



class GuildGmUpdateInfoAction : public IAction
{
public:
    GuildGmUpdateInfoAction() { }
    virtual ~GuildGmUpdateInfoAction() { }

    void SetGuildId(uint64_t ullGuildId) { m_ullGuildId = ullGuildId; }

    virtual void Reset();

    virtual int Execute(Transaction* pObjTrans);

    virtual void OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen);

private:
    uint64_t m_ullGuildId;
    GuildTransaction* m_pObjTrans;
};
