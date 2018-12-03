#include "AsyncPvpPlayer.h"
#include "../rank/AsyncPvpRank.h"
#include "AsyncPvpRecordMgr.h"
#include "FakeRandom.h"
#include "AsyncPvpPlayerMgr.h"

using namespace PKGMETA;

bool AsyncPvpPlayer::InitFromDB(DT_ASYNC_PVP_PLAYER_WHOLE_DATA & rstPlayerData)
{
    memcpy(&m_stShowData, &rstPlayerData.m_stShowdata, sizeof(DT_ASYNC_PVP_PLAYER_SHOW_DATA));

    m_bOpponentCount = rstPlayerData.m_bOpponentCount;
    for (uint8_t i = 0; i < m_bOpponentCount; i++)
    {
        m_OpponentList[i] = rstPlayerData.m_OpponentInfo[i];
    }

    m_bRecordCount = rstPlayerData.m_bRecordCount;
    for (uint8_t i = 0; i < m_bRecordCount; i++)
    {
        m_RecordList[i] = rstPlayerData.m_astFightRecord[i].m_ullRecordNo;
        AsyncPvpRecordMgr::Instance().AddRecord(rstPlayerData.m_astFightRecord[i]);
    }

    m_bInFight = false;

    //!!!!每次重新初始化玩家,都需要重置一下rank(以RankList中的为准),以保证玩家身上的rank和ranklist中的一致
    m_stShowData.m_stBaseInfo.m_dwRank = AsyncPvpRank::Instance().GetRankByPlayer(m_stShowData.m_stBaseInfo.m_ullUin);

    return true;
}


void AsyncPvpPlayer::Clear()
{
    for (int i = 0; i < m_bRecordCount; i++)
    {
        AsyncPvpRecordMgr::Instance().DelRecord(m_RecordList[i]);
    }
}


void AsyncPvpPlayer::GetWholeData(DT_ASYNC_PVP_PLAYER_WHOLE_DATA& rstPlayerData)
{
    memcpy(&rstPlayerData.m_stShowdata, &m_stShowData, sizeof(DT_ASYNC_PVP_PLAYER_SHOW_DATA));

    rstPlayerData.m_bOpponentCount = m_bOpponentCount;
    for (uint8_t i=0; i<m_bOpponentCount; i++)
    {
        rstPlayerData.m_OpponentInfo[i] = m_OpponentList[i];
    }

    rstPlayerData.m_bRecordCount = m_bRecordCount;
    for (uint8_t i=0; i<m_bRecordCount; i++)
    {
        rstPlayerData.m_astFightRecord[i].m_ullRecordNo = m_RecordList[i];
        AsyncPvpRecordMgr::Instance().GetRecord(rstPlayerData.m_astFightRecord[i]);
    }
}


void AsyncPvpPlayer::GetShowData(DT_ASYNC_PVP_PLAYER_SHOW_DATA& rstShowData)
{
    memcpy(&rstShowData, &m_stShowData, sizeof(DT_ASYNC_PVP_PLAYER_SHOW_DATA));
}


void AsyncPvpPlayer::UptShowData(DT_ASYNC_PVP_PLAYER_SHOW_DATA& rstShowData)
{
    rstShowData.m_stBaseInfo.m_dwRank = m_stShowData.m_stBaseInfo.m_dwRank;
    rstShowData.m_stBaseInfo.m_dwWinCnt = m_stShowData.m_stBaseInfo.m_dwWinCnt;
    memcpy(&m_stShowData, &rstShowData, sizeof(DT_ASYNC_PVP_PLAYER_SHOW_DATA));

    AsyncPvpPlayerMgr::Instance().AddToDirtyList(this);
}


void AsyncPvpPlayer::GetBaseData(DT_ASYNC_PVP_PLAYER_BASE_INFO& rstBaseInfo)
{
    memcpy(&rstBaseInfo, &m_stShowData.m_stBaseInfo, sizeof(DT_ASYNC_PVP_PLAYER_BASE_INFO));
}


uint8_t AsyncPvpPlayer::GetOpponentList(uint32_t astOpponentList[])
{
    for (int i = 0; i < m_bOpponentCount; i++)
    {
        astOpponentList[i] = m_OpponentList[i];
    }

    return m_bOpponentCount;
}


void AsyncPvpPlayer::RefreshOpponentList()
{
    uint8_t bSelectCount = 5;
    uint32_t dwRank = m_stShowData.m_stBaseInfo.m_dwRank;
    m_bOpponentCount = 0;

    if (dwRank > 11)
    {
        //随机选择的下界
        //选人的规则: Rank代表自己的名次，从Rank*0.35到Rank的范围内随机选择最多4个人，Rank到Rank*1.5+15范围内随机选择剩下的人

        //Rank*0.35下界为11
        uint32_t dwLow = (dwRank * 35 /100) < 11 ? 11 : (dwRank * 35 /100);
        uint8_t bLowCout = (dwRank - dwLow) > 4 ? 4 : (dwRank - dwLow);

        //选择规则:将Rank*0.35到Rank的范围内的人平均分为M份，M为选择的人数(最大为4)
        //从这M份中分别随机选出1个人
        uint32_t dwInterval = (dwRank - dwLow) / bLowCout;
        uint32_t dwRemainder = (dwRank - dwLow) - (dwInterval * bLowCout);
        uint32_t dwMinRange, dwMaxRange = dwLow;
        for (int i = 0; i < bLowCout; i++)
        {
            dwMinRange = dwMaxRange;
            dwMaxRange += dwInterval;
            if (i < (int)dwRemainder)
            {
                dwMaxRange++;
            }
            uint32_t dwRandom = CFakeRandom::Instance().Random(dwMinRange, dwMaxRange);
            m_OpponentList[m_bOpponentCount + i] = dwRandom;
        }

        m_bOpponentCount += bLowCout;
        bSelectCount -= bLowCout;
    }

    //选择规则:从Rank到Rank*1.5+15范围内随机选择剩余人
    uint32_t dwMaxRank = AsyncPvpRank::Instance().GetRankMax();
    uint32_t dwHigh = (dwRank * 15/10 + 15) > (dwMaxRank + 1) ? (dwMaxRank + 1) : (dwRank * 15/10 + 15);
    uint32_t dwLow = (dwRank + 1) < 11 ? 11 : (dwRank + 1);
    uint8_t bHighCout = bSelectCount > (dwHigh-dwLow) ? (dwHigh-dwLow) : bSelectCount;

    if (bHighCout > 0)
    {
        uint32_t dwInterval = (dwHigh - dwLow) / bHighCout;
        uint32_t dwRemainder = (dwHigh - dwLow) - (dwInterval * bHighCout);
        uint32_t dwMinRange, dwMaxRange = dwLow;

        for (int i = 0; i < bHighCout; i++)
        {
            dwMinRange = dwMaxRange;
            dwMaxRange += dwInterval;
            if (i < (int)dwRemainder)
            {
                dwMaxRange++;
            }
            uint32_t dwRandom = CFakeRandom::Instance().Random(dwMinRange, dwMaxRange);
            m_OpponentList[m_bOpponentCount + i] = dwRandom;
        }
        m_bOpponentCount += bHighCout;
    }

    AsyncPvpPlayerMgr::Instance().AddToDirtyList(this);
}


bool AsyncPvpPlayer::CheckOpponent(uint32_t dwOpponent)
{
    for (uint8_t i = 0; i < m_bOpponentCount; i++)
    {
        if (m_OpponentList[i] == dwOpponent)
        {
            return true;
        }
    }

    if ((dwOpponent <= 10) && (m_stShowData.m_stBaseInfo.m_dwRank <= 20))
    {
        return true;
    }

    return false;
}


int AsyncPvpPlayer::AddRecord(DT_ASYNC_PVP_FIGHT_RECORD& rstRecord)
{
    if (this->GetPlayerId() <= MAX_NUM_ASYNC_PVP_FAKE_PLAYER_UIN)
    {
        return ERR_NONE;
    }

    if (m_bRecordCount >= MAX_NUM_ASYNC_PVP_RECORD)
    {
        this->_DelRecord();
    }

    m_RecordList[m_bRecordCount++] = rstRecord.m_ullRecordNo;
    AsyncPvpRecordMgr::Instance().AddRecord(rstRecord);

    AsyncPvpPlayerMgr::Instance().AddToDirtyList(this);

    return ERR_NONE;
}


void AsyncPvpPlayer::_DelRecord()
{
    AsyncPvpRecordMgr::Instance().DelRecord(m_RecordList[0]);

    for (int i = 0; i < MAX_NUM_ASYNC_PVP_RECORD -1; i++)
    {
        m_RecordList[i] = m_RecordList[i+1];
    }
    m_bRecordCount--;

    return;
}


uint8_t AsyncPvpPlayer::GetRecordList(DT_ASYNC_PVP_FIGHT_RECORD astRecordList[])
{
    for (int i = 0; i < m_bRecordCount; i++)
    {
        astRecordList[i].m_ullRecordNo = m_RecordList[i];
        AsyncPvpRecordMgr::Instance().GetRecord(astRecordList[i]);
    }

    return m_bRecordCount;
}

void AsyncPvpPlayer::AddWorshipGold(int32_t iWorshippedGoldOnce, int32_t iWorshippedGoldMax)
{
	if (m_iGoldWorshipped+iWorshippedGoldOnce <= iWorshippedGoldMax)
	{
		m_iGoldWorshipped += iWorshippedGoldOnce;
	}

	return;
}

uint32_t AsyncPvpPlayer::GetWorshipGold()
{
	int32_t iRet = m_iGoldWorshipped;

	m_iGoldWorshipped = 0;

	return iRet;
}
