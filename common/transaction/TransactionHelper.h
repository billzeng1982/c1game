#ifndef _TRANSACTION_HELPER_H_
#define _TRANSACTION_HELPER_H_

#include "transaction.h"

/*
	token: | transaction id (4bytes)|composite action id(2bytes)| action id(2bytes) |
	зЂвт id = index + 1
*/

class TransactionHelper
{
public:
	TransactionHelper(){}
	~TransactionHelper(){}

	static TActionToken MakeActionToken(TTransactionID dwTransactionID, TActionIndex iCompoActionID, TActionIndex iActionID );
	static void ParseActionToken(const TActionToken& ullToken, 
		TTransactionID& dwTransactionID, TActionIndex& iCompoActionID, TActionIndex& iActionID);

	// not used
	static unsigned short CalcCheckCode(const void* pvPtr);
};


#endif

