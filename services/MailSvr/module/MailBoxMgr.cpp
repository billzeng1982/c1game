#include "MailBoxMgr.h"
#include "MailSvrMsgLayer.h"
#include "MailPubServData.h"
#include "MailTransaction.h"
#include "MailSvrPool.h"

using namespace PKGMETA;

MailBoxMgr::MailBoxMgr()
{
}

MailBoxMgr::~MailBoxMgr()
{
}

bool MailBoxMgr::Init(MAILSVRCFG* pstConfig)
{
    m_pstConfig = pstConfig;

    //初始化CoDataFrame类
    this->BaseInit(MailSvrPool::SSPKG_POOL_MAX_NUM);
    this->SetCoroutineEnv(MailSvrMsgLayer::Instance().GetCoroutineEnv());

    //初始化时间链表
    TLIST_INIT(&m_stTimeListHead);

    //初始化内存池
    m_iCurSize = 0;
    m_iMaxSize = pstConfig->m_dwMailBoxMaxNum;
    if (m_oMailBoxPool.CreatePool(m_pstConfig->m_dwMailBoxMaxNum) < 0)
    {
        LOGERR("Create m_oMailBoxPool pool[num=%u] failed.", m_pstConfig->m_dwMailBoxMaxNum);
        return false;
    }
    m_oMailBoxPool.RegisterSlicedIter(&m_oMailBoxPoolIter);

    //初始化脏数据相关
    m_iDirtyNodeNum = 0;
    m_iDirtyNodeMax = m_pstConfig->m_iDirtyNumMax;
    m_iWriteTimeVal = m_pstConfig->m_iUpdateInterval;
    m_ullLastWriteTimestamp = CGameTime::Instance().GetCurrSecond();
    TLIST_INIT(&m_stDirtyListHead);

    return true;
}

void MailBoxMgr::Fini()
{
    MailBoxInfo* poMailBox = NULL;

    m_oMailBoxPoolIter.Begin();
    for (int i=0; !m_oMailBoxPoolIter.IsEnd(); m_oMailBoxPoolIter.Next(), i++)
    {
        if (i % MAIL_UPT_NUM_PER_FRAME == 0)
        {
            MsSleep(10);
        }

        MailBoxNode* poMailBoxNode = m_oMailBoxPoolIter.CurrItem();
        if (!poMailBoxNode)
        {
            LOGERR("In GuildMgr fini, pstMailBoxNode is null");
            break;
        }

        poMailBox = &poMailBoxNode->m_oMailBox;
        this->_SaveMailBox(poMailBox);
    }

    return;
}

void MailBoxMgr::Update()
{
    CoDataFrame::Update();

    this->_WriteDirtyToDB();
}

MailBoxInfo* MailBoxMgr::GetMailBoxInfo(uint64_t ullPlayerId)
{
    return (MailBoxInfo*)CoDataFrame::GetData((void*)&ullPlayerId);
}

void MailBoxMgr::_SaveMailBox(MailBoxInfo* poMailBoxInfo)
{
    SSPKG& rstSsPkg = MailSvrMsgLayer::Instance().GetSsPkg();
    rstSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_PRI_TABLE_UPDATE_NTF;
    rstSsPkg.m_stHead.m_ullUin = poMailBoxInfo->GetPlayerId();
    SS_PKG_MAIL_PRI_TABLE_UPDATE_NTF& rstSsMailUptNtf = rstSsPkg.m_stBody.m_stMailPriTableUpdateNtf;
    int iRet = poMailBoxInfo->PackMailBoxInfo(rstSsMailUptNtf.m_stData);
    if (iRet != ERR_NONE)
    {
        LOGERR("Save Uin(%lu) MailBox pack failed, Ret=%d.", poMailBoxInfo->GetPlayerId(), iRet);
        return ;
    }

    MailSvrMsgLayer::Instance().SendToMailDBSvr(rstSsPkg);
}

void MailBoxMgr::_WriteDirtyToDB()
{
    TLISTNODE* pstPos = NULL;
    TLISTNODE* pstNext = NULL;
    TLISTNODE* pstHead = &(m_stDirtyListHead);

    MailBoxNode* poMailBoxNode = NULL;
    MailBoxInfo* poMailBox = NULL;

    uint64_t ullCurTime = CGameTime::Instance().GetCurrSecond();

    if (m_iDirtyNodeNum >= m_iDirtyNodeMax ||
        ullCurTime - m_ullLastWriteTimestamp >= (uint64_t)m_iWriteTimeVal)
    {
        m_ullLastWriteTimestamp = ullCurTime;

        TLIST_FOR_EACH_SAFE(pstPos, pstNext, pstHead)
        {
            poMailBoxNode = TLIST_ENTRY(pstPos, MailBoxNode, m_stDirtyListNode);
            poMailBox = &poMailBoxNode->m_oMailBox;
            this->_SaveMailBox(poMailBox);

            TLIST_INIT(pstPos);
        }

        TLIST_INIT(pstHead);
        m_iDirtyNodeNum = 0;
    }
}

MailBoxInfo* MailBoxMgr::_NewMailBox(DT_MAIL_BOX_DATA& rstMailBoxData)
{
    MailBoxInfo* poMailBox = NULL;
    MailBoxNode* pstMailBoxNode = NULL;

    //DebugCheck();
    
    //mempool中有空余时
    if (m_iCurSize < m_iMaxSize)
    {
        DebugCheck();

        pstMailBoxNode = m_oMailBoxPool.NewData();
        if (!pstMailBoxNode)
        {
            LOGERR("pstMailBoxNode is NULL, get MailBoxNode from pool failed");
            return NULL;
        }

        //DebugCheck();

        //初始化Guild
        poMailBox = &pstMailBoxNode->m_oMailBox;
        poMailBox->InitFromDB(rstMailBoxData);

        DebugCheck();

        //加入时间链表头
        TLIST_INSERT_NEXT(&m_stTimeListHead, &(pstMailBoxNode->m_stTimeListNode));
        m_iCurSize++;

        //DebugCheck();
    }
    //mempool中没有空余时，需要置换，置换采用LRU算法，将最近最少使用的节点回写到数据库
    else
    {
        //DebugCheck();
    
        TLISTNODE* pstSwapNode = TLIST_PREV(&m_stTimeListHead);
        pstMailBoxNode = container_of(pstSwapNode, MailBoxNode, m_stTimeListNode);
        poMailBox = &pstMailBoxNode->m_oMailBox;

        //被置换的GuildNode需要回写数据库
        this->_SaveMailBox(poMailBox);

        //被置换的GuildNode需要从Map中删除
        this->_DelMailBoxFromMap(poMailBox);

        //初始化新的Guild
        poMailBox->InitFromDB(rstMailBoxData);

        //DebugCheck();

        //将此节点从时间链表尾移到链表头
        TLIST_DEL(pstSwapNode);
        TLIST_INSERT_NEXT(&m_stTimeListHead, pstSwapNode);
    }

    //加入Map
    this->_AddMailBoxToMap(poMailBox);

    DebugCheck();

    return poMailBox;
}

void MailBoxMgr::_AddMailBoxToMap(MailBoxInfo* poMailBox)
{
    m_oMailBoxMap.insert(Player2MailBox_t::value_type(poMailBox->GetPlayerId(), poMailBox));
}

void MailBoxMgr::_DelMailBoxFromMap(MailBoxInfo* poMailBox)
{
    poMailBox->Clear();
    m_oMailBoxMapIter = m_oMailBoxMap.find(poMailBox->GetPlayerId());
    if (m_oMailBoxMapIter != m_oMailBoxMap.end())
    {
        m_oMailBoxMap.erase(m_oMailBoxMapIter);
    }
    else
    {
        LOGERR("Del Uin(%lu) MailBox from map failed, not found", poMailBox->GetPlayerId());
    }
}

void MailBoxMgr::_Move2TimeListFirst(MailBoxInfo* poMailBox)
{
    MailBoxNode* pstMailBoxNode = container_of(poMailBox, MailBoxNode, m_oMailBox);
    TLISTNODE* pstNode = &pstMailBoxNode->m_stTimeListNode;

    //将此节点移到时间链表头
    TLIST_DEL(pstNode);
    TLIST_INSERT_NEXT(&m_stTimeListHead, pstNode);
}

void MailBoxMgr::AddToDirtyList(MailBoxInfo* poMailBox)
{
    MailBoxNode* pstMailBoxNode = container_of(poMailBox, MailBoxNode, m_oMailBox);
    TLISTNODE* pstNode = &pstMailBoxNode->m_stTimeListNode;

    //已经加入DirtyList,不能重复加入
    if (!TLIST_IS_EMPTY(pstNode))
    {
        return;
    }

    //加入DirtyList表头
    TLIST_INSERT_NEXT(&m_stDirtyListHead, pstNode);
    m_iDirtyNodeNum++;
}

bool MailBoxMgr::IsInMem(void* pResult)
{
    DT_MAIL_BOX_DATA* poMailBox = (DT_MAIL_BOX_DATA*)pResult;
    m_oMailBoxMapIter = m_oMailBoxMap.find(poMailBox->m_stBaseInfo.m_ullUin);
    return (m_oMailBoxMapIter==m_oMailBoxMap.end()) ? false : true;
}

bool MailBoxMgr::SaveInMem(void* pResult)
{
    DT_MAIL_BOX_DATA* poMailBox = (DT_MAIL_BOX_DATA*)pResult;
    return this->_NewMailBox(*poMailBox);
}

void* MailBoxMgr::_GetDataInMem(void* key)
{
    uint64_t ullPlayerId = *(uint64_t*)key;
    m_oMailBoxMapIter = m_oMailBoxMap.find(ullPlayerId);
    return (m_oMailBoxMapIter == m_oMailBoxMap.end()) ? NULL : (void*)(m_oMailBoxMapIter->second);
}

CoGetDataAction* MailBoxMgr::_CreateGetDataAction(void* key)
{
    AsyncGetMailBoxAction* poAction = MailSvrPool::Instance().AsyncActionPool().Get();
    poAction->SetPlayerId(*(uint64_t*)key);
    return poAction;
}

void MailBoxMgr::_ReleaseGetDataAction(CoGetDataAction* poAction)
{
    AsyncGetMailBoxAction* poGetMailBoxAction = (AsyncGetMailBoxAction*)poAction;
    MailSvrPool::Instance().AsyncActionPool().Release(poGetMailBoxAction);
}





































