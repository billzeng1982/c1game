#include "StrSearch.h"

int main()
{
    char str[1024];
    PatternStr pattern("tangxing", 8);

    strncpy(str, "123456tangxin123tangxing", 100);
    FindStr(str, strlen(str), &pattern);

    strncpy(str, "123456tangxing", 100);
    FindStr(str, strlen(str), &pattern);

    strncpy(str, "123456tangxin1234565", 100);
    FindStr(str, strlen(str), &pattern);

    return 0;
}
