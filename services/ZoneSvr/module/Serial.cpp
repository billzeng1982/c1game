#include "Serial.h"
#include "common_proto.h"

using namespace PKGMETA;

int Serial::SerialIdCmp(const void *pstFirst, const void *pstSecond)
{
    int iFirst = *(uint16_t*)pstFirst;
    int iSecond = *(uint16_t*)pstSecond;

    return iFirst - iSecond;
}

int Serial::CheckDrawSerail(PlayerData* pstData, char* pszSerial)
{
    pszSerial[MAX_SERIAL_NUM_LEN -1] = '\0';
    if (!_CheckValid(pszSerial, MAX_SERIAL_NUM_LEN-1))
    {
        return ERR_SERIAL_NUM_INVAILD;
    }

    uint16_t wSerialId = (uint16_t)_GetSerialId(pszSerial);

    DT_ROLE_MISC_INFO& rstMiscInfo = pstData->GetMiscInfo();

    int iEqual = 0;
    MyBSearch(&wSerialId, rstMiscInfo.m_SerialDrawList, rstMiscInfo.m_bSerialDrawCnt, sizeof(uint16_t), &iEqual, SerialIdCmp);
    if (!iEqual)
    {
        return ERR_NONE;
    }
    else
    {
        return ERR_SERIAL_NUM_ALREADY_USED;
    }
}

int Serial::AddUsedSerial(PlayerData* pstData, uint16_t wSerialId)
{
    DT_ROLE_MISC_INFO& rstMiscInfo = pstData->GetMiscInfo();

    size_t nmemb = (size_t)rstMiscInfo.m_bSerialDrawCnt;
    if(nmemb >= MAX_DRAW_SERIAL_NUM)
    {
        LOGERR("rstMiscInfo.m_bSerialDrawCnt<%d> reaches the max.", rstMiscInfo.m_bSerialDrawCnt);
        return -1;
    }
    MyBInsert(&wSerialId, rstMiscInfo.m_SerialDrawList, &nmemb, sizeof(uint16_t), 1, SerialIdCmp);
    rstMiscInfo.m_bSerialDrawCnt = (uint8_t)nmemb;
    return 0;
}


int8_t Serial::_GetValue(char* pszChar)
{
    if ((*pszChar >= '0') && (*pszChar <= '9'))
    {
        return (*pszChar - '0');
    }
    else if ((*pszChar >= 'A') && (*pszChar <= 'Z'))
    {
        return (*pszChar - 'A' + 10);
    }
    else
    {
        return -1;
    }
}

uint32_t Serial::_GetSerialId(char* pszSerial)
{
    uint32_t dwSerialId = 0;
    uint32_t dwBase = 1;
    for (int i=0; i<3; i++)
    {
        int8_t cValue1 = 0;
        for (int j=i*3; j<(i+1)*3; j++)
        {
            cValue1 += this->_GetValue(&pszSerial[j]);
        }
        cValue1 = cValue1 % 36;

        int8_t cValue2 = this->_GetValue(&pszSerial[i+9]) + 36;
        int8_t cValue3 = (cValue2 - cValue1) % 36;

        dwSerialId += dwBase * cValue3;

        dwBase *= 36;
    }
    return dwSerialId;
}

bool Serial::_CheckValid(char* pszSerial, int iLen)
{
    for (int i=0; i<iLen; i++)
    {
        if (_GetValue(&pszSerial[i]) < 0)
        {
            return false;
        }
    }

    return true;
}

