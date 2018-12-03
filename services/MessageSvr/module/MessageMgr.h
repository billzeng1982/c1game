#pragma once

#include "define.h"
#include "singleton.h"
#include "mempool.h"
#include "iterator.h"
#include "functors.h"
#include "list_i.h"
#include "./MessageInfo/Message.h"
#include <set>

using namespace PKGMETA;
using std::set;

class MessageCmp
{
public:
    bool operator() (Message* const& left, Message* const& right) const
    {
        return *left < *right;
    }
};

class MessageMgr : public TSingleton<MessageMgr>
{

private:


    typedef set<Message*, MessageCmp> MessageSet_t;
public:
    MessageMgr();
    virtual ~MessageMgr() {};
    bool Init(MESSAGESVRCFG* pstConfig);
    void Update(bool bIdle);
    void Fini();

    Message* NewMessage();
    Message* GetMessageByKey(uint64_t ullUin, uint8_t bChannel);
    int DelMessage(Message* poMessage);
    int HandleSSMsg(SSPKG& rstSsReqPkg);

public:     //  逻辑消息操作
    int MessageSend(DT_MESSAGE_ONE_RECORD_INFO& rstRecord);
    int MessageGetPlayerAllBox(uint64_t ullUin, uint64_t ullGuild, OUT SS_PKG_MESSAGE_GET_BOX_RSP& rstSSBodyRsp);
    int MessageDelBox(uint64_t ullUin, uint8_t bChannel);
public:
    void AddToMessageSet(Message* poMessage);
    void DelFromMessageSet(Message* poMessage);
    void HandleDBThreadRsp(DT_MESSAGE_DB_RSP& rstDBRsp);
    void _HandleDBThreadRspGetMessage(DT_MESSAGE_DB_RSP& rstDBRsp);
public:
    void DelDirtyList(TLISTNODE* pNode);
    void AddDirtyList(TLISTNODE* pNode);
    void AddTimeList(TLISTNODE* pNode);
    void DelTimeList(TLISTNODE* pNode);
    void MoveToTimeListHead(TLISTNODE* pNode);
public:
    DT_MESSAGE_WHOLE_DATA m_oDTMessageWohleData;    
    TLISTNODE m_stTimeListHead;			// LRU TimeList 头节点
    TLISTNODE m_stDirtyListHead;		//待写数据链表头,回收内存池
    int m_iDirtyNodeNum;				//脏  数据个数
    int m_iTimeNodeNum;					//干净数据个数
    time_t m_tLastUpdateTimeMs;			//上次回写数据库时间
    //先检查脏数据个数是否达到条件,达到条件回写
    //再检查上次回写时间,达到条件回写
    // 
    int m_iUpdateIntervalTime;			//回写策略, 间隔时间(毫秒) 
    int m_iUpdateDirtyNum;				//回写策略,脏数据个数
private:
    int m_iPoolSize;
    CMemPool<Message> m_oMessagePool;
    CMemPool<Message>::UsedIterator m_oUptIter;   // update iterator;
    SSPKG m_stSsRspPkg;
    DT_MESSAGE_DB_REQ m_stDBReq;
    MESSAGESVRCFG* m_pstConfig;
private:
    Message m_stTmpMessage;
    MessageSet_t m_oMessageSet;
    MessageSet_t::iterator m_oMessageSetIter;

};

// #define DEL_DIRTY_LIST(Node) do{ TLIST_DEL(Node);m_iDirtyNum--; } while (0)
// #define ADD_DIRTY_LIST(Node) do{ TLIST_INSERT_NEXT(&m_stDirtyListHead, (Node));m_iDirtyNum++; } while (0)

