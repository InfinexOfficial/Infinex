// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERTRADEHISTORY_H
#define USERTRADEHISTORY_H

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <set>
#include "userconnection.h"
#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"

class CUserTradeHistory;
class CUserTradeHistoryManager;
class CUserTradeHistorySetting;

typedef std::map<int, std::shared_ptr<CUserTradeHistory>> mapUserTradeHistoryById;
typedef std::pair<int, mapUserTradeHistoryById> pairLastCounterUserTradeHistory;
typedef std::map<int, pairLastCounterUserTradeHistory> mapUserTradeHistoryById2;
typedef std::map<std::string, mapUserTradeHistoryById> mapUserTradeHistoryByPubKey;
extern std::map<int, mapUserTradeHistoryById> mapMarketTradeHistories;
extern std::map<int, CUserTradeHistorySetting> mapUserTradeHistorySetting;
extern std::map<std::string, mapUserTradeHistoryById2> mapUserTradeHistoriesByTradePair;
extern std::set<std::string> mapMarketTradeHistoryHash;
extern std::set<std::string> mapUserTradeHistoryHash;
extern CUserTradeHistoryManager userTradeHistoryManager;

class CUserTradeHistory
{
private:
	std::vector<unsigned char> mnMarketvchSig;
	std::vector<unsigned char> mnUservchSig;

public:
	int nMarketTradeHistoryID;
	int nTradePairID;
	int nUser1TradeHistoryID;
	int nUser2TradeHistoryID;
	std::string nUserPubKey1;
	std::string nUserPubKey2;
	uint64_t nPrice;
	uint64_t nQty;
	uint64_t nAmount;
	bool nIsBid;
	std::string nMNMarketHash;
	std::string nMNMarketPubKey;
	std::string nMNUserHash;
	std::string nMNUserPubKey;
	uint64_t nTradeTime;

	CUserTradeHistory(int nTradePairID, std::string nUserPubKey1, std::string nUserPubKey2, uint64_t nPrice, uint64_t nQty, uint64_t nAmount, bool nIsBid, uint64_t nTradeTime) :
		nMarketTradeHistoryID(0),
		nTradePairID(nTradePairID),
		nUser1TradeHistoryID(0),
		nUser2TradeHistoryID(0),
		nUserPubKey1(nUserPubKey1),
		nUserPubKey2(nUserPubKey2),
		nPrice(nPrice),
		nQty(nQty),
		nAmount(nAmount),
		nIsBid(nIsBid),
		nMNMarketHash(""),
		nMNMarketPubKey(""),
		nMNUserHash(""),
		nMNUserPubKey(""),
		nTradeTime(nTradeTime)
	{}

	CUserTradeHistory() :
		nMarketTradeHistoryID(0),
		nTradePairID(0),
		nUser1TradeHistoryID(0),
		nUser2TradeHistoryID(0),
		nUserPubKey1(""),
		nUserPubKey2(""),
		nPrice(0),
		nQty(0),		
		nAmount(0),
		nIsBid(false),
		nMNMarketHash(""),
		nMNMarketPubKey(""),
		nMNUserHash(""),
		nMNUserPubKey(""),
		nTradeTime(0)
	{}

	void SetMNUserHash();
	void SetMNMarketHash();
	bool MNUserSign();
	bool MNMarketSign();
	bool VerifyMNUserSignature();
	bool VerifyMNMarketSignature();
};

class CUserTradeHistoryManager
{
private:
	std::vector<unsigned char> vchSig;

public:
	CUserTradeHistoryManager() {}
	void InitTradePair(int TradePairID);
	void AssignMarketTradeHistoryBroadcastRole(int TradePairID, bool toAssign = true);
	void InputUserTradeHistory(const std::shared_ptr<CUserTradeHistory>& tradeHistory);
	void InputMarketTradeHistory(const std::shared_ptr<CUserTradeHistory>& tradeHistory);
	void InputUserTradeHistoryFromNetwork(CUserTradeHistory tradeHistory);
};

class CUserTradeHistorySetting
{
public:
	int TradePairID;
	int LastMarketTradeHistoryID;
	int ToStoreLimit;
	bool InChargeOfBroadcastMarketTradeHistory;	

	CUserTradeHistorySetting(int TradePairID):
		TradePairID(TradePairID),
		LastMarketTradeHistoryID(0),
		ToStoreLimit(100),
		InChargeOfBroadcastMarketTradeHistory(false)
	{}

	CUserTradeHistorySetting():
		TradePairID(0),
		LastMarketTradeHistoryID(0),
		ToStoreLimit(100),
		InChargeOfBroadcastMarketTradeHistory(false)
	{}
};

#endif