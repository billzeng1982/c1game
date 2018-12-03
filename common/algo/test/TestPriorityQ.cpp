#include <stdio.h>
#include "DynArray.h"
#include "PriorityQueue.h"
#include "functors.h"

int main()
{
    PriorityQueue< int, SDftGreater<int> > oMaxPQ(10);
    PriorityQueue< int, SDftLess<int> >  oMinPQ(10);

    for( int i = 0; i < 20; i++ )
    {
        oMaxPQ.Push( i+1 );
        oMinPQ.Push( 20 - i );
    }

    // update
    for( int i = 0; i < 20; i++ )
    {
        oMinPQ[ i ] = 100 - i;
        oMinPQ.Update(i);
        
        oMaxPQ[ i ] = i;
        oMaxPQ.Update(i);
    }

    printf("max priority queue:\n");
    while( !oMaxPQ.Empty() )
    {
        printf("%d ", oMaxPQ.Top() );
        oMaxPQ.Pop();
    }
    printf("\n");

    printf("min priority queue:\n");
    while( !oMinPQ.Empty() )
    {
        printf("%d ", oMinPQ.Top() );
        oMinPQ.Pop();
    }
    printf("\n");

    return 0;
}

