#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "HashCache.h"

#define SHM_KEY 601430

enum TEST_HASH_CACHE_OP
{
    TEST_HASH_CACHE_OP_ADD = 1,
    TEST_HASH_CACHE_OP_DEL = 2,
    TEST_HASH_CACHE_OP_GET = 3,
};

void ShowHelp( char* name )
{
    printf("%s MaxNum OpCode HashKey [Value]\n", name);
    printf("\t HashKey is a string (<32), Value is an integer\n");
    printf("\t Opcode:\n");
    printf("\t 1 - add\n");
    printf("\t 2 - delete\n");
    printf("\t 3 - get value\n");
}


int main( int argc, char** argv )
{
    int iOpt = 0;

    while((iOpt = getopt(argc, argv, "h")) != -1)
    {
        switch(iOpt)
        {
        case 'h':
            ShowHelp(argv[0]);
            return 0;
        default:
            break;
        }
    }

    if( argc < 3 )
    {
        ShowHelp( argv[0] );
        return 0;
    }

    int iMax=atoi(argv[1]);
    int iOpCode = atoi(argv[2]);
    char szHashKey[32]={0};
    int iRet = 0;
    int iValue = 0;
    int* pi = NULL;

    CHashCache< char[32], int, SHashFcnStr, SEqualStr > oCache;

    iRet = oCache.CreateCache_shm( (key_t)SHM_KEY, iMax, iMax, ( EHASH_CACHE_SWAP_ON | EHASH_CACHE_LRU_READ_SENS ) );
    if( iRet < 0 )
    {
        printf("iRet=%d, %s\n", iRet, oCache.GetErrMsg() );
        return iRet;
    }
    
    switch( iOpCode )
    {
        case TEST_HASH_CACHE_OP_ADD:
        {
            assert( argc == 5 );
            strncpy( szHashKey, argv[3], sizeof(szHashKey)-1 );
            iValue = atoi(argv[4]);

            pi = oCache.AddData( szHashKey, iValue );
            printf("iRet=%d\n", *pi);
        }
        break;

        case TEST_HASH_CACHE_OP_DEL:
        {
            assert( argc == 4 );
            strncpy( szHashKey, argv[3], sizeof(szHashKey)-1 );

            iRet = oCache.DelDataByKey(szHashKey);
            printf("iRet=%d\n", iRet);
        }
        break;

        case TEST_HASH_CACHE_OP_GET:
        {
            assert( argc == 4 );
            strncpy( szHashKey, argv[3], sizeof(szHashKey)-1 );

            int iVal=0;
            iRet = oCache.GetDataCopy( szHashKey, iVal);
            if( 0 == iRet )
            {
                printf("Get iVal=%d\n", iVal );
            }else
            {
                printf("Get error!\n");
                printf("iRet=%d, %s\n", iRet, oCache.GetErrMsg() );
            }
        }
        break;

        default:
        {
            ShowHelp(argv[0]);
        }
    }

    return iRet;
}

