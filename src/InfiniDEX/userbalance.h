// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERBALANCE_H
#define USERBALANCE_H

#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"

class CUserBalance;
class CUserBalanceManager;


class CUserBalance
{
public:
    uint256 nUserPubKey;
	uint256 nTradePairID;    
    uint64_t nAvailableBalance;
	uint64_t nLastUpdateTime;

	CMarketTradeHistory(uint256 nUserPubKey, uint256 nTradePairID, uint64_t nAvailableBalance, uint64_t nLastUpdateTime) :
		nUserPubKey(nUserPubKey),
        nTradePairID(nTradePairID),
        nAvailableBalance(nAvailableBalance),
        nLastUpdateTime(nLastUpdateTime)
	{}

	CMarketTradeHistory() :
		nUserPubKey(0),
        nTradePairID(0),
        nAvailableBalance(0),
        nLastUpdateTime(0)
	{}

	ADD_SERIALIZE_METHODS;

	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
		READWRITE(nUserPubKey);
		READWRITE(nTradePairID);
		READWRITE(nAvailableBalance);
		READWRITE(nLastUpdateTime);
	}

	uint256 GetHash() const
	{
		CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
		ss << nUserPubKey;
		ss << nTradePairID;
		ss << nAvailableBalance;
		ss << nLastUpdateTime;
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
	void ProvideRecentMarketTradeHistory(uint256 nTradePairID, string ip);
	void ProvidePastMarketTradeHistory(uint256 nTradePairID, string ip);
};

#endif
