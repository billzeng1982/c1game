#pragma once

#include "define.h"
#include "common_proto.h"
#include "cs_proto.h"
#include "singleton.h"
#include "../player/PlayerData.h"
#include <vector>

class GmFile
{
public:
    GmFile() {};
    ~GmFile() {};
    bool Init(const char* pszName);
    void SaveData2File();
    void Release();
    int InsertCloseActIdList(uint32_t dwAtcId);
    int IsInCloseActId(uint32_t dwActId);
    int DeleteCloseActId(uint32_t dwActId);
    int DeleteMallGoodsId(uint32_t dwGoodsId);
    int InsertMallGoddsId(uint32_t dwGoodsId, uint8_t bType);
    DT_GM_MALL_GOODS* FindInMallGoodsIdlist(uint32_t dwGoodsId);

public:
    DT_GM_FILE_INFO m_stGmFileInfo;
private:
    char m_szFileName[MAX_LEN_FILEPATH];
    FILE* m_fp;
};
