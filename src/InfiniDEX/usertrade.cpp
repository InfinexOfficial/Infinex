// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "actualtrade.h"
#include "orderbook.h"
#include "tradepair.h"
#include "userbalance.h"
#include "usertrade.h"

#include <boost/multiprecision/cpp_int.hpp>
#include <chrono>

class CUserTrade;
class CUserTradeManager;

std::map<int, PairBidAskCUserTrade> mapUserTradeRequest; //trade pair and bid ask data
CUserTradeManager userTradeManager;

uint64_t COrderBookManager::GetAdjustedTime()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

uint64_t CUserTradeManager::GetBidRequiredAmount(uint64_t Price, uint64_t Qty, int TradeFee)
{
	//overflow prevention
	boost::multiprecision::uint128_t amount = Price * Qty * 10000 / (10000 + TradeFee);
	return (uint64_t)amount;
}

uint64_t CUserTradeManager::GetAskExpectedAmount(uint64_t Price, uint64_t Qty, int TradeFee)
{
	//overflow prevention
	boost::multiprecision::uint128_t amount = Price * Qty * 10000 / (10000 - TradeFee);
	return (uint64_t)amount;
}

bool CUserTradeManager::IsSubmittedBidAmountValid(CUserTrade userTrade, int nTradeFee)
{	
	uint64_t ExpectedAmount = GetBidRequiredAmount(userTrade.nPrice, userTrade.nQuantity, nTradeFee);
	if ((ExpectedAmount - 1) <= userTrade.nAmount <= (ExpectedAmount + 1))
		return true;
	return false;
}

bool CUserTradeManager::IsSubmittedAskAmountValid(CUserTrade userTrade, int nTradeFee)
{
	uint64_t ExpectedAmount = GetAskExpectedAmount(userTrade.nPrice, userTrade.nQuantity, nTradeFee);
	if ((ExpectedAmount - 1) <= userTrade.nAmount <= (ExpectedAmount + 1))
		return true;
	return false;
}


//change to enum for more return info
bool CUserTradeManager::IsSubmittedBidValid(CUserTrade UserTrade, CTradePair TradePair)
{
	if (TradePair.nTradePairID != UserTrade.nTradePairID)
		return false;

	if (UserTrade.nAmount < TradePair.nMaximumTradeAmount)
		return false;

	if (UserTrade.nAmount > TradePair.nMaximumTradeAmount)
		return false;

	if (!IsSubmittedBidAmountValid(UserTrade, TradePair.nBidTradeFee))
		return false;	

	return true;
}

//change to enum for more return info
bool CUserTradeManager::IsSubmittedAskValid(CUserTrade UserTrade, CTradePair TradePair)
{
	if (TradePair.nTradePairID != UserTrade.nTradePairID)
		return false;

	if (UserTrade.nAmount < TradePair.nMaximumTradeAmount)
		return false;

	if (UserTrade.nAmount > TradePair.nMaximumTradeAmount)
		return false;

	if (!IsSubmittedAskAmountValid(UserTrade, TradePair.nAskTradeFee))
		return false;	

	return true;
}

//to convert into enum return
void CUserTradeManager::UserSellRequest(CUserTrade userTrade)
{
	CTradePair tradePair = tradePairManager.GetTradePair(userTrade.nTradePairID);
	if (tradePair.nTradePairID != userTrade.nTradePairID)
		return;

	std::shared_ptr<CUserTrade> cut = std::make_shared<CUserTrade>(userTrade);
	MapPriceCUserTrade mpcut;
	MapPubKeyCUserTrade mucut;

	std::map<int, PairBidAskCUserTrade>::iterator it = mapUserTradeRequest.find(cut->nTradePairID);
	if (it != mapUserTradeRequest.end())
	{
		if (!IsSubmittedAskValid(userTrade, tradePair))
			return;

		userbalance_to_exchange_enum_t result = userBalanceManager.BalanceToExchange(tradePair.nCoinInfoID1, cut->nUserPubKey, cut->nAmount);
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
			if (cut->nQuantity == 0)
			{
				return;
			}

			if (itBuySideRequest->first >= cut->nPrice)
			{
				for (int i = 0; i < itBuySideRequest->second.size(); i++)
				{
					std::shared_ptr<CUserTrade> ExistingTrade = itBuySideRequest->second[i].second;
					int qty = 0;
					if (cut->nBalanceQty <= ExistingTrade->nBalanceQty)
					{
						qty = cut->nBalanceQty;
					}
					else
					{
						qty = ExistingTrade->nBalanceQty;
					}

					if (qty <= 0)
					{
						//should not be here, let's think what should we do
					}

					ExistingTrade->nBalanceQty -= qty;
					cut->nBalanceQty -= qty;
					int bidTradeFee = (ExistingTrade->nTradeFee < tradePair.nBidTradeFee) ? ExistingTrade->nTradeFee : tradePair.nBidTradeFee;
					int askTradeFee = (cut->nTradeFee < tradePair.nAskTradeFee) ? cut->nTradeFee : tradePair.nAskTradeFee;
					uint64_t bidAmount = GetBidRequiredAmount(ExistingTrade->nPrice, qty, bidTradeFee);
					uint64_t askAmount = GetAskExpectedAmount(cut->nPrice, qty, askTradeFee);
					uint64_t actualAmount = cut->nPrice*qty;
					userBalanceManager.UpdateAfterTradeBalance(ExistingTrade->nUserPubKey, cut->nUserPubKey, tradePair.nCoinInfoID1, tradePair.nCoinInfoID2, -bidAmount, qty, -qty, askAmount);
					orderBookManager.AdjustAskQuantity(tradePair.nTradePairID, ExistingTrade->nPrice, qty);
					actualTradeManager.InputNewCompletedTrade(CActualTrade(ExistingTrade->nUserPubKey, cut->nUserPubKey, tradePair.nTradePairID, ExistingTrade->nPrice, qty, actualAmount, bidAmount - actualAmount, tradePair.nBidTradeFeeCoinID, actualAmount - askAmount, tradePair.nAskTradeFeeCoinID, "", GetAdjustedTime()));
					if (cut->nBalanceQty == 0)
						return;
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