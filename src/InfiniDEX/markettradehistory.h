// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MARKETTRADEHISTORY_H
#define MARKETTRADEHISTORY_H

#include <iostream>
#include <vector>
#include <map>
#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"

class CMarketTradeHistory;
class CMarketTradeHistoryManager;

typedef std::pair<uint64_t, uint64_t> TimeRange;
extern std::map<int, std::vector<CMarketTradeHistory>> mapRecentMarketTradeHistory; //trade pair and recent trade data
extern std::map<int, std::vector<CMarketTradeHistory>> mapSecondMarketTradeHistory; //trade pair and past seconds trade data
extern std::map<int, std::vector<CMarketTradeHistory>> mapMinuteMarketTradeHistory; //trade pair and past minutes trade data
extern std::map<int, std::vector<CMarketTradeHistory>> mapHourMarketTradeHistory; //trade pair and past hours trade data
extern std::map<int, std::vector<CMarketTradeHistory>> mapDayMarketTradeHistory; //trade pair and past days trade data
extern CMarketTradeHistoryManager marketTradeHistoryManager;

class CMarketTradeHistory
{
private:
	std::vector<unsigned char> vchSig;

public:
	int nTradePairID;
	uint64_t nTradePrice;
	uint64_t nQuantity;
	uint64_t nTradeValue1;
	uint64_t nTradeValue2;
	uint64_t nTradeProcessTime;

	CMarketTradeHistory(int nTradePairID, uint64_t nTradePrice, uint64_t nQuantity, uint64_t nTradeValue1, uint64_t nTradeValue2, uint64_t nTradeProcessTime) :
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
};

class CMarketTradeHistoryManager
{
private:
	std::vector<unsigned char> vchSig;

public:

	CMarketTradeHistoryManager() {}
};

#endif
