/*
    ��tdr��������������ת��Ϊphp��������
    ֧�� unpack ��pack ����
    php��֧��enum
    
    ע��:
    1. php��Ҫutf8�����ʽ, xmlЭ����ansi�ģ�������Ҫ�ȵ�����xml copy����װ����utf8��ʽ
    2. ��֧��tdr include����!
    3. unpack����Ĳ����� json_decode($json), ����ֱ�Ӵ���json�ַ���!
    
    �÷�:
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

