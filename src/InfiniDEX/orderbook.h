// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <iostream>
#include <vector>
#include <map>
#include "net.h"
#include "utilstrencodings.h"

class COrderBook;
class COrderBookManager;

typedef std::map<uint64_t, COrderBook> PriceOrderBook; //price and order data
extern std::map<int, PriceOrderBook> mapOrderBidBook;
extern std::map<int, PriceOrderBook> mapOrderAskBook;
extern COrderBookManager orderBookManager;

class COrderBook
{
private:
	std::vector<unsigned char> vchSig;

public:
	int nTradePairID;
	bool nIsBid;
	uint64_t nPrice;
	uint64_t nQty;
	uint64_t nLastUpdateTime;
	std::string nMNPubKey;

	COrderBook(int nTradePairID, bool nIsBid, uint64_t nPrice, uint64_t nQty, uint64_t nLastUpdateTime, std::string nMNPubKey) :
		nTradePairID(nTradePairID),
		nIsBid(nIsBid),
		nPrice(nPrice),
		nQty(nQty),
		nLastUpdateTime(nLastUpdateTime),
		nMNPubKey(nMNPubKey)
	{}

	COrderBook() :
		nTradePairID(0),
		nIsBid(true),
		nPrice(0),
		nQty(0),
		nLastUpdateTime(0),
		nMNPubKey("")
	{}

	ADD_SERIALIZE_METHODS;
	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
		READWRITE(nTradePairID);
		READWRITE(nIsBid);
		READWRITE(nPrice);
		READWRITE(nQty);
		READWRITE(nLastUpdateTime);
		READWRITE(nMNPubKey);
		READWRITE(vchSig);
	}

	bool VerifySignature();
	bool Sign();
	void Broadcast();
};

class COrderBookManager
{
public:
	void InitTradePair(int TradePairID);
	void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
	void AdjustBidQuantity(int TradePairID, uint64_t Price, int64_t Qty);
	void AdjustAskQuantity(int TradePairID, uint64_t Price, int64_t Qty);
	void UpdateBidQuantity(int TradePairID, uint64_t Price, uint64_t Quantity);
	void UpdateAskQuantity(int TradePairID, uint64_t Price, uint64_t Quantity);
};
#endif