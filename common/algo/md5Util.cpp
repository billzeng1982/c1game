#include "md5Util.h"
#include <string.h>

bool CMD5Util::CmpMD5(void *pvInput, uint32_t dwInputLen, INOUT unsigned char szMd5[MD5_DIGEST_SIZE])
{
	unsigned char szCalcMd5[MD5_DIGEST_SIZE];

    CMD5Util::CalcMD5( pvInput, dwInputLen, szCalcMd5 );

    if( 0 == memcmp( szCalcMd5, szMd5, MD5_DIGEST_SIZE ) )
    {
        return false;
    }else
    {
        memcpy( szMd5, szCalcMd5, MD5_DIGEST_SIZE );
        return true;
    }
}


void CMD5Util::CalcMD5(void *pvInput, uint32_t dwInputLen, unsigned char szResult[MD5_DIGEST_SIZE])
{
	MD5_CTX md5c; 
	MD5Init(&md5c);
	MD5Update( &md5c, (unsigned char *)pvInput, dwInputLen );
	MD5Final( szResult,&md5c );
}
            
