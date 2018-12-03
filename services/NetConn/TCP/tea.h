#include <memory.h>
#include <string>
#include <cstdlib>
#include "../utils/FakeRandom.h"
#include "../utils/oi_str.h"
#include "singleton.h"
#include "cs_proto.h"

using namespace PKGMETA;

class TEAHelper : public TSingleton<TEAHelper>
{
public:
	TEAHelper();
	~TEAHelper();
	bool Decrypt(unsigned char* In, int iInLen, unsigned char* Out, int& iOutLen, unsigned char* key, int keyLen);
	//º”√‹
	void Encrypt(unsigned char* In, int iInLen, unsigned char* Out, int& iOutLen, unsigned char* key, int keyLen);

	void GenerKey(char* pszKey, int iLength);
private:
	unsigned char m_szDeBuf[sizeof(PKGMETA::SCPKG) + 20];
	unsigned char m_szEnBuf[sizeof(PKGMETA::SCPKG) + 20];
private:
	void InitRand();

	void code(unsigned char* In, int inLen, int inPos, 
		unsigned char* Out, int outLen, int outPos, unsigned char* key, int keyLen);

	void decode(unsigned char* In, int inLen, int inPos, 
		unsigned char* Out, int outLen, int outPos, unsigned char* key, int keyLen);

	unsigned int ConvertByteArrayToUInt(unsigned char* v, int offset, int len);

	unsigned int* FormatKey(unsigned char* key, int keyLen);

	unsigned char* ConvertUIntToByteArray(unsigned int v);
};