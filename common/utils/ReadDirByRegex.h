#ifndef QQSG_READ_DIR_BY_REGEX_H
#define QQSG_READ_DIR_BY_REGEX_H

// 通过正则表达式匹配并返回某个目录下的文件的文件名，注意获取到的文件名不含路径信息
// Regex - regular expression
// 以下三个函数在使用时需要顺次调用，缺一不可!

bool ReadDirByRegexInit( const char* pszDir, const char* pszFileRegex );

char* GetNextFileNameByRegex( );

void ReadDirByRegexEnd( );

#endif

