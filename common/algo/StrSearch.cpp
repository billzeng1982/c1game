#include "StrSearch.h"

int FindStr(char* text, int text_len, PatternStr* pattern_str)
{
    if (text_len < pattern_str->m_len)
    {
        return -1;
    }

    int i, j = 0;
    while(j <= text_len - pattern_str->m_len)
    {
        for(i=pattern_str->m_len-1; i>=0 && pattern_str->m_pattern[i]==text[i + j]; i--);
        if(i < 0)
        {
            printf("find str, position=%d\n", j);
            return j;
        }
        else
        {
            j += GET_MAX(pattern_str->m_bmbc[(unsigned char)text[i + j]]-pattern_str->m_len+1+i, pattern_str->m_bmgs[i]);
        }
    }

    printf("not found\n");

    return -1;
}

PatternStr::PatternStr(char* pattern, int pattern_len)
{
    m_len = pattern_len;

    m_pattern = new char[m_len+1];
    strncpy(m_pattern, pattern, m_len);
    m_pattern[m_len] = '\0';

    m_bmbc = new char[CHAR_MAX_];
    m_bmgs = new char[m_len];
    m_suff = new char[m_len];

    this->PreBmBc();

    this->PreBmGs();
}

PatternStr::~PatternStr()
{
    delete []m_pattern;
    delete []m_bmbc;
    delete []m_bmgs;
    delete []m_suff;
}

void PatternStr::PreBmBc()
{
    //计算坏字符数组
    //先全部赋值为模式串长度len
    for (int i=0; i<CHAR_MAX_; i++)
    {
        m_bmbc[i] = m_len;
    }

    //在模式串中出现的字符，取其最右位置
    for (int i=0; i<m_len -1; i++)
    {
        m_bmbc[(unsigned char)m_pattern[i]] = m_len - i -1;
    }
}

void PatternStr::PreBmGs()
{
    int i, j;

    //计算后缀数组
    this->Suffix();

    // 先全部赋值为m
    for(i = 0; i < m_len; i++)
    {
        m_bmgs[i] = m_len;
    }

    j = 0;
    for(i=m_len-1; i>=0; i--)
    {
        if(m_suff[i] == i + 1)
        {
            for(; j < m_len - 1 - i; j++)
            {
                if(m_bmgs[j] == m_len)
                    m_bmgs[j] = m_len - 1 - i;
            }
        }
    }

    for(i=0; i <m_len-1; i++)
    {
        m_bmgs[m_len - 1 - m_suff[i]] = m_len - 1 - i;
    }
}

void PatternStr::Suffix()
{
    m_suff[m_len - 1] = m_len;

    for(int i=m_len - 2; i>=0; i--)
    {
        int j = i;

        while(j >= 0 && m_pattern[j] == m_pattern[m_len - 1 - i + j])
        {
            j--;
        }

        m_suff[i] = i - j;
    }
}

