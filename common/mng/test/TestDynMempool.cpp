#include "../DynMempool.h"

int main()
{
    DynMempool<int> pool;

    pool.Init( 10, 10 );

    int* pa[20];

    for( int i = 0; i < 15; i++ )
    {
        pa[i] = pool.Get();
        *pa[i] = i+1;
    }

    printf("capacity: %d, free: %d\n", pool.GetCapacity(), pool.GetFreeNum());

    for( int i = 0; i < 15; i++ )
    {
        pool.Release( pa[i] );
    }

    printf("capacity: %d, free: %d\n", pool.GetCapacity(), pool.GetFreeNum());

    // test maxcapactiy
    DynMempool<int> pool2;
    pool2.Init( 1, 1, 2 );
    int* pi;
    for( int i = 0; i < 3; i++ )
    {
        pi = pool2.Get();
        if( !pi )
        {
            printf("Can not get free node any more!, i = %d\n" , i);
        }
    }

    return 0;
}

