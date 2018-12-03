#pragma once

#include "singleton.h"
#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase_c.h"
#include "MsgLayerBase_c.h"
#include "../cfg/MineSvrCfgDesc.h"


class MineSvrMsgLayer : public CMsgLayerBase_c, public TSingleton<MineSvrMsgLayer>
{
public:
    MineSvrMsgLayer();
	~MineSvrMsgLayer(){}

	bool Init();

	/*
		返回处理的包数量
		<0: error
	*/
	int DealPkg();

	IMsgBase_c* GetMsgHandler( int iMsgID );
	int SendToClusterGate(PKGMETA::SSPKG& rstSsPkg);
    int SendToMineDBSvr(PKGMETA::SSPKG& rstSsPkg);
    PKGMETA::SSPKG& GetSsPkg() { return m_stSsPkg; }
    PKGMETA::SSPKG& GetSsPkgData() { return m_stSsPkgData; }
private:
	void _RegistServerMsg();


private:
	MINESVRCFG*	m_pstConfig;
    PKGMETA::SSPKG m_stSsPkg;
    PKGMETA::SSPKG m_stSsPkgData; //专为数据操作使用
};

