#pragma once
#include "define.h"
#include "singleton.h"
#include "player/PlayerData.h"

class Serial : public TSingleton<Serial>
{
public:
	Serial() {}
	~Serial() {}

	static int SerialIdCmp(const void *pstFirst, const void *pstSecond);

public:
	int CheckDrawSerail(PlayerData* pstData, char* pszSerial);

	int AddUsedSerial(PlayerData* pstData, uint16_t wSerialId);

private:
	int8_t _GetValue(char* pszChar);
	uint32_t _GetSerialId(char* pszSerial);
	bool _CheckValid(char* pszSerial, int iLen);
};
