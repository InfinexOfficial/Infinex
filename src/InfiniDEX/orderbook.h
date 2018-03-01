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

extern std::map<uint256, COrderBook> orderBidBook;
extern std::map<uint256, COrderBook> orderAskBook;
extern COrderBookManager orderBookManager;


class COrderBook
{
private:
	std::vector<unsigned char> vchSig;

public:
	int nOrderBookID;
    int64_t nOrderPrice;
	int64_t nQuantity;
    int64_t nAmount;
	int64_t nOrderTime;

	COrderBook(int nOrderBookID, int64_t nOrderPrice, int64_t nQuantity, int64_t nAmount, int64_t nOrderTime) :
		nOrderBookID(nOrderBookID),
        nOrderPrice(nOrderPrice),
        nQuantity(nQuantity),
        nAmount(nAmount),
        nOrderTime(nOrderTime)
	{}

	COrderBook() :
		nOrderBookID(0),
        nOrderPrice(0),
        nQuantity(0),
        nAmount(0),
        nOrderTime(0)
	{}

	ADD_SERIALIZE_METHODS;

	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
		READWRITE(nOrderBookID);
		READWRITE(nOrderPrice);
		READWRITE(nQuantity);
		READWRITE(nAmount);
        READWRITE(nOrderTime);
	}

	uint256 GetHash() const
	{
		CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
		ss << nOrderBookID;
		ss << nOrderPrice;
		ss << nQuantity;
		ss << nAmount;
        ss << nOrderTime;
		return ss.GetHash();
	}

	bool Sign();
	bool CheckSignature();
	void Relay(CConnman& connman);
};

class COrderBookManager
{
private:
    std::vector<unsigned char> vchSig;
};
#endif
