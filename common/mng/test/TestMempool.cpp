#include "../mempool.h"
#include <stdio.h>

int main()
{
    CMemPool<int> pool;

    pool.CreatePool(10);

    for( int i = 0; i < 10; i++ )
    {
        int* pi = pool.NewData( );
        *pi = i+1;
    }

    // ·Ç×¢²áÄ£Ê½
    CMemPool<int>::UsedIterator iter;
    iter.SetMempool( &pool );

    for( iter.Begin(); !iter.IsEnd(); iter.Next() )
    {
        int* pi = iter.CurrItem();

        printf("item: %d\n", *pi);
    }

    printf("=============================\n");

    // ²âÊÔ±ßµü´ú±ßÉ¾³ý
    pool.RegisterSlicedIter(&iter);
    int j = 0;
    for( iter.Begin(); !iter.IsEnd(); iter.Next() )
    {
        int* pi = iter.CurrItem();

        j++;
        if( j%2 )
        {
            pool.DeleteData(pi);
            continue;
        }

        printf("item: %d\n", *pi);
    }

    for( iter.Begin(); !iter.IsEnd(); iter.Next() )
    {
         int* pi = iter.CurrItem();
         pool.DeleteData(pi);
    }

    printf("used num: %d\n", pool.GetUsedNum());
    
    return 0;
}

