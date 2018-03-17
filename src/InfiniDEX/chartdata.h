// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CHARTDATA_H
#define CHARTDATA_H

#include <iostream>
#include <vector>
#include <map>

class CChartData;
class CChartDataManager;

typedef std::pair<uint64_t, uint64_t> TimeRange;
typedef std::pair<TimeRange, CChartData> pairTimeData;

class CChartData
{
public:
	int nTradePairID;
	uint64_t nStartTime;
    uint64_t nEndTime;
    uint64_t nOpenPrice;
    uint64_t nHighPrice;
    uint64_t nLowPrice;
    uint64_t nClosePrice;
    uint64_t nAmount;
    uint64_t nQty;
    uint64_t nNoOfTrades;

	CChartData(int nTradePairID, uint64_t nStartTime, uint64_t nEndTime, uint64_t nOpenPrice, uint64_t nHighPrice, uint64_t nLowPrice,
        uint64_t nClosePrice, uint64_t nAmount, uint64_t nQty, uint64_t nNoOfTrades) :
		nTradePairID(nTradePairID),
		nStartTime(nStartTime),
		nEndTime(nEndTime),
		nOpenPrice(nOpenPrice),
		nHighPrice(nHighPrice),
		nLowPrice(nLowPrice),
		nClosePrice(nClosePrice),
		nAmount(nAmount),
		nQty(nQty),
        nNoOfTrades(nNoOfTrades)
	{}

	CChartData() :
		nTradePairID(0),
		nStartTime(0),
		nEndTime(0),
		nOpenPrice(0),
		nHighPrice(0),
		nLowPrice(0),
		nClosePrice(0),
		nAmount(0),
		nQty(0),
        nNoOfTrades(0)
	{}
};

class CChartDataManager
{
public:
	CChartDataManager() {}
};

#endif