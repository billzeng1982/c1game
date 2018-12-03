#include "Message.h"
#include "strutil.h"
#include "ss_proto.h"
#include "common_proto.h"
#include "../../framework/GameObjectPool.h"
using namespace PKGMETA;

static DT_MESSAGE_BOX_INFO g_stMessageBoxInfo ;


void Message::Reset()
{
    bzero(&m_stMessageBaseInfo, sizeof(m_stMessageBaseInfo));
    bzero(&m_stMessageBoxAddtInfo, sizeof(m_stMessageBoxAddtInfo));
    TLIST_INIT(&m_stTimeListNode);
    TLIST_INIT(&m_stDirtyListNode);
}

bool Message::InitFromDB(IN DT_MESSAGE_WHOLE_DATA& rstMessageWholeData)
{
    //复制BaseInfo,没有打包的
    memcpy(&m_stMessageBaseInfo, &rstMessageWholeData.m_stBaseInfo, sizeof(m_stMessageBaseInfo));

    //unpack MessageBoxBlob
    bzero(&g_stMessageBoxInfo, sizeof(g_stMessageBoxInfo));
    int iRet = g_stMessageBoxInfo.unpack((char*)rstMessageWholeData.m_stBoxBlob.m_szData, rstMessageWholeData.m_stBoxBlob.m_iLen);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("unpack DT_MESSAGE_BOX_BLOB failed, Ret=%d", iRet);
        return false;
    }
    for (int i = 0; i < g_stMessageBoxInfo.m_wCount; i++)
    {
        OneRecordObj* pOneRecordObj = GET_GAMEOBJECT(OneRecordObj, GAMEOBJ_MESSAGE_ONE_RECORD);
        if (NULL == pOneRecordObj)
        {
            //如果分配不到,这里会影响前台正确解析聊天记录,所以直接false
            LOGERR_r("pOneRecordObj is null");
            return false; 
        }
        memcpy(&pOneRecordObj->stOneRecord, &g_stMessageBoxInfo.m_astAllRecord[i], sizeof(DT_MESSAGE_ONE_RECORD_INFO));
        m_stMessageBoxAddtInfo.m_astAllRecordObjs[i] = pOneRecordObj;
    }
    m_stMessageBoxAddtInfo.m_wCount = g_stMessageBoxInfo.m_wCount;
    m_stMessageBoxAddtInfo.m_wLastPos = g_stMessageBoxInfo.m_wLastPos;
    return true;
}

bool Message::PackMessageWholeData(DT_MESSAGE_WHOLE_DATA& rstMessageWholeData)
{
    memcpy(&rstMessageWholeData.m_stBaseInfo, &m_stMessageBaseInfo, sizeof(m_stMessageBaseInfo));

    LOGRUN_r("Pack:Id<%lu>,Cnt<%d>, Channel<%hhu>",m_stMessageBaseInfo.m_ullUin, m_stMessageBoxAddtInfo.m_wCount,
        m_stMessageBaseInfo.m_bChannel);
    // pack MessageBoxBlob
    bzero(&g_stMessageBoxInfo, sizeof(g_stMessageBoxInfo));
    for (int i = 0; i < m_stMessageBoxAddtInfo.m_wCount; i++)
    {
        if (NULL == m_stMessageBoxAddtInfo.m_astAllRecordObjs[i])
        {
            LOGERR_r("pOneRecordObj is null !");
            return false; 
        }
        LOGRUN_r("Pack:Id=<%lu>,Record=<%s>",m_stMessageBaseInfo.m_ullUin, m_stMessageBoxAddtInfo.m_astAllRecordObjs[i]->stOneRecord.m_szRecord);
        memcpy(&g_stMessageBoxInfo.m_astAllRecord[i], &m_stMessageBoxAddtInfo.m_astAllRecordObjs[i]->stOneRecord, sizeof(DT_MESSAGE_ONE_RECORD_INFO));
    }
    g_stMessageBoxInfo.m_wCount = m_stMessageBoxAddtInfo.m_wCount ;
    g_stMessageBoxInfo.m_wLastPos = m_stMessageBoxAddtInfo.m_wLastPos ;
    size_t ulUseSize = 0;
    int iRet = g_stMessageBoxInfo.pack((char*)rstMessageWholeData.m_stBoxBlob.m_szData, MAX_LEN_MESSAGE_BOX_BLOB, &ulUseSize);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("pack DT_MESSAGE_BOX_BLOB failed, Ret=%d", iRet);
        return false;
    }
    rstMessageWholeData.m_stBoxBlob.m_iLen = (int)ulUseSize;
    return true;

}


void Message::GetMessage(OUT DT_MESSAGE_BOX_INFO& rstBoxInfo)
{
    for (int i = 0; i < m_stMessageBoxAddtInfo.m_wCount; i++)
    {
        memcpy(&rstBoxInfo.m_astAllRecord[i], &m_stMessageBoxAddtInfo.m_astAllRecordObjs[i]->stOneRecord, sizeof(DT_MESSAGE_ONE_RECORD_INFO));
    }
    rstBoxInfo.m_wCount = m_stMessageBoxAddtInfo.m_wCount;
    rstBoxInfo.m_wLastPos = m_stMessageBoxAddtInfo.m_wLastPos;
}

int  Message::AddRecord(DT_MESSAGE_ONE_RECORD_INFO& rstRecord)
{
    OneRecordObj* pOneRecordObj = GET_GAMEOBJECT(OneRecordObj, GAMEOBJ_MESSAGE_ONE_RECORD);
    if (NULL == pOneRecordObj)
    {
        LOGERR_r("pOneRecordObj is null, SendUin<%lu> Channel<%hhu> RecvUin<%lu>",
            rstRecord.m_ullSenderUin, rstRecord.m_bChannel, rstRecord.m_ullReceiverUin);
        return ERR_SYS; 
    }
    memcpy(&pOneRecordObj->stOneRecord, &rstRecord, sizeof(DT_MESSAGE_ONE_RECORD_INFO));
    if (m_stMessageBoxAddtInfo.m_wCount < MAX_MESSAGE_RECORD_CNT)
    {
        m_stMessageBoxAddtInfo.m_astAllRecordObjs[m_stMessageBoxAddtInfo.m_wCount++] = pOneRecordObj;
    }
    else
    {//满了顶掉最老的
        m_stMessageBoxAddtInfo.m_astAllRecordObjs[m_stMessageBoxAddtInfo.m_wLastPos] = pOneRecordObj;
        m_stMessageBoxAddtInfo.m_wLastPos = (m_stMessageBoxAddtInfo.m_wLastPos + 1) % MAX_MESSAGE_RECORD_CNT;
    }
    return ERR_NONE;
}

void Message::UpdateMessageWholeData()
{

} 


bool Message::InitNew(uint64_t ullUin, uint8_t bChannel)
{
    m_stMessageBaseInfo.m_ullUin = ullUin;
    m_stMessageBaseInfo.m_bChannel = bChannel;
    return true;
}

void Message::DelMessageBox()
{
    for (int i = 0; i < m_stMessageBoxAddtInfo.m_wCount; i++)
    {
        if (NULL == m_stMessageBoxAddtInfo.m_astAllRecordObjs[i])
        {
            continue; 
        }
        RELEASE_GAMEOBJECT(m_stMessageBoxAddtInfo.m_astAllRecordObjs[i]);
    }
    m_stMessageBoxAddtInfo.m_wCount = 0;
    m_stMessageBoxAddtInfo.m_wLastPos = 0;
}

