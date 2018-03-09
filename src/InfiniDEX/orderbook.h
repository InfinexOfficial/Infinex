// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"

class COrderBook;
class COrderBookManager;

typedef std::map<uint64_t, COrderBook> PriceOrderBook; //price and order data
extern std::map<int, PriceOrderBook> orderBidBook; //trade pair and bid data
extern std::map<int, PriceOrderBook> orderAskBook; //trade pair and ask data
extern COrderBookManager orderBookManager;


class COrderBook
{
private:
	std::vector<unsigned char> vchSig;

public:
    uint64_t nOrderPrice;
	uint64_t nQuantity;
    uint64_t nAmount;
	uint64_t nLastUpdateTime;

	COrderBook(uint64_t nOrderPrice, uint64_t nQuantity, uint64_t nAmount, uint64_t nLastUpdateTime) :
        nOrderPrice(nOrderPrice),
        nQuantity(nQuantity),
        nAmount(nAmount),
        nLastUpdateTime(nLastUpdateTime)
	{}

	COrderBook() :
        nOrderPrice(0),
        nQuantity(0),
        nAmount(0),
        nLastUpdateTime(0)
	{}

	ADD_SERIALIZE_METHODS;

	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
		READWRITE(nOrderPrice);
		READWRITE(nQuantity);
		READWRITE(nAmount);
        READWRITE(nLastUpdateTime);
	}

	uint256 GetHash() const
	{
		CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
		ss << nOrderPrice;
		ss << nQuantity;
		ss << nAmount;
        ss << nLastUpdateTime;
		return ss.GetHash();
	}

	bool Sign();
	bool CheckSignature();
	void AddQuantity(uint64_t Quantity);
};

class COrderBookManager
{
private:
    std::vector<unsigned char> vchSig;

public:
	void AddToBid(int CoinID, uint64_t OrderPrice, uint64_t Quantity);
	void AddToAsk(int CoinID, uint64_t OrderPrice, uint64_t Quantity);
};
#endif
