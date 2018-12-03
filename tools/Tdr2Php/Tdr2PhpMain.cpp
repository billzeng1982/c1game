/*
    把tdr描述的数据类型转化为php数据类型
    支持 unpack 和pack 操作
    php不支持enum
    
    注意:
    1. php需要utf8编码格式, xml协议是ansi的，所以需要先单独把xml copy出来装换成utf8格式
    2. 不支持tdr include操作!
    3. unpack传入的参数是 json_decode($json), 不能直接传入json字符串!
    
    用法:
    ./Tdr2Php --xml oss_proto.xml
*/

#include <getopt.h>
#include <unistd.h>
#include "Tdr2Php.h"


static void PrintHelp( char* proc )
{
    printf("Usage:\n");
    printf("\t--xml input xml file\n");
    printf("\tExample: ./Tdr2Php --xml oss_proto.xml\n");
    return;
}


int main( int argc, char** argv )
{
    if( argc < 3 )
    {
        PrintHelp( argv[0] );
        return 0;
    }

    struct option long_options[] = 
    {  
        { "xml", required_argument, 0, 1},
        { 0, 0, 0, 0},  
    };

    int iOpt = 0;
    char* pszXmlFile = NULL;
    while( ( iOpt = getopt_long( argc, argv, "", long_options, NULL ) ) != -1 )
    {
        switch( iOpt )
        {
            case 1:
            {
                pszXmlFile = optarg; 
                break;
            }
            default:
                break;
        }
    }

    if( NULL == pszXmlFile || pszXmlFile[0] == '\0' )
    {
        PrintHelp(argv[0]);
        return 0;
    }

    CTdr2Php oTdr2PhpTrans;
    if( !oTdr2PhpTrans.Translate( pszXmlFile ) )
    {
        fprintf( stderr, "translate failed!\n" );
    }else
    {
        fprintf( stderr, "translate succeed!\n" ); 
    }
    return 0;
}

