// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MARKETTRADEHISTORY_H
#define MARKETTRADEHISTORY_H

#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"

class CMarketTradeHistory;
class CMarketTradeHistoryManager;

extern std::map<uint256, CMarketTradeHistory> mapRecentMarketTradeHistory;
extern std::map<uint256, CMarketTradeHistory> mapPastMarketTradeHistory;
extern CMarketTradeHistoryManager marketTradeHistoryManager;


class CMarketTradeHistory
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

	CMarketTradeHistory(uint256 nTradePairID, uint64_t nTradePrice, uint64_t nQuantity, uint64_t nTradeValue1, uint64_t nTradeValue2, uint64_t nTradeProcessTime) :
		nTradePairID(nTradePairID),
        nTradePrice(nTradePrice),
        nQuantity(nQuantity),
        nTradeValue1(nTradeValue1),
        nTradeValue2(nTradeValue2),
        nTradeProcessTime(nTradeProcessTime)
	{}

	CMarketTradeHistory() :
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

class CMarketTradeHistoryManager
{
private:
    std::vector<unsigned char> vchSig;

public:

    CMarketTradeHistoryManager() {}

    void ProcessMarketTradeHistory(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
    void BroadcastRecentMarketTradeHistory(uint256 nTradePairID);
	void BroadcastPastMarketTradeHistory(uint256 nTradePairID);
};

#endif
