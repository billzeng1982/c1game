#include "md5Util.h"
#include "common_proto.h"
#include <stdio.h>

using namespace PKGMETA;

int main()
{
    DT_ROLE_INFO stRoleInfo;
    bzero( &stRoleInfo, sizeof( stRoleInfo ) );
    unsigned char digest[ 16 ];
    char digestStr[32];

    MD5_CTX *md5Info, md5InfoBuffer;
	md5Info = &md5InfoBuffer;

    CMD5Util::CalcMD5( &stRoleInfo, sizeof(stRoleInfo), digest );
	//MD5Init( md5Info );
	//MD5Update( md5Info, (unsigned char *)&stRoleInfo, sizeof( stRoleInfo )  );
	//MD5Final( digest, md5Info );
    ::Md5HexString( (char*)digest, digestStr );
    printf( "digest: <%s> \n", digestStr );

    stRoleInfo.m_dwExp = 20;
    CMD5Util::CalcMD5( &stRoleInfo, sizeof(stRoleInfo),digest );
    //MD5Init( md5Info );
	//MD5Update( md5Info, (unsigned char *)&stRoleInfo, sizeof( stRoleInfo )  );
	//MD5Final( digest, md5Info );
    ::Md5HexString( (char*)digest, digestStr );
    printf( "digest: <%s> \n", digestStr );

    stRoleInfo.m_dwExp = 30;
    CMD5Util::CalcMD5( &stRoleInfo, sizeof(stRoleInfo),digest );
    //MD5Init( md5Info );
	//MD5Update( md5Info, (unsigned char *)&stRoleInfo, sizeof( stRoleInfo )  );
	//MD5Final( digest, md5Info );
    ::Md5HexString( (char*)digest, digestStr );
    printf( "digest: <%s> \n", digestStr );

    return 0;
}

