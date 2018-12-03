#include "GmFile.h"
#include "strutil.h"
#include "PKGMETA_metalib.h"

static int CompareActId(const void* pA, const void* pB)
{
    return *((uint32_t*)pA) - *((uint32_t*)pB);
}
static int CompareGoods(const void* PA, const void* pB)
{
    return ((DT_GM_MALL_GOODS*) PA)->m_dwMallGoodsId - ((DT_GM_MALL_GOODS*) pB)->m_dwMallGoodsId;
}

bool GmFile::Init(const char* pszName)
{
    StrCpy(m_szFileName, pszName, MAX_LEN_FILEPATH);
    if ( 0 != access(m_szFileName, 0))
    {
        m_fp = fopen(m_szFileName, "wb+");  //打开或创建文件,允许读写
        if (NULL == m_fp)
        {
            LOGERR("create file<%s> failed", m_szFileName);
            return false;
        }
        //初始化数据
        bzero(&m_stGmFileInfo, sizeof(DT_GM_FILE_INFO));

    }
    else
    {
        m_fp = fopen(m_szFileName, "rb+");  //读写打开或建立一个二进制文件,允许读和写
        if (NULL == m_fp)
        {
            LOGERR("open file<%s> failed!", m_szFileName);
            return false;
        }
        DT_GM_FILE_BLOB stFileBlob = {0};
        //从文件中初始化数据
        if ( 1 != fread(&stFileBlob, sizeof(stFileBlob), 1, m_fp))
        {
            LOGERR("read data from file<%s> faild", m_szFileName);
            return false;
        }
        //解压
        size_t ullSizeUesed = 0;
        int iRet = m_stGmFileInfo.unpack((char*)stFileBlob.m_szData, stFileBlob.m_iLen, &ullSizeUesed, stFileBlob.m_iVersion);
        if (TdrError::TDR_NO_ERROR != iRet)
        {
            LOGERR("unpack m_stGmFileInfo failed, Ret=%d", iRet);
            return false;
        }
    }
    return true;
}

void GmFile::SaveData2File()
{
    if (NULL == m_fp)
    {
        LOGERR("m_fp is NULL, file<%s>", m_szFileName);
        return;
    }
    DT_GM_FILE_BLOB stFileBlob = {0};
    //Pack数据
    size_t ullSizeUesed = 0;
    int iRet = m_stGmFileInfo.pack((char*)stFileBlob.m_szData, MAX_GM_FILE_BLOB_LEN, &ullSizeUesed, PKGMETA::MetaLib::getVersion());
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("pack m_stApplyInfo failed, Ret=%d", iRet);
        return ;
    }
    stFileBlob.m_iLen = (int)ullSizeUesed;
    stFileBlob.m_iVersion = PKGMETA::MetaLib::getVersion();

    fseek(m_fp, 0, SEEK_SET);
    if( fwrite(&stFileBlob, sizeof(stFileBlob), 1, m_fp) != 1 )
    {
       LOGERR("write data to file<%s> failed!", m_szFileName);
       return;
    }
     fflush(m_fp);
}

void GmFile::Release()
{
    if (NULL == m_fp)
    {
        return;
    }
    fclose(m_fp);
}


int GmFile::InsertCloseActIdList(uint32_t dwActId)
{
    if (m_stGmFileInfo.m_dwCloseActIdNum > MAX_GM_CLOSE_ID_NUM)
    {
        LOGERR("InsertCloseActId err, Full!!");
        return ERR_SYS;
    }

    size_t nmemb = (size_t)m_stGmFileInfo.m_dwCloseActIdNum;
    if (nmemb >= MAX_GM_CLOSE_ID_NUM)
    {
        LOGERR("m_stGmFileInfo.m_dwCloseActIdNum<%d> reaches the max.", m_stGmFileInfo.m_dwCloseActIdNum);
        return ERR_SYS;
    }
    if (!MyBInsert(&dwActId, m_stGmFileInfo.m_CloseActIdList, &nmemb, sizeof(uint32_t), 1, CompareActId))
    {
        LOGERR("InsertCloseActId  err!");
        return ERR_SYS;
    }
    m_stGmFileInfo.m_dwCloseActIdNum = (uint16_t)nmemb;
    return ERR_NONE;
}

int GmFile::IsInCloseActId(uint32_t dwActId)
{
    int iEqual = 0;
    MyBSearch(&dwActId, m_stGmFileInfo.m_CloseActIdList, m_stGmFileInfo.m_dwCloseActIdNum, sizeof(uint32_t), &iEqual, CompareActId);
    return iEqual;
}

int GmFile::DeleteCloseActId(uint32_t dwActId)
{
    size_t nmemb = (size_t)m_stGmFileInfo.m_dwCloseActIdNum;
    if (!MyBDelete(&dwActId, m_stGmFileInfo.m_CloseActIdList, &nmemb, sizeof(uint32_t), CompareActId))
    {
        LOGERR("DeleteCloseActId  err!");
        return ERR_SYS;
    }
    m_stGmFileInfo.m_dwCloseActIdNum = (uint16_t)nmemb;
    return ERR_NONE;
}
//  ***** MallGoods *******
//删除
int GmFile::DeleteMallGoodsId(uint32_t dwGoodsId)
{
    DT_GM_MALL_GOODS tmpMallGoods = {0};
    tmpMallGoods.m_dwMallGoodsId = dwGoodsId;
    size_t nmemb = (size_t)m_stGmFileInfo.m_dwMallGoodsNum;
    if (!MyBDelete(&tmpMallGoods, m_stGmFileInfo.m_astMallGoodsList, &nmemb, sizeof(tmpMallGoods), CompareGoods))
    {
        LOGERR("DeleteCloseActId  err!");
        return ERR_SYS;
    }
    m_stGmFileInfo.m_dwMallGoodsNum = (uint32_t)nmemb;
    return ERR_NONE;
}

//插入
int GmFile::InsertMallGoddsId(uint32_t dwGoodsId, uint8_t bType)
{
    if (m_stGmFileInfo.m_dwMallGoodsNum > MAX_GM_MALL_GOODS_ID_NUM)
    {
        LOGERR("InsertMallGoods err, Full!!");
        return ERR_SYS;
    }

    size_t nmemb = (size_t)m_stGmFileInfo.m_dwMallGoodsNum;
    if (nmemb >= MAX_GM_MALL_GOODS_ID_NUM)
    {
        LOGERR("m_stGmFileInfo.m_dwMallGoodsNum<%d> reaches the max.", m_stGmFileInfo.m_dwMallGoodsNum);
        return ERR_SYS;
    }
    DT_GM_MALL_GOODS tmpMallGoods = {0};
    tmpMallGoods.m_dwMallGoodsId = dwGoodsId;
    tmpMallGoods.m_bMallGoodsUp = bType;

    if (!MyBInsert(&tmpMallGoods, m_stGmFileInfo.m_astMallGoodsList, &nmemb, sizeof(tmpMallGoods), 1, CompareGoods))
    {
        LOGERR("InsertMallGoods  err!");
        return ERR_SYS;
    }
    m_stGmFileInfo.m_dwCloseActIdNum = (uint32_t)nmemb;
    return ERR_NONE;
}
//找到索引
DT_GM_MALL_GOODS* GmFile::FindInMallGoodsIdlist(uint32_t dwGoodsId)
{
    int iEqual = 0;
    DT_GM_MALL_GOODS tmpMallGoods = {0};
    tmpMallGoods.m_dwMallGoodsId = dwGoodsId;
    int iIndex = MyBSearch(&tmpMallGoods, m_stGmFileInfo.m_astMallGoodsList, m_stGmFileInfo.m_dwMallGoodsNum, sizeof(tmpMallGoods), &iEqual, CompareGoods);
    if (1 == iEqual)
    {
       return &m_stGmFileInfo.m_astMallGoodsList[iIndex];
    }

    return NULL;
}




