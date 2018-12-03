#include "ss_proto.h"
#include <stdio.h>
#include "tdr/tdr.h"
#include "strutil.h"

using namespace PKGMETA;

#if 0
static void TestOutput( LPTDRMETALIB pstMetaLib,  LPTDRMETA pstMeta )
{
    OSSPkg stOssPkg;
    stOssPkg.m_stHead.m_wMsgID = OSS_MSG_GET_UIN_REQ;
    stOssPkg.m_stHead.m_ullUin = 0;
    stOssPkg.m_stHead.m_ullReservID = 111;
    StrCpy( stOssPkg.m_stBody.m_stGetUinReq.m_szAccountName, "billzeng", OSS_MAX_NAME_LENGTH );
    
    TDRDATA stHost;
    TDRDATA stJson;
    
    stHost.pszBuff = (char*)&stOssPkg;
    stHost.iBuff = sizeof(stOssPkg);

    char buffer[1024] = {0};
    stJson.iBuff = sizeof(buffer);
    stJson.pszBuff = buffer;

    int iRet = tdr_output_json(pstMeta, &stJson, &stHost, 0);
    assert(!TDR_ERR_IS_ERROR(iRet));
    printf("json: %s\n",  stJson.pszBuff);

    //iRet = tdr_output_json_file(pstMeta, "json.txt", &stHost, 0);
    //assert(!TDR_ERR_IS_ERROR(iRet));
}


static void TestOutputArray( LPTDRMETALIB pstMetaLib,  LPTDRMETA pstMeta ) 
{
    OSSPkg stOssPkg;
    stOssPkg.m_stHead.m_wMsgID = OSS_MSG_ARRAY_TEST;
    stOssPkg.m_stHead.m_ullUin = 0;
    stOssPkg.m_stHead.m_ullReservID = 111;  

    for( int i = 0; i < 5; i++ )
    {
        stOssPkg.m_stBody.m_stArrayTest.m_astArrTest[i].m_iAge = 10 + i;
        stOssPkg.m_stBody.m_stArrayTest.m_astArrTest[i].m_ullUin = 10001 + i;
    }
    stOssPkg.m_stBody.m_stArrayTest.m_iCount = 5;

    TDRDATA stHost;
    TDRDATA stJson;
    
    stHost.pszBuff = (char*)&stOssPkg;
    stHost.iBuff = sizeof(stOssPkg);

    char buffer[1024] = {0};
    stJson.iBuff = sizeof(buffer);
    stJson.pszBuff = buffer;

    int iRet = tdr_output_json(pstMeta, &stJson, &stHost, 0);
    assert(!TDR_ERR_IS_ERROR(iRet));
    printf("json: %s\n",  stJson.pszBuff);
}


static void TestInput( LPTDRMETALIB pstMetaLib,  LPTDRMETA pstMeta )
{
    TDRDATA stHost;

    OSSPkg stOssPkgDst;
    stHost.pszBuff = (char*)&stOssPkgDst;
    stHost.iBuff = sizeof(stOssPkgDst);
    
    int iRet = tdr_input_json_file(pstMeta, &stHost, "json.txt", 0);
    if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("load metalib failed! %s\n", tdr_error_string(iRet));
	}
}

#endif

static void TestInputChinese( LPTDRMETALIB pstMetaLib )
{
     LPTDRMETA pstMeta = tdr_get_meta_by_name(pstMetaLib, "DT_SDK_GET_ORDER");

     FILE* fp = fopen("./JsonSrc.txt", "r");
     if( !fp )
    {
        printf("open file error!\n");
        return;
    }

    char jsonStr[512];
    fgets( jsonStr, sizeof(jsonStr), fp );
    
    printf("Input: %s\n", jsonStr);
    
    DT_SDK_GET_ORDER stSDKRsp;
    TDRDATA stHost, stJson;
    stJson.pszBuff = jsonStr;
    stJson.iBuff = sizeof(jsonStr);
    stHost.pszBuff = (char*)&stSDKRsp;
    stHost.iBuff = sizeof(stSDKRsp);

    int iRet = tdr_input_json_ex( pstMeta, &stHost, &stJson, 0);
    //int iRet = tdr_input_json( pstMeta, &stHost, &stJson, 0);
    if (TDR_ERR_IS_ERROR(iRet))
    {
        printf("tdr_input_json_ex failed <%s>\n", tdr_error_string(iRet));
        return;
    }

    printf("extension: %s\n", stSDKRsp.m_stData.m_szExtension);
}

int main()
{
    LPTDRMETALIB pstMetaLib;
    int iRet = tdr_load_metalib( &pstMetaLib,  "../../deploy/protocol/ss_proto.bin" );
    if (TDR_ERR_IS_ERROR(iRet))
	{
		printf("load metalib failed! %s\n", tdr_error_string(iRet));
		return iRet;
	}

   TestInputChinese(pstMetaLib);
    
    tdr_free_lib(&pstMetaLib);
}

