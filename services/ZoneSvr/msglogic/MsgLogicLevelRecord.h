#pragma once
#include "MsgBase.h"

class LevelRecordReadRsp_SS :  public IMsgBase
{
public:
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );
};