#include "LogMacros.h"
#include "Gm.h"

int GmMgr::HandleMsgCmd(CDBWorkThread* poWorkThread, SSPKG& stSsReqPkg)
{
    SS_PKG_ROLE_GM_UPDATE_INFO_REQ& rstGmReq = stSsReqPkg.m_stBody.m_stRoleGmUpdateInfoReq;
    rstGmReq.m_szCmd[MAX_GM_CMD_NUM-1] = '\0';	//前台参数保护,防止访问越界
    m_ArgsMap.clear();
    m_poRoleTable = &poWorkThread->GetRoleTable();
    int iRet = ParseCmd(rstGmReq.m_szCmd);
	uint64_t ullUin = rstGmReq.m_ullUin;
	if (NULL != rstGmReq.m_szRoleName && '\0' != rstGmReq.m_szRoleName)
	{
		//used role name
		m_poRoleTable->GetRoleUin(rstGmReq.m_szRoleName, ullUin);
	}
    if (ERR_NONE != iRet || m_ArgsMap.size() < 1)
    {
        return iRet;
    }
     if (strcmp(m_ArgsMap[0], CMD_SET_EXP) == 0)
    {
        return GmSetExp(ullUin);
    }
    else if (strcmp(m_ArgsMap[0], CMD_SET_LV) == 0)
    {
        return GmSetLv(ullUin);
    }
    else if (strcmp(m_ArgsMap[0], CMD_SET_AP) == 0)
    {
        return GmSetAp(ullUin);
    }
    else if (strcmp(m_ArgsMap[0], CMD_BLACK_ROOM) == 0)
    {
        return GmBlackRoom(ullUin);
    }
    else if(strcmp(m_ArgsMap[0], CMD_GET_UIN) == 0)
    {
        return GmGetUin(poWorkThread, stSsReqPkg);
    }
    else if( strcmp(m_ArgsMap[0], CMDG_GET_ROLE_NAME ) == 0)
    {
        return GmGetRoleName(poWorkThread, stSsReqPkg);
    }

    return ERR_NONE;
}

int GmMgr::GmSetLv(uint64_t ullUin)
{
    uint32_t iLv = 0;
    sscanf(m_ArgsMap[2], "%u", &iLv);
    bzero(&m_stMajestyBlob, sizeof(m_stMajestyBlob));
    bzero(&m_stMajestyInfo, sizeof(m_stMajestyInfo));
    m_poRoleTable->GetRoleMajestyInfo(ullUin, m_stMajestyBlob);
    
    int iRet = m_stMajestyInfo.unpack((char*)m_stMajestyBlob.m_szData, sizeof(m_stMajestyBlob.m_szData));
    if (iRet != TdrError::TDR_NO_ERROR) {
        LOGERR("unpack DT_ROLE_MAJESTY_BLOB failed!");
        return ERR_SYS;
    }
    m_stMajestyInfo.m_wLevel = iLv;

    bzero(&m_stMajestyBlob, sizeof(m_stMajestyBlob));
    size_t ulUseSize = 0;
    iRet = m_stMajestyInfo.pack((char*)m_stMajestyBlob.m_szData, sizeof(m_stMajestyBlob.m_szData), &ulUseSize);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("pack DT_ROLE_MAJESTY_BLOB failed!");
        return false;
    }
    m_stMajestyBlob.m_iLen = (int)ulUseSize;
    iRet = m_poRoleTable->SaveRoleMajestyInfo(ullUin, m_stMajestyBlob);
    return iRet;

}


int GmMgr::GmSetExp(uint64_t ullUin)
{
    int iExp = 0;
    sscanf(m_ArgsMap[2], "%d", &iExp);

    bzero(&m_stMajestyBlob, sizeof(m_stMajestyBlob));
    bzero(&m_stMajestyInfo, sizeof(m_stMajestyInfo));

    //读取数据库
    m_poRoleTable->GetRoleMajestyInfo(ullUin, m_stMajestyBlob);
    int iRet = m_stMajestyInfo.unpack((char*)m_stMajestyBlob.m_szData, sizeof(m_stMajestyBlob.m_szData));
    if (iRet != TdrError::TDR_NO_ERROR) 
    {
        LOGERR("unpack DT_ROLE_MAJESTY_BLOB failed!");
        return ERR_SYS;
    }

    //修改数据
    m_stMajestyInfo.m_dwExp = iExp;

    //写会数据库
    bzero(&m_stMajestyBlob, sizeof(m_stMajestyBlob));
    size_t ulUseSize = 0;
    iRet = m_stMajestyInfo.pack((char*)m_stMajestyBlob.m_szData, sizeof(m_stMajestyBlob.m_szData), &ulUseSize);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("pack DT_ROLE_MAJESTY_BLOB failed!");
        return false;
    }
    m_stMajestyBlob.m_iLen = (int)ulUseSize;
    iRet = m_poRoleTable->SaveRoleMajestyInfo(ullUin, m_stMajestyBlob);

    return iRet;
}

int GmMgr::GmSetAp(uint64_t ullUin)
{
    uint32_t iAp = 0;
    sscanf(m_ArgsMap[2], "%u", &iAp);

    bzero(&m_stMajestyBlob, sizeof(m_stMajestyBlob));
    bzero(&m_stMajestyInfo, sizeof(m_stMajestyInfo));

    //读取数据库
    m_poRoleTable->GetRoleMajestyInfo(ullUin, m_stMajestyBlob);
    int iRet = m_stMajestyInfo.unpack((char*)m_stMajestyBlob.m_szData, sizeof(m_stMajestyBlob.m_szData));
    if (iRet != TdrError::TDR_NO_ERROR) 
    {
        LOGERR("unpack DT_ROLE_MAJESTY_BLOB failed!");
        return ERR_SYS;
    }

    //修改数据
    m_stMajestyInfo.m_dwAP = iAp;

    //写会数据库
    bzero(&m_stMajestyBlob, sizeof(m_stMajestyBlob));
    size_t ulUseSize = 0;
    iRet = m_stMajestyInfo.pack((char*)m_stMajestyBlob.m_szData, sizeof(m_stMajestyBlob.m_szData), &ulUseSize);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("pack DT_ROLE_MAJESTY_BLOB failed!");
        return false;
    }
    m_stMajestyBlob.m_iLen = (int)ulUseSize;
    iRet = m_poRoleTable->SaveRoleMajestyInfo(ullUin, m_stMajestyBlob);

    return iRet;
}

int GmMgr::GmBlackRoom(uint64_t ullUin)
{
    uint64_t tTime = 0;
    sscanf(m_ArgsMap[2], "%lu", &tTime);
    int iRet = m_poRoleTable->SaveRoleBackRoomTime(ullUin, tTime);
    return iRet;
}

int GmMgr::ParseCmd(char* pszCmd,char const* pszDelims)
{
    if (NULL == pszCmd || '\0' == pszCmd[0])
    {
        return ERR_SYS;
    }

    char* p = strtok(pszCmd, pszDelims); 
    while(p != NULL)
    {
        m_ArgsMap.push_back(p);
        p = strtok(NULL, pszDelims); 
    }
    return ERR_NONE;
}

int GmMgr::GmGetUin(CDBWorkThread* poWorkThread, SSPKG& stSsReqPkg)
{
    char szName[MAX_NAME_LENGTH] = {0};
    sscanf(m_ArgsMap[1], "%s", szName);
    bzero(&m_stSsPkg, sizeof(m_stSsPkg));
    int iRet = m_poRoleTable->GetRoleUin(szName, m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoRsp.m_szResult, 1024);
    m_stSsPkg.m_stHead.m_ullUin = stSsReqPkg.m_stHead.m_ullUin;
    m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoRsp.m_nErrNo = iRet;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ROLE_GM_UPDATE_INFO_RSP;
    poWorkThread->SendPkg(m_stSsPkg);
    return ERR_NONE;

 
}

int GmMgr::GmGetRoleName(CDBWorkThread* poWorkThread, SSPKG& stSsReqPkg)
{
    uint64_t ullUin = 0;
    sscanf(m_ArgsMap[1], "%lu", &ullUin);
    bzero(&m_stSsPkg, sizeof(m_stSsPkg));
    int iRet = m_poRoleTable->GetRoleName(ullUin, m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoRsp.m_szResult, 1024);
    m_stSsPkg.m_stHead.m_ullUin = stSsReqPkg.m_stHead.m_ullUin;
    m_stSsPkg.m_stBody.m_stRoleGmUpdateInfoRsp.m_nErrNo = iRet;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_ROLE_GM_UPDATE_INFO_RSP;
    poWorkThread->SendPkg(m_stSsPkg);
    return ERR_NONE;
}

