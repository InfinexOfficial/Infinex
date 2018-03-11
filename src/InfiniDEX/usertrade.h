// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERTRADE_H
#define USERTRADE_H

#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"

class CUserTrade;
class CUserTradeManager;

class CUserTrade
{
private:
	std::vector<unsigned char> vchSig;

public:
	int nTradePairID;
    uint64_t nPrice;
    uint64_t nQuantity;
    uint64_t nAmount;    
	std::string nUserPubKey;
	uint64_t nTimeSubmit;
	int64_t nBalanceQty;
    int64_t nBalanceAmount;
    uint64_t nLastUpdate;

	CUserTrade(int nTradePairID, uint64_t nPrice, uint64_t nQuantity, uint64_t nAmount, std::string nUserPubKey, uint64_t nTimeSubmit, int64_t nBalanceQty, int64_t nBalanceAmount, uint64_t nLastUpdate) :
		nTradePairID(nTradePairID),
		nPrice(nPrice),
		nQuantity(nQuantity),
		nAmount(nAmount),
        nUserPubKey(nUserPubKey),
        nTimeSubmit(nTimeSubmit),
        nBalanceQty(nBalanceQty),
        nBalanceAmount(nBalanceAmount),
        nLastUpdate(nLastUpdate)
	{}

	CUserTrade() :
		nTradePairID(0),
		nPrice(0),
		nQuantity(0),
		nAmount(0),
        nUserPubKey(""),
        nTimeSubmit(0),
        nBalanceQty(0),
        nBalanceAmount(0),
        nLastUpdate(0)
	{}


	ADD_SERIALIZE_METHODS;

	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
		READWRITE(nTradePairID);
		READWRITE(nPrice);
		READWRITE(nQuantity);
		READWRITE(nAmount);
        READWRITE(nUserPubKey);
		READWRITE(nTimeSubmit);
		READWRITE(nBalanceQty);
		READWRITE(nBalanceAmount);
        READWRITE(nLastUpdate);
		READWRITE(vchSig);
	}

	uint256 GetHash() const
	{
		CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
		ss << nTradePairID;
		ss << nPrice;
		ss << nQuantity;
		ss << nAmount;
        ss << nUserPubKey;
		ss << nTimeSubmit;
		ss << nBalanceQty;
		ss << nBalanceAmount;
		ss << nLastUpdate;
		return ss.GetHash();
	}

	bool Sign(std::string strSignKey);
	bool CheckSignature();
	void Relay(CConnman& connman);
};

#endif
