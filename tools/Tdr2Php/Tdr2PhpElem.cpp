#include "Tdr2PhpElem.h"
#include "Tdr2Php.h"
#include "tdr/tdr.h"
#include "tdr/tdr_XMLtags.h" 

/*
    不支持tdr的void类型
*/
STdrPhpTypeInfo g_types_Tdr2Php[] =
{
    /* xml_tag_name,    sprefix,    mPrefix,    phptype,            php_init_val*/
    { TDR_TAG_UNION,    "st",       "ast",      PHP_TYPE_UNION,     "null" },
    { TDR_TAG_STRUCT,   "st",       "ast",      PHP_TYPE_STRUCT,    "null" },
    { TDR_TAG_TINYINT,  "ch",       "sz",       PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_TINYUINT, "b",        "sz",       PHP_TYPE_NUMERIC,   "0" },
    
    { TDR_TAG_SMALLINT, "n",        "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_SMALLUINT,"w",        "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_INT,      "i",        "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_UINT,     "dw",       "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_BIGINT,   "ll",       "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_BIGUINT,  "ull",      "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_INT8,     "ch",       "sz",       PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_UINT8,    "b",        "sz",       PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_INT16,    "n",        "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_UINT16,   "w",        "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_INT32,    "i",        "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_UINT32,   "dw",       "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_INT64,    "ll",       "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_UINT64,   "ull",      "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_FLOAT,    "f",        "",         PHP_TYPE_NUMERIC,   "0.0" },
    { TDR_TAG_DOUBLE,   "d",        "",         PHP_TYPE_NUMERIC,   "0.0" },
    { TDR_TAG_DECIMAL,  "f",        "",         PHP_TYPE_NUMERIC,   "0.0" },
    { TDR_TAG_DATE,     "t",        "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_TIME,     "t",        "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_DATETIME, "t",        "",         PHP_TYPE_NUMERIC,   "0" },

    { TDR_TAG_STRING,   "sz",       "asz",      PHP_TYPE_STRING,   "''" },
    { TDR_TAG_BYTE,     "b",        "sz",       PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_IP,       "ul",       "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_WCHAR,    "w",        "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_WSTRING,  "sz",       "asz",      PHP_TYPE_STRING,   "''" },
    { TDR_TAG_CHAR,     "ch",       "sz",       PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_UCHAR,    "b",        "sz",       PHP_TYPE_NUMERIC,   "0" },

    { TDR_TAG_SHORT,    "n",        "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_USHORT,   "w",        "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_LONG,     "l",        "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_ULONG,    "ul",       "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_LONGLONG, "ll",       "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_ULONGLONG,"ull",      "",         PHP_TYPE_NUMERIC,   "0" },
    { TDR_TAG_MONEY,    "m",        "",         PHP_TYPE_NUMERIC,   "0" },
};

int GetTdr2PhpTypesSize()
{
    return int(sizeof(g_types_Tdr2Php) / sizeof(STdrPhpTypeInfo));
}

CTdr2PhpElem::CTdr2PhpElem()
{
   this->poTdr2Php = NULL;
}

CTdr2PhpElem::CTdr2PhpElem( CTdr2Php* poTdr2Php )
{
    this->poTdr2Php = poTdr2Php;
}

