#ifndef _CMD5Util_H_
#define _CMD5Util_H_
#include <string.h>
#include <stdio.h>
#include <tsec/md5.h>
#include "define.h"

class CMD5Util
{
public:
	//����md5��
	static void CalcMD5(void* pvInput, uint32_t dwInputLen, unsigned char szMd5[MD5_DIGEST_SIZE]);

	// �Ƚ�md5��, return true -> ��� reurn false ->�����
	static bool CmpMD5(void* pvInput, uint32_t dwInputLen, INOUT unsigned char szResult[MD5_DIGEST_SIZE]);
};
#endif

