// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "usertradehistory.h"

class CUserTradeHistory;
class CUserTradeHistoryManager;

std::map<int, mapUserTradeHistoryByPubKey> mapUserTradeHistories;
std::map<int, mapUserTradeHistoryById> mapMarketTradeHistories;
std::map<int, CUserTradeHistorySetting> mapUserTradeHistorySetting;
CUserTradeHistoryManager userTradeHistoryManager;

bool CUserTradeHistoryManager::IsTradePairInlist(int TradePairID)
{
	return mapUserTradeHistorySetting.count(TradePairID);
}

void CUserTradeHistoryManager::InitTradePair(int TradePairID)
{
	if (IsTradePairInlist(TradePairID))
		return;

	mapUserTradeHistories.insert(std::make_pair(TradePairID, mapUserTradeHistoryByPubKey()));
	mapMarketTradeHistories.insert(std::make_pair(TradePairID, mapUserTradeHistoryById()));
	mapUserTradeHistorySetting.insert(std::make_pair(TradePairID, CUserTradeHistorySetting(TradePairID)));
}

CUserTradeHistorySetting& CUserTradeHistoryManager::GetTradeHistorySetting(int TradePairID)
{
	return mapUserTradeHistorySetting[TradePairID];
}

void CUserTradeHistoryManager::InputNewUserTradeHistory(CUserTradeHistory UTH)
{
	CUserTradeHistorySetting& setting = GetTradeHistorySetting(UTH.nTradePairID);
	if (setting.TradePairID == 0)
		return;

	if (!setting.IsinChargeOfMarketTradeHistory && !setting.IsInChargeOfUserTradeHistory)
		return;

	std::shared_ptr<CUserTradeHistory> ut = std::make_shared<CUserTradeHistory>(UTH);
	ut->nTradeHistoryID = ++setting.LastUserTradeHistoryID;
	std::pair<int, std::shared_ptr<CUserTradeHistory>> pairTemp = std::make_pair(ut->nTradeHistoryID, ut);
	mapUserTradeHistoryById& market = mapMarketTradeHistories[ut->nTradePairID];
	market.insert(pairTemp);

	if (!mapUserTradeHistories[ut->nTradePairID].count(ut->nUser1PubKey))
		mapUserTradeHistories[ut->nTradePairID].insert(std::make_pair(ut->nUser1PubKey, mapUserTradeHistoryById()));
	mapUserTradeHistoryById& user1 = mapUserTradeHistories[ut->nTradePairID][ut->nUser1PubKey];
	user1.insert(pairTemp);

	if (!mapUserTradeHistories[ut->nTradePairID].count(ut->nUser2PubKey))
		mapUserTradeHistories[ut->nTradePairID].insert(std::make_pair(ut->nUser2PubKey, mapUserTradeHistoryById()));
	mapUserTradeHistoryById& user2 = mapUserTradeHistories[ut->nTradePairID][ut->nUser2PubKey];
	user2.insert(pairTemp);
}

bool CUserTradeHistoryManager::SetNodeAsMarketTradeHistoryProvider(CUserTradeHistorySetting& setting, bool mode)
{
	setting.IsinChargeOfMarketTradeHistory = mode;
	//sync and setup for enabled
	//data clearing if required for disabled
	return true;
}

bool CUserTradeHistoryManager::SetNodeAsUserTradeHistoryProvider(CUserTradeHistorySetting& setting, bool mode)
{	
	setting.IsInChargeOfUserTradeHistory = mode;
	//sync and setup for enabled
	//data clearing if required for disabled
	return true;
}