#include "MessageMgr.h"
#include "common_proto.h"
#include "LogMacros.h"
#include "hash_func.h"
#include "./MessageInfo/DBWorkThreadMgr.h"
#include "../framework/MessageTransFrame.h"

using namespace PKGMETA;

MessageMgr::MessageMgr()
{

}


bool MessageMgr::Init(MESSAGESVRCFG* pstConfig)
{
    if (NULL == pstConfig)
    {
        LOGERR_r("pstConfig is NULL ,MessageMgr Init Failed!");
        return false;
    }
    if (0 == pstConfig->m_iUpdateIntervalTime && 0 == pstConfig->m_iUpdateDirtyNum)
    {
        LOGERR_r("In MESSAGESVRCFG, 'm_iUpdateIntervalTime' and 'm_iUpdateDirtyNum' are both '0' !");
        return false;
    }
    m_iUpdateIntervalTime = pstConfig->m_iUpdateIntervalTime;
    m_iUpdateDirtyNum = pstConfig->m_iUpdateDirtyNum;
    m_iPoolSize = pstConfig->m_wMessageMaxNum;
    if (m_oMessagePool.CreatePool(m_iPoolSize) < 0)
    {
        LOGERR_r("Create oBuffMessage pool[num=%d] failed.", m_iPoolSize);
        return false;
    }
    m_oMessagePool.RegisterSlicedIter(&m_oUptIter);
    TLIST_INIT(&m_stTimeListHead);
    TLIST_INIT(&m_stDirtyListHead);
    m_oMessageSet.clear();
    m_iDirtyNodeNum = 0;
    m_iTimeNodeNum = 0;
    return true;
}

//
void MessageMgr::Update(bool bIdle)
{
    time_t tCurTimeMs = 0;
    Message* poMessage = NULL;
    tCurTimeMs = CGameTime::Instance().GetCurrTimeMs();
    if (!(	(0 != m_iUpdateDirtyNum  && m_iDirtyNodeNum >= m_iUpdateDirtyNum) || 
            (0 != m_iUpdateIntervalTime && tCurTimeMs - m_tLastUpdateTimeMs >=  m_iUpdateIntervalTime)	))
    {//不满足回写条件,
        return;
    }

    int iCheckNum = MIN(bIdle ? 100 : 50, m_iDirtyNodeNum);
    
    for (int iNum = 0; iNum < iCheckNum; iNum++)
    {
        poMessage = container_of(TLIST_PREV(&m_stDirtyListHead), Message, m_stDirtyListNode);
        
        //bzero(&m_stDBReq, sizeof(m_stDBReq));
        if (!poMessage->PackMessageWholeData(m_stDBReq.m_stWholeData))
        {
            LOGERR_r("Pack Message failed!Owner<%lu> Channel<%hhu> !", poMessage->GetMessageOwner(), poMessage->GetMessageChannel());
            continue;
        }
        m_stDBReq.m_bType = MESSAGE_DB_TYPE_UPDATE;
        DBWorkThreadMgr::Instance().SendReq(m_stDBReq);
        LOGWARN_r(" update Message to DB, id<%lu>, Channel<%hhu> TimeNodeNum<%d> DirtyNodeNum<%d>", 
            poMessage->GetMessageOwner(), poMessage->GetMessageChannel(), m_iTimeNodeNum, m_iDirtyNodeNum);
        //不回写内存,切换到TimeList
        DelDirtyList(&poMessage->m_stDirtyListNode);
        AddTimeList(&poMessage->m_stTimeListNode);
        //把有修改的数据写会DB,释放节点内存
        //DelMessage(poMessage);

    }
    m_tLastUpdateTimeMs = tCurTimeMs;
}

void MessageMgr::Fini()
{
    for (m_oUptIter.Begin(); !m_oUptIter.IsEnd(); m_oUptIter.Next() )
    {
        Message* poMessage = m_oUptIter.CurrItem();
        if (NULL == poMessage)
        {
            LOGERR_r("poMessage is null");
            continue;
        }
        if (!poMessage->PackMessageWholeData(m_stDBReq.m_stWholeData))
        {
            LOGERR_r("Pack Message(%lu) failed", poMessage->GetMessageOwner());
            continue;
        }
        m_stDBReq.m_bType = MESSAGE_DB_TYPE_UPDATE;
        DBWorkThreadMgr::Instance().SendReq(m_stDBReq);
    }
    return;
}


//从内存池中分配一个新空间
Message* MessageMgr::NewMessage()
{
    Message* poMessage = NULL;
    if (m_oMessagePool.GetUsedNum() < m_iPoolSize)
    {//内存池中还有空的节点
        poMessage = m_oMessagePool.NewData();
        if (NULL == poMessage)
        {
            LOGERR_r("poMessage is NULL, Get New Message failed");
            return NULL;
        }
        poMessage->Reset();
        return poMessage;
    }
    else
    {   
        TLISTNODE* pstSwapNode = NULL;
        //从TimeList或DirtyList中选取相应节点,并回写DB
        if (TLIST_IS_EMPTY(&m_stTimeListHead))
        {//LRU列表为空,回写DirtyList
            pstSwapNode = TLIST_PREV(&m_stDirtyListHead);
            poMessage = container_of(pstSwapNode, Message, m_stDirtyListNode);
            DelDirtyList(pstSwapNode);
            if (!poMessage->PackMessageWholeData(m_stDBReq.m_stWholeData))
            {
                LOGERR_r("Pack Message(%lu) failed", poMessage->GetMessageOwner());
                return NULL;
            }
            m_stDBReq.m_bType = MESSAGE_DB_TYPE_UPDATE;
            DBWorkThreadMgr::Instance().SendReq(m_stDBReq);
        }
        else
        {//回收TimeList中的节点,
            pstSwapNode = TLIST_PREV(&m_stTimeListHead);
            poMessage = container_of(pstSwapNode, Message, m_stTimeListNode);
            DelTimeList(pstSwapNode);							// 删除LRU时间队列
        }
        DelFromMessageSet(poMessage);			//删除Map
        poMessage->Reset();
        return poMessage;
    }
}
//	删除Message整个对象,并释放其所拥有的所有记录内存,删除关联的Map映射等
int MessageMgr::DelMessage(Message* poMessage)
{
    DelTimeList(&poMessage->m_stTimeListNode);
    DelDirtyList(&poMessage->m_stDirtyListNode);
    poMessage->DelMessageBox();
    // delete from all dict
    DelFromMessageSet(poMessage);
    m_oMessagePool.DeleteData(poMessage);
    return ERR_NONE;
    
}

Message* MessageMgr::GetMessageByKey(uint64_t ullUin, uint8_t bChannel)
{
    m_stTmpMessage.SetFindKey(ullUin, bChannel);
    m_oMessageSetIter = m_oMessageSet.find(&m_stTmpMessage);
    if (m_oMessageSetIter != m_oMessageSet.end())
    {
        MoveToTimeListHead( &(*m_oMessageSetIter)->m_stTimeListNode );
        return *m_oMessageSetIter;
    }
    return NULL;
}

void MessageMgr::AddToMessageSet(Message* poMessage)
{
    if (NULL == poMessage)
    {
        LOGERR_r("poMessage is null.");
        return ;
    }
    m_oMessageSet.insert(poMessage);
    return;
}

void MessageMgr::DelFromMessageSet(Message* poMessage)
{
    if (NULL == poMessage)
    {
        LOGERR_r("poMessage is null");
        return;
    }
    m_oMessageSetIter = m_oMessageSet.find(poMessage);
    if (m_oMessageSetIter != m_oMessageSet.end())
    {
        m_oMessageSet.erase(m_oMessageSetIter);
    }
    return;
}


//客户端请求,发送留言给某玩家
int MessageMgr::MessageSend(DT_MESSAGE_ONE_RECORD_INFO& rstRecord)
{
    Message* poMessage = GetMessageByKey(rstRecord.m_ullReceiverUin, rstRecord.m_bChannel);
    if (NULL == poMessage)
    {
        LOGERR_r("poMessage is null,GetMessageByUin<%lu> Channel<%hhu>error",rstRecord.m_ullReceiverUin, rstRecord.m_bChannel);
        return ERR_SYS;
    }
    int iRet = poMessage->AddRecord(rstRecord);
    if (ERR_NONE != iRet)
    {
        return iRet;
    }
    //从TimeList中删除,并加入到DiryList
    DelTimeList(&poMessage->m_stTimeListNode);
    AddDirtyList(&poMessage->m_stDirtyListNode);
    return iRet;
}

int MessageMgr::MessageGetPlayerAllBox(uint64_t ullUin, uint64_t ullGuild, OUT SS_PKG_MESSAGE_GET_BOX_RSP& rstSSBodyRsp)
{
    rstSSBodyRsp.m_stPrivateBox.m_wCount = 0;
    rstSSBodyRsp.m_stGuildBox.m_wCount = 0;
    Message* poMessage = GetMessageByKey(ullUin, MESSAGE_CHANNEL_PRIVATE);
    if (NULL == poMessage)
    {
        LOGERR_r("poMessage is null,GetMessageByUin<%lu> PRIVATE  error", ullUin);
    }
    else
    {
        poMessage->GetMessage(rstSSBodyRsp.m_stPrivateBox);
    }
    if (ullGuild == 0)
    {
        return ERR_NONE;
    }
    poMessage = GetMessageByKey(ullGuild, MESSAGE_CHANNEL_GUILD);
    if (NULL == poMessage)
    {
        LOGERR_r("poMessage is null,GetMessageByUin<%lu> GUILD  error", ullUin);
    }
    else
    {
        poMessage->GetMessage(rstSSBodyRsp.m_stGuildBox);
    }
     return ERR_NONE;
}

//客户端请求,删除所有玩家所有留言信息
int MessageMgr::MessageDelBox(uint64_t ullUin, uint8_t bChannel)
{
    Message* poMessage = GetMessageByKey(ullUin, bChannel);
    if (NULL == poMessage)
    {
        LOGERR_r("poMessage is null,GetMessageByUin<%lu> channel<%hhu>  error", ullUin, bChannel);
        return ERR_NONE;
    }
    poMessage->DelMessageBox();
    DelTimeList(&poMessage->m_stTimeListNode);
    AddDirtyList(&poMessage->m_stDirtyListNode);
    return ERR_NONE;
}


int MessageMgr::HandleSSMsg(SSPKG& rstSsReqPkg)
{
    int iRet = 0;
    switch (rstSsReqPkg.m_stHead.m_wMsgId)
    {
    case SS_MSG_MESSAGE_GET_BOX_REQ:
        {
            SS_PKG_MESSAGE_GET_BOX_REQ& rstGetBoxReq = rstSsReqPkg.m_stBody.m_stMessageGetBoxReq;
            iRet = MessageGetPlayerAllBox(rstGetBoxReq.m_ullUin, rstGetBoxReq.m_ullGuild, m_stSsRspPkg.m_stBody.m_stMessageGetBoxRsp);
            m_stSsRspPkg.m_stHead.m_wMsgId = SS_MSG_MESSAGE_GET_BOX_RSP;
            m_stSsRspPkg.m_stHead.m_ullUin = rstSsReqPkg.m_stHead.m_ullUin;
            m_stSsRspPkg.m_stBody.m_stMessageGetBoxRsp.m_nErrNo = iRet;
            MessageSvrMsgLayer::Instance().SendToZoneSvr(m_stSsRspPkg);
        }
        break;
    case SS_MSG_MESSAGE_DEL_BOX_REQ:
        {
            SS_PKG_MESSAGE_DEL_BOX_REQ& rstDelBoxReq = rstSsReqPkg.m_stBody.m_stMessageDelBoxReq;
            iRet = MessageDelBox(rstDelBoxReq.m_ullUin, rstDelBoxReq.m_bChannel);
            m_stSsRspPkg.m_stHead.m_wMsgId = SS_MSG_MESSAGE_DEL_BOX_RSP;
            m_stSsRspPkg.m_stHead.m_ullUin = rstSsReqPkg.m_stHead.m_ullUin;
            m_stSsRspPkg.m_stBody.m_stMessageDelBoxRsp.m_nErrNo = iRet;
            MessageSvrMsgLayer::Instance().SendToZoneSvr(m_stSsRspPkg);
        }
        break;
    case SS_MSG_MESSAGE_SEND_REQ:
    {
        iRet = MessageSend(rstSsReqPkg.m_stBody.m_stMessageSendReq.m_stRecord);

    }
    break;
    default:
        iRet = ERR_WRONG_PARA;
        break;
    }
    return iRet;
}

void MessageMgr::HandleDBThreadRsp(DT_MESSAGE_DB_RSP& rstDBRsp)
{
    switch (rstDBRsp.m_bType)
    {
    case MESSAGE_DB_TYPE_GET:
        _HandleDBThreadRspGetMessage(rstDBRsp);
        break;
    default:
        LOGERR_r("Message DBThread type error!");
        break;
    }
}

void MessageMgr::_HandleDBThreadRspGetMessage(DT_MESSAGE_DB_RSP& rstDBRsp)
{
    //@TODO_DEBUG_DEL  删除
    //LOGWARN_r("_HandleDBThreadRspGetMessage to DB! Type<%d>,  , Uin<%lu> " , rstDBRsp.m_bType ,rstDBRsp.m_stWholeData.m_stBaseInfo.m_ullUin);
    if (NULL != MessageMgr::GetMessageByKey(rstDBRsp.m_stWholeData.m_stBaseInfo.m_ullUin, rstDBRsp.m_stWholeData.m_stBaseInfo.m_bChannel))
    {
        //别的请求已获取到了,强制唤醒事务
        MessageTransFrame::Instance().AsyncActionDone(rstDBRsp.m_ullToken, (void*)1, 1);
        return;
    }
    Message* poMessage = NULL;
    if (ERR_NONE == rstDBRsp.m_nErrNo || ERR_NOT_FOUND == rstDBRsp.m_nErrNo )
    {
        poMessage = NewMessage();
        if (NULL == poMessage)
        {
            LOGERR_r("new poMessage error");
            //强制唤醒
            MessageTransFrame::Instance().AsyncActionDone(rstDBRsp.m_ullToken, (void*)1, 1);
            return;
        }
        if (ERR_NOT_FOUND == rstDBRsp.m_nErrNo)
        {
            //LOGRUN_r("DB RSP: Initnew <%lu> ", rstDBRsp.m_stWholeData.m_stBaseInfo.m_ullUin);
            poMessage->InitNew(rstDBRsp.m_stWholeData.m_stBaseInfo.m_ullUin, rstDBRsp.m_stWholeData.m_stBaseInfo.m_bChannel);
            AddTimeList(&poMessage->m_stTimeListNode);
            AddToMessageSet(poMessage);   //加入map里
        }
        else
        {
            if (poMessage->InitFromDB(rstDBRsp.m_stWholeData))
            {
                //LOGRUN_r("DB RSP: InitFromDB <%lu> <%hhu>", rstDBRsp.m_stWholeData.m_stBaseInfo.m_ullUin, rstDBRsp.m_stWholeData.m_stBaseInfo.m_bChannel);
                AddTimeList(&poMessage->m_stTimeListNode);
                AddToMessageSet(poMessage);   //加入map里
            }
            else
            {
                m_oMessagePool.DeleteData(poMessage);
                poMessage = NULL;
            }
        }
    }

    //强制唤醒
    MessageTransFrame::Instance().AsyncActionDone(rstDBRsp.m_ullToken, (void*)1, 1);
    return;
}

void MessageMgr::AddDirtyList(TLISTNODE* pNode) 
{ 
    if (!TLIST_IS_EMPTY(pNode))
    {
        return;
    }

    LOGWARN_r("AddDirtyList node %lu DirtyNum<%d>", container_of(pNode, Message, m_stDirtyListNode)->GetMessageOwner(), m_iDirtyNodeNum);
    TLIST_INSERT_NEXT(&m_stDirtyListHead, pNode);
    m_iDirtyNodeNum++; 
}

void MessageMgr::DelDirtyList(TLISTNODE* pNode) 
{
    if (TLIST_IS_EMPTY(pNode))
    {
        return;
    }
    LOGWARN_r("DelDirtyList node %lu DirtyNum<%d>", container_of(pNode, Message, m_stDirtyListNode)->GetMessageOwner(), m_iDirtyNodeNum);
    TLIST_DEL(pNode); 
    m_iDirtyNodeNum--;  
}

void MessageMgr::AddTimeList(TLISTNODE* pNode) 
{ 
    if (!TLIST_IS_EMPTY(pNode))
    {
        return;
    }
    LOGWARN_r("AddTimeList node %lu TimeNum<%d>", container_of(pNode, Message, m_stTimeListNode)->GetMessageOwner(), m_iTimeNodeNum);
    TLIST_INSERT_NEXT(&m_stTimeListHead, pNode);
    m_iTimeNodeNum++;

}

void MessageMgr::DelTimeList(TLISTNODE* pNode) 
{
    if (TLIST_IS_EMPTY(pNode))
    {
        return;
    }
    LOGWARN_r("DelTimeList node %lu TimeNum<%d>", container_of(pNode, Message, m_stTimeListNode)->GetMessageOwner(), m_iTimeNodeNum);
    TLIST_DEL(pNode); 
    m_iTimeNodeNum--;
}

void MessageMgr::MoveToTimeListHead(TLISTNODE* pNode)
{
    if (TLIST_IS_EMPTY(pNode))
    {
        return;
    }
    TLIST_DEL(pNode);
    TLIST_INSERT_NEXT(&m_stTimeListHead, pNode);
}

