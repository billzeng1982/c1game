
#include "Mall.h"

#include "Consume.h"
#include "Item.h"
#include "dwlog_svr.h"
#include "ZoneLog.h"
#include "Friend.h"
#include "Mail.h"
#include "../framework/ZoneSvrMsgLayer.h"
#include "Gm/Gm.h"
#include "Task.h"
#include "TimeCycle.h"

using namespace PKGMETA;

#define GOODS_CAN_BUY_NOT_LIMIT 1       //不限量
#define GOODS_CAN_BUY_DAY_LIMIT 2       //每日限量
#define GOODS_CAN_BUY_EVER_LIMIT 3      //永久限量

#define GOODS_NO_DISCOUNT   100         //折扣

static int CompareId(const void* pA, const void* pB)
{
    return ((DT_MALL_GOODS_BUY_INFO*)pA)->m_dwGoodsId - ((DT_MALL_GOODS_BUY_INFO*)pB)->m_dwGoodsId;
}

bool Mall::Init()
{
   //初始化 更新时间 相关
    RESBASIC* pResBasic = CGameDataMgr::Instance().GetResBasicMgr().Find(COMMON_UPDATE_TIME);
    if (!pResBasic)
    {
        LOGERR("Mall init error");
        return false;
    }
    m_iUptTime = (int)(pResBasic->m_para[0]);
    m_ullDayUpdateLastTime = CGameTime::Instance().GetSecOfHourInCurrDay(m_iUptTime) * 1000;
    if (CGameTime::Instance().GetCurrHour() < m_iUptTime)
    {
        m_ullDayUpdateLastTime -= MSSECONDS_OF_DAY;
    }
    UpdateFromGameData();
    return true;
}
//  服务器更新
void Mall::UpdateServer()
{
    // 每日活动副本更新
    uint64_t ullUpdateTime = 0;
    if (CGameTime::Instance().IsNeedUpdateByHour(m_ullDayUpdateLastTime, m_iUptTime, ullUpdateTime))
    {
        m_ullDayUpdateLastTime = ullUpdateTime;
    }
}

void Mall::UpdatePlayerData(PlayerData* pstData)
{
    DT_MALL_INFO& rstMallInfo = pstData->GetMiscInfo().m_stMallInfo;
    if ( rstMallInfo.m_ullDayLastUpdateTime < m_ullDayUpdateLastTime)
    {
        for (size_t i = 0; i< rstMallInfo.m_wDayLimitBuyNum; i++)
        {
            rstMallInfo.m_astDayLimitBuyList[i].m_wBuyNum = 0;
            rstMallInfo.m_astDayLimitBuyList[i].m_dwGoodsId = 0;
        }
        rstMallInfo.m_wDayLimitBuyNum = 0 ;
        rstMallInfo.m_ullDayLastUpdateTime = m_ullDayUpdateLastTime;
    }
}

void Mall::UpdateFromGameData()
{
    m_GoodsBuyTag.clear();
    uint32_t dwNum = (uint32_t)CGameDataMgr::Instance().GetResMallPropsMgr().GetResNum();
    RESMALLPROPS* poResMallProp = NULL;
    for (uint32_t i = 0; i < dwNum; i++)
    {
        poResMallProp = CGameDataMgr::Instance().GetResMallPropsMgr().GetResByPos(i);
        if (NULL == poResMallProp)
        {
            LOGERR("Get  ResMallProps error, Pos<%u>", i);
            continue;
        }
        m_GoodsBuyTag[poResMallProp->m_dwId] = poResMallProp->m_bCanBuy;
    }
}

void Mall::SendNtf(PlayerData* pstData)
{

}

//  获取商品折扣
//      折扣时间无效,不打折,返回100
//      折扣时间有效,返回打折点数
uint8_t Mall::_GetGoodsDiscount(RESMALLPROPS* poResMallProp)
{
    if (0 == _IsDiscountTime(poResMallProp->m_dwDiscountTimeId))
    {
        return poResMallProp->m_bDiscount;
    }
    else
    {
        return GOODS_NO_DISCOUNT;
    }
}

//  判断折扣时间是否有效
//      0:默认值,有效
//     return 0 代表有效
int Mall::_IsDiscountTime(uint32_t dwTimeId)
{
    if (dwTimeId == 0)
    {
        return -1;
    }
    return TimeCycle::Instance().CheckTime(dwTimeId);
}

//  判断是否限时购买
//      0:默认值,可购买
//     return 0 代表有效
int Mall::_IsBuyTime(uint32_t dwTimeId)
{
    if (dwTimeId == 0)
    {
        return -1;
    }
    return TimeCycle::Instance().CheckTime(dwTimeId);
    //return CheckTime(dwTimeId);
}

//判断商品购买数量是否达到上限
//      1:默认,不限制
//      return: ture#可以购买|false#已达上限
int Mall::_IsBuyNumLimit(DT_MALL_GOODS_BUY_INFO* poGoodsBuyInfo, DT_MALL_INFO& rstMallInfo, RESMALLPROPS* poResMallProps)
{
    if (GOODS_CAN_BUY_NOT_LIMIT == poResMallProps->m_bLimitType)
    {// 不限量类型
        return 0;
    }
    if ( NULL != poGoodsBuyInfo)
    {//已有购买信息, 判断数量

        return poGoodsBuyInfo->m_wBuyNum < poResMallProps->m_dwLimitNum ? 0 : -1;
    }
    if (0 == poResMallProps->m_dwLimitNum)
    {//限制数量为0,不能购买
        return -2;
    }

    if (GOODS_CAN_BUY_DAY_LIMIT == poResMallProps->m_bLimitType)
    {//购买信息数量超过上限
        return rstMallInfo.m_wDayLimitBuyNum < MAX_MALL_LIMIT_BUY_NUM ? 0 : -3;
    }
    else if (GOODS_CAN_BUY_EVER_LIMIT == poResMallProps->m_bLimitType)
    {//购买信息数量超过上限
        return rstMallInfo.m_wEverLimitBuyNum < MAX_MALL_LIMIT_BUY_NUM ? 0 : -4;
    }
    return -5;
}

//增加购买物品 限制数量
//      不限量的不做处理
void Mall::_AddBuyNum(DT_MALL_GOODS_BUY_INFO* poGoodsBuyInfo, uint16_t wBuyNum, DT_MALL_INFO& rstMallInfo, RESMALLPROPS* poResMallProps)
{
    if (GOODS_CAN_BUY_NOT_LIMIT != poResMallProps->m_bLimitType)
    {
        if (NULL != poGoodsBuyInfo)
        {
            poGoodsBuyInfo->m_wBuyNum += wBuyNum;
            return;
        }
        else
        {
            if (GOODS_CAN_BUY_DAY_LIMIT == poResMallProps->m_bLimitType)
            {
                size_t nmemb = (size_t)rstMallInfo.m_wDayLimitBuyNum;
                if (nmemb >= MAX_MALL_LIMIT_BUY_NUM)
                {
                    LOGERR("rstMallInfo.m_wDayLimitBuyNum<%d> reaches MAX_MALL_LIMIT_BUY_NUM", rstMallInfo.m_wDayLimitBuyNum);
                    return;
                }
                DT_MALL_GOODS_BUY_INFO stTmpInfo = {0};
                stTmpInfo.m_dwGoodsId = poResMallProps->m_dwId;
                stTmpInfo.m_wBuyNum = wBuyNum;
                if (!MyBInsert(&stTmpInfo, rstMallInfo.m_astDayLimitBuyList, &nmemb, sizeof(DT_MALL_GOODS_BUY_INFO), 1, CompareId))
                {
                    LOGERR("Insert DT_MALL_GOODS_BUY_INFO  err!");
                    return;
                }

                rstMallInfo.m_wDayLimitBuyNum = (uint16_t)nmemb;
            }
            else if(GOODS_CAN_BUY_EVER_LIMIT == poResMallProps->m_bLimitType)
            {
                size_t nmemb = (size_t)rstMallInfo.m_wEverLimitBuyNum;
                if (nmemb >= MAX_MALL_LIMIT_BUY_NUM)
                {
                    LOGERR("rstMallInfo.m_wEverLimitBuyNum<%d> reaches MAX_MALL_LIMIT_BUY_NUM", rstMallInfo.m_wEverLimitBuyNum);
                    return;
                }
                DT_MALL_GOODS_BUY_INFO stTmpInfo = {0};
                stTmpInfo.m_dwGoodsId = poResMallProps->m_dwId;
                stTmpInfo.m_wBuyNum = wBuyNum;
                if (!MyBInsert(&stTmpInfo, rstMallInfo.m_astEverLimitBuyList, &nmemb, sizeof(DT_MALL_GOODS_BUY_INFO), 1, CompareId))
                {
                    LOGERR("Insert DT_MALL_GOODS_BUY_INFO  err!");
                    return;
                }
                rstMallInfo.m_wEverLimitBuyNum = (uint16_t)nmemb;
            }
            return;
        }
    }
    return;
}

DT_MALL_GOODS_BUY_INFO* Mall::_FindGoodsBuyInfo(RESMALLPROPS* poResMallProp, DT_MALL_INFO& rstMallInfo)
{
    if (GOODS_CAN_BUY_DAY_LIMIT == poResMallProp->m_bLimitType)
    {
        DT_MALL_GOODS_BUY_INFO tmpBuyInfo = {0};
        tmpBuyInfo.m_dwGoodsId = poResMallProp->m_dwId;
        int iEqual = 0;
        uint16_t wIndex = MyBSearch(&tmpBuyInfo, rstMallInfo.m_astDayLimitBuyList, rstMallInfo.m_wDayLimitBuyNum, sizeof(DT_MALL_GOODS_BUY_INFO), &iEqual, CompareId);
        if (!iEqual)
        {
            return NULL;
        }
        else
        {
            return &rstMallInfo.m_astDayLimitBuyList[wIndex];
        }
    }
    else if( GOODS_CAN_BUY_EVER_LIMIT == poResMallProp->m_bLimitType)
    {
        DT_MALL_GOODS_BUY_INFO tmpBuyInfo = {0};
        tmpBuyInfo.m_dwGoodsId = poResMallProp->m_dwId;
        int iEqual = 0;
        uint16_t wIndex = MyBSearch(&tmpBuyInfo, rstMallInfo.m_astEverLimitBuyList, rstMallInfo.m_wEverLimitBuyNum, sizeof(DT_MALL_GOODS_BUY_INFO), &iEqual, CompareId);
        if (!iEqual)
        {
            return NULL;
        }
        else
        {
            return &rstMallInfo.m_astEverLimitBuyList[wIndex];
        }
    }
    else
    {
        return NULL;
    }
}

//购买接口
int Mall::Buy(PlayerData* pstData, CS_PKG_MALL_BUY_REQ& rstReq, SC_PKG_MALL_BUY_RSP& rstRsp)
{
    uint32_t dwGoodsId = rstReq.m_dwGoodsId;
    RESMALLPROPS* poResMallProp = CGameDataMgr::Instance().GetResMallPropsMgr().Find(dwGoodsId);
    if (NULL == poResMallProp)
    {
        LOGERR("Uin<%lu> the ResMallProps id<%u> is NULL", pstData->m_ullUin, dwGoodsId);
        return ERR_SYS;
    }
    /**
     *  购买类型
     *  1.每天物品限量购买几次;
     *  2.物品永久购买次数
     *  3.
     **/

    if (!_IsGoodsUp(dwGoodsId))
    {
        LOGERR("Uin<%lu> Mall Goods<%u> is taken down", pstData->m_ullUin, dwGoodsId);
        return ERR_GOODS_TAKEN_DOWN;
    }
    int iRet = _IsBuyTime(poResMallProp->m_dwLimitTimeId);
    if (0 != iRet)
    {
        LOGERR("Uin<%lu> Mall Goods<%u> is limit buytime<%u> iRet<%d>", pstData->m_ullUin, dwGoodsId, poResMallProp->m_dwLimitTimeId, iRet);
        return ERR_GOODS_LIMIT_BUY_TIME;
    }
    DT_MALL_INFO& rstMallInfo = pstData->GetMiscInfo().m_stMallInfo;
    DT_MALL_GOODS_BUY_INFO* poGoodsBuyInfo = _FindGoodsBuyInfo(poResMallProp, rstMallInfo);
    iRet = _IsBuyNumLimit(poGoodsBuyInfo, rstMallInfo, poResMallProp);
    if ( 0 != iRet )
    {
        uint16_t wBoughtNum = 0;    //物品已购买次数
        if (NULL != poGoodsBuyInfo)
        {
            wBoughtNum = poGoodsBuyInfo->m_wBuyNum;
        }
        LOGERR("Uin<%lu> Mall Goods<%u> buy num limit Num<%u>,BoughtyNum<%u>, Ret<%d> ", pstData->m_ullUin, dwGoodsId, poResMallProp->m_dwLimitNum, wBoughtNum, iRet);
        return ERR_GOODS_LIMIT_BUY_NUM;
    }
    uint8_t dwDiscount = _GetGoodsDiscount(poResMallProp);
    uint16_t wBuyNum = rstReq.m_wNum;
    uint32_t dwConsumeNum = poResMallProp->m_dwMoney * dwDiscount * wBuyNum / GOODS_NO_DISCOUNT;
    if ( !Consume::Instance().IsEnough(pstData, poResMallProp->m_bMoneyType, dwConsumeNum) )
    {
        LOGERR("Uin<%lu> Mall Goods<%u>, no enough money<%u>", pstData->m_ullUin, dwGoodsId, dwConsumeNum);
        return ERR_GOODS_NOT_ENOUGHT_MONEY;
    }

    //可以购买,处理
    //扣除消耗
    _AddBuyNum(poGoodsBuyInfo, wBuyNum, rstMallInfo, poResMallProp);
    rstRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    Item::Instance().ConsumeItem(pstData, poResMallProp->m_bMoneyType, 0, -dwConsumeNum, rstRsp.m_stSyncItemInfo, DWLOG::METHOD_MALL_BUY);
   //给物品
    Item::Instance().RewardItem(pstData, ITEM_TYPE_PROPS, poResMallProp->m_dwPropId, wBuyNum, rstRsp.m_stSyncItemInfo, DWLOG::METHOD_MALL_BUY);

    //修改任务
    Task::Instance().ModifyData(pstData, TASK_VALUE_TYPE_BUY, 1, 3/*para1*/);

    //记日志
    DT_ITEM_SHOW stTempItem = {0};
    stTempItem.m_bItemType = ITEM_TYPE_PROPS;
    stTempItem.m_dwItemId = poResMallProp->m_dwPropId;
    stTempItem.m_dwItemNum = wBuyNum;
    stTempItem.m_bPriceType = poResMallProp->m_bMoneyType;
    stTempItem.m_dwPriceValue = dwConsumeNum;
    ZoneLog::Instance().WriteItemPurchaseLog(pstData, stTempItem, DWLOG::METHOD_MALL_BUY);
    return ERR_NONE;
}

// 赠送好友逻辑,需检查特有的条件
int Mall::GiveFriendGift(PlayerData* pstData, CS_PKG_MALL_BUY_REQ& rstReq, SC_PKG_MALL_BUY_RSP& rstRsp)
{
    if (!Friend::Instance().IsFriend(pstData, rstReq.m_ullFriendUin))
    {
        LOGERR("Uin<%lu>  not friend with <%lu>", pstData->m_ullUin, rstReq.m_ullFriendUin);
        return ERR_FRIEND_NOT_FRIEND;
    }
    uint32_t dwGoodsId = rstReq.m_dwGoodsId;
    RESMALLPROPS* poResMallProp = CGameDataMgr::Instance().GetResMallPropsMgr().Find(dwGoodsId);
    if (NULL == poResMallProp)
    {
        LOGERR("Uin<%lu> the ResMallProps id<%u> is NULL", pstData->m_ullUin, dwGoodsId);
        return ERR_SYS;
    }
    RESPRIMAIL* poResPriMail = CGameDataMgr::Instance().GetResPriMailMgr().Find(10001);
    if (NULL == poResPriMail)
    {
        LOGERR("Uin<%lu> ResPriMail <10001> is null",pstData->m_ullUin);
        return ERR_SYS;
    }
    /**
     *  购买类型
     *  1.每天物品限量购买几次;
     *  2.物品永久购买次数
     *  3.
     **/
    if (!_IsGoodsUp(dwGoodsId))
    {
        LOGERR("Uin<%lu> Mall Goods<%u> is taken down", pstData->m_ullUin, dwGoodsId);
        return ERR_GOODS_TAKEN_DOWN;
    }
    if (!_CheckLvLimit(pstData, poResMallProp))
    {
        LOGERR("Uin<%lu> Mall Goods<%u> lv<%hu> LimitS<%hu> limitE<%hu>", pstData->m_ullUin, dwGoodsId,
            pstData->GetMajestyInfo().m_wLevel, poResMallProp->m_dwLevelLimitStart, poResMallProp->m_dwLevelLimitEnd);
        return ERR_LEVEL_LIMIT;
    }

    int iRet = _IsBuyTime(poResMallProp->m_dwLimitTimeId);
    if (0 != iRet)
    {
        LOGERR("Uin<%lu> Mall Goods<%u> is limit buytime<%u>, iRet<%d>", pstData->m_ullUin, dwGoodsId, poResMallProp->m_dwLimitTimeId, iRet);
        return ERR_GOODS_LIMIT_BUY_TIME;
    }
    DT_MALL_INFO& rstMallInfo = pstData->GetMiscInfo().m_stMallInfo;
    DT_MALL_GOODS_BUY_INFO* poGoodsBuyInfo = _FindGoodsBuyInfo(poResMallProp, rstMallInfo);
    iRet = _IsBuyNumLimit(poGoodsBuyInfo, rstMallInfo, poResMallProp);
    if (0 != iRet )
    {
        uint16_t wBoughtNum = 0;    //物品已购买次数
        if (NULL != poGoodsBuyInfo)
        {
            wBoughtNum = poGoodsBuyInfo->m_wBuyNum;
        }
        LOGERR("Uin<%lu> Mall Goods<%u> buy num limit Num<%u>,BoughtyNum<%u>, iRet<%d>", pstData->m_ullUin, dwGoodsId, poResMallProp->m_dwLimitNum, wBoughtNum, iRet);
        return ERR_GOODS_LIMIT_BUY_NUM;
    }
    uint8_t dwDiscount = _GetGoodsDiscount(poResMallProp);
    uint16_t wBuyNum = rstReq.m_wNum;
    uint32_t dwConsumeNum = poResMallProp->m_dwMoney * dwDiscount * wBuyNum / GOODS_NO_DISCOUNT;
    if ( !Consume::Instance().IsEnough(pstData, poResMallProp->m_bMoneyType, dwConsumeNum) )
    {
        LOGERR("Uin<%lu> Mall Goods<%u>, no enough money<%u>", pstData->m_ullUin, dwGoodsId, dwConsumeNum);
        return ERR_GOODS_NOT_ENOUGHT_MONEY;
    }
    if (1 != poResMallProp->m_bCanGiving)
    {//无法赠送
        LOGERR("Uin<%lu> Mall prop <%u> can't be given", pstData->m_ullUin, dwGoodsId);
        return ERR_SYS;
    }
    //可以购买,处理
    //扣除消耗
    _AddBuyNum(poGoodsBuyInfo, wBuyNum, rstMallInfo, poResMallProp);
    rstRsp.m_stSyncItemInfo.m_bSyncItemCount = 0;

    Item::Instance().ConsumeItem(pstData, poResMallProp->m_bMoneyType, 0, -dwConsumeNum, rstRsp.m_stSyncItemInfo, DWLOG::METHOD_MALL_BUY_SEND_GIFT);
    HandleMsgSendMail(pstData, rstReq.m_ullFriendUin, wBuyNum, poResMallProp->m_dwPropId, poResPriMail);

    //记日志
    DT_ITEM_SHOW stTempItem = {0};
    stTempItem.m_bItemType = ITEM_TYPE_PROPS;
    stTempItem.m_dwItemId = poResMallProp->m_dwPropId;
    stTempItem.m_dwItemNum = wBuyNum;
    stTempItem.m_bPriceType = poResMallProp->m_bMoneyType;
    stTempItem.m_dwPriceValue = dwConsumeNum;
    ZoneLog::Instance().WriteItemPurchaseLog(pstData, stTempItem, DWLOG::METHOD_MALL_BUY_SEND_GIFT);

    return ERR_NONE;
}

int Mall::HandleMsgSendMail(PlayerData* pstData, uint64_t FriendUin, uint16_t wNum, uint32_t dwPropId, RESPRIMAIL* poResPriMail)
{
    //rstGmReq.m_stHandle.m_stSendMail.;
    bzero(&m_stSsPkg, sizeof(m_stSsPkg));
    m_stSsPkg.m_stHead.m_wMsgId = SS_MSG_MAIL_ADD_REQ;
    m_stSsPkg.m_stBody.m_stMailAddReq.m_nUinCount = 1;
    m_stSsPkg.m_stBody.m_stMailAddReq.m_UinList[0] = FriendUin;

    DT_MAIL_DATA& rstMailData = m_stSsPkg.m_stBody.m_stMailAddReq.m_stMailData;
    rstMailData.m_bType = MAIL_TYPE_PRIVATE_SYSTEM;
    rstMailData.m_bState = MAIL_STATE_OPENED;
    StrCpy(rstMailData.m_szTitle, poResPriMail->m_szTitle, MAX_MAIL_TITLE_LEN);
    snprintf(rstMailData.m_szContent, MAX_MAIL_CONTENT_LEN, poResPriMail->m_szContent, pstData->GetRoleBaseInfo().m_szRoleName);
    rstMailData.m_ullFromUin = 0;
    rstMailData.m_bAttachmentCount = 1;
    rstMailData.m_astAttachmentList[0].m_bItemType = ITEM_TYPE_PROPS;
    rstMailData.m_astAttachmentList[0].m_dwItemId = dwPropId;
    rstMailData.m_astAttachmentList[0].m_iValueChg = wNum;
    rstMailData.m_ullTimeStampMs = CGameTime::Instance().GetCurrSecond();

    ZoneSvrMsgLayer::Instance().SendToMailSvr(m_stSsPkg);
    return ERR_NONE;
}


bool Mall::_CheckLvLimit(PlayerData* pstData, RESMALLPROPS* poResMallProp)
{
    uint16_t wLv = pstData->GetMajestyInfo().m_wLevel;
    return wLv >= poResMallProp->m_dwLevelLimitStart && wLv <= poResMallProp->m_dwLevelLimitEnd;
}
