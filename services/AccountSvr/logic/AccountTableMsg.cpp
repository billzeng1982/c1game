#include "AccountTableMsg.h"
#include "LogMacros.h"
#include "strutil.h"
#include "AccountTable.h"
#include "../AccountSvr.h"
#include "../framework/AccountSvrMsgLayer.h"
#include "../thread/DBWorkThread.h"

using namespace PKGMETA;


//SS_MSG_ACCLOGIN_REQ
int CSsAccLoginReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
    PKGMETA::SS_PKG_ACCOUNT_LOGIN_REQ& rstAccLoginReq = rstSsPkg.m_stBody.m_stAccountLoginReq;
    if( rstAccLoginReq.m_szAccountName[0] == '\0' )
    {
        LOGERR_r("Account name is empty!");
        return -1;
    }

    SSPKG stTempSspkg;

    stTempSspkg.m_stHead.m_wMsgId = SS_MSG_ACCOUNT_LOGIN_RSP;
    stTempSspkg.m_stHead.m_iDstProcId = rstSsPkg.m_stHead.m_iSrcProcId;
    PKGMETA::SS_PKG_ACCOUNT_LOGIN_RSP& rstAccLoginRsp = stTempSspkg.m_stBody.m_stAccountLoginRsp;

    StrCpy( rstAccLoginRsp.m_szAccountName, rstAccLoginReq.m_szAccountName, sizeof(rstAccLoginRsp.m_szAccountName) );

    int iErrNo = poWorkThread->GetAccountTable().GetAccountData( rstAccLoginReq.m_szAccountName, 
		rstAccLoginReq.m_szChannelID, rstSsPkg.m_stHead.m_ullUin, rstAccLoginRsp.m_stRspData.m_stAccountData );
    if( iErrNo != ERR_NONE )
    {
        rstAccLoginRsp.m_chResult = 0;
        rstAccLoginRsp.m_nErrNo = iErrNo;
    }
    else
    {
        rstAccLoginRsp.m_nErrNo = ERR_NONE; // need modify ÔÝÊ±²»¼ì²éÃÜÂë
        rstAccLoginRsp.m_chResult = 1;
    }

    poWorkThread->SendPkg(stTempSspkg);

    return 0;
}


int CSsGmUpdateBanTimeReq::HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara )
{
    CDBWorkThread* poWorkThread = (CDBWorkThread*)pvPara;
    PKGMETA::SS_PKG_ACCOUNT_GM_UPDATE_BANTIME_REQ& rstReq = rstSsPkg.m_stBody.m_stAccountGmUpdateBanTimeReq;

    if (ERR_NONE != poWorkThread->GetAccountTable().GmUpdateAccountBanTime( rstReq.m_ullUin, rstReq.m_ullBanTime ))
    {
        LOGCORE_r("Update BanTime error, player uin<%lu>", rstReq.m_ullUin);
    }

    return 0;
}

