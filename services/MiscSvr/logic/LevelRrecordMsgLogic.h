#pragma once
#include "MsgBase_r.h"
#include "tdr/tdr.h"
#include "ss_proto.h"

class LevelRecordReadReq_SS: public IMsgBase_r
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );



private:
	PKGMETA::SSPKG 	m_stSsPkg;
};

class LevelRecordSaveReq_SS: public IMsgBase_r
{
public:
	virtual int HandleServerMsg(PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL );

private:
	PKGMETA::SSPKG 	m_stSsPkg;
};
