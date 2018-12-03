#pragma once

#include "MsgBase.h"

class SdkDMTDataSendOrderNtf : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};