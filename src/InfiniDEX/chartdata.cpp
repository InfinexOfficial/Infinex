// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "activemasternode.h"
#include "chartdata.h"
#include "messagesigner.h"
#include "net_processing.h"
#include "noderole.h"
#include "timedata.h"
#include "tradepair.h"
#include "userconnection.h"
#include <boost/lexical_cast.hpp>

class CChartData;
class CChartDataManager;
class CChartDataSetting;

std::map<int, mapPeriodTimeData> mapChartData;
std::map<int, std::vector<std::shared_ptr<CChartData>>> completeChartData;
std::map<int, CChartDataSetting> mapChartDataSetting;
CChartDataManager ChartDataManager;

void CChartDataManager::ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{
	if (strCommand == NetMsgType::DEXCHARTSDATA)
	{
		std::vector<CChartData> incomings;
		vRecv >> incomings;
		
		for (auto incoming : incomings)
		{
			if (!incoming.VerifySignature()) {
				LogPrintf("CChartDataManager::ProcessMessage -- invalid signature\n");
				Misbehaving(pfrom->GetId(), 100);
				return;
			}

			InputChartData(incoming);
		}
	}
	else if (strCommand == NetMsgType::DEXCHARTDATA)
	{
		CChartData incoming;
		vRecv >> incoming;

		if (!incoming.VerifySignature()) {
			LogPrintf("CChartDataManager::ProcessMessage -- invalid signature\n");
			Misbehaving(pfrom->GetId(), 100);
			return;
		}

		InputChartData(incoming);		
	}
	else if (strCommand == NetMsgType::DEXGETCHARTSDATA)
	{
		int TradePairID;
		vRecv >> TradePairID;
		if (TradePairID < 1)
		{
			//banning or reducing score here
			return;
		}
		ProcessChartDataRequest(TradePairID, pfrom, connman);
	}
}

void CChartDataManager::ProcessChartDataRequest(int TradePairID, CNode* node, CConnman& connman)
{
	if (mapChartDataSetting.count(TradePairID))
	{
		auto& a = mapChartDataSetting[TradePairID];
		if (a.IsInChargeOfChartData && !a.SyncInProgress && completeChartData.count(TradePairID))
		{
			auto& b = completeChartData[TradePairID];
			if (b.size() > 0)
				for (auto& c : b)
					connman.PushMessage(node, NetMsgType::DEXCHARTDATA, *c.get());
		}
	}
}

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
	std::vector<CChartData> toBroadcast;
	//process minute range
	auto& a = mapChartData[TradePairID][MINUTE_CHART_DATA];
	mapTimeData::reverse_iterator ri = a.rbegin();
	if (ri == a.rend())
	{
		int TimeRoundDown = TradeTime % 60000;
		uint64_t newMinuteStart = TradeTime - TimeRoundDown;
		uint64_t newMinuteEnd = newMinuteStart + 60000;
		TimeRange tr = std::make_pair(newMinuteStart, newMinuteEnd);
		std::shared_ptr<CChartData> cd = std::make_shared<CChartData>(TradePairID, newMinuteStart, newMinuteEnd, Price, Price, Price, Price, Price * Qty, Qty, 1, GetAdjustedTime(), MNPubKey);
		a.insert(std::make_pair(tr, cd));
		completeChartData[TradePairID].push_back(cd);
		toBroadcast.push_back(CChartData(TradePairID, newMinuteStart, newMinuteEnd, Price, Price, Price, Price, Price * Qty, Qty, 1, GetAdjustedTime(), MNPubKey));
	}
	else
	{
		uint64_t lastMinuteEnd = ri->first.second;
		if (lastMinuteEnd >= TradeTime)
		{
			++ri->second->nNoOfTrades;
			ri->second->nQty += Qty;
			ri->second->nAmount += (Qty * Price);
			ri->second->nClosePrice = Price;
			if (Price > ri->second->nHighPrice)
				ri->second->nHighPrice = Price;
			else if (Price < ri->second->nLowPrice || ri->second->nLowPrice == 0)
				ri->second->nLowPrice = Price;
			ri->second->nLastUpdate = GetAdjustedTime();
			toBroadcast.push_back(*ri->second);
		}
		else
		{
			uint64_t newMinuteStart = lastMinuteEnd + 1;
			uint64_t newMinuteEnd = lastMinuteEnd + 60000;
			TimeRange tr = std::make_pair(newMinuteStart, newMinuteEnd);
			std::shared_ptr<CChartData> cd = std::make_shared<CChartData>(TradePairID, newMinuteStart, newMinuteEnd, Price, Price, Price, Price, Price * Qty, Qty, 1, GetAdjustedTime(), MNPubKey);
			a.insert(std::make_pair(tr, cd));
			completeChartData[TradePairID].push_back(cd);
			toBroadcast.push_back(CChartData(TradePairID, newMinuteStart, newMinuteEnd, Price, Price, Price, Price, Price * Qty, Qty, 1, GetAdjustedTime(), MNPubKey));
		}
	}

	//process hour range
	auto& b = mapChartData[TradePairID][HOUR_CHART_DATA];
	mapTimeData::reverse_iterator ri2 = b.rbegin();
	if (ri2 == b.rend())
	{
		int TimeRoundDown = TradeTime % 3600000;
		uint64_t newHourStart = TradeTime - TimeRoundDown;
		uint64_t newHourEnd = newHourStart + 3600000;
		TimeRange tr = std::make_pair(newHourStart, newHourEnd);
		std::shared_ptr<CChartData> cd = std::make_shared<CChartData>(TradePairID, newHourStart, newHourEnd, Price, Price, Price, Price, Price * Qty, Qty, 1, GetAdjustedTime(), MNPubKey);
		b.insert(std::make_pair(tr, cd));
		completeChartData[TradePairID].push_back(cd);
	}
	else
	{
		uint64_t lastHourEnd = ri2->first.second;
		if (lastHourEnd >= TradeTime)
		{
			++ri2->second->nNoOfTrades;
			ri2->second->nQty += Qty;
			ri2->second->nAmount += (Qty * Price);
			ri2->second->nClosePrice = Price;
			if (Price > ri2->second->nHighPrice)
				ri2->second->nHighPrice = Price;
			else if (Price < ri2->second->nLowPrice || ri->second->nLowPrice == 0)
				ri2->second->nLowPrice = Price;
			ri->second->nLastUpdate = GetAdjustedTime();
		}
		else
		{
			uint64_t newHourStart = lastHourEnd + 1;
			uint64_t newHourEnd = newHourStart + 3600000;
			TimeRange tr = std::make_pair(newHourStart, newHourEnd);
			std::shared_ptr<CChartData> cd = std::make_shared<CChartData>(TradePairID, newHourStart, newHourEnd, Price, Price, Price, Price, Price * Qty, Qty, 1, GetAdjustedTime(), MNPubKey);
			b.insert(std::make_pair(tr, cd));
			completeChartData[TradePairID].push_back(cd);
		}
	}

	//process day range
	auto& c = mapChartData[TradePairID][DAY_CHART_DATA];
	mapTimeData::reverse_iterator ri3 = c.rbegin();
	if (ri3 == c.rend())
	{
		int TimeRoundDown = TradeTime % 86400000;
		uint64_t newDayStart = TradeTime - TimeRoundDown;
		uint64_t newDayEnd = newDayStart + 86400000;
		TimeRange tr = std::make_pair(newDayStart, newDayEnd);
		std::shared_ptr<CChartData> cd = std::make_shared<CChartData>(TradePairID, newDayStart, newDayEnd, Price, Price, Price, Price, Price * Qty, Qty, 1, GetAdjustedTime(), MNPubKey);
		b.insert(std::make_pair(tr, cd));
		completeChartData[TradePairID].push_back(cd);
	}
	else
	{
		uint64_t lastDayEnd = ri3->first.second;
		if (lastDayEnd >= TradeTime)
		{
			++ri3->second->nNoOfTrades;
			ri3->second->nQty += Qty;
			ri3->second->nAmount += (Qty * Price);
			ri3->second->nClosePrice = Price;
			if (Price > ri3->second->nHighPrice)
				ri3->second->nHighPrice = Price;
			else if (Price < ri3->second->nLowPrice || ri->second->nLowPrice == 0)
				ri3->second->nLowPrice = Price;
			ri->second->nLastUpdate = GetAdjustedTime();
		}
		else
		{
			uint64_t newDayStart = lastDayEnd + 1;
			uint64_t newDayEnd = newDayStart + 86400000;
			TimeRange tr = std::make_pair(newDayStart, newDayEnd);
			std::shared_ptr<CChartData> cd = std::make_shared<CChartData>(TradePairID, newDayStart, newDayEnd, Price, Price, Price, Price, Price * Qty, Qty, 1, GetAdjustedTime(), MNPubKey);
			b.insert(std::make_pair(tr, cd));
			completeChartData[TradePairID].push_back(cd);
		}
	}
}

bool CChartData::VerifySignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nStartTime) + boost::lexical_cast<std::string>(nEndTime)
		+ boost::lexical_cast<std::string>(nOpenPrice) + boost::lexical_cast<std::string>(nHighPrice) + boost::lexical_cast<std::string>(nLowPrice)
		+ boost::lexical_cast<std::string>(nClosePrice) + boost::lexical_cast<std::string>(nAmount) + boost::lexical_cast<std::string>(nQty)
		+ boost::lexical_cast<std::string>(nNoOfTrades) + boost::lexical_cast<std::string>(nLastUpdate) + nMNPubKey;
	CPubKey pubkey(ParseHex(nMNPubKey));
	if (!CMessageSigner::VerifyMessage(pubkey, vchSig, strMessage, strError)) {
		LogPrintf("CChartData::VerifySignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}
	return true;
}

bool CChartData::Sign()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nStartTime) + boost::lexical_cast<std::string>(nEndTime)
		+ boost::lexical_cast<std::string>(nOpenPrice) + boost::lexical_cast<std::string>(nHighPrice) + boost::lexical_cast<std::string>(nLowPrice)
		+ boost::lexical_cast<std::string>(nClosePrice) + boost::lexical_cast<std::string>(nAmount) + boost::lexical_cast<std::string>(nQty)
		+ boost::lexical_cast<std::string>(nNoOfTrades) + boost::lexical_cast<std::string>(nLastUpdate) + nMNPubKey;
	if (!CMessageSigner::SignMessage(strMessage, vchSig, activeMasternode.keyMasternode)) {
		LogPrintf("CChartData::Sign -- SignMessage() failed\n");
		return false;
	}
	if (!CMessageSigner::VerifyMessage(activeMasternode.pubKeyMasternode, vchSig, strMessage, strError)) {
		LogPrintf("CChartData::Sign -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}
	return true;
}