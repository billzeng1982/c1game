#include "LevelRank.h"
#include "LogMacros.h"
#include "GameTime.h"
#include "../framework/ZoneSvrMsgLayer.h"

using namespace PKGMETA;

#define FAST_RECORD 0
#define FIRST_RECOMMEND_RECORD 1
#define SECOND_RECOMMEND_RECORD 2

#define BLOB_NAME_FIGHT_LEVEL_RECORD_FAST             "FightLevelRecordFast"
#define BLOB_NAME_FIGHT_LEVEL_RECORD_RECOMMEND_FIRST  "FightLevelRecordRecommendFirst"
#define BLOB_NAME_FIGHT_LEVEL_RECORD_RECOMMEND_SECOND "FightLevelRecordRecommendSecond"

#define FAKE_DATA 2

#define TIME_LOW_LIMT 20000

int LevelRankCmp(const void *pstFirst, const void *pstSecond)
{
    DT_FIGHT_LEVEL_RECORD_BIG_ITEM* pstItemFirst  = (DT_FIGHT_LEVEL_RECORD_BIG_ITEM*)pstFirst;
    DT_FIGHT_LEVEL_RECORD_BIG_ITEM* pstItemSecond = (DT_FIGHT_LEVEL_RECORD_BIG_ITEM*)pstSecond;

    int iResult = (int)pstItemFirst->m_dwLevelId - (int)pstItemSecond->m_dwLevelId;
    return iResult;
}

LevelRank::LevelRank()
{

}

LevelRank::~LevelRank()
{

}

bool LevelRank::Init(ZONESVRCFG* p_stConfig)
{
#if 0
    m_stRecordInfo.m_iCount = 0;
    m_bRecommendIndex = 0;
    m_bIsUpdated = false;
    m_dwEfficiency = 0;
    m_ullLastTm = CGameTime::Instance().GetCurrSecond();
    m_bIsUpdateServer = true;
    _GetRecordDataFromDB();
	m_pstConfig = p_stConfig;
#endif
    return true;
}

bool LevelRank::Fini()
{
#if 0
    _SetRecordDataToDB();
#endif
    return true;
}


void LevelRank::UpdateRecord(PlayerData* pstData, CS_PKG_FIGHT_PVE_SETTLE_REQ& rstCsPkgBodyReq)
{
#if 0
    //是否是攻城成功或者boss死亡
    if(!rstCsPkgBodyReq.m_bIsPass || (FIGHTSETTLE_CITYBREAK!=rstCsPkgBodyReq.m_bPassReason && FIGHTSETTLE_BOSSDEAD!=rstCsPkgBodyReq.m_bPassReason))
    {
        return;
    }

	// if(rstCsPkgBodyReq.m_ullCostTime<=TIME_LOW_LIMT)//低于20秒不做更新
	// {
	// 	return;
	// }

 //    //玩家信息
 //    DT_ROLE_MAJESTY_INFO& rstPlayerInfo = pstData->GetMajestyInfo();

 //    int iEqual = 0;
 //    DT_FIGHT_LEVEL_RECORD_BIG_ITEM stItem;
 //    stItem.m_dwLevelId = rstCsPkgBodyReq.m_dwFLevelID;
 //    int iIndex = MyBSearch(&stItem, m_stRecordInfo.m_astData, m_stRecordInfo.m_iCount, sizeof(DT_FIGHT_LEVEL_RECORD_BIG_ITEM), &iEqual, LevelRankCmp);

 //    //获取关卡推荐战力数据档
 //    RESFIGHTLEVEL* pResFightLevel = CGameDataMgr::Instance().GetResFightLevelMgr().Find(rstCsPkgBodyReq.m_dwFLevelID);
 //    uint32_t m_dwLowLimit  = pResFightLevel->m_forceLimit[1];//二档战斗力
 //    uint32_t m_dwHighLimit = pResFightLevel->m_forceLimit[3];//四档战斗力
 //    m_dwEfficiency = GeneralCard::Instance().GetTeamLi(pstData);//角色战斗力

 //    if (!iEqual)
 //    {
 //        //该关卡尚无记录
 //        m_bRecommendIndex = 0; //推荐记录替换编号
 //        //若符合条件，更新速战速决;
 //        _UpdateFastRecord(stItem, rstCsPkgBodyReq, rstPlayerInfo);

 //        //若符合条件，更新推荐阵容
 //        if(m_dwEfficiency>=m_dwLowLimit && m_dwEfficiency<=m_dwHighLimit)
 //        {
 //            stItem.m_bIndex = 1;
 //            _UpdateRecommendRecord(stItem, rstCsPkgBodyReq, rstPlayerInfo);
 //            bzero(&stItem.m_astRecommend[1], sizeof(stItem.m_astRecommend[1]));
 //            stItem.m_astRecommend[1].m_bIsFastedRecord = FAKE_DATA;//伪造另外一条推荐数据
 //        }
 //        //否则伪造推荐数据
 //        else
 //        {
 //            stItem.m_bIndex = 0;
 //            bzero(&stItem.m_astRecommend[0], sizeof(stItem.m_astRecommend[0]));
 //            bzero(&stItem.m_astRecommend[1], sizeof(stItem.m_astRecommend[1]));
 //            stItem.m_astRecommend[0].m_bIsFastedRecord = FAKE_DATA; //错误码表示数据是伪造的,两份
 //            stItem.m_astRecommend[1].m_bIsFastedRecord = FAKE_DATA;
 //        }
 //        //插入新的数据
 //        size_t nmemb = (size_t)m_stRecordInfo.m_iCount;
 //        MyBInsert(&stItem, m_stRecordInfo.m_astData, &nmemb, sizeof(DT_FIGHT_LEVEL_RECORD_BIG_ITEM), 1, LevelRankCmp);
 //        m_stRecordInfo.m_iCount = (int32_t)nmemb;
 //    }
 //    else
 //    {
 //        //存在关卡的记录
 //        m_bRecommendIndex = m_stRecordInfo.m_astData[iIndex].m_bIndex;
 //        //若符合条件，更新速战速决
 //        if(rstCsPkgBodyReq.m_ullCostTime < m_stRecordInfo.m_astData[iIndex].m_stFast.m_dwFinishTime)
 //        {
 //            _UpdateFastRecord(m_stRecordInfo.m_astData[iIndex], rstCsPkgBodyReq, rstPlayerInfo);
 //        }
 //        //若符合条件，更新推荐阵容
 //        if(m_dwEfficiency>=m_dwLowLimit && m_dwEfficiency<=m_dwHighLimit)
 //        {
 //            if(m_stRecordInfo.m_astData[iIndex].m_astRecommend[m_bRecommendIndex].m_bIsFastedRecord == FAKE_DATA)//若该记录是伪造的
 //            {
 //                _UpdateRecommendRecord(m_stRecordInfo.m_astData[iIndex], rstCsPkgBodyReq, rstPlayerInfo);
 //            }
 //            else if(rstCsPkgBodyReq.m_bStarEvalResult >= m_stRecordInfo.m_astData[iIndex].m_astRecommend[m_bRecommendIndex].m_bStarEvalResult)//有记录，符合条件
 //            {
 //                _UpdateRecommendRecord(m_stRecordInfo.m_astData[iIndex], rstCsPkgBodyReq, rstPlayerInfo);
 //            }
 //        }
 //        //设置推荐记录替换编号
 //        _SetNextRecommendIndex(m_stRecordInfo.m_astData[iIndex]);
 //    }
#endif
}


int LevelRank::GetRecord(CS_PKG_FIGHT_LEVEL_RECORD_REQ& rstCsPkgBodyReq, SC_PKG_FIGHT_LEVEL_RECORD_RSP& rstScPkgBodyRsp)
{
#if 0
    // MyBSearch(pstItem, m_stRecordInfo.m_astData, m_stRecordInfo.m_iCount, sizeof(DT_ITEM_GCARD), &iEqual, GeneralCardCmp);
    int iEqual = 0;
    DT_FIGHT_LEVEL_RECORD_BIG_ITEM stItem;
    stItem.m_dwLevelId = rstCsPkgBodyReq.m_dwFLevelID;
    int iIndex = MyBSearch(&stItem, m_stRecordInfo.m_astData, m_stRecordInfo.m_iCount, sizeof(DT_FIGHT_LEVEL_RECORD_BIG_ITEM), &iEqual, LevelRankCmp);
    if (!iEqual)
    {
        rstScPkgBodyRsp.m_astFightLevelRecordItem[FAST_RECORD].m_bIsFastedRecord = FAKE_DATA;
        rstScPkgBodyRsp.m_astFightLevelRecordItem[FIRST_RECOMMEND_RECORD].m_bIsFastedRecord = FAKE_DATA;
        rstScPkgBodyRsp.m_astFightLevelRecordItem[SECOND_RECOMMEND_RECORD].m_bIsFastedRecord = FAKE_DATA;
        rstScPkgBodyRsp.m_bItemCount = 0;
        rstScPkgBodyRsp.m_dwFLevelID = rstCsPkgBodyReq.m_dwFLevelID;
        return ERR_NONE;
    }

    rstScPkgBodyRsp.m_astFightLevelRecordItem[FAST_RECORD] = m_stRecordInfo.m_astData[iIndex].m_stFast;
    rstScPkgBodyRsp.m_astFightLevelRecordItem[FIRST_RECOMMEND_RECORD] = m_stRecordInfo.m_astData[iIndex].m_astRecommend[0];
    rstScPkgBodyRsp.m_astFightLevelRecordItem[SECOND_RECOMMEND_RECORD] = m_stRecordInfo.m_astData[iIndex].m_astRecommend[1];
    rstScPkgBodyRsp.m_nErrNo = ERR_NONE;
    rstScPkgBodyRsp.m_dwFLevelID = rstCsPkgBodyReq.m_dwFLevelID;
    rstScPkgBodyRsp.m_bItemCount = 3;
#endif
    return ERR_NONE;
}

void LevelRank::_SetNextRecommendIndex(DT_FIGHT_LEVEL_RECORD_BIG_ITEM& stItem)
{
    if(stItem.m_astRecommend[0].m_bIsFastedRecord != FAKE_DATA && stItem.m_astRecommend[0].m_bIsFastedRecord != FAKE_DATA) //两条都是真实数据
    {
        if(stItem.m_astRecommend[0].m_bStarEvalResult == stItem.m_astRecommend[1].m_bStarEvalResult)
        {
            //若星级相同，1变0，0变1
            stItem.m_bIndex ^= 1;
        }
        else
        {
            //星级数目不同，选小的
            stItem.m_bIndex = stItem.m_astRecommend[0].m_bStarEvalResult < stItem.m_astRecommend[1].m_bStarEvalResult ? 0:1;
        }
    }
    else
    {
        if(stItem.m_astRecommend[0].m_bIsFastedRecord == FAKE_DATA)
        {
            stItem.m_bIndex = 0;
        }
        else
        {
            stItem.m_bIndex = 1;
        }
    }
}

void LevelRank::_UpdateFastRecord(DT_FIGHT_LEVEL_RECORD_BIG_ITEM& stItem, CS_PKG_FIGHT_PVE_SETTLE_REQ& rstCsPkgBodyReq, DT_ROLE_MAJESTY_INFO& rstPlayerInfo)
{
    stItem.m_stFast.m_nErrNo = ERR_NONE; //错误码
    stItem.m_stFast.m_bIsFastedRecord = 1;//是否为最快记录
	stItem.m_stFast.m_wHeadIconId = rstPlayerInfo.m_wIconId;
	stItem.m_stFast.m_wHeadFrameId = rstPlayerInfo.m_wFrameId;
	stItem.m_stFast.m_wHeadTitleId = rstPlayerInfo.m_wTitleId;
    StrCpy(stItem.m_stFast.m_szPlayerName, rstPlayerInfo.m_szName, MAX_NAME_LENGTH);//角色名
    stItem.m_stFast.m_dwEfficiency = m_dwEfficiency;//战力
    stItem.m_stFast.m_bStarEvalResult = rstCsPkgBodyReq.m_bStarEvalResult;//星级
    stItem.m_stFast.m_dwMSkillID = rstCsPkgBodyReq.m_dwMSkillID;//军师技
    stItem.m_stFast.m_bGeneralCount = MAX_TROOP_NUM_PVP;//上场武将个数
    memcpy(&stItem.m_stFast.m_GeneralList, rstCsPkgBodyReq.m_GeneralList, sizeof(uint32_t)*MAX_TROOP_NUM_PVP);//武将列表
    stItem.m_stFast.m_dwFinishTime = rstCsPkgBodyReq.m_ullCostTime; //通关耗时
    m_bIsUpdated = true;
}

void LevelRank::_UpdateRecommendRecord(DT_FIGHT_LEVEL_RECORD_BIG_ITEM& stItem, CS_PKG_FIGHT_PVE_SETTLE_REQ& rstCsPkgBodyReq, DT_ROLE_MAJESTY_INFO& rstPlayerInfo)
{
    stItem.m_astRecommend[m_bRecommendIndex].m_nErrNo = ERR_NONE; //错误码
    stItem.m_astRecommend[m_bRecommendIndex].m_bIsFastedRecord = 0;//是否为最快记录
    //stItem.m_astRecommend[m_bRecommendIndex].m_dwPlayerHeadId  = rstPlayerInfo.m_wImageId;//玩家头像
	stItem.m_astRecommend[m_bRecommendIndex].m_wHeadFrameId = rstPlayerInfo.m_wFrameId;
	stItem.m_astRecommend[m_bRecommendIndex].m_wHeadTitleId = rstPlayerInfo.m_wTitleId;
    StrCpy(stItem.m_astRecommend[m_bRecommendIndex].m_szPlayerName, rstPlayerInfo.m_szName, MAX_NAME_LENGTH);//角色名
    stItem.m_astRecommend[m_bRecommendIndex].m_dwEfficiency = m_dwEfficiency;//战力
    stItem.m_astRecommend[m_bRecommendIndex].m_bStarEvalResult = rstCsPkgBodyReq.m_bStarEvalResult;//星级
    stItem.m_astRecommend[m_bRecommendIndex].m_dwMSkillID = rstCsPkgBodyReq.m_dwMSkillID;//军师技
    stItem.m_astRecommend[m_bRecommendIndex].m_bGeneralCount = MAX_TROOP_NUM_PVP;//上场武将个数
    memcpy(&stItem.m_astRecommend[m_bRecommendIndex].m_GeneralList, rstCsPkgBodyReq.m_GeneralList, sizeof(uint32_t)*MAX_TROOP_NUM_PVP);//武将列表
    stItem.m_astRecommend[m_bRecommendIndex].m_dwFinishTime = rstCsPkgBodyReq.m_ullCostTime; //通关耗时
    m_bIsUpdated = true;
}

bool LevelRank::_GetRecordDataFromDB()
{
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_LEVEL_RECORD_READ_REQ;
	//SS_PKG_LEVEL_RECORD_READ_REQ& rstLevelRecordReadReq = m_stSsPkg.m_stBody.m_stLevelRecordSaveReq;
    ZoneSvrMsgLayer::Instance().SendToMiscSvr(m_stSsPkg);
    return true;
}

bool LevelRank::_SetRecordDataToDB()
{
	//设置包体
	m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_LEVEL_RECORD_SAVE_REQ;
	SS_PKG_LEVEL_RECORD_SAVE_REQ& rstLevelRecordSaveReq = m_stSsPkg.m_stBody.m_stLevelRecordSaveReq;

    uint64_t ullUseSize = 0;
    int iRet = m_stRecordInfo.pack((char*)rstLevelRecordSaveReq.m_szData, MAX_LEN_LEVEL_RECORD, &ullUseSize);
    if( iRet != TdrError::TDR_NO_ERROR)
    {
        LOGERR_r("pack rstLevelRecordSaveReq failed, Ret=%d", iRet);
        return false;
    }
    rstLevelRecordSaveReq.m_dwLen = (int)ullUseSize;

	ZoneSvrMsgLayer::Instance().SendToMiscSvr(m_stSsPkg);
    return true;
}

void LevelRank::SetRecordBlob(SS_PKG_LEVEL_RECORD_READ_RSP& rstLevelRecordRsp)
{
#if 0
	int iRet = m_stRecordInfo.unpack((char*)rstLevelRecordRsp.m_szData, sizeof(rstLevelRecordRsp.m_szData));
	if (iRet != TdrError::TDR_NO_ERROR)
    {
		LOGERR("unpack DT_FIGHT_LEVEL_RECORD_BLOB failed!");
        m_stRecordInfo.m_iCount = 0;
	}
#endif
	return;
}

void LevelRank::UpdateServer()
{
#if 0
	time_t m_ullCurrTm = CGameTime::Instance().GetCurrSecond();
	if( m_bIsUpdateServer && ((m_ullCurrTm - m_ullLastTm) >= m_pstConfig->m_iPlayerUptDBFreq))
	{
		_SetRecordDataToDB();
		m_ullLastTm = m_ullCurrTm;
	}
#endif
}
