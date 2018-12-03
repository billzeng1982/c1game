#ifndef _HASH_FUNC_H_2011_02_02
#define _HASH_FUNC_H_2011_02_02

#include<string.h> 
#include "define.h"
#include "functors.h"

/**
  * @pKey: pointer to the key
  * @uiKeyLen: length of the key
  * @uiSize: size of the hash table
  * ∑µªÿslot
  */
uint32_t zend_inline_hash_func(const void* pKey, uint32_t uiKeyLen, uint32_t uiSize );

// ∑µªÿhash÷µ
uint32_t zend_inline_hash_func(const void* pKey, uint32_t uiKeyLen );

bool HashCmp_uint64( void* pvKey1, void* pvKey2 );

bool HashCmp_int( void* pvKey1, void* pvKey2 );

bool HashCmp_string( void* pvKey1, void* pvKey2 );

#endif

