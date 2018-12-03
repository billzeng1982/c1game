#pragma once

#include "MsgBase.h"
#include "ss_proto.h"

class ZoneSvrStatNtf_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};


