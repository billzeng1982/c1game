#include <stdio.h>
#include "IndexedPriorityQ.h"

int main()
{
    IndexedPriorityQ<int, int> oPrioQ(10);

    for( int i = 0; i < 10; i++ )
    {
        //oPrioQ.Push( i + 1 );
        oPrioQ.Push( 10 - i );
    }

    for( int i = 0; i < 10; i++ )
    {
        int iPos = oPrioQ.Find(i+1);
        printf("key: %d, pos: %d, pos-val: %d \n", i+1,  iPos, oPrioQ[iPos] );
    }
    
    printf("queue contentes:\n");
    while( !oPrioQ.Empty() )
    {
        printf("%d, pos: %d\n ", oPrioQ.Top(), oPrioQ.Find( oPrioQ.Top() ));
        
        oPrioQ.Pop();
    }
    printf("\n");

    return 0;
}

