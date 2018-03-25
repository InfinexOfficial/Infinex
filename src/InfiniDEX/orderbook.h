// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <iostream>
#include <vector>
#include <map>
#include "userconnection.h"

class COrderBook;
class COrderBookSetting;
class COrderBookManager;

typedef std::map<uint64_t, COrderBook> PriceOrderBook; //price and order data
typedef std::pair<PriceOrderBook, PriceOrderBook> BidAskOrderBook;
typedef std::pair<COrderBookSetting, BidAskOrderBook> OrderBookInfo;
extern std::map<int, OrderBookInfo> mapOrderBook;
extern COrderBookManager orderBookManager;

class COrderBookSetting
{	
	bool IsInChargeOfBroadcast;
	bool SyncInProgress;
	uint64_t LastBroadcastTime;

	COrderBookSetting(bool IsInChargeOfBroadcast, bool SyncInProgress, uint64_t LastBroadcastTime):
		IsInChargeOfBroadcast(IsInChargeOfBroadcast),
		SyncInProgress(SyncInProgress),
		LastBroadcastTime(LastBroadcastTime)
	{}

	COrderBookSetting():
		IsInChargeOfBroadcast(false),
		SyncInProgress(false),
		LastBroadcastTime(0)
	{}
};

class COrderBook
{
private:
	std::vector<unsigned char> vchSig;

public:
	uint64_t nOrderPrice;
	uint64_t nQuantity;
	uint64_t nAmount;
	std::string nMNPubKey;
	uint64_t nLastUpdateTime;
	std::string nHash;

	COrderBook(uint64_t nOrderPrice, uint64_t nQuantity, uint64_t nAmount, std::string nMNPubKey, uint64_t nLastUpdateTime, std::string nHash) :
		nOrderPrice(nOrderPrice),
		nQuantity(nQuantity),
		nAmount(nAmount),
		nMNPubKey(nMNPubKey),
		nLastUpdateTime(nLastUpdateTime),
		nHash(nHash)
	{}

	COrderBook() :
		nOrderPrice(0),
		nQuantity(0),
		nAmount(0),
		nMNPubKey(""),
		nLastUpdateTime(0),
		nHash("")
	{}

	bool Verify();
	std::string Sign();
	void RelayTo(CNode* node, CConnman& connman);
	void Relay(CConnman& connman);
};

class COrderBookManager
{
private:
	std::vector<unsigned char> vchSig;
	uint64_t GetAdjustedTime();
	bool InsertNewBidOrder(int TradePairID, uint64_t Price, int64_t Qty);
	bool InsertNewAskOrder(int TradePairID, uint64_t Price, int64_t Qty);

public:
	void AssignRole(int TradePairID);
	void AdjustBidQuantity(int TradePairID, uint64_t Price, int64_t Qty);
	void AdjustAskQuantity(int TradePairID, uint64_t Price, int64_t Qty);
	void UpdateBidOrder(int TradePairID, uint64_t Price, uint64_t Quantity);
	void UpdateAskOrder(int TradePairID, uint64_t Price, uint64_t Quantity);
};
#endif