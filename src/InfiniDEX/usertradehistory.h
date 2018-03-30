// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERTRADEHISTORY_H
#define USERTRADEHISTORY_H

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include "userconnection.h"

class CUserTradeHistory;
class CUserTradeHistoryManager;
class CUserTradeHistorySetting;

typedef std::map<int, std::shared_ptr<CUserTradeHistory>> mapUserTradeHistoryById;
typedef std::map<std::string, mapUserTradeHistoryById> mapUserTradeHistoryByPubKey;
extern std::map<int, mapUserTradeHistoryByPubKey> mapUserTradeHistories;
extern std::map<int, mapUserTradeHistoryById> mapMarketTradeHistories;
extern std::map<int, CUserTradeHistorySetting> mapUserTradeHistorySetting;
extern CUserTradeHistoryManager userTradeHistoryManager;

class CUserTradeHistory
{
private:
	std::vector<unsigned char> vchSig;

public:
	int nTradeHistoryID;
	int nTradePairID;
	std::string nUser1PubKey;
	std::string nUser2PubKey;
	uint64_t nTradePrice;
	uint64_t nQty;
	uint64_t nAmount;
	bool nIsBid;
	uint64_t nTradeProcessTime;

	CUserTradeHistory(int nTradePairID, std::string nUser1PubKey, std::string nUser2PubKey, uint64_t nTradePrice, uint64_t nQty,
		uint64_t nAmount, bool nIsBid, uint64_t nTradeProcessTime) :
		nTradeHistoryID(0),
		nTradePairID(nTradePairID),
		nUser1PubKey(nUser1PubKey),
		nUser2PubKey(nUser2PubKey),
		nTradePrice(nTradePrice),
		nQty(nQty),
		nAmount(nAmount),
		nIsBid(nIsBid),
		nTradeProcessTime(nTradeProcessTime)
	{}

	CUserTradeHistory(int nTradeHistoryID, int nTradePairID, std::string nUser1PubKey, std::string nUser2PubKey, uint64_t nTradePrice, uint64_t nQty, 
		uint64_t nAmount, bool nIsBid, uint64_t nTradeProcessTime) :
		nTradeHistoryID(nTradeHistoryID),
		nTradePairID(nTradePairID),
		nUser1PubKey(nUser1PubKey),
		nUser2PubKey(nUser2PubKey),
		nTradePrice(nTradePrice),
		nQty(nQty),
		nAmount(nAmount),
		nIsBid(nIsBid),
		nTradeProcessTime(nTradeProcessTime)
	{}

	CUserTradeHistory() :
		nTradeHistoryID(0),
		nTradePairID(0),
		nUser1PubKey(""),
		nUser2PubKey(""),
		nTradePrice(0),
		nQty(0),		
		nAmount(0),
		nIsBid(false),
		nTradeProcessTime(0)
	{}
};

class CUserTradeHistoryManager
{
private:
	std::vector<unsigned char> vchSig;

public:
	CUserTradeHistoryManager() {}
	bool IsTradePairInlist(int TradePairID);
	void InitTradePair(int TradePairID);
	CUserTradeHistorySetting& GetTradeHistorySetting(int TradePairID);
	void InputNewUserTradeHistory(CUserTradeHistory UTH);
	void InputUserTradeHistory(mapUserTradeHistoryById& container, std::shared_ptr<CUserTradeHistory> history);
	bool SetNodeAsMarketTradeHistoryProvider(CUserTradeHistorySetting& setting, bool mode);
	bool SetNodeAsUserTradeHistoryProvider(CUserTradeHistorySetting& setting, bool mode);
};

class CUserTradeHistorySetting
{
public:
	int TradePairID;
	int LastUserTradeHistoryID;
	bool IsInChargeOfUserTradeHistory;
	bool IsinChargeOfMarketTradeHistory;

	CUserTradeHistorySetting(int TradePairID):
		TradePairID(TradePairID),
		LastUserTradeHistoryID(0),
		IsInChargeOfUserTradeHistory(false),
		IsinChargeOfMarketTradeHistory(false)
	{}

	CUserTradeHistorySetting():
		TradePairID(0),
		LastUserTradeHistoryID(0),
		IsInChargeOfUserTradeHistory(false),
		IsinChargeOfMarketTradeHistory(false)
	{}
};

#endif