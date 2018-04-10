// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chartdata.h"
#include "tradepair.h"

class CChartData;
class CChartDataManager;

std::map<int, mapPeriodTimeData> mapChartData;
std::map<int, CChartDataSetting> mapChartDataSetting;
CChartDataManager ChartDataManager;

bool CChartDataManager::IsInChargeOfChartData(int TradePairID)
{
	return mapChartDataSetting[TradePairID].IsInChargeOfChartData;
}

bool CChartDataManager::IsTradePairInList(int TradePairID)
{
	return mapChartData.count(TradePairID);
}

bool CChartDataManager::InitTradePair(int TradePairID)
{
	if (!tradePairManager.IsValidTradePairID(TradePairID))
		return false;

	if (!IsTradePairInList(TradePairID))
	{
		mapPeriodTimeData init;
		init.insert(std::make_pair(MINUTE_CHART_DATA, mapTimeData()));
		init.insert(std::make_pair(HOUR_CHART_DATA, mapTimeData()));
		init.insert(std::make_pair(DAY_CHART_DATA, mapTimeData()));
		mapChartData.insert(std::make_pair(TradePairID, init));
		return true;
	}
	if (!mapChartData[TradePairID].count(MINUTE_CHART_DATA))
	{
		mapChartData[TradePairID].insert(std::make_pair(MINUTE_CHART_DATA, mapTimeData()));
	}
	if (!mapChartData[TradePairID].count(HOUR_CHART_DATA))
	{
		mapChartData[TradePairID].insert(std::make_pair(HOUR_CHART_DATA, mapTimeData()));
	}
	if (!mapChartData[TradePairID].count(DAY_CHART_DATA))
	{
		mapChartData[TradePairID].insert(std::make_pair(DAY_CHART_DATA, mapTimeData()));
	}
	return true;
}

void CChartDataManager::InputNewTrade(int TradePairID, uint64_t Price, uint64_t Qty, uint64_t TradeTime)
{
	if (!IsTradePairInList(TradePairID))
	{
		//check if current node is assigned to process
		//if true
		InitTradePair(TradePairID);
	}

	//process minute range
	auto& a = mapChartData[TradePairID][MINUTE_CHART_DATA];
	mapTimeData::reverse_iterator ri = a.rbegin();
	if (ri == a.rend())
	{
		//need to decide tradetime in second or milisecond
		int TimeRoundDown = TradeTime % 60;
		uint64_t newMinuteStart = TradeTime - TimeRoundDown;
		uint64_t newMinuteEnd = newMinuteStart + 60;
		TimeRange tr = std::make_pair(newMinuteStart, newMinuteEnd);
		CChartData cd(TradePairID, newMinuteStart, newMinuteEnd, Price, Price, Price, Price, Price * Qty, Qty, 1);
		a.insert(std::make_pair(tr, cd));
	}
	else
	{
		uint64_t lastMinuteEnd = ri->first.second;
		if (lastMinuteEnd >= TradeTime)
		{
			ri->second.nNoOfTrades++;
			ri->second.nQty += Qty;
			ri->second.nAmount += (Qty * Price);
			ri->second.nClosePrice = Price;
			if (Price > ri->second.nHighPrice)
				ri->second.nHighPrice = Price;
			else if (Price < ri->second.nLowPrice || ri->second.nLowPrice == 0)
				ri->second.nLowPrice = Price;
		}
		else
		{
			uint64_t newMinuteStart = lastMinuteEnd + 1;
			uint64_t newMinuteEnd = lastMinuteEnd + 60;
			TimeRange tr = std::make_pair(newMinuteStart, newMinuteEnd);
			CChartData cd(TradePairID, newMinuteStart, newMinuteEnd, Price, Price, Price, Price, Price * Qty, Qty, 1);
			a.insert(std::make_pair(tr, cd));
		}
	}

	//process hour range
	auto& b = mapChartData[TradePairID][HOUR_CHART_DATA];
	mapTimeData::reverse_iterator ri2 = b.rbegin();
	if (ri2 == b.rend())
	{
		int TimeRoundDown = TradeTime % 3600;
		uint64_t newHourStart = TradeTime - TimeRoundDown;
		uint64_t newHourEnd = newHourStart + 3600;
		TimeRange tr = std::make_pair(newHourStart, newHourEnd);
		CChartData cd(TradePairID, newHourStart, newHourEnd, Price, Price, Price, Price, Price * Qty, Qty, 1);
		b.insert(std::make_pair(tr, cd));
	}
	else
	{
		uint64_t lastHourEnd = ri2->first.second;
		if (lastHourEnd >= TradeTime)
		{
			ri2->second.nNoOfTrades++;
			ri2->second.nQty += Qty;
			ri2->second.nAmount += (Qty * Price);
			ri2->second.nClosePrice = Price;
			if (Price > ri2->second.nHighPrice)
				ri2->second.nHighPrice = Price;
			else if (Price < ri2->second.nLowPrice || ri->second.nLowPrice == 0)
				ri2->second.nLowPrice = Price;
		}
		else
		{
			uint64_t newHourStart = lastHourEnd + 1;
			uint64_t newHourEnd = newHourStart + 3600;
			TimeRange tr = std::make_pair(newHourStart, newHourEnd);
			CChartData cd(TradePairID, newHourStart, newHourEnd, Price, Price, Price, Price, Price * Qty, Qty, 1);
			b.insert(std::make_pair(tr, cd));
		}
	}

	//process day range
	auto& c = mapChartData[TradePairID][DAY_CHART_DATA];
	mapTimeData::reverse_iterator ri3 = c.rbegin();
	if (ri3 == c.rend())
	{
		int TimeRoundDown = TradeTime % 86400;
		uint64_t newDayStart = TradeTime - TimeRoundDown;
		uint64_t newDayEnd = newDayStart + 86400;
		TimeRange tr = std::make_pair(newDayStart, newDayEnd);
		CChartData cd(TradePairID, newDayStart, newDayEnd, Price, Price, Price, Price, Price * Qty, Qty, 1);
		c.insert(std::make_pair(tr, cd));
	}
	else
	{
		uint64_t lastDayEnd = ri3->first.second;
		if (lastDayEnd >= TradeTime)
		{
			ri3->second.nNoOfTrades++;
			ri3->second.nQty += Qty;
			ri3->second.nAmount += (Qty * Price);
			ri3->second.nClosePrice = Price;
			if (Price > ri3->second.nHighPrice)
				ri3->second.nHighPrice = Price;
			else if (Price < ri3->second.nLowPrice || ri->second.nLowPrice == 0)
				ri3->second.nLowPrice = Price;
		}
		else
		{
			uint64_t newDayStart = lastDayEnd + 1;
			uint64_t newDayEnd = newDayStart + 86400;
			TimeRange tr = std::make_pair(newDayStart, newDayEnd);
			CChartData cd(TradePairID, newDayStart, newDayEnd, Price, Price, Price, Price, Price * Qty, Qty, 1);
			c.insert(std::make_pair(tr, cd));
		}
	}
}