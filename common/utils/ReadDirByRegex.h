#ifndef QQSG_READ_DIR_BY_REGEX_H
#define QQSG_READ_DIR_BY_REGEX_H

// ͨ��������ʽƥ�䲢����ĳ��Ŀ¼�µ��ļ����ļ�����ע���ȡ�����ļ�������·����Ϣ
// Regex - regular expression
// ��������������ʹ��ʱ��Ҫ˳�ε��ã�ȱһ����!

bool ReadDirByRegexInit( const char* pszDir, const char* pszFileRegex );

char* GetNextFileNameByRegex( );

void ReadDirByRegexEnd( );

#endif

