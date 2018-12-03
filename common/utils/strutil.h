#ifndef _STR_UTIL_H_
#define _STR_UTIL_H_

#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_CAT_LEN 262144 // 256k 

#define StrCat( str, size, format, ... ) do { \
	char szMsg[MAX_CAT_LEN]; \
	snprintf( szMsg, MAX_CAT_LEN-1, format, ##__VA_ARGS__ ) ; \
	szMsg[MAX_CAT_LEN-1] = 0; \
	int iCatLen = (size) - strnlen( (str), (size) ); \
	if( iCatLen > 0 ) \
		strncat( (str), szMsg, iCatLen ); \
}while(0)


#define StrCpy( dest, src, n ) do { \
	strncpy( (dest), (src), (n)-1 ); \
	dest[(n)-1] = 0; \
}while(0)

#ifdef __cplusplus
}
#endif

class StrUtil
{
public:
	static bool IsStringNull( const char* str );
};


#endif

