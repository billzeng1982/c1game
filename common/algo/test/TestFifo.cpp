#include "fifo.h"
#include <stdio.h>


int main()
{
    CFifo<int> oIntFifo( 4 );

    int a = 1;
    oIntFifo.FifoIn( &a, 1 );
    a++;
    oIntFifo.FifoIn( &a, 1 );
    a++;
    oIntFifo.FifoIn( &a, 1 );
    a++;
    oIntFifo.FifoIn( &a, 1 );
   
    printf("Over size, FifoIn Result: %d\n", oIntFifo.FifoIn( &a, 1 ) );

    int arrOut[4];
    int iOutLen;

    iOutLen = oIntFifo.FifoOut( arrOut, 3 );
    for( int i = 0; i < iOutLen; i++ )
    {
        printf("%d: %d\n", i, arrOut[i]);
    }

    printf("unused: %d\n", oIntFifo.FifoUnused() );

    a++;
    oIntFifo.FifoIn( &a, 1 );
    a++;
    oIntFifo.FifoIn( &a, 1 );

    iOutLen = oIntFifo.FifoOut( arrOut, 3 );
    for( int i = 0; i < iOutLen; i++ )
    {
        printf("%d: %d\n", i, arrOut[i]);
    }

    printf("unused: %d\n", oIntFifo.FifoUnused() );

    int *p = arrOut;
    printf("p : %lx\n", (uint64_t)(p));

    p = p + 2;
    printf("p : %lx\n", (uint64_t)(p));

    return 0;
}


