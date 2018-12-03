#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "list_sort.h"

typedef struct ST_IntList
{
    int i;
    struct list_head link;
}ST_IntList;

void PrintList( struct list_head* head )
{
    ST_IntList* tpos=NULL;
    list_for_each_entry(tpos, head, link)
    {
        printf("%d ", tpos->i );
    }
    printf("\n");
}

static int TestCompareInt( const struct list_head* node1, const struct list_head* node2 )
{
    ST_IntList* entry1 = list_entry(node1, ST_IntList, link);
    ST_IntList* entry2 = list_entry(node2, ST_IntList, link);

    return ( entry1->i - entry2->i );
}

/**
  * argv[1]: 1 - 初始整数串正序(从小到大), 2 - 初始整数串逆序, 3 - 随机
  * argv[2]: 节点个数
  */
int main( int argc, char** argv )
{
    assert(argc == 3 );
    int iType = atoi(argv[1]);
    int iNum = atoi(argv[2]);

    assert( iType > 0 && iNum >= 0 );

    struct list_head head;
    INIT_LIST_HEAD( &head );

    ST_IntList* pastIntList = new ST_IntList[iNum];
    assert(pastIntList);
    bzero( pastIntList, sizeof(ST_IntList)*iNum );

    if( 1 == iType )
    {
        for( int i = 0; i < iNum; i++ )
        {
            pastIntList[i].i = i+1;
            list_add_tail( &(pastIntList[i].link), &head );
        }
    }
    else if( 2 == iType )
    {
        for( int i = 0; i < iNum; i++ )
        {
            pastIntList[i].i = i+1;
            list_add( &(pastIntList[i].link), &head );
        }
    }
    else if( 3 == iType )
    {
        unsigned long CurrTime=time(NULL);
        int iRandLen=iNum*5;
        srand( CurrTime );
        for( int i=0; i < iNum; i++)
        {
            pastIntList[i].i = rand()%iRandLen;
            list_add_tail( &(pastIntList[i].link), &head );
        }
    }
    else
    {
        printf("error input parameters!\n");
        return 0;
    }

    printf("before sort:\n");
    PrintList( &head );

    list_insert_sort( &head, TestCompareInt );

    printf("after sort:\n");
    PrintList( &head );

    delete [] pastIntList;
}

