#pragma once

#include "MsgBase.h"

class ClusterServStatNtf_SS : public IMsgBase
{
public:
    virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};

