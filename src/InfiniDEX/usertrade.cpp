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
	
	CUserTrade* cut = new CUserTrade(userTrade);
	MapPriceCUserTrade mpcut;
	MapPubKeyCUserTrade mucut;

	std::map<int, PairBidAskCUserTrade>::iterator it = mapUserTradeRequest.find(userTrade.nTradePairID);
	if (it != mapUserTradeRequest.end())
	{
		userbalance_to_exchange_enum_t result = userBalanceManager.BalanceToExchange(AskSideCoinID, userTrade.nAmount);
		if (result != USER_BALANCE_DEDUCTED)
		{
			//we should not be here
			//for future user banning
			return;
		}

		MapPriceCUserTrade buySideRequest = it->second.first.first;		
		MapPriceCUserTrade::reverse_iterator itBuySideRequest = buySideRequest.rbegin();
		while (itBuySideRequest != buySideRequest.rend())
		{
			if (userTrade.nQuantity < 0)
			{
				//add code to buy back to match to 0
			}
			else if (userTrade.nQuantity == 0)
			{
				return;
			}

			if (itBuySideRequest->first >= userTrade.nPrice)
			{
				for (int i = 0; i < itBuySideRequest->second.size(); i++)
				{
					CUserTrade* temp = itBuySideRequest->second[i].second;
					if (userTrade.nBalanceQty <= temp->nBalanceQty)
					{
						int qty = userTrade.nBalanceQty;
						temp->nBalanceQty -= qty;
						userTrade.nBalanceQty -= qty;
						return;
					}
					else
					{
						int qty = temp->nBalanceQty;
						temp->nBalanceQty -= qty;
						userTrade.nBalanceQty -= qty;
					}
				}
				++itBuySideRequest;
			}
			else
			{
				break;
			}
		}

		PairPricePubKeyCUserTrade sellSidePairPricePubKeyContainer = it->second.second;
		mpcut = sellSidePairPricePubKeyContainer.first;
		mucut = sellSidePairPricePubKeyContainer.second;

		if (mpcut.count(userTrade.nPrice))
		{
			mapUserTradeRequest[userTrade.nTradePairID].second.first[userTrade.nPrice].push_back(std::make_pair(userTrade.nUserSignature, cut));
		}
		else
		{
			std::vector<PairSignatureUserTrade> vecCut;
			vecCut.push_back(std::make_pair(userTrade.nUserSignature, cut));
			mapUserTradeRequest[userTrade.nTradePairID].second.first.insert(std::make_pair(userTrade.nPrice, vecCut));
		}
		if (mucut.count(userTrade.nUserPubKey))
		{
			mapUserTradeRequest[userTrade.nTradePairID].second.second[userTrade.nUserPubKey].push_back(std::make_pair(userTrade.nUserSignature, cut));
		}
		else
		{
			std::vector<PairSignatureUserTrade> vecCut;
			vecCut.push_back(std::make_pair(userTrade.nUserSignature, cut));
			mapUserTradeRequest[userTrade.nTradePairID].second.second.insert(std::make_pair(userTrade.nUserPubKey, vecCut));
		}
	}
	else
	{
		//for testing purpose
		//to remove on actual implementation
		std::vector<PairSignatureUserTrade> vecCut;
		vecCut.push_back(std::make_pair(userTrade.nUserSignature, cut));
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
	CUserTrade* cut = new CUserTrade(userTrade);
	MapPriceCUserTrade mpcut;
	MapPubKeyCUserTrade mucut;

	std::map<int, PairBidAskCUserTrade>::iterator it = mapUserTradeRequest.find(userTrade.nTradePairID);
	if (it != mapUserTradeRequest.end())
	{
		userbalance_to_exchange_enum_t result = userBalanceManager.BalanceToExchange(BuySideCoinID, userTrade.nAmount);
		if (result != USER_BALANCE_DEDUCTED)
		{
			//we should not be here
			//for future user banning
			return;
		}

		MapPriceCUserTrade sellSideRequest = it->second.first.first;
		MapPriceCUserTrade::iterator itSellSideRequest = sellSideRequest.begin();
		while (itSellSideRequest != sellSideRequest.end())
		{
			if (itSellSideRequest->first <= userTrade.nPrice)
			{
				for (int i = 0; i < itSellSideRequest->second.size(); i++)
				{
					CUserTrade* temp = itSellSideRequest->second[i].second;
					if (userTrade.nBalanceQty <= temp->nBalanceQty)
					{
						int qty = userTrade.nBalanceQty;
						temp->nBalanceQty -= qty;
						userTrade.nBalanceQty -= qty;
						return;
					}
					else
					{
						int qty = temp->nBalanceQty;
						temp->nBalanceQty -= qty;
						userTrade.nBalanceQty -= qty;
					}
				}
				++itSellSideRequest;
			}
			else
			{
				break;
			}
		}

		PairPricePubKeyCUserTrade buySidePairPricePubKeyContainer = it->second.first;
		mpcut = buySidePairPricePubKeyContainer.first;
		mucut = buySidePairPricePubKeyContainer.second;

		if (mpcut.count(userTrade.nPrice))
		{
			mapUserTradeRequest[userTrade.nTradePairID].first.first[userTrade.nPrice].push_back(std::make_pair(userTrade.nUserSignature, cut));
		}
		else
		{
			std::vector<PairSignatureUserTrade> vecCut;
			vecCut.push_back(std::make_pair(userTrade.nUserSignature, cut));
			mapUserTradeRequest[userTrade.nTradePairID].first.first.insert(std::make_pair(userTrade.nPrice, vecCut));
		}
		if (mucut.count(userTrade.nUserPubKey))
		{
			mapUserTradeRequest[userTrade.nTradePairID].first.second[userTrade.nUserPubKey].push_back(std::make_pair(userTrade.nUserSignature, cut));
		}
		else
		{
			std::vector<PairSignatureUserTrade> vecCut;
			vecCut.push_back(std::make_pair(userTrade.nUserSignature, cut));
			mapUserTradeRequest[userTrade.nTradePairID].first.second.insert(std::make_pair(userTrade.nUserPubKey, vecCut));
		}
	}
	else
	{
		//for testing purpose
		//to remove on actual implementation
		std::vector<PairSignatureUserTrade> vecCut;
		vecCut.push_back(std::make_pair(userTrade.nUserSignature, cut));
		mpcut.insert(std::make_pair(userTrade.nPrice, vecCut));
		mucut.insert(std::make_pair(userTrade.nUserPubKey, vecCut));
		PairPricePubKeyCUserTrade buySide = std::make_pair(mpcut, mucut);

		MapPriceCUserTrade mpcut2;
		MapPubKeyCUserTrade mucut2;
		PairPricePubKeyCUserTrade sellSide = std::make_pair(mpcut2, mucut2);

		mapUserTradeRequest.insert(std::make_pair(userTrade.nTradePairID, std::make_pair(buySide, sellSide)));
	}
}