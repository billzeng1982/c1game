#include "../module/MessageMgr.h"
#include "../framework/MessageTransFrame.h"
#include "LogMacros.h"
#include "../framework/MessageSvrMsgLayer.h"
#include "MessageTransaction.h"
#include "./MessageInfo/DBWorkThreadMgr.h"

void MessageTransaction::Reset()
{
    Transaction::Reset();
    bzero(&m_stSsReqPkg, sizeof(m_stSsReqPkg));
    m_iActionErrNo = 0;
    return;
}
//事务中的动作执行完后开始执行业务逻辑
void MessageTransaction::OnFinished(int iErrCode)
{
    LOGRUN_r("MessageTransaction OnFinished! iErrCode<%d>, ReqMsgId<%u>",iErrCode, m_stSsReqPkg.m_stHead.m_wMsgId);
    MessageMgr::Instance().HandleSSMsg(m_stSsReqPkg);
    return;
}


void GetMessageAction::Reset()
{
    IAction::Reset();
    m_pObjTrans = NULL;
    bzero(&m_stDBReq, sizeof(m_stDBReq));
    return;
}

int GetMessageAction::Execute(Transaction* pObjTrans)
{
    assert(pObjTrans != NULL);
    m_pObjTrans = (MessageTransaction*)pObjTrans;
    uint64_t ullToken = (uint64_t)this->GetToken();
    Message* poMessage = MessageMgr::Instance().GetMessageByKey(m_ullUin, m_bChannel);
    if (NULL == poMessage)
    {
        m_stDBReq.m_bType = MESSAGE_DB_TYPE_GET;
        m_stDBReq.m_ullToken = ullToken;
        m_stDBReq.m_stWholeData.m_stBaseInfo.m_ullUin = m_ullUin;
        m_stDBReq.m_stWholeData.m_stBaseInfo.m_bChannel = m_bChannel;
        DBWorkThreadMgr::Instance().SendReq(m_stDBReq);
        //@TODO_DEBUG_DEL
        LOGRUN_r("GetMessageAction Wait Async OK! Uin<%lu>, Channel<%hhu> Token<%lu>", m_ullUin, m_bChannel, ullToken);
        return 0;
    }
    //@TODO_DEBUG_DEL
    LOGRUN_r("GetMessageAction Execute OK! Uin<%lu>, Channel<%hhu> Token<%lu>", m_ullUin,  m_bChannel, ullToken);
    this->SetFiniFlag(1);
    return 1;
}

void GetMessageAction::OnAsyncRspMsg(TActionIndex iActionIdx, const void* pResult, unsigned int dwResLen)
{
    //@TODO_DEBUG_DEL
    LOGRUN_r("GetMessageAction OnAsyncRspMsg OK! Uin<%lu>, Channel<%hhu> Token<%lu>", m_ullUin, m_bChannel, (uint64_t)GetToken());
    this->SetFiniFlag(1);
}

