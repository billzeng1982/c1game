#include <string.h>
#include "strutil.h"
#include "LogMacros.h"
#include "MsgLogicMessage.h"
#include "ss_proto.h"
#include "../framework/MessageSvrMsgLayer.h"
#include "../module/MessageMgr.h"
#include "../module/MessageTransaction.h"
#include "../framework/GameObjectPool.h"
#include "../framework/MessageTransFrame.h"
using namespace PKGMETA;

//  发送消息
int MessageSendReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    DT_MESSAGE_ONE_RECORD_INFO& rstRecord = rstSsPkg.m_stBody.m_stMessageSendReq.m_stRecord;
	Message* poMessage = MessageMgr::Instance().GetMessageByKey(rstRecord.m_ullReceiverUin, rstRecord.m_bChannel);
	if (poMessage)
	{
		MessageMgr::Instance().HandleSSMsg(rstSsPkg);
	}
	else
    {
        MessageTransaction* poSendMessageTrans = GET_GAMEOBJECT(MessageTransaction, GAMEOBJ_MESSAGE_TRANSACTION);
        assert(poSendMessageTrans != NULL);
        GetMessageAction* poGetMessageAction = GET_GAMEOBJECT(GetMessageAction, GAMEOBJ_GET_MESSAGE_ACTION);
        assert(poGetMessageAction != NULL);
        poSendMessageTrans->Reset();
        poGetMessageAction->Reset();
        poSendMessageTrans->AddAction(poGetMessageAction);
        poGetMessageAction->SetKey(rstRecord.m_ullReceiverUin, rstRecord.m_bChannel);
        memcpy(&poSendMessageTrans->m_stSsReqPkg, &rstSsPkg, sizeof(rstSsPkg));  //还是要把发送包传进去
        MessageTransFrame::Instance().ScheduleTransaction(poSendMessageTrans);
    }
    return 0;
}

//获取
int MessageGetBoxReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_MESSAGE_GET_BOX_REQ& rstReq = rstSsPkg.m_stBody.m_stMessageGetBoxReq;
	Message* poPrivateMessage = MessageMgr::Instance().GetMessageByKey(rstReq.m_ullUin, MESSAGE_CHANNEL_PRIVATE);
    Message* poGuildMessage = MessageMgr::Instance().GetMessageByKey(rstReq.m_ullGuild, MESSAGE_CHANNEL_GUILD);
	if (poPrivateMessage && poGuildMessage)
	{
		MessageMgr::Instance().HandleSSMsg(rstSsPkg);
	}
	else
    {
        MessageTransaction* poGetBoxTrans = GET_GAMEOBJECT(MessageTransaction, GAMEOBJ_MESSAGE_TRANSACTION);
        assert(poGetBoxTrans);
        poGetBoxTrans->Reset();
        if (!poPrivateMessage)
        {
            GetMessageAction* poGetPrivateMessageAction = GET_GAMEOBJECT(GetMessageAction, GAMEOBJ_GET_MESSAGE_ACTION);
            assert(poGetPrivateMessageAction);
            poGetPrivateMessageAction->Reset();
            poGetPrivateMessageAction->SetKey(rstReq.m_ullUin, MESSAGE_CHANNEL_PRIVATE);
            poGetBoxTrans->AddAction(poGetPrivateMessageAction);
        }
        if (rstReq.m_ullGuild != 0 && !poGuildMessage)
        {
            GetMessageAction* poGetGuildMessageAction = GET_GAMEOBJECT(GetMessageAction, GAMEOBJ_GET_MESSAGE_ACTION);
            assert(poGetGuildMessageAction);
            poGetGuildMessageAction->Reset();
            poGetGuildMessageAction->SetKey(rstReq.m_ullGuild, MESSAGE_CHANNEL_GUILD);
            poGetBoxTrans->AddAction(poGetGuildMessageAction);
        }

        memcpy(&poGetBoxTrans->m_stSsReqPkg, &rstSsPkg, sizeof(rstSsPkg));  //还是要把发送包传进去
        MessageTransFrame::Instance().ScheduleTransaction(poGetBoxTrans);
    }
    return 0;
}

//删除
int MessageDelBoxReq_SS::HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara)
{
    // 只删除 私人聊天记录

    SS_PKG_MESSAGE_DEL_BOX_REQ& rstReq = rstSsPkg.m_stBody.m_stMessageDelBoxReq;

    Message* poPrivateMessage = MessageMgr::Instance().GetMessageByKey(rstReq.m_ullUin, MESSAGE_CHANNEL_PRIVATE);
    if (poPrivateMessage)
    {
        MessageMgr::Instance().HandleSSMsg(rstSsPkg);
    }
    else
    {
        MessageTransaction* poGetBoxTrans = GET_GAMEOBJECT(MessageTransaction, GAMEOBJ_MESSAGE_TRANSACTION);
        assert(poGetBoxTrans);
        GetMessageAction* poGetMessageAction = GET_GAMEOBJECT(GetMessageAction, GAMEOBJ_GET_MESSAGE_ACTION);
        assert(poGetMessageAction);
        poGetBoxTrans->Reset();
        poGetMessageAction->Reset();
        poGetMessageAction->SetKey(rstReq.m_ullUin, MESSAGE_CHANNEL_PRIVATE);
        poGetBoxTrans->AddAction(poGetMessageAction);
        memcpy(&poGetBoxTrans->m_stSsReqPkg, &rstSsPkg, sizeof(rstSsPkg));  //还是要把发送包传进去
        MessageTransFrame::Instance().ScheduleTransaction(poGetBoxTrans);
    }

    return 0;
}
