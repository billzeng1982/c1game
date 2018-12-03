#include "GuildApply.h"
#include "LogMacros.h"
#include "GuildDataDynPool.h"
#include "../Player/PlayerMgr.h"

using namespace PKGMETA;

GuildApply::GuildApply()
{
    m_iCount = 0;
    m_iMaxNum = MAX_NUM_APPLY;
}


bool GuildApply::InitFromDB(DT_GUILD_APPLY_BLOB& rstBlob, uint64_t ullGuildId, int iMaxNum, uint16_t wVersion)
{
    DT_GUILD_APPLY_INFO stGuildApplyInfo;
    size_t ulUseSize = 0;
    int iRet = stGuildApplyInfo.unpack((char*)rstBlob.m_szData, sizeof(rstBlob.m_szData), &ulUseSize, wVersion);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Guild(%lu) init Apply failed, unpack DT_GUILD_APPLY_INFO failed, Ret=%d", ullGuildId, iRet);
        return false;
    }
    m_ullUptTimestamp = stGuildApplyInfo.m_ullUptTimestamp;
    m_iCount = 0;
    m_iMaxNum = iMaxNum;

    uint64_t ullTimestamp = PlayerMgr::Instance().GetUptTimestamp();
    if (m_ullUptTimestamp < ullTimestamp)
    {
        m_ullUptTimestamp = ullTimestamp;
    }
    else
    {
        for (int i=0; i<stGuildApplyInfo.m_wCount; i++)
        {
            iRet = this->Add(stGuildApplyInfo.m_astApplyList[i]);
            if (iRet != ERR_NONE)
            {
                LOGERR_r("Guild(%lu) init Apply failed, add apply(%lu) failed", ullGuildId, stGuildApplyInfo.m_astApplyList[i].m_ullUin);
                return false;
            }
        }
    }
    m_ullGuildId = ullGuildId;

    return true;
}


bool GuildApply::PackGuildApplyInfo(DT_GUILD_APPLY_BLOB& rstBlob, uint16_t wVersion)
{
    DT_GUILD_APPLY_INFO stGuildApplyInfo;
    stGuildApplyInfo.m_wCount= m_iCount;
    stGuildApplyInfo.m_ullUptTimestamp = m_ullUptTimestamp;

    m_oUinToApplyMapIter = m_oUinToApplyMap.begin();
    for (int i=0; m_oUinToApplyMapIter!= m_oUinToApplyMap.end() && i<m_iCount; m_oUinToApplyMapIter++, i++)
    {
        memcpy(&stGuildApplyInfo.m_astApplyList[i], m_oUinToApplyMapIter->second, sizeof(DT_ONE_GUILD_MEMBER));
    }

    size_t ulUseSize = 0;
    int iRet = stGuildApplyInfo.pack((char*)rstBlob.m_szData, MAX_LEN_GUILD_APPLY, &ulUseSize, wVersion);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Guild(%lu) pack DT_GUILD_APPLY_INFO failed, iRet=%d, UsedSize=%lu", m_ullGuildId, iRet, ulUseSize);
        return false;
    }
    rstBlob.m_iLen = (int)ulUseSize;

    return true;
}


int GuildApply::Add(DT_ONE_GUILD_MEMBER& rstOneApply)
{
    if (m_iCount >= m_iMaxNum)
    {
        return ERR_GUILD_MAX_APPLY;
    }

    m_oUinToApplyMapIter = m_oUinToApplyMap.find(rstOneApply.m_ullUin);
    if (m_oUinToApplyMapIter != m_oUinToApplyMap.end())
    {
        return ERR_ALREADY_EXISTED;
    }

    DT_ONE_GUILD_MEMBER* pstOneApply = GuildDataDynPool::Instance().GuildApplyPool().Get();
    if (pstOneApply == NULL)
    {
        LOGERR_r("Get pstOneApply from GuildDataDynPool failed");
        return ERR_SYS;
    }

    memcpy(pstOneApply, &rstOneApply, sizeof(DT_ONE_GUILD_MEMBER));

    m_oUinToApplyMap.insert(map<uint64_t, DT_ONE_GUILD_MEMBER*>::value_type(rstOneApply.m_ullUin, pstOneApply));
    m_iCount++;

    return ERR_NONE;
}


DT_ONE_GUILD_MEMBER* GuildApply::Find(uint64_t ullUin)
{
    m_oUinToApplyMapIter = m_oUinToApplyMap.find(ullUin);
    if (m_oUinToApplyMapIter == m_oUinToApplyMap.end())
    {
        return NULL;
    }
    else
    {
        return m_oUinToApplyMapIter->second;
    }
}


int GuildApply::Del(uint64_t ullUin)
{
    if (m_iCount <= 0)
    {
        return ERR_NOT_FOUND;
    }

    m_oUinToApplyMapIter = m_oUinToApplyMap.find(ullUin);
    if (m_oUinToApplyMapIter == m_oUinToApplyMap.end())
    {
        return ERR_NOT_FOUND;
    }

    DT_ONE_GUILD_MEMBER* pstOneApply = m_oUinToApplyMapIter->second;
    GuildDataDynPool::Instance().GuildApplyPool().Release(pstOneApply);
    m_oUinToApplyMap.erase(m_oUinToApplyMapIter);
    m_iCount--;

    return ERR_NONE;
}


void GuildApply::Clear()
{
    m_oUinToApplyMapIter = m_oUinToApplyMap.begin();
    for (; m_oUinToApplyMapIter!= m_oUinToApplyMap.end(); m_oUinToApplyMapIter++)
    {
        DT_ONE_GUILD_MEMBER* pstOneMember = m_oUinToApplyMapIter->second;
        GuildDataDynPool::Instance().GuildApplyPool().Release(pstOneMember);
    }

    m_oUinToApplyMap.clear();
    m_iCount = 0;
}
