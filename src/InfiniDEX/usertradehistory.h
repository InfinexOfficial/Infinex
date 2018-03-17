// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERTRADEHISTORY_H
#define USERTRADEHISTORY_H

#include <iostream>
#include <vector>
#include <map>

class CUserTradeHistory;
class CUserTradeHistoryManager;

typedef std::map<std::string, CUserTradeHistory> mapHashUserTradeHistory;
typedef std::map<std::string, mapHashUserTradeHistory> mapUserTradeHistories;
extern std::map<int, mapUserTradeHistories> mapTradePairUsersTradeHistories;
extern CUserTradeHistoryManager userTradeHistoryManager;

class CUserTradeHistory
{
private:
	std::vector<unsigned char> vchSig;

public:
	int nTradePairID;
	std::string nUserPubKey;
	uint64_t nTradePrice;
	uint64_t nQty1;
	uint64_t nQty2;
	uint64_t nAmount;
	bool nIsBid;
	std::string nHash;
	uint64_t nTradeProcessTime;

	CUserTradeHistory(int nTradePairID, std::string nUserPubkey, uint64_t nTradePrice, uint64_t nQty1, uint64_t nQty2, uint64_t nAmount, bool nIsBid, 
		std::string nHash, uint64_t nTradeProcessTime) :
		nTradePairID(nTradePairID),
		nUserPubKey(nUserPubkey),
		nTradePrice(nTradePrice),
		nQty1(nQty1),
		nQty2(nQty2),
		nAmount(nAmount),
		nIsBid(nIsBid),
		nHash(nHash),
		nTradeProcessTime(nTradeProcessTime)
	{}

	CUserTradeHistory(int nTradePairID, std::string nUserPubKey, uint64_t nTradePrice, uint64_t nQty1, uint64_t nQty2, uint64_t nAmount, bool nIsBid, 
		std::string nHash, uint64_t nTradeProcessTime) :
		nTradePairID(nTradePairID),
		nUserPubKey(nUserPubKey),
		nTradePrice(nTradePrice),
		nQty1(nQty1),
		nQty2(nQty2),
		nAmount(nAmount),
		nIsBid(nIsBid),
		nHash(nHash),
		nTradeProcessTime(nTradeProcessTime)
	{}

	CUserTradeHistory() :
		nTradePairID(0),
		nUserPubKey(""),
		nTradePrice(0),
		nQty1(0),
		nQty2(0),
		nAmount(0),
		nIsBid(false),
		nHash(""),
		nTradeProcessTime(0)
	{}
};

class CUserTradeHistoryManager
{
private:
	std::vector<unsigned char> vchSig;

public:
	CUserTradeHistoryManager() {}
	void InputNewUserTradeHistory(CUserTradeHistory UTH);
	bool GetUserTradeHistories(int TradePairID, std::string UserPubKey, mapHashUserTradeHistory& UserTradeHistories);
	bool GetPairTradeHistories(int TradePairID, mapUserTradeHistories& PairTradeHistories);
};

#endif