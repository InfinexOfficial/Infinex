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

std::map<int, PairBidAskCUserTrade> mapUserTradeRequest; //trade pair and bid ask data
CUserTradeManager userTradeManager;

//to convert into enum return
void CUserTradeManager::UserSellRequest(CUserTrade userTrade)
{
	int AskSideCoinID = tradePairManager.GetAskSideCoinInfoID(userTrade.nTradePairID);
	if (AskSideCoinID < 0)
		return;

	//to shift into if loop after remove else on actual implementation
	userTrade.nAmount = userTrade.nQuantity * userTrade.nPrice;
	CUserTrade* cut = new CUserTrade(userTrade);
	MapPriceCUserTrade mpcut;
	MapPubKeyCUserTrade mucut;

	std::map<int, PairBidAskCUserTrade>::iterator it = mapUserTradeRequest.find(userTrade.nTradePairID);
	if (it != mapUserTradeRequest.end())
	{
		userbalance_to_exchange_enum_t result = userBalanceManager.BalanceToExchange(AskSideCoinID, userTrade.nAmount);
		if (result != USERBALANCE_DEDUCTED)
		{
			//we should not be here
			//for future user banning
			return;
		}

		MapPriceCUserTrade BuySideRequest = it->second.first.first;


		PairPricePubKeyCUserTrade pairPricePubKeyContainer = it->second.second;
		mpcut = pairPricePubKeyContainer.first;
		mucut = pairPricePubKeyContainer.second;

		if (mpcut.count(userTrade.nPrice))
		{
			mapUserTradeRequest[userTrade.nTradePairID].second.first[userTrade.nPrice].push_back(cut);
		}
		else
		{
			std::vector<CUserTrade*> vecCut;
			vecCut.push_back(cut);
			mapUserTradeRequest[userTrade.nTradePairID].second.first.insert(std::make_pair(userTrade.nPrice, vecCut));
		}
		if (mucut.count(userTrade.nUserPubKey))
		{
			mapUserTradeRequest[userTrade.nTradePairID].second.second[userTrade.nUserPubKey].push_back(cut);
		}
		else
		{
			std::vector<CUserTrade*> vecCut;
			vecCut.push_back(cut);
			mapUserTradeRequest[userTrade.nTradePairID].second.second.insert(std::make_pair(userTrade.nUserPubKey, vecCut));
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
		PairPricePubKeyCUserTrade sellSide = std::make_pair(mpcut, mucut);

		MapPriceCUserTrade mpcut2;
		MapPubKeyCUserTrade mucut2;
		PairPricePubKeyCUserTrade buySide = std::make_pair(mpcut2, mucut2);

		mapUserTradeRequest.insert(std::make_pair(userTrade.nTradePairID, std::make_pair(buySide, sellSide)));
	}
}

//to convert into enum return
void CUserTradeManager::UserBuyRequest(CUserTrade userTrade)
{
	int BuySideCoinID = tradePairManager.GetBidSideCoinInfoID(userTrade.nTradePairID);
	if (BuySideCoinID < 0)
		return;

	//to shift into if loop after remove else on actual implementation
	userTrade.nAmount = userTrade.nQuantity * userTrade.nPrice;
	CUserTrade* cut = new CUserTrade(userTrade);
	MapPriceCUserTrade mpcut;
	MapPubKeyCUserTrade mucut;

	std::map<int, PairBidAskCUserTrade>::iterator it = mapUserTradeRequest.find(userTrade.nTradePairID);
	if (it != mapUserTradeRequest.end())
	{
		userbalance_to_exchange_enum_t result = userBalanceManager.BalanceToExchange(BuySideCoinID, userTrade.nAmount);
		if (result != USERBALANCE_DEDUCTED)
		{
			//we should not be here
			//for future user banning
			return;
		}

		PairPricePubKeyCUserTrade pairPricePubKeyContainer = it->second.first;
		mpcut = pairPricePubKeyContainer.first;
		mucut = pairPricePubKeyContainer.second;

		if (mpcut.count(userTrade.nPrice))
		{
			mapUserTradeRequest[userTrade.nTradePairID].first.first[userTrade.nPrice].push_back(cut);
		}
		else
		{
			std::vector<CUserTrade*> vecCut;
			vecCut.push_back(cut);
			mapUserTradeRequest[userTrade.nTradePairID].first.first.insert(std::make_pair(userTrade.nPrice, vecCut));
		}
		if (mucut.count(userTrade.nUserPubKey))
		{
			mapUserTradeRequest[userTrade.nTradePairID].first.second[userTrade.nUserPubKey].push_back(cut);
		}
		else
		{
			std::vector<CUserTrade*> vecCut;
			vecCut.push_back(cut);
			mapUserTradeRequest[userTrade.nTradePairID].first.second.insert(std::make_pair(userTrade.nUserPubKey, vecCut));
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
		PairPricePubKeyCUserTrade buySide = std::make_pair(mpcut, mucut);

		MapPriceCUserTrade mpcut2;
		MapPubKeyCUserTrade mucut2;
		PairPricePubKeyCUserTrade sellSide = std::make_pair(mpcut2, mucut2);

		mapUserTradeRequest.insert(std::make_pair(userTrade.nTradePairID, std::make_pair(buySide, sellSide)));
	}
}