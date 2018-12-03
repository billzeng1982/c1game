#include "LogQueryMsgLogic.h"
#include "LogMacros.h"
#include "LogQuerySvrMsgLayer.h"
#include "PythonBinding.h"
#include "../module/RoleTableMgr.h"
#include "strutil.h"
#include "common_proto.h"

using namespace PKGMETA;

int IdipReq_SS::HandleServerMsg(SSPKG& rstSsPkg, void* pvPara)
{
    SS_PKG_IDIP_REQ& rstIdipReq = rstSsPkg.m_stBody.m_stIdipReq;
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_IDIP_RSP;

    size_t ulUseSize = 0;
    int iRet = m_stIdipPkg.unpack((char*)rstIdipReq.m_szData, sizeof(rstIdipReq.m_szData), &ulUseSize);

    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR("unpack IdipPkg failed!");
        return -1;
    }

    switch (m_stIdipPkg.m_stHead.m_wMsgId)
    {
    case IDIP_MSG_GM_GET_PLAYER_LOG_REQ:
        _HandleGetPlayerLogMsg(m_stIdipPkg, rstSsPkg.m_stHead.m_ullUin);
        break;
    default:
        LOGERR("IdipPkg MsgId(%d) error", m_stIdipPkg.m_stHead.m_wMsgId);
        break;
    }

    return 0;
}

void IdipReq_SS::_HandleGetPlayerLogMsg(IdipPkg& rstIdipPkg, uint64_t ullKey)
{
    IDIP_PKG_GM_GET_PLAYER_LOG_REQ& rstGetPlayerLogReq = rstIdipPkg.m_stBody.m_stGmGetPlayerLogReq;

    if ((rstGetPlayerLogReq.m_ullUin == 0) && (rstGetPlayerLogReq.m_szName != "\0"))
    {
        RoleTableMgr::Instance().GetRoleUinByName(rstGetPlayerLogReq.m_szName, rstGetPlayerLogReq.m_ullUin);
        LOGRUN("the rstGetPlayerLogReq.m_ullUin<%lu>", rstGetPlayerLogReq.m_ullUin);
    }

    if(rstGetPlayerLogReq.m_bType != PKGMETA::GM_OPT_TYPE_QUERY_PLAYER_INFO)
	{
        char tmp[64];
        sprintf(tmp, "%lu", rstGetPlayerLogReq.m_ullUin);
        PyObject* pArgs = Py_BuildValue("sssi", tmp, rstGetPlayerLogReq.m_szStartTime,
                                     rstGetPlayerLogReq.m_szEndTime, rstGetPlayerLogReq.m_bType);

        PyObject* pRet = PythonBinding::Instance().func_QueryLog("PlayerLog", pArgs);
    	
    	uint64_t ullUin = rstGetPlayerLogReq.m_ullUin;
        m_stIdipPkg.m_stHead.m_wMsgId = IDIP_MSG_GM_GET_PLAYER_LOG_RSP;
        m_stIdipPkg.m_stHead.m_ullReservId = ullUin;
        IDIP_PKG_GM_GET_PLAYER_LOG_RSP& rstGetPlayerLogRsp = m_stIdipPkg.m_stBody.m_stGmGetPlayerLogRsp;

		m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_IDIP_RSP;
        m_stSsPkg.m_stHead.m_ullUin = ullKey;
		SS_PKG_IDIP_RSP& rstIdipRsp = m_stSsPkg.m_stBody.m_stIdipRsp;
		rstIdipRsp.m_dwLen = 0;
		rstGetPlayerLogRsp.m_wCount = 0;

		if (NULL != pRet)
		{
			int iSize = PyList_Size(pRet);
			rstGetPlayerLogRsp.m_wCount = (uint16_t)iSize;

			for (int i = 0; i < iSize; i++)
			{
				PyObject *pItem = PyList_GetItem(pRet, i);
				char* pszBuf;
				PyArg_Parse(pItem, "s", &pszBuf);

				if (NULL == pszBuf)
				{
					LOGERR("error para");
					return;
				}

				//StrCpy(rstGetPlayerLogRsp.m_aszLogInfo[i], pszBuf, strlen(pszBuf));
				snprintf(rstGetPlayerLogRsp.m_aszLogInfo[i], MAX_LEN_GM_LOG, "%s", pszBuf);
			}

			Py_DECREF(pRet);
		}

        size_t ulUseSize = 0;
        int iRet = m_stIdipPkg.pack((char*)rstIdipRsp.m_szData, sizeof(rstIdipRsp.m_szData), &ulUseSize);

        if( iRet != TdrError::TDR_NO_ERROR)
        {
            LOGERR("pack DT_ROLE_MAJESTY_BLOB failed!");
            return;
        }

        rstIdipRsp.m_dwLen = (uint32_t)ulUseSize;

        LogQuerySvrMsgLayer::Instance().SendToIdipAgent(m_stSsPkg);

        return;
    }
    else
	{
        uint64_t ullUin = rstGetPlayerLogReq.m_ullUin;
        m_stIdipPkg.m_stHead.m_wMsgId = IDIP_MSG_GM_GET_PLAYER_LOG_RSP;
        m_stIdipPkg.m_stHead.m_ullReservId = ullUin;
        IDIP_PKG_GM_GET_PLAYER_LOG_RSP& rstGetPlayerLogRsp = m_stIdipPkg.m_stBody.m_stGmGetPlayerLogRsp;

        //ȡֵ
        DT_ROLE_MAJESTY_BLOB m_stMajestyBlob;
        DT_ROLE_MAJESTY_INFO m_stMajestyInfo;
        bzero(&m_stMajestyBlob, sizeof(m_stMajestyBlob));
        bzero(&m_stMajestyInfo, sizeof(m_stMajestyInfo));
		RoleTableMgr::Instance().GetRoleMajestyInfo(rstGetPlayerLogReq.m_ullUin, m_stMajestyBlob);
        int iRet = m_stMajestyInfo.unpack((char*)m_stMajestyBlob.m_szData, sizeof(m_stMajestyBlob.m_szData));

        if (iRet != TdrError::TDR_NO_ERROR) 
		{
            LOGERR("unpack DT_ROLE_MAJESTY_BLOB failed!");
            return;
        }

        DT_ROLE_GUILD_BLOB m_stGuildBlob;
        DT_ROLE_GUILD_INFO m_stGuildInfo;
        bzero(&m_stGuildBlob, sizeof(m_stGuildBlob));
        bzero(&m_stGuildInfo, sizeof(m_stGuildInfo));
        RoleTableMgr::Instance().GetRoleGuildInfo(rstGetPlayerLogReq.m_ullUin, m_stGuildBlob);
        iRet = m_stGuildInfo.unpack((char*)m_stGuildBlob.m_szData, sizeof(m_stGuildBlob.m_szData));

        if (iRet != TdrError::TDR_NO_ERROR) 
		{
            LOGERR("unpack DT_ROLE_GUILD_BLOB failed!");
            return;
        }

        DT_ROLE_GCARD_BLOB m_stGcardBlob;
        DT_ROLE_GCARD_INFO m_stGcardInfo;
        bzero(&m_stGcardBlob, sizeof(m_stGcardBlob));
        RoleTableMgr::Instance().GetRoleGcardInfo(rstGetPlayerLogReq.m_ullUin, m_stGcardBlob);
        iRet = m_stGcardInfo.unpack((char*)m_stGcardBlob.m_szData, sizeof(m_stGcardBlob.m_szData));

        if (iRet != TdrError::TDR_NO_ERROR) 
		{
            LOGERR("unpack DT_ROLE_GCARD_BLOB failed!");
            return;
        }

        int iSize = 0;

        //获取渠道名
        char szChannelName[64];
        RoleTableMgr::Instance().GetChannelName(ullUin, szChannelName, 64);

        char ret_tmp[MAX_LEN_GM_LOG];
        sprintf(ret_tmp, "{'Uin':%lu, 'RoleName':'%s', 'Level':%d, 'GuildId':%ld, 'Gold':%d, 'Diamond':%d, 'Exp':%d, 'VipLv':%d, 'VipExp':%d, 'ChannelName':'%s'}\n",
                rstGetPlayerLogReq.m_ullUin, m_stMajestyInfo.m_szName, m_stMajestyInfo.m_wLevel, m_stGuildInfo.m_ullGuildId, m_stMajestyInfo.m_dwGold, m_stMajestyInfo.m_dwDiamond,
                 m_stMajestyInfo.m_dwExp, m_stMajestyInfo.m_bVipLv, m_stMajestyInfo.m_dwVipExp, szChannelName);

        snprintf(rstGetPlayerLogRsp.m_aszLogInfo[iSize++], MAX_LEN_GM_LOG, "%s", ret_tmp);

		for (int i = 0; i < m_stGcardInfo.m_iCount; i++)
		{
			ret_tmp[0] = '\0';
			sprintf(ret_tmp,"{'ID':%d, 'Level':%d, 'LvPhase':%d, 'Exp':%d, 'Star':%d, 'Phase':%d, 'SkillLevel1':%d, 'SkillLevel2':%d, 'SkillLevel3':%d, 'SkillLevel4':%d, 'SkillLevel5':%d, 'SkillLevel6':%d, 'CheatsType':%d, 'ArmyLv':%d, 'ArmyPhase':%d}\n",
				m_stGcardInfo.m_astData[i].m_dwId, m_stGcardInfo.m_astData[i].m_bLevel, m_stGcardInfo.m_astData[i].m_bLvPhase, m_stGcardInfo.m_astData[i].m_dwExp, m_stGcardInfo.m_astData[i].m_bStar, m_stGcardInfo.m_astData[i].m_bPhase,
				m_stGcardInfo.m_astData[i].m_szSkillLevel[0], m_stGcardInfo.m_astData[i].m_szSkillLevel[1], m_stGcardInfo.m_astData[i].m_szSkillLevel[2], m_stGcardInfo.m_astData[i].m_szSkillLevel[3], m_stGcardInfo.m_astData[i].m_szSkillLevel[4],
                m_stGcardInfo.m_astData[i].m_szSkillLevel[5], m_stGcardInfo.m_astData[i].m_bCheatsType, m_stGcardInfo.m_astData[i].m_bArmyLv, m_stGcardInfo.m_astData[i].m_bArmyPhase);
			snprintf(rstGetPlayerLogRsp.m_aszLogInfo[iSize++], MAX_LEN_GM_LOG, "%s", ret_tmp);
		}
		
        rstGetPlayerLogRsp.m_wCount = (uint16_t)iSize;

        m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_IDIP_RSP;
        m_stSsPkg.m_stHead.m_ullUin = ullKey;
        SS_PKG_IDIP_RSP& rstIdipRsp = m_stSsPkg.m_stBody.m_stIdipRsp;
        size_t ulUseSize = 0;
        iRet = m_stIdipPkg.pack((char*)rstIdipRsp.m_szData, sizeof(rstIdipRsp.m_szData), &ulUseSize);

        if( iRet != TdrError::TDR_NO_ERROR)
        {
            LOGERR("pack DT_ROLE_MAJESTY_BLOB failed!");
            return;
        }

        rstIdipRsp.m_dwLen = (uint32_t)ulUseSize;

        LogQuerySvrMsgLayer::Instance().SendToIdipAgent(m_stSsPkg);

        return;
    }
}
