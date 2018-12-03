#pragma once


#include "object.h"
#include "TransactionFrame.h"
#include "../module/MessageInfo/Message.h"
#include "ss_proto.h"

using namespace PKGMETA;

class MessageTransaction : public Transaction
{
public:
    MessageTransaction() {}
    virtual ~MessageTransaction() {}
    virtual void Reset() ;
    virtual void OnFinished(int iErrCode);
public: 
    SSPKG m_stSsReqPkg;
    int m_iActionErrNo;
};


class GetMessageAction : public IAction
{
public:
    GetMessageAction() { }
    virtual ~GetMessageAction() { }
    virtual void Reset();
    virtual void OnFinished() { LOGRUN_r("GetMessageAction OnFinished! Uin<%lu> Channel<%hhu> Token<%lu>",
        m_ullUin, m_bChannel, (uint64_t)GetToken()); }
    virtual int Execute(Transaction* pObjTrans);
    virtual void OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen);
    void SetKey(uint64_t ullUin, uint8_t bChannel){ m_ullUin = ullUin; m_bChannel = bChannel;}
private:
    uint64_t m_ullUin;
    uint8_t m_bChannel;
    MessageTransaction* m_pObjTrans;
    DT_MESSAGE_DB_REQ m_stDBReq;
};