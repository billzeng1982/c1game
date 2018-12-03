#include <stdio.h>
#include "DynArray.h"


int main( )
{
    CDynArray<int> oDynArray(1);

    for( int i = 0; i < 5; i++ )  
    {
        oDynArray.PushBack( i + 1 );
    }

    oDynArray.PopBack();
    
    for( uint32_t ui = 0; ui < oDynArray.Length(); ui++ )
    {
        printf("%d ", oDynArray[ui]);
    }

    printf("\n");

    return 0;
};

