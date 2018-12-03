#include "strutil.h"

bool StrUtil::IsStringNull( const char* str )
{
    if( NULL == str || '\0' == str[0] )
    {
        return true;
    }

    return false;
}

