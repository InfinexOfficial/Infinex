// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "usertradehistory.h"

class CUserTradeHistory;
class CUserTradeHistoryManager;

std::map<int, mapUserTradeHistories> mapTradePairUsersTradeHistories;
CUserTradeHistoryManager userTradeHistoryManager;

void CUserTradeHistoryManager::InputNewUserTradeHistory(CUserTradeHistory UTH)
{
	if (!mapTradePairUsersTradeHistories.count(UTH.nTradePairID))
	{
		//sync from seed server
	}
	if (!mapTradePairUsersTradeHistories[UTH.nTradePairID].count(UTH.nUserPubKey))
	{
		mapTradePairUsersTradeHistories[UTH.nTradePairID].insert(std::make_pair(UTH.nUserPubKey, mapHashUserTradeHistory()));
	}
	if (mapTradePairUsersTradeHistories[UTH.nTradePairID][UTH.nUserPubKey].count(UTH.nHash))
	{
		return;
	}
	mapTradePairUsersTradeHistories[UTH.nTradePairID][UTH.nUserPubKey].insert(std::make_pair(UTH.nHash, UTH));
}

bool CUserTradeHistoryManager::GetUserTradeHistories(int TradePairID, std::string UserPubKey, mapHashUserTradeHistory& UserTradeHistories)
{
	if (!mapTradePairUsersTradeHistories[TradePairID].count(UserPubKey))
		return false;
	UserTradeHistories = mapTradePairUsersTradeHistories[TradePairID][UserPubKey];
	return true;
}

bool CUserTradeHistoryManager::GetPairTradeHistories(int TradePairID, mapUserTradeHistories& PairTradeHistories)
{
	if (!mapTradePairUsersTradeHistories.count(TradePairID))
	{
		//sync from seed server
		return false;
	}
	PairTradeHistories = mapTradePairUsersTradeHistories[TradePairID];
	return true;
}