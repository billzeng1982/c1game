#include "BitMap.h"
#include <stdio.h>

int main()
{
    CBitMap<2> o2Bitmap( 5 );

    o2Bitmap.SetVal( 0, 1 );
    o2Bitmap.SetVal( 19, 1 );
    o2Bitmap.SetVal( 6, 0 );
    o2Bitmap.SetVal( 11, 1 );

    printf("BitPos 0: %d\n", o2Bitmap.GetVal(0));
    printf("BitPos 19: %d\n", o2Bitmap.GetVal(19));
    printf("BitPos 6: %d\n", o2Bitmap.GetVal(6));
    printf("BitPos 11: %d\n", o2Bitmap.GetVal(11));
    printf("BitPos 1: %d\n", o2Bitmap.GetVal(1));
}

