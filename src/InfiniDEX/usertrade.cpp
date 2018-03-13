// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "usertrade.h"
#include "userbalance.h"
#include "tradepair.h"
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

class CUserTrade;
class CUserTradeManager;

std::map<int, TradeUserPricePair> mapBidTradeUserPrice; //trade pair and bid data
std::map<int, TradeUserPricePair> mapAskTradeUserPrice; //trade pair and ask dataCUserTradeManager userTradeManager;
CUserTradeManager userTradeManager;

void CUserTradeManager::AddToAsk(CUserTrade userTrade)
{
	int AskSideCoinID = tradePairManager.GetAskSideCoinInfoID(userTrade.nTradePairID);
	if (AskSideCoinID < 0)
		return;	
	
	//to shift into if loop after remove else on actual implementation
	userTrade.nAmount = userTrade.nQuantity * userTrade.nPrice;
	CUserTrade* cut = new CUserTrade(userTrade);
	MapPriceCUserTrade mpcut;
	MapUserCUserTrade mucut;

	std::map<int, TradeUserPricePair>::iterator it = mapAskTradeUserPrice.find(userTrade.nTradePairID);
	if (it != mapAskTradeUserPrice.end())
	{
		userbalance_to_exchange_enum_t result = userBalanceManager.BalanceToExchange(0, userTrade.nAmount);
		if (result != USERBALANCE_DEDUCTED)
		{
			//we should not be here
			//for future user banning
			return;
		}

		mpcut = it->second.first;
		mucut = it->second.second;

		if (mpcut.count(userTrade.nPrice))
		{
			mapAskTradeUserPrice[userTrade.nTradePairID].first[userTrade.nPrice].push_back(cut);
		}
		else
		{
			std::vector<CUserTrade*> vecCut;
			vecCut.push_back(cut);
			mapAskTradeUserPrice[userTrade.nTradePairID].first.insert(std::make_pair(userTrade.nPrice, vecCut));
		}
		if (mucut.count(userTrade.nUserPubKey))
		{
			mapAskTradeUserPrice[userTrade.nTradePairID].second[userTrade.nUserPubKey].push_back(cut);
		}
		else
		{
			std::vector<CUserTrade*> vecCut;
			vecCut.push_back(cut);
			mapAskTradeUserPrice[userTrade.nTradePairID].second.insert(std::make_pair(userTrade.nUserPubKey, vecCut));
		}
	}
	else
	{
		//for testing purpose
		//to remove on actual implementation
		std::vector<CUserTrade*> vecCut;
		vecCut.push_back(cut);
		mpcut.insert(std::make_pair(userTrade.nPrice, vecCut));
		mucut.insert(std::make_pair(userTrade.nUserPubKey, vecCut));
		mapAskTradeUserPrice.insert(std::make_pair(userTrade.nTradePairID, std::make_pair(mpcut, mucut)));
	}
}