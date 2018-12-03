#pragma once
#include <stdio.h>
#include <string.h>

class PatternStr;

#define CHAR_MAX_ 256
#define GET_MAX(x, y) (x) > (y) ? (x) : (y)

int FindStr(char* text, int text_len, PatternStr* pattern_str);

class PatternStr
{
public:
    PatternStr(char* pattern, int pattern_len);
    ~PatternStr();

	//目标串长度
	int m_len;

	//坏字符数组
	char* m_bmbc;

	//好后缀数组
	char* m_bmgs;

	//要搜索的目标串
    char* m_pattern;

	//好后缀的长度
    char* m_suff;

private:
    void PreBmBc();

    void PreBmGs();

    void Suffix();
};

