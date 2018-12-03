#pragma once

#include "object.h"
#include "ss_proto.h"
#include "../../framework/MessageSvrMsgLayer.h"
#include "list_i.h"

using namespace PKGMETA;

struct OneRecordObj : public IObject
{
    DT_MESSAGE_ONE_RECORD_INFO stOneRecord;

};

struct MESSAGE_BOX_ADDR_INFO
{
    uint16_t m_wCount; // 留言条数
    uint16_t m_wLastPos; // 当前最早留言的位置
    OneRecordObj* m_astAllRecordObjs[MAX_MESSAGE_RECORD_CNT];
    
};



class Message
{
public:
    Message() {};
    ~Message() {};
    bool operator < (const Message &m2) const
    {
        return m_stMessageBaseInfo.m_ullUin < m2.m_stMessageBaseInfo.m_ullUin
            || (m_stMessageBaseInfo.m_ullUin == m2.m_stMessageBaseInfo.m_ullUin
                && m_stMessageBaseInfo.m_bChannel < m2.m_stMessageBaseInfo.m_bChannel);
    }
public:
    void SetFindKey(uint64_t ullUin, uint8_t bChannel) 
    {
        m_stMessageBaseInfo.m_ullUin = ullUin;
        m_stMessageBaseInfo.m_bChannel = bChannel;
    }
    bool InitFromDB(IN DT_MESSAGE_WHOLE_DATA& rstMessageWholeData);
    bool InitNew(uint64_t ullUin, uint8_t bChannel);
    bool PackMessageWholeData(DT_MESSAGE_WHOLE_DATA& rstMessageWholeData);
    void GetMessage(OUT DT_MESSAGE_BOX_INFO& rstBoxInfo);
    uint64_t GetMessageOwner() {return m_stMessageBaseInfo.m_ullUin;} 
    uint8_t GetMessageChannel() {return m_stMessageBaseInfo.m_bChannel;}
    int AddRecord(DT_MESSAGE_ONE_RECORD_INFO& rstRecord);
    void DelMessageBox();
    void Reset();
    void UpdateMessageWholeData();
public:
    TLISTNODE m_stTimeListNode;			// LRU TimeList 节点.
    TLISTNODE m_stDirtyListNode;		//待写数据链,回收内存池
private:
    DT_MESSAGE_BASE_INFO m_stMessageBaseInfo;
    MESSAGE_BOX_ADDR_INFO m_stMessageBoxAddtInfo;
};

