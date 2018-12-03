#pragma once
#include "singleton.h"
#include "mempool.h"
#include "MailBox.h"
#include "../cfg/MailSvrCfgDesc.h"
#include "comm/tlist.h"
#include "CoDataFrame.h"
#include <map>
#include "common_proto.h"

using namespace std;

struct MailBoxNode
{
    MailBoxInfo m_oMailBox;
    TLISTNODE m_stTimeListNode; //时间链表节点，用于置换
    TLISTNODE m_stDirtyListNode; //脏数据链表节点，用于回写数据库
};

class MailBoxMgr : public TSingleton<MailBoxMgr>, public CoDataFrame
{
private:
    typedef map<uint64_t, MailBoxInfo*> Player2MailBox_t;

    const static uint16_t MAILBOX_UPDATE_FREQ = 300;
    const static int MAIL_UPT_NUM_PER_FRAME = 10;

public:
    MailBoxMgr();
    ~MailBoxMgr();

    bool Init(MAILSVRCFG* pstConfig);

    void Update();

    void Fini();

    MailBoxInfo* GetMailBoxInfo(uint64_t ullPlayerId);

    //(继承自CoDataFrame,由继承的类具体实现) 判断数据是否在内存中
    virtual bool IsInMem(void* pResult);

    //(继承自CoDataFrame,由继承的类具体实现) 将结果保存在内存中
    virtual bool SaveInMem(void* pResult);

	void AddToDirtyList(MailBoxInfo* pstMailBoxInfo);

protected:
    void _SaveMailBox(MailBoxInfo* pstMailBoxInfo);

    void _WriteDirtyToDB();

    MailBoxInfo* _NewMailBox(DT_MAIL_BOX_DATA& rstMailBoxData);

    void _AddMailBoxToMap(MailBoxInfo* pstMailBoxInfo);
    void _DelMailBoxFromMap(MailBoxInfo* pstMailBoxInfo);

    void _Move2TimeListFirst(MailBoxInfo* pstMailBoxInfo);

    //(继承自CoDataFrame,由继承的类具体实现) 通过key在内存中查找数据
	virtual void* _GetDataInMem(void* key);

    //(继承自CoDataFrame,由继承的类具体实现) 创建获取数据action
	virtual CoGetDataAction* _CreateGetDataAction(void* key);

    //(继承自CoDataFrame,由继承的类具体实现) 释放获取数据的action
	virtual void _ReleaseGetDataAction(CoGetDataAction* poAction);
public:
	PKGMETA::DT_MAIL_BOX_INFO m_stMailBoxInfo4Unpack; 
	PKGMETA::DT_MAIL_BOX_INFO m_stMailBoxInfo4Pack;
private:
    MAILSVRCFG* m_pstConfig;

    TLISTNODE m_stTimeListHead; // 时间链表头,用于LRU算法

    int m_iDirtyNodeNum; //脏数据个数
    int m_iDirtyNodeMax; //脏数据上限
    int m_iWriteTimeVal; //回写的时间间隔
    uint64_t m_ullLastWriteTimestamp;//上次回写脏数据的时间
    TLISTNODE m_stDirtyListHead; // 脏数据链表头,用于回写数据库

    Player2MailBox_t m_oMailBoxMap;
    Player2MailBox_t::iterator m_oMailBoxMapIter;

    int m_iCurSize;
    int m_iMaxSize;
    CMemPool<MailBoxNode> m_oMailBoxPool;
    CMemPool<MailBoxNode>::UsedIterator m_oMailBoxPoolIter;
};


