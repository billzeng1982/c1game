#include "Player.h"
#include "common_proto.h"
#include "LogMacros.h"
#include "PlayerMgr.h"
#include "PKGMETA_metalib.h"

using namespace PKGMETA;

Player::Player()
{
}

Player::~Player()
{
}


void Player::Clear()
{
    m_stApplyInfo.m_bApplyCount = 0;
}


bool Player::InitFromDB(DT_GUILD_PLAYER_DATA& rstPlayerData)
{
    memcpy(&m_stBaseInfo, &rstPlayerData.m_stBaseInfo, sizeof(m_stBaseInfo));

    uint16_t wVersion = m_stBaseInfo.m_wVersion;
    size_t ulUseSize = 0;
    m_stBaseInfo.m_wVersion = PKGMETA::MetaLib::getVersion();

    int iRet = m_stApplyInfo.unpack((char*)rstPlayerData.m_stApplyBlob.m_szData, sizeof(rstPlayerData.m_stApplyBlob.m_szData), &ulUseSize, wVersion);
    if (iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Player(%lu) unpack DT_GUILD_PLAYER_DATA, Ret=%d", m_stBaseInfo.m_ullUin, iRet);
        return false;
    }


    uint64_t ullTimestamp = PlayerMgr::Instance().GetUptTimestamp();
    if (ullTimestamp > m_stApplyInfo.m_ullUptTimestamp)
    {
        m_stApplyInfo.m_bApplyCount= 0;
        m_stApplyInfo.m_ullUptTimestamp = ullTimestamp;
    }

    return true;
}


bool Player::PackPlayerData(DT_GUILD_PLAYER_DATA& rstPlayerData, uint16_t wVersion)
{
    memcpy(&rstPlayerData.m_stBaseInfo, &m_stBaseInfo, sizeof(m_stBaseInfo));

    uint64_t ullTimestamp = PlayerMgr::Instance().GetUptTimestamp();
    if (ullTimestamp > m_stApplyInfo.m_ullUptTimestamp)
    {
        m_stApplyInfo.m_bApplyCount= 0;
        m_stApplyInfo.m_ullUptTimestamp = ullTimestamp;
    }

    size_t ulUseSize = 0;
    int iRet = m_stApplyInfo.pack((char*)rstPlayerData.m_stApplyBlob.m_szData, MAX_LEN_GUILD_PLAYER_APPLY, &ulUseSize, wVersion);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("Player(%lu) pack DT_PKG_GUILF_PLAYER_APPLY failed, Ret=%d", m_stBaseInfo.m_ullUin, iRet);
        return false;
    }
    rstPlayerData.m_stApplyBlob.m_iLen = (int)ulUseSize;

    return true;
}


int Player::FindApply(uint64_t ullGuildId)
{
    for (int i=0; i<m_stApplyInfo.m_bApplyCount; i++)
    {
        if (m_stApplyInfo.m_ApplyList[i] == ullGuildId)
        {
            return i;
        }
    }

    return -1;
}


int Player::AddApply(uint64_t ullGuildId)
{
    uint64_t ullTimestamp = PlayerMgr::Instance().GetUptTimestamp();
    if (ullTimestamp > m_stApplyInfo.m_ullUptTimestamp)
    {
        this->ClearApply();
        m_stApplyInfo.m_ullUptTimestamp = ullTimestamp;
    }

    if (m_stApplyInfo.m_bApplyCount>= MAX_APPLYGUILD_NUM)
    {
        return ERR_PLAYER_MAX_APPLY;
    }

    int i = FindApply(ullGuildId);
    if (i >= 0)
    {
        return ERR_ALREADY_EXISTED;
    }

    m_stApplyInfo.m_ApplyList[m_stApplyInfo.m_bApplyCount++] = ullGuildId;

    //数据变化，需要回写数据库
    PlayerMgr::Instance().AddToDirtyList(this);
    return ERR_NONE;
}


int Player::DelApply(uint64_t ullGuildId)
{
    for (int i=0; i<m_stApplyInfo.m_bApplyCount; i++)
    {
        if (m_stApplyInfo.m_ApplyList[i] == ullGuildId)
        {
            m_stApplyInfo.m_ApplyList[i] = m_stApplyInfo.m_ApplyList[--m_stApplyInfo.m_bApplyCount];

            //数据变化，需要回写数据库
            PlayerMgr::Instance().AddToDirtyList(this);
            return ERR_NONE;
        }
    }

    return ERR_NOT_FOUND;
}


void Player::SetGuildId(uint64_t ullGuildId)
{
    m_stBaseInfo.m_ullGuildId = ullGuildId;

    //数据变化，需要回写数据库
    PlayerMgr::Instance().AddToDirtyList(this);
}

void Player::ClearApply()
{
    m_stApplyInfo.m_bApplyCount= 0;

    //数据变化，需要回写数据库
    PlayerMgr::Instance().AddToDirtyList(this);
}


