#include "GuildReplay.h"
#include "LogMacros.h"
#include "GuildDataDynPool.h"
#include "GuildSvr.h"

using namespace PKGMETA;

GuildReplay::GuildReplay()
{
    m_iCount = 0;
}

GuildReplay::~GuildReplay()
{
}

bool GuildReplay::InitFromDB(DT_GUILD_REPLAY_BLOB& rstBlob, uint64_t ullGuildId, uint16_t wVersion)
{
    DT_GUILD_REPLAY_INFO stGuildReplayInfo;
    size_t ulUseSize = 0;
    int iRet = stGuildReplayInfo.unpack((char*)rstBlob.m_szData, sizeof(rstBlob.m_szData), &ulUseSize, wVersion);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Guild(%lu) init Repaly failed, unpack DT_GUILD_REPLAY_BLOB failed, Ret=%d", ullGuildId, iRet);
        return false;
    }

    m_iCount = 0;
    for (int i=0; i<stGuildReplayInfo.m_wCount; i++)
    {
        DT_REPLAY_INFO* pstOneReplay = GuildDataDynPool::Instance().GuildReplayPool().Get();
        assert(pstOneReplay);
        *pstOneReplay = stGuildReplayInfo.m_astReplayList[i];
        iRet = this->Add(pstOneReplay);
        if (iRet != ERR_NONE)
        {
            LOGERR_r("Guild(%lu) init Repaly failed, Ret=%d", ullGuildId, iRet);
            return false;
        }
    }

    m_ullGuildId = ullGuildId;
    return true;
}

bool GuildReplay::PackGuildReplayInfo(DT_GUILD_REPLAY_BLOB& rstBlob, uint16_t wVersion)
{
    DT_GUILD_REPLAY_INFO stGuildReplayInfo;
    stGuildReplayInfo.m_wCount = m_iCount;

    m_listReplayIter = m_listReplay.begin();
    for (int i=0; m_listReplayIter!= m_listReplay.end() && i<m_iCount; m_listReplayIter++, i++)
    {
        memcpy(&stGuildReplayInfo.m_astReplayList[i], *m_listReplayIter, sizeof(DT_REPLAY_INFO));
    }

    size_t ulUseSize = 0;
    int iRet = stGuildReplayInfo.pack((char*)rstBlob.m_szData, MAX_LEN_GUILD_REPLAY, &ulUseSize, wVersion);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Guild(%lu) pack DT_GUILD_REPLAY_INFO failed, iRet=%d, UsedSize=%lu", m_ullGuildId, iRet, ulUseSize);
        return false;
    }
    rstBlob.m_iLen = (int)ulUseSize;

    return true;
}

int GuildReplay::Add(DT_REPLAY_INFO* pstOneReplay)
{
    if (m_iCount >= MAX_NUM_GUILD_REPLAY)
    {
         DT_REPLAY_INFO* pstReplayInfo = m_listReplay.back();
         this->_RemoveReplay(pstReplayInfo->m_stFileHead.m_dwRaceNumber);
         GuildDataDynPool::Instance().GuildReplayPool().Release(pstReplayInfo);
         m_listReplay.pop_back();
         m_iCount--;
    }

    m_listReplay.push_front(pstOneReplay);
    m_iCount++;

    return ERR_NONE;
}

void GuildReplay::_RemoveReplay(uint32_t dwRaceNumber)
{
    char szFilePath[MAX_LEN_FILEPATH];
    sprintf(szFilePath, "%s/%u", GuildSvr::Instance().GetConfig().m_szRootDir, dwRaceNumber);
    int iRet = remove(szFilePath);
    if (iRet != 0)
    {
        LOGERR_r("remove Guild(%lu) Replay(%u) failed, Ret=%d", m_ullGuildId, dwRaceNumber, iRet);
    }
}

void GuildReplay::Clear()
{
    m_listReplayIter = m_listReplay.begin();
	for (; m_listReplayIter!= m_listReplay.end(); m_listReplayIter++)
	{
        GuildDataDynPool::Instance().GuildReplayPool().Release(*m_listReplayIter);
	}

    m_listReplay.clear();
    m_iCount = 0;
}

