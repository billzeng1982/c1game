#pragma once

/*
 注意单线程进程使用
*/

#include "cs_proto.h"
#include "ss_proto.h"
#include "MyTdrBuf.h"
#include <map>
using namespace PKGMETA;


class IMsgBase
{
public:
	IMsgBase() {}
	virtual ~IMsgBase(){}

	/*
		return - 0: succ <0: error
	*/
	virtual int HandleClientMsg( const PKGMETA::CONNSESSION* pstSession, PKGMETA::CSPKG& rstCsPkg, void* pvPara = NULL  ) { return 0; }

	/*
		return - 0: succ <0: error
	*/
	virtual int HandleServerMsg( PKGMETA::SSPKG& rstSsPkg, void* pvPara = NULL ) { return 0; }

	virtual int HandleServerMsgRaw( MyTdrBuf* pstPkgBuf, void* pvPara = NULL ) { return 0; }

protected:
	// 注意加 static 实现内存复用 !!
	static SCPKG m_stScPkg; 
	static SSPKG m_stSsPkg;
};

typedef std::map< int /*msg id*/, IMsgBase* > MsgHandlerMap_t;

#define REGISTER_MSG_HANDLER( MsgID, ClassName, MsgHandlerMap ) \
	do { \
		IMsgBase* poMsg = new ClassName(); \
		MsgHandlerMap.insert( MsgHandlerMap_t::value_type( MsgID, poMsg )); \
	}while(0)

