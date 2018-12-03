#pragma once

#include "singleton.h"
#include "CommBusLayer.h"
#include "ss_proto.h"
#include "MsgBase_c.h"
#include "MsgLayerBase_c.h"
#include "../cfg/CloneBattleSvrCfgDesc.h"


class CloneBattleSvrMsgLayer : public CMsgLayerBase_c, public TSingleton<CloneBattleSvrMsgLayer>
{
public:
    CloneBattleSvrMsgLayer();
	~CloneBattleSvrMsgLayer(){}

	bool Init();

	/*
		返回处理的包数量
		<0: error
	*/
	int DealPkg();

	IMsgBase_c* GetMsgHandler( int iMsgID );
	int SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg);
    int SendToMiscSvr(PKGMETA::SSPKG& rstSsPkg);
    PKGMETA::SSPKG& GetSsPkg() { return m_stSsPkg; }
    PKGMETA::SSPKG& GetSsPkgData() { return m_stSsPkgData; }
private:
	void _RegistServerMsg();


private:
	CLONEBATTLESVRCFG*	m_pstConfig;
    PKGMETA::SSPKG m_stSsPkg;
    PKGMETA::SSPKG m_stSsPkgData; //专为数据操作使用
};

