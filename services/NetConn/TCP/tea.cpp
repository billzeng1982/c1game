#include "tea.h"
#include "comm_func.h"
#include "LogMacros.h"

TEAHelper::TEAHelper()
{
	InitRand();
	memset(m_szDeBuf, 0, sizeof(m_szDeBuf));
	memset(m_szEnBuf, 0, sizeof(m_szEnBuf));
}

TEAHelper::~TEAHelper()
{

}

bool TEAHelper::Decrypt(unsigned char* In, int iInLen, unsigned char* Out, int& iOutLen, unsigned char* key, int keyLen)
{
	// 因为加密之后至少是16字节，并且肯定是8的倍数，这里检查这种情况
	if ((iInLen % 8 != 0) || (iInLen < 16))
	{
		return false;
	}

	for (int i = 0; i < iInLen; i += 8)
	{
		decode(In, iInLen, i, m_szDeBuf, iInLen, i, key, keyLen);
	}

	for (int i = 8; i < iInLen; i++)
	{
		m_szDeBuf[i] = (unsigned char)(m_szDeBuf[i] ^ In[i - 8]);
	}

	int pos = m_szDeBuf[0] & 0x07;
	iOutLen = iInLen - pos - 10;
	if (iOutLen < 0)
	{
		return false;
	}
	
	memcpy(Out, m_szDeBuf + pos + 3, iOutLen);
	return true;
}

void TEAHelper::Encrypt(unsigned char* In, int iInLen, unsigned char* Out, int& iOutLen, unsigned char* key, int keyLen)
{
	// 计算头部填充字节数	
	int pos = (iInLen+ 10) % 8;
	if (pos != 0)
	{
		pos = 8 - pos;
	}

	iOutLen = iInLen + pos + 10;

	m_szEnBuf[0] = (unsigned char)((rand() & 0xF8) | pos);
	for (int i = 1; i < pos + 3; i++)
	{
		m_szEnBuf[i] = (unsigned char)(rand() & 0xFF);
	}

	memcpy(m_szEnBuf + pos + 3, In, iInLen);

	for (int i = pos + 3 + iInLen; i < iOutLen; i++)
	{
		m_szEnBuf[i] = 0x0;
	}

	for (int i = 0; i < iOutLen; i += 8)
	{
		code(m_szEnBuf, iOutLen, i, Out, iOutLen, i, key, keyLen);
	}
}

void TEAHelper::GenerKey(char* pszKey, int iLength)
{
	randstr(pszKey, iLength);
}

void TEAHelper::InitRand()
{
	srand( (unsigned int )time(0));
}

void TEAHelper::code(unsigned char* In, int inLen, int inPos, 
					 unsigned char* Out, int outLen, int outPos, unsigned char* key, int keyLen)
{
	if (outPos > 0)
	{
		for (int i = 0; i < 8; i++)
		{
			In[outPos + i] = (unsigned char)(In[inPos + i] ^ Out[outPos + i - 8]);
		}
	}

	unsigned int* formattedKey = FormatKey(key, keyLen);
	unsigned int y = ConvertByteArrayToUInt(In, outPos, inLen);
	unsigned int z = ConvertByteArrayToUInt(In, outPos + 4, inLen);
	unsigned int sum = 0;
	unsigned int delta = 0x9e3779b9;
	unsigned int n = 16;

	while (n-- > 0)
	{
		sum += delta;
		y += ((z << 4) + formattedKey[0]) ^ (z + sum) ^ ((z >> 5) + formattedKey[1]);
		z += ((y << 4) + formattedKey[2]) ^ (y + sum) ^ ((y >> 5) + formattedKey[3]);
	}

	memcpy(Out + outPos, ConvertUIntToByteArray(y), 4);			
	memcpy(Out + outPos + 4, ConvertUIntToByteArray(z), 4);		

	if (inPos > 0)
	{
		for (int i = 0; i < 8; i++)
		{
			Out[outPos + i] = (unsigned char)(Out[outPos + i] ^ In[inPos + i - 8]);
		}
	}
}

void TEAHelper::decode(unsigned char* In, int inLen, int inPos, 
					   unsigned char* Out, int outLen, int outPos, unsigned char* key, int keyLen)
{
	if (outPos > 0)
	{
		for (int i = 0; i < 8; i++)
		{
			Out[outPos + i] = (unsigned char)(In[inPos + i] ^ Out[outPos + i - 8]);
		}
	}
	else
	{	
		memcpy(Out, In, 8);	
	}
	unsigned int* formattedKey = FormatKey(key, keyLen);
	unsigned int y = ConvertByteArrayToUInt(Out, outPos, outLen);
	unsigned int z = ConvertByteArrayToUInt(Out, outPos + 4, outLen);
	unsigned int sum = 0xE3779B90;
	unsigned int delta = 0x9e3779b9;
	unsigned int n = 16;

	while (n-- > 0)
	{
		z -= ((y << 4) + formattedKey[2]) ^ (y + sum) ^ ((y >> 5) + formattedKey[3]);
		y -= ((z << 4) + formattedKey[0]) ^ (z + sum) ^ ((z >> 5) + formattedKey[1]);
		sum -= delta;
	}
	memcpy(Out + outPos, ConvertUIntToByteArray(y), 4);	
	memcpy(Out + outPos + 4, ConvertUIntToByteArray(z), 4);
}

unsigned int TEAHelper::ConvertByteArrayToUInt(unsigned char* v, int offset, int len)
{
	if (offset + 4 > len)
	{
		return 0;
	}

	unsigned int output;
	output = (unsigned int)(v[offset] << 24);
	output |= (unsigned int)(v[offset + 1] << 16);
	output |= (unsigned int)(v[offset + 2] << 8);
	output |= (unsigned int)(v[offset + 3] << 0);
	return output;
}

unsigned int* TEAHelper::FormatKey(unsigned char* key, int keyLen)
{
	static unsigned int formattedKey[4] = {};		
	unsigned char refineKey[16] = {};
	if (keyLen < 16)
	{
		memcpy(refineKey, key, keyLen);
		for (int k = keyLen; k < 16; k++)
		{
			refineKey[k] = 0x20;
		}
	}
	else
	{
		memcpy(refineKey, key, 16);
	}

	int j = 0;
	for (int i = 0; i < 16; i += 4)
	{
		formattedKey[j++] = ConvertByteArrayToUInt(refineKey, i, 16);
	}
	return formattedKey;
}

unsigned char* TEAHelper::ConvertUIntToByteArray(unsigned int v)
{
	static unsigned char result[4] = {};
	result[0] = (unsigned char)((v >> 24) & 0xFF);
	result[1] = (unsigned char)((v >> 16) & 0xFF);
	result[2] = (unsigned char)((v >> 8) & 0xFF);
	result[3] = (unsigned char)((v >> 0) & 0xFF);
	return result;
}