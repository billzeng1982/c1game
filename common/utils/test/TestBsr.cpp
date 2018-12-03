#include "comm_func.h"


void test_bsr_int64()
{
    uint64_t x = (uint64_t)1 << 63;
    uint64_t l = x;
    int i = 0;
    for( i = 0; i < 64; i++ )
    {
        l = x >> i;
        int bit = bsr_int64(l);
        printf("i=%d, l=%lx, bit=%d\n", i, l, bit);
    }

    l = 0;
    int bit = bsr_int64(l);
    printf("i=%d, l=%lx, bit=%d\n", i, l, bit);
}

void test_bsr_int32()
{
    uint32_t x = (uint32_t)1 << 31;
    uint32_t l = x;
    int i = 0;
    for( i = 0; i < 32; i++ )
    {
        l = x >> i;
        int bit = bsr_int64(l);
        printf("i=%d, l=%x, bit=%d\n", i, l, bit);
    }

    l = 0;
    int bit = bsr_int64(l);
    printf("i=%d, l=%x, bit=%d\n", i, l, bit);
}

int main()
{
    test_bsr_int64();
    test_bsr_int32();

    return 0;
}

