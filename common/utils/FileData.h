#ifndef _FILE_DATA_H
#define _FILE_DATA_H
#include <stdio.h>
#include "LogMacros.h"
#include "strutil.h"



/*
    Data 需要通过TDR定义,支持pack和unpack
*/

template<typename Data>
class FileData
{
public:
    struct Base
    {
        uint32_t m_dwVersion;
        uint32_t m_dwDataLen;
    };
public:
    Base m_oBase;
    Data m_oData;                                   //DataStruct, blob pack之后的结构
    char m_szBuff[sizeof(Data) + sizeof(Base) + 1]; //缓冲区
    char m_szFileName[255];                         //文件名
    FILE* m_fp;                                     //文件句柄

public:
    FileData() {}
    ~FileData() { if (m_fp) fclose(m_fp); }
    int ReadFile();
    int SaveFile(uint32_t dwVersion = 0);

    //bIsDataExist=ture#文件数据存在
    //调用Init后如果文件不存在,需要初始数据为有效数据,并写入
    bool Init(const char * pszFileName, OUT bool* bIsDataExist = NULL);
};

template<typename Data>
bool FileData<Data>::Init(const char * pszFileName, OUT bool* pbIsDataExist)
{
    if (pbIsDataExist)
    {
        *pbIsDataExist = false;
    }
    if (!pszFileName)
    {
        LOGERR_r("pszFileName is NULL");
        return false;
    }
    StrCpy(m_szFileName, pszFileName, sizeof(m_szFileName));
    if (0 != access(m_szFileName, 0))
    {
        m_fp = fopen(m_szFileName, "wb+");  //打开或创建文件,允许读写
        if (NULL == m_fp)
        {
            LOGERR_r("create file<%s> failed", m_szFileName);
            return false;
        }
        //初始化数据
        bzero(&m_oData, sizeof(Data));
        bzero(&m_oBase, sizeof(Base));
    }
    else
    {
        if (pbIsDataExist)
        {
            *pbIsDataExist = true;
        }
        m_fp = fopen(m_szFileName, "rb+");  //读写打开或建立一个二进制文件,允许读和写
        if (NULL == m_fp)
        {
            LOGERR_r("open file<%s> failed!", m_szFileName);
            return false;
        }

        if (this->ReadFile() != 0)
        {
            return false;
        }
        
    }

    return true;
}


template<typename Data>
int FileData<Data>::ReadFile()
{
    if (!m_fp)
    {
        LOGERR_r("readfile File not exist.");
        return -1;
    }

    //从文件中初始化数据
    if (1 != fread(&m_oBase, sizeof(m_oBase), 1, m_fp))
    {
        LOGERR_r("read data from file<%s> faild", m_szFileName);
        return -2;
    }
    if (1 != fread(m_szBuff, m_oBase.m_dwDataLen, 1, m_fp))
    {
        LOGERR_r("read data from file<%s> faild", m_szFileName);
        return -3;
    }

    //解压
    size_t ullSizeUesed = 0;
    int iRet = m_oData.unpack(m_szBuff, sizeof(m_szBuff), &ullSizeUesed, m_oBase.m_dwVersion);
    if (0 != iRet)
    {
        LOGERR_r("unpack m_oData failed, FileName<%s> Ret=%d", m_szFileName, iRet);
        return -3;
    }
    return 0;
}

template<typename Data>
int FileData<Data>::SaveFile(uint32_t dwVersion)
{
    if (!m_fp)
    {
        LOGERR_r("SaveFile file not exist.");
        return -1;
    }
    size_t ullSizeUesed = 0;
    int iRet = m_oData.pack(m_szBuff, sizeof(m_szBuff), &ullSizeUesed, dwVersion /*PKGMETA::MetaLib::getVersion()*/);
    if (iRet != 0)
    {
        LOGERR_r("pack m_oData failed,file<%s> Ret=%d", m_szFileName, iRet);
        return -2;
    }
    m_oBase.m_dwDataLen = (uint32_t)ullSizeUesed;
    m_oBase.m_dwVersion = dwVersion;

    fseek(m_fp, 0, SEEK_SET);
    if (fwrite(&m_oBase, sizeof(m_oBase), 1, m_fp) != 1)
    {
        LOGERR_r("write data to file<%s> failed!", m_szFileName);
        return -3;
    }
    if (fwrite(m_szBuff, sizeof(m_szBuff), 1, m_fp) != 1)
    {
        LOGERR_r("write data to file<%s> failed!", m_szFileName);
        return -4;
    }
    fflush(m_fp);
    return 0;
}

#endif
