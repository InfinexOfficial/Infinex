// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "chartdata.h"
#include "tradepair.h"

class CChartData;
class CChartDataManager;

std::map<int, mapPeriodTimeData> mapChartData;
CChartDataManager ChartDataManager;

bool CChartDataManager::InitTradePair(int TradePairID)
{
	if (!tradePairManager.IsValidTradePair(TradePairID))
		return false;

	if (!mapChartData.count(TradePairID))
	{
		mapPeriodTimeData init;
		init.insert(std::make_pair(MINUTE_CHART_DATA, std::vector<pairTimeData>()));
		init.insert(std::make_pair(HOUR_CHART_DATA, std::vector<pairTimeData>()));
		init.insert(std::make_pair(DAY_CHART_DATA, std::vector<pairTimeData>()));
		mapChartData.insert(std::make_pair(TradePairID, init));
		return true;
	}
	if (!mapChartData[TradePairID].count(MINUTE_CHART_DATA))
	{
		mapChartData[TradePairID].insert(std::make_pair(MINUTE_CHART_DATA, std::vector<pairTimeData>()));
	}
	if (!mapChartData[TradePairID].count(HOUR_CHART_DATA))
	{
		mapChartData[TradePairID].insert(std::make_pair(HOUR_CHART_DATA, std::vector<pairTimeData>()));
	}
	if (!mapChartData[TradePairID].count(DAY_CHART_DATA))
	{
		mapChartData[TradePairID].insert(std::make_pair(DAY_CHART_DATA, std::vector<pairTimeData>()));
	}
	return true;
}

void CChartDataManager::InputNewTrade(int TradePairID, uint64_t Price, uint64_t Qty, uint64_t TradeTime)
{
	if (!InitTradePair(TradePairID))
		return;

	//process minute range
	if (!mapChartData[TradePairID][MINUTE_CHART_DATA][mapChartData[TradePairID][MINUTE_CHART_DATA].size() - 1].first.first)
	{

	}
	else
	{
		uint64_t lastMinuteEnd = mapChartData[TradePairID][MINUTE_CHART_DATA][mapChartData[TradePairID][MINUTE_CHART_DATA].size() - 1].first.second;
		if (lastMinuteEnd >= TradeTime)
		{

		}
		else
		{
			uint64_t newMinuteStart = lastMinuteEnd + 1;
			uint64_t newMinuteEnd = lastMinuteEnd + 60;
			TimeRange tr = std::make_pair(newMinuteStart, newMinuteEnd);
			CChartData cd(TradePairID, newMinuteStart, newMinuteEnd, Price, Price, Price, Price, Price*Qty, Qty, 1);			
		}
	}
}