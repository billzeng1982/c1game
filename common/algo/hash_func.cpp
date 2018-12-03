#include <assert.h>
#include <string.h>
#include "hash_func.h"

/**
  * @pKey: pointer to the key
  * @uiKeyLen: length of the key
  * @uiSize: size of the hash table
  * ∑µªÿslot
  */
uint32_t zend_inline_hash_func(const void* pKey, uint32_t uiKeyLen, uint32_t uiSize )
{
    
    return ( zend_inline_hash_func(pKey, uiKeyLen) % uiSize );
}


// ∑µªÿhash÷µ
uint32_t zend_inline_hash_func(const void* pKey, uint32_t uiKeyLen )
{
    const char* arKey = static_cast<const char*>(pKey); 
    register uint32_t hash = 5381;

    /* variant with the hash unrolled eight times */
    for (; uiKeyLen >= 8; uiKeyLen -= 8) {
            hash = ((hash << 5) + hash) + *arKey++;
            hash = ((hash << 5) + hash) + *arKey++;
            hash = ((hash << 5) + hash) + *arKey++;
            hash = ((hash << 5) + hash) + *arKey++;
            hash = ((hash << 5) + hash) + *arKey++;
            hash = ((hash << 5) + hash) + *arKey++;
            hash = ((hash << 5) + hash) + *arKey++;
            hash = ((hash << 5) + hash) + *arKey++;
    }
    
    switch (uiKeyLen)
    {
            case 7: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
            case 6: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
            case 5: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
            case 4: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
            case 3: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
            case 2: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
            case 1: hash = ((hash << 5) + hash) + *arKey++; break;
            case 0: break;
            default: break;
    }

    return hash;
}


bool HashCmp_uint64( void* pvKey1, void* pvKey2 )
{
    assert( pvKey1 && pvKey2 );

    return ( *(uint64_t*)pvKey1 == *(uint64_t*)pvKey2 );
}


bool HashCmp_int( void* pvKey1, void* pvKey2 )
{
    assert( pvKey1 && pvKey2 );

    return ( *(int*)pvKey1 == *(int*)pvKey2 );
}


bool HashCmp_string( void* pvKey1, void* pvKey2 )
{
    assert( pvKey1 && pvKey2 );

    return ( 0 == strcmp( (char*)pvKey1, (char*)pvKey2 ) );
}

