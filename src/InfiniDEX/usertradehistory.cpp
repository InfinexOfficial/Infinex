// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "noderole.h"
#include "userconnection.h"
#include "usertradehistory.h"

class CUserTradeHistory;
class CUserTradeHistoryManager;

std::map<std::string, mapUserTradeHistoryById2> mapUserTradeHistoriesByTradePair;
std::map<int, mapUserTradeHistoryById> mapMarketTradeHistories;
std::map<int, CUserTradeHistorySetting> mapUserTradeHistorySetting;
std::set<std::string> mapMarketTradeHistoryHash;
std::set<std::string> mapUserTradeHistoryHash;
CUserTradeHistoryManager userTradeHistoryManager;

void CUserTradeHistoryManager::InitTradePair(int TradePairID)
{
	if (mapUserTradeHistorySetting.count(TradePairID))
		return;

	mapMarketTradeHistories.insert(std::make_pair(TradePairID, mapUserTradeHistoryById()));
	mapUserTradeHistorySetting.insert(std::make_pair(TradePairID, CUserTradeHistorySetting(TradePairID)));
}

void CUserTradeHistoryManager::AssignMarketTradeHistoryBroadcastRole(int TradePairID, bool toAssign)
{
	InitTradePair(TradePairID);
	mapUserTradeHistorySetting[TradePairID].InChargeOfBroadcastMarketTradeHistory = true;
}

//initiated from actual trade info
void CUserTradeHistoryManager::InputUserTradeHistory(const std::shared_ptr<CUserTradeHistory>& tradeHistory)
{
	tradeHistory->SetMNUserHash();
	tradeHistory->nMNUserPubKey = MNPubKey;

	if (!mapUserTradeHistoriesByTradePair.count(tradeHistory->nUserPubKey1))
		mapUserTradeHistoriesByTradePair.insert(std::make_pair(tradeHistory->nUserPubKey1, mapUserTradeHistoryById2()));

	if (!mapUserTradeHistoriesByTradePair.count(tradeHistory->nUserPubKey2))
		mapUserTradeHistoriesByTradePair.insert(std::make_pair(tradeHistory->nUserPubKey2, mapUserTradeHistoryById2()));

	auto& a = mapUserTradeHistoriesByTradePair[tradeHistory->nUserPubKey1];
	if (!a.count(tradeHistory->nTradePairID))
		a.insert(std::make_pair(tradeHistory->nTradePairID, std::make_pair(0, mapUserTradeHistoryById())));

	auto& c = mapUserTradeHistoriesByTradePair[tradeHistory->nUserPubKey2];
	if (!c.count(tradeHistory->nTradePairID))
		c.insert(std::make_pair(tradeHistory->nTradePairID, std::make_pair(0, mapUserTradeHistoryById())));

	auto& b = a[tradeHistory->nTradePairID];
	tradeHistory->nUser1TradeHistoryID = ++b.first;
	
	auto& d = c[tradeHistory->nTradePairID];
	tradeHistory->nUser2TradeHistoryID = ++b.first;
	
	if (!tradeHistory->MNUserSign())
		return;

	b.second.insert(std::make_pair(tradeHistory->nUser1TradeHistoryID, tradeHistory));
	d.second.insert(std::make_pair(tradeHistory->nUser2TradeHistoryID, tradeHistory));
	mapUserTradeHistoryHash.insert(tradeHistory->nMNUserHash);
}

//initiated from actual trade info
void CUserTradeHistoryManager::InputMarketTradeHistory(const std::shared_ptr<CUserTradeHistory>& tradeHistory)
{
	CUserTradeHistorySetting& setting = mapUserTradeHistorySetting[tradeHistory->nTradePairID];
	if (setting.TradePairID == 0)
	{
		//to check from node list whether in charge of market trade history
		//if not found, check status from seed server
		//if in charge
		InitTradePair(tradeHistory->nTradePairID);
		mapUserTradeHistorySetting[tradeHistory->nTradePairID].InChargeOfBroadcastMarketTradeHistory = true;
	}

	tradeHistory->SetMNMarketHash();
	tradeHistory->nMarketTradeHistoryID = ++setting.LastMarketTradeHistoryID;
	tradeHistory->nMNMarketPubKey = MNPubKey;
	if (!tradeHistory->MNMarketSign())
		return;

	mapUserTradeHistoryById& market = mapMarketTradeHistories[tradeHistory->nTradePairID];
	market.insert(std::make_pair(tradeHistory->nMarketTradeHistoryID, tradeHistory));
	mapMarketTradeHistoryHash.insert(tradeHistory->nMNMarketHash);
	int dataCount = market.size();
	if (dataCount > setting.ToStoreLimit)
	{
		mapUserTradeHistoryById::iterator it = market.begin();
		int exceedCount = dataCount - setting.ToStoreLimit;
		if (exceedCount == 1)
			market.erase(it->first);
		else
		{
			for (int i = 0; i < exceedCount; i++)
			{
				if (it != market.end())
				{
					market.erase(it->first);
					++it; //to check
				}
				else
					break;
			}
		}
	}
}

void CUserTradeHistoryManager::InputUserTradeHistoryFromNetwork(CUserTradeHistory tradeHistory)
{

}

void CUserTradeHistory::SetMNUserHash()
{
	nMNUserHash = ""; //to update
}

void CUserTradeHistory::SetMNMarketHash()
{
	nMNMarketHash = ""; //to update
}

bool CUserTradeHistory::VerifyMNUserSignature()
{
	return true;
}

bool CUserTradeHistory::VerifyMNMarketSignature()
{
	return true;
}

bool CUserTradeHistory::MNUserSign()
{
	return true;
}

bool CUserTradeHistory::MNMarketSign()
{
	return true;
}