#pragma once

#include "MsgBase.h"

class UploadReplayReq_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

