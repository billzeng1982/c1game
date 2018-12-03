#pragma once

#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase.h"

class CMsgLayerBase
{
public:
	CMsgLayerBase(size_t uScSize, size_t uSsSize); 
	virtual ~CMsgLayerBase(){}	

	IMsgBase* GetClientMsgHandler(int iMsgId);
	IMsgBase* GetServerMsgHandler(int iMsgId);
	//IMsgBase* GetOssMsgHandler(int iMsgId);

	int SendToServer(int iAddr, PKGMETA::SSPKG& rstSsPkg);
	int SendToServer(int iAddr, MyTdrBuf* pstTdrBuf);
	
	virtual int DealPkg() = 0;
	virtual bool Init() = 0;

	PKGMETA::SSPKG* UnpackSsPkg( MyTdrBuf* pstPkgBuf );
	MyTdrBuf* GetSsSendBuf() { return &m_stSsSendBuf; }

	bool RefreshBusChannel() { return m_oCommBusLayer.RefreshHandle(); }
	CCommBusLayer& GetCommBusLayer(){ return m_oCommBusLayer; }
	void DealSvrPkg(PKGMETA::SSPKG& rstSsPkg);
	
protected:
	virtual void _RegistClientMsg( ){}
	virtual void _RegistServerMsg( ){}
	//virtual void _RegistOssMsg( ){}

	bool _Init(int iBusCGIMKey, int iProcID );
	void _DealSvrPkg();    //Used to Deal Msg

	// 返回发送的字节数, <0: error
	int _SendToClient( int iConnAddr, const PKGMETA::CONNSESSION* pstConnSess, PKGMETA::SCPKG* pstScPkg, 
					   char cSessCmd=PKGMETA::CONNSESSION_CMD_INPROC, uint16_t wCutVersion = 0);
	
protected:
	MsgHandlerMap_t m_oCsMsgHandlerMap; 
	MsgHandlerMap_t	m_oSsMsgHandlerMap;
	//MsgHandlerMap_t	m_oOssMsgHandlerMap;
	
    CCommBusLayer	m_oCommBusLayer;
			
	PKGMETA::CSPKG	m_stCsRecvPkg; 
	MyTdrBuf		m_stScSendBuf; 
			
	PKGMETA::SSPKG	m_stSsRecvPkg;
	MyTdrBuf		m_stSsSendBuf; 

	int m_iProcId;
	int	m_iBusGCIMKey;
};

