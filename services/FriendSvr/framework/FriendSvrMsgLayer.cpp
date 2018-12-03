#include "FriendSvrMsgLayer.h"
#include "hash_func.cpp"
#include "LogMacros.h"
#include "ObjectPool.h"
#include "../FriendSvr.h"
#include "../logic/MsgLogicFriend.h"

using namespace PKGMETA;

FriendSvrMsgLayer::FriendSvrMsgLayer() : CMsgLayerBase(sizeof(PKGMETA::SCPKG) + 256, sizeof(PKGMETA::SSPKG) + 256)
{
    m_pstConfig = NULL;
}

bool FriendSvrMsgLayer::Init()
{
    FRIENDSVRCFG& rstDBSvrCfg =  FriendSvr::Instance().GetConfig();
    if( _Init(rstDBSvrCfg.m_iBusGCIMKey, rstDBSvrCfg.m_iProcID) < 0 )
    {
        return false;
    }

    m_pstConfig = &rstDBSvrCfg;
    
    return true;
}

void FriendSvrMsgLayer::_RegistServerMsg()
{
    REGISTER_MSG_HANDLER( SS_MSG_FRIEND_HANDLE_REQ,         CSsFriendHandleReq,    m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_FRIEND_GET_LIST_REQ,       CSsFriendGetListReq,    m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_FRIEND_SEARCH_REQ,         CSsFriendSearchReq,    m_oSsMsgHandlerMap );
    REGISTER_MSG_HANDLER( SS_MSG_FRIEND_EVENT_NTF,          CSsFriendEventNtf,    m_oSsMsgHandlerMap );
	REGISTER_MSG_HANDLER( SS_MSG_FRIEND_SEND_AP_REQ,        CSsFriendSendApReq,    m_oSsMsgHandlerMap );
	REGISTER_MSG_HANDLER( SS_MSG_FRIEND_GET_AP_REQ,         CSsFriendGetApReq,     m_oSsMsgHandlerMap );
	REGISTER_MSG_HANDLER( SS_MSG_FRIEND_CHANGE_NAME_REQ,    CSsFriendChangeNameReq,     m_oSsMsgHandlerMap );
}

int FriendSvrMsgLayer::DealPkg()
{ 
    int iRecvBytes = 0;
    int iDealPkgNum = 0;
    int i = 0;
    
    for (; i<DEAL_PKG_PER_LOOP; i++ )
    {
        iRecvBytes = m_oCommBusLayer.Recv();
        if( iRecvBytes < 0 )
        {
            LOGERR_r("bus recv error!");
            return -1;
        }
        if( 0 == iRecvBytes )
        {
            break;
        }

        this->_DealSvrPkg();       
        iDealPkgNum++;
    }

    return iDealPkgNum;
}

IMsgBase* FriendSvrMsgLayer::GetMsgHandler( int iMsgID )
{
    MsgHandlerMap_t::iterator it = m_oSsMsgHandlerMap.find( iMsgID );
    return ( it == m_oSsMsgHandlerMap.end()) ? NULL : it->second;
}

int FriendSvrMsgLayer::SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg)
{
    return SendToServer(_GetZoneSvrProcId(), rstSsPkg);
}

