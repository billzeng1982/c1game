#pragma once

// 多线程版本

#include "cs_proto.h"
#include "ss_proto.h"
#include "MyTdrBuf.h"
#include <map>
using namespace PKGMETA;

class IMsgBase_r
{
public:
    IMsgBase_r() {}
    virtual ~IMsgBase_r() {}

	/*
		return - 0: succ <0: error
	*/
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL ) { return 0; }
    	virtual int HandleServerMsgRaw( MyTdrBuf* pstPkgBuf, void* pvPara = NULL ) { return 0; }
};

typedef std::map< int /*msg id*/, IMsgBase_r* > MsgHandlerMap_r_t;

#define REGISTER_MSG_HANDLER_r( MsgID, ClassName, MsgHandlerMap_r ) \
	do { \
		IMsgBase_r* poMsg = new ClassName(); \
		MsgHandlerMap_r.insert( MsgHandlerMap_r_t::value_type( MsgID, poMsg )); \
	}while(0)


