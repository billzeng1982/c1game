#include "SerialNumTableMsg.h"
#include "../thread/RedisWorkThread.h"
#include "../framework/SerialNumSvrMsgLayer.h"
#include "LogMacros.h"
#include "strutil.h"
#include "SerialNumTable.h"
#include "GameDataMgr.h"
#include "GameTime.h"

using namespace PKGMETA;

int CSsCheckSerialNumReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
	RedisWorkThread* poWorkThread = (RedisWorkThread*)pvPara;

    SSPKG& stSsPkg = poWorkThread->GetSendSsPkg();
	stSsPkg.m_stHead.m_wMsgId = SS_MSG_CHECK_SERIAL_NUM_RSP;
	stSsPkg.m_stHead.m_ullUin = rstSsPkg.m_stHead.m_ullUin;
    stSsPkg.m_stHead.m_ullReservId = rstSsPkg.m_stHead.m_iSrcProcId;
    SS_PKG_CHECK_SERIAL_NUM_RSP& rstCheckSerialNumRsp = stSsPkg.m_stBody.m_stCheckSerialNumRsp;
    rstCheckSerialNumRsp.m_stSerialInfo.m_bRewardCnt = 0;

	SS_PKG_CHECK_SERIAL_NUM_REQ& rstCheckSerialNumReq = rstSsPkg.m_stBody.m_stCheckSerialNumReq;
    rstCheckSerialNumReq.m_szSerialNum[MAX_SERIAL_NUM_LEN -1] = '\0';
    StrCpy(rstCheckSerialNumRsp.m_stSerialInfo.m_szSerialNum, rstCheckSerialNumReq.m_szSerialNum, MAX_SERIAL_NUM_LEN);
    rstCheckSerialNumRsp.m_stSerialInfo.m_bRepeatFlag = 0;

    uint32_t dwSerialId = this->_GetSerialId(rstCheckSerialNumReq.m_szSerialNum);
    do
    {
        RESSERIALNUM* pResResial = CGameDataMgr::Instance().GetResSerialMgr().Find(dwSerialId);
        if (!pResResial)
        {
            LOGERR_r("Uin<%lu> Serial<%s> Id<%u> not found in ResResial",
                     rstSsPkg.m_stHead.m_ullUin, rstCheckSerialNumReq.m_szSerialNum, dwSerialId);
            rstCheckSerialNumRsp.m_nErrNo = ERR_SERIAL_NUM_INVAILD;
            break;
        }

        if ((pResResial->m_dwChannelId != 0) && (pResResial->m_dwChannelId != (uint32_t)rstCheckSerialNumReq.m_wChannelId))
        {
            LOGERR_r("Uin<%lu> Serial<%s> Id<%u> Channel<%u>, Serial Channel<%u> is not match",
                     rstSsPkg.m_stHead.m_ullUin, rstCheckSerialNumReq.m_szSerialNum, dwSerialId, (uint32_t)rstCheckSerialNumReq.m_wChannelId, pResResial->m_dwChannelId);
            rstCheckSerialNumRsp.m_nErrNo = ERR_SERIAL_NUM_INVAILD;
            break;
        }

        time_t tTime;
        if (CGameTime::Instance().ConvStrToTime(pResResial->m_szEndTime, tTime) < 0)
        {
            LOGERR_r("Uin<%lu> Serial<%s> Id<%u> ConvStrToTime failed",
                     rstSsPkg.m_stHead.m_ullUin, rstCheckSerialNumReq.m_szSerialNum, dwSerialId);
            rstCheckSerialNumRsp.m_nErrNo = ERR_SERIAL_NUM_INVAILD;
            break;
        }

        if (CGameTime::Instance().GetCurrSecond() > tTime)
        {
            LOGERR_r("Uin<%lu> Serial<%s> Id<%u> out of time",
                    rstSsPkg.m_stHead.m_ullUin, rstCheckSerialNumReq.m_szSerialNum, dwSerialId);
            rstCheckSerialNumRsp.m_nErrNo = ERR_SERIAL_NUM_INVAILD;
            break;
        }

        SerialNumTable* poTable = poWorkThread->GetSerialNumTable();
        int iRet = poTable->CheckSerial(dwSerialId, rstCheckSerialNumReq.m_szSerialNum);
        if (iRet <= 0)
        {
            LOGERR_r("Uin<%lu> Serial<%s> Id<%u> not found in Redis, Ret<%d>",
                     rstSsPkg.m_stHead.m_ullUin, rstCheckSerialNumReq.m_szSerialNum, dwSerialId, iRet);
            rstCheckSerialNumRsp.m_nErrNo = ERR_SERIAL_NUM_INVAILD;
            break;
        }

        if (pResResial->m_bDelFlag != 0)
        {
            poTable->DelSerial(dwSerialId, rstCheckSerialNumReq.m_szSerialNum);
            rstCheckSerialNumRsp.m_stSerialInfo.m_bRepeatFlag = pResResial->m_bRepeatFlag;
        }

        rstCheckSerialNumRsp.m_nErrNo = ERR_NONE;
        rstCheckSerialNumRsp.m_stSerialInfo.m_dwSerialId = dwSerialId;
        rstCheckSerialNumRsp.m_stSerialInfo.m_bRewardCnt = pResResial->m_bRewardCnt;
        for (int i=0; i<pResResial->m_bRewardCnt; i++)
        {
            rstCheckSerialNumRsp.m_stSerialInfo.m_astRewardList[i].m_dwItemId = pResResial->m_rewardId[i];
            rstCheckSerialNumRsp.m_stSerialInfo.m_astRewardList[i].m_bItemType = pResResial->m_szRewardType[i];
            rstCheckSerialNumRsp.m_stSerialInfo.m_astRewardList[i].m_dwItemNum = pResResial->m_rewardNum[i];
        }
    }while(false);
	stSsPkg.m_stHead.m_ullReservId |= (uint64_t)PROC_ZONE_SVR << 32;
	poWorkThread->SendPkg(stSsPkg);

    LOGRUN_r("Uin<%lu> Draw Serial<%s> Id<%u>",
             rstSsPkg.m_stHead.m_ullUin, rstCheckSerialNumReq.m_szSerialNum, dwSerialId);

	return 0;
}

uint32_t CSsCheckSerialNumReq::_GetSerialId(char* pszSerial)
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

int8_t CSsCheckSerialNumReq::_GetValue(char* pszChar)
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

