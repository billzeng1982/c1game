#pragma once

#include "cs_proto.h"
#include "ss_proto.h"
#include "MyTdrBuf.h"
#include <map>

using namespace PKGMETA;

class IMsgBase_c
{
public:
	IMsgBase_c() {}
	virtual ~IMsgBase_c(){}

	/*
		return - 0: succ <0: error
	*/
	virtual int HandleServerMsg(SSPKG* pstSsPkg) { return 0; }

	virtual int HandleServerMsgRaw(MyTdrBuf* pstPkgBuf) { return 0; }
};

typedef std::map< int /*msg id*/, IMsgBase_c* > MsgHandlerMap_c_t;

#define REGISTER_MSG_HANDLER_c( MsgID, ClassName, MsgHandlerMap_c ) \
	do { \
		IMsgBase_c* poMsg = new ClassName(); \
		MsgHandlerMap_c.insert( MsgHandlerMap_c_t::value_type( MsgID, poMsg )); \
	}while(0)

