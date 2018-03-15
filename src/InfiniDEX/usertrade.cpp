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

	std::shared_ptr<CUserTrade> cut = std::make_shared<CUserTrade>(userTrade);
	MapPriceCUserTrade mpcut;
	MapPubKeyCUserTrade mucut;

	std::map<int, PairBidAskCUserTrade>::iterator it = mapUserTradeRequest.find(cut->nTradePairID);
	if (it != mapUserTradeRequest.end())
	{
		userbalance_to_exchange_enum_t result = userBalanceManager.BalanceToExchange(AskSideCoinID, cut->nUserPubKey, cut->nAmount);
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
			if (cut->nQuantity < 0)
			{
				//add code to buy back to match to 0
			}
			else if (cut->nQuantity == 0)
			{
				return;
			}

			if (itBuySideRequest->first >= cut->nPrice)
			{
				for (int i = 0; i < itBuySideRequest->second.size(); i++)
				{
					std::shared_ptr<CUserTrade> ExistingTrade = itBuySideRequest->second[i].second;
					if (cut->nBalanceQty <= ExistingTrade->nBalanceQty)
					{
						int qty = cut->nBalanceQty;
						ExistingTrade->nBalanceQty -= qty;
						cut->nBalanceQty -= qty;
						return;
					}
					else
					{
						int qty = ExistingTrade->nBalanceQty;
						ExistingTrade->nBalanceQty -= qty;
						cut->nBalanceQty -= qty;
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

		if (mpcut.count(cut->nPrice))
		{
			mapUserTradeRequest[cut->nTradePairID].second.first[cut->nPrice].push_back(std::make_pair(cut->nUserSignature, cut));
		}
		else
		{
			std::vector<PairSignatureUserTrade> vecCut;
			vecCut.push_back(std::make_pair(cut->nUserSignature, cut));
			mapUserTradeRequest[cut->nTradePairID].second.first.insert(std::make_pair(cut->nPrice, vecCut));
		}
		if (mucut.count(cut->nUserPubKey))
		{
			mapUserTradeRequest[cut->nTradePairID].second.second[cut->nUserPubKey].push_back(std::make_pair(cut->nUserSignature, cut));
		}
		else
		{
			std::vector<PairSignatureUserTrade> vecCut;
			vecCut.push_back(std::make_pair(cut->nUserSignature, cut));
			mapUserTradeRequest[cut->nTradePairID].second.second.insert(std::make_pair(cut->nUserPubKey, vecCut));
		}
	}
	else
	{
		//for testing purpose
		//to remove on actual implementation
		std::vector<PairSignatureUserTrade> vecCut;
		vecCut.push_back(std::make_pair(cut->nUserSignature, cut));
		mpcut.insert(std::make_pair(cut->nPrice, vecCut));
		mucut.insert(std::make_pair(cut->nUserPubKey, vecCut));
		PairPricePubKeyCUserTrade sellSide = std::make_pair(mpcut, mucut);

		MapPriceCUserTrade mpcut2;
		MapPubKeyCUserTrade mucut2;
		PairPricePubKeyCUserTrade buySide = std::make_pair(mpcut2, mucut2);

		mapUserTradeRequest.insert(std::make_pair(cut->nTradePairID, std::make_pair(buySide, sellSide)));
	}
}

//to convert into enum return
void CUserTradeManager::UserBuyRequest(CUserTrade userTrade)
{
	int BuySideCoinID = tradePairManager.GetBidSideCoinInfoID(userTrade.nTradePairID);
	if (BuySideCoinID < 0)
		return;

	//to shift into if loop after remove else on actual implementation	
	std::shared_ptr<CUserTrade> cut = std::make_shared<CUserTrade>(userTrade);
	MapPriceCUserTrade mpcut;
	MapPubKeyCUserTrade mucut;

	std::map<int, PairBidAskCUserTrade>::iterator it = mapUserTradeRequest.find(cut->nTradePairID);
	if (it != mapUserTradeRequest.end())
	{
		userbalance_to_exchange_enum_t result = userBalanceManager.BalanceToExchange(BuySideCoinID, cut->nUserPubKey, cut->nAmount);
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
			if (itSellSideRequest->first <= cut->nPrice)
			{
				for (int i = 0; i < itSellSideRequest->second.size(); i++)
				{
					std::shared_ptr<CUserTrade> ExistingTrade = itSellSideRequest->second[i].second;
					if (cut->nBalanceQty <= ExistingTrade->nBalanceQty)
					{
						int qty = cut->nBalanceQty;
						ExistingTrade->nBalanceQty -= qty;
						cut->nBalanceQty -= qty;
						return;
					}
					else
					{
						int qty = ExistingTrade->nBalanceQty;
						ExistingTrade->nBalanceQty -= qty;
						cut->nBalanceQty -= qty;
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

		if (mpcut.count(cut->nPrice))
		{
			mapUserTradeRequest[cut->nTradePairID].first.first[cut->nPrice].push_back(std::make_pair(cut->nUserSignature, cut));
		}
		else
		{
			std::vector<PairSignatureUserTrade> vecCut;
			vecCut.push_back(std::make_pair(cut->nUserSignature, cut));
			mapUserTradeRequest[cut->nTradePairID].first.first.insert(std::make_pair(cut->nPrice, vecCut));
		}
		if (mucut.count(cut->nUserPubKey))
		{
			mapUserTradeRequest[cut->nTradePairID].first.second[cut->nUserPubKey].push_back(std::make_pair(cut->nUserSignature, cut));
		}
		else
		{
			std::vector<PairSignatureUserTrade> vecCut;
			vecCut.push_back(std::make_pair(cut->nUserSignature, cut));
			mapUserTradeRequest[cut->nTradePairID].first.second.insert(std::make_pair(cut->nUserPubKey, vecCut));
		}
	}
	else
	{
		//for testing purpose
		//to remove on actual implementation
		std::vector<PairSignatureUserTrade> vecCut;
		vecCut.push_back(std::make_pair(cut->nUserSignature, cut));
		mpcut.insert(std::make_pair(cut->nPrice, vecCut));
		mucut.insert(std::make_pair(cut->nUserPubKey, vecCut));
		PairPricePubKeyCUserTrade buySide = std::make_pair(mpcut, mucut);

		MapPriceCUserTrade mpcut2;
		MapPubKeyCUserTrade mucut2;
		PairPricePubKeyCUserTrade sellSide = std::make_pair(mpcut2, mucut2);

		mapUserTradeRequest.insert(std::make_pair(cut->nTradePairID, std::make_pair(buySide, sellSide)));
	}
}