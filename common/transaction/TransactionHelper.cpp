#include "TransactionHelper.h"

TActionToken TransactionHelper::MakeActionToken(TTransactionID dwTransactionID, TActionIndex iCompoActionID, TActionIndex iActionID )
{
    TActionToken ulltoken = 0;
    ulltoken = (((TActionToken)dwTransactionID) << 32 ) | ((iCompoActionID<<16) & 0xFFFF0000) | (iActionID&0xFFFF);
    return ulltoken;
}


void TransactionHelper::ParseActionToken(const TActionToken& ullToken, 
    TTransactionID& dwTransactionID, TActionIndex& iCompoActionID, TActionIndex& iActionID)
{
    dwTransactionID   = (TTransactionID)(ullToken >> 32);
    iCompoActionID = (TActionIndex)((ullToken & 0xFFFF0000)>>16);
    iActionID = (TActionIndex)(ullToken & 0xFFFF);
}


unsigned short TransactionHelper::CalcCheckCode(const void* pvPtr)
{
    unsigned short wRet = 0x0305;
    unsigned long ulPtr = (unsigned long)pvPtr;
    unsigned short* pField = (unsigned short*)(void*)&ulPtr;
    for(unsigned int i = 0; i < sizeof(unsigned long)/sizeof(unsigned short); ++i)
    {
        wRet = (wRet<<1) ^ (pField[i] & 0xA3A5);
    }

    return wRet;
}

