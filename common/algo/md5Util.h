#ifndef _CMD5Util_H_
#define _CMD5Util_H_
#include <string.h>
#include <stdio.h>
#include <tsec/md5.h>
#include "define.h"

class CMD5Util
{
public:
	//计算md5码
	static void CalcMD5(void* pvInput, uint32_t dwInputLen, unsigned char szMd5[MD5_DIGEST_SIZE]);

	// 比较md5码, return true -> 打包 reurn false ->不打包
	static bool CmpMD5(void* pvInput, uint32_t dwInputLen, INOUT unsigned char szResult[MD5_DIGEST_SIZE]);
};
#endif

