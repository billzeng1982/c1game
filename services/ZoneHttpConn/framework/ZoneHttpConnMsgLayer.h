#pragma once

#ifndef ZONE_HTTP_CONN_MSG_LAYER_H_
#define ZONE_HTTP_CONN_MSG_LAYER_H_

#include "utils/singleton.h"
#include "msglayer/MsgBase.h"
#include "msglayer/MsgLayerBase.h"
#include "msglayer/CommBusLayer.h"
#include "protocol/PKGMETA/ss_proto.h"
#include "cfg/ZoneHttpConnCfgDesc.h"
#include "http/HttpControler.h"

class ZoneHttpConnMsgLayer : public CMsgLayerBase, public TSingleton<ZoneHttpConnMsgLayer>
{
public:
    ZoneHttpConnMsgLayer();
    ~ZoneHttpConnMsgLayer();

    bool Init();

    int DealPkg();

    IMsgBase* GetMsgHandler(int iMsgID);

    int SendToZoneSvr(PKGMETA::SSPKG& rstSsPkg);

    HttpControler& GetHttpControler() { return m_oHttpControler; }

protected:
    void _RegistServerMsg();

private:
    ZONEHTTPCONNCFG m_stConfig;
    HttpControler m_oHttpControler;
};

#endif
