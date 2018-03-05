// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERTRADEHISTORY_H
#define USERTRADEHISTORY_H

#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"

class CUserTradeHistory;
class CUserTradeHistoryManager;

extern std::map<uint256, CUserTradeHistory> mapUserTradeHistory;
extern CUserTradeHistoryManager userTradeHistoryManager;


class CUserTradeHistory
{
private:
	std::vector<unsigned char> vchSig;

public:
	uint256 nTradePairID;
    uint64_t nTradePrice;
	uint64_t nQuantity;
    uint64_t nTradeValue1;
    uint64_t nTradeValue2;
	uint64_t nTradeProcessTime;

	CUserTradeHistory(uint256 nTradePairID, uint64_t nTradePrice, uint64_t nQuantity, uint64_t nTradeValue1, uint64_t nTradeValue2, uint64_t nTradeProcessTime) :
		nTradePairID(nTradePairID),
        nTradePrice(nTradePrice),
        nQuantity(nQuantity),
        nTradeValue1(nTradeValue1),
        nTradeValue2(nTradeValue2),
        nTradeProcessTime(nTradeProcessTime)
	{}

	CUserTradeHistory() :
		nTradePairID(0),
        nTradePrice(0),
        nQuantity(0),
        nTradeValue1(0),
        nTradeValue2(0),
        nTradeProcessTime(0)
	{}

	ADD_SERIALIZE_METHODS;

	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
		READWRITE(nTradePairID);
		READWRITE(nTradePrice);
		READWRITE(nQuantity);
		READWRITE(nTradeValue1);
        READWRITE(nTradeValue2);
        READWRITE(nTradeProcessTime);
		READWRITE(vchSig);
	}

	uint256 GetHash() const
	{
		CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
		ss << nTradePairID;
		ss << nTradePrice;
		ss << nQuantity;
		ss << nTradeValue1;
        ss << nTradeValue2;
        ss << nTradeProcessTime;
		return ss.GetHash();
	}

	bool Sign();
	bool CheckSignature();
	void Relay(CConnman& connman);
};

class CUserTradeHistoryManager
{
private:
    std::vector<unsigned char> vchSig;

public:

    CUserTradeHistoryManager() {}
};

#endif
