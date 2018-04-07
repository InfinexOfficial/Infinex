// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "trade.h"
#include "chartdata.h"
#include "orderbook.h"
#include "tradepair.h"
#include "userbalance.h"
#include "usertradehistory.h"

#include <boost/multiprecision/cpp_int.hpp>
#include <chrono>

class CUserTrade;
class CUserTradeSetting;
class CUserTradeManager;

class CActualTrade;
class CActualTradeSetting;
class CActualTradeManager;

std::map<std::string, pULTIUTC> mapUserTrades;

std::map<int, mUTPIUTV> mapBidUserTradeByPrice;
std::map<int, mUTPIUTV> mapAskUserTradeByPrice;

std::map<int, mUTPKUTV> mapBidUserTradeByPubkey;
std::map<int, mUTPKUTV> mapAskUserTradeByPubkey;

std::map<int, CUserTradeSetting> mapUserTradeSetting;
std::map<int, std::set<std::string>> mapUserTradeHash;

std::map<std::string, mUTImAT> mapUserActualTrades;
std::map<int, mATIAT> mapActualTradeByActualTradeID;
std::map<int, mUTImAT> mapActualTradeByUserTradeID;
std::map<int, std::vector<CActualTrade>> mapConflictTrade;
std::map<int, CActualTradeSetting> mapActualTradeSetting;
std::map<int, std::set<std::string>> mapActualTradeHash;
CActualTradeManager actualTradeManager;
CUserTradeManager userTradeManager;

void CUserTradeManager::InitTradeCancelRequest(CCancelTrade& cancelTrade)
{
	if (!cancelTrade.VerifyUserSignature())
		return;

	CUserTradeSetting& setting = mapUserTradeSetting[cancelTrade.nTradePairID];
	if (setting.nTradePairID != cancelTrade.nTradePairID)
		return;

	if (cancelTrade.nMNProcessTime > 0)
	{
		if (setting.nIsInChargeOfProcessUserTrade)
			ReturnTradeCancelBalance(cancelTrade);

		if (setting.nInChargeOfBidBroadcast && cancelTrade.isBid)
		{
			orderBookManager.AdjustBidQuantity(cancelTrade.nTradePairID, cancelTrade.nPrice, cancelTrade.nBalanceQty);
			orderBookManager.BroadcastBidOrder(cancelTrade.nTradePairID, cancelTrade.nPrice);
		}

		if (setting.nInChargeOfAskBroadcast && !cancelTrade.isBid)
		{
			orderBookManager.AdjustAskQuantity(cancelTrade.nTradePairID, cancelTrade.nPrice, cancelTrade.nBalanceQty);
			orderBookManager.BroadcastAskOrder(cancelTrade.nTradePairID, cancelTrade.nPrice);
		}
	}
	else
	{
		if (setting.nIsInChargeOfMatchUserTrade)
			ProcessTradeCancelRequest(cancelTrade);
	}
}

void CUserTradeManager::ProcessTradeCancelRequest(CCancelTrade& cancelTrade)
{
	std::shared_ptr<CUserTrade> existingUserTrade;
	if (cancelTrade.isBid)
	{
		mINTUT& temp = mapBidUserTradeByPubkey[cancelTrade.nTradePairID][cancelTrade.nUserPubKey];
		existingUserTrade = temp[cancelTrade.nUserTradeID];
		if (existingUserTrade->nUserTradeID != cancelTrade.nUserTradeID)
			return;
		temp.erase(cancelTrade.nUserTradeID);
		mapBidUserTradeByPrice[cancelTrade.nTradePairID][existingUserTrade->nPrice].erase(cancelTrade.nUserTradeID);
	}
	else
	{
		mINTUT& temp = mapAskUserTradeByPubkey[cancelTrade.nTradePairID][cancelTrade.nUserPubKey];
		existingUserTrade = temp[cancelTrade.nUserTradeID];
		if (existingUserTrade->nUserTradeID != cancelTrade.nUserTradeID)
			return;
		temp.erase(cancelTrade.nUserTradeID);
		mapAskUserTradeByPrice[cancelTrade.nTradePairID][existingUserTrade->nPrice].erase(cancelTrade.nUserTradeID);
	}
	
	mapUserTradeHash[cancelTrade.nTradePairID].erase(existingUserTrade->nUserHash);
	cancelTrade.nBalanceQty = existingUserTrade->nBalanceQty;
	cancelTrade.nBalanceAmount = existingUserTrade->nBalanceAmount;
	cancelTrade.nMNProcessTime = GetAdjustedTime();
	if (!cancelTrade.MNSign())
		return;
}

void CUserTradeManager::ReturnTradeCancelBalance(CCancelTrade& cancelTrade)
{
	if (!cancelTrade.VerifyMNSignature())
		return;

	CTradePair tradePair = tradePairManager.GetTradePair(cancelTrade.nTradePairID);
	if (tradePair.nTradePairID != cancelTrade.nTradePairID)
		return;

	std::shared_ptr<CUserTrade> existingUserTrade;
	if (cancelTrade.isBid)
	{
		mINTUT& temp = mapBidUserTradeByPubkey[cancelTrade.nTradePairID][cancelTrade.nUserPubKey];
		existingUserTrade = temp[cancelTrade.nUserTradeID];
		if (existingUserTrade->nUserTradeID != cancelTrade.nUserTradeID)
			return;
		if (existingUserTrade->nBalanceAmount != cancelTrade.nBalanceAmount)
			return;
		temp.erase(cancelTrade.nUserTradeID);
		mapBidUserTradeByPrice[cancelTrade.nTradePairID][existingUserTrade->nPrice].erase(cancelTrade.nUserTradeID);
		userBalanceManager.ExchangeToBalance(tradePair.nCoinID2, cancelTrade.nUserPubKey, cancelTrade.nBalanceAmount);
	}
	else
	{
		mINTUT& temp = mapAskUserTradeByPubkey[cancelTrade.nTradePairID][cancelTrade.nUserPubKey];
		existingUserTrade = temp[cancelTrade.nUserTradeID];
		if (existingUserTrade->nUserTradeID != cancelTrade.nUserTradeID)
			return;
		if (existingUserTrade->nBalanceQty != cancelTrade.nBalanceQty)
			return;
		temp.erase(cancelTrade.nUserTradeID);
		mapAskUserTradeByPrice[cancelTrade.nTradePairID][existingUserTrade->nPrice].erase(cancelTrade.nUserTradeID);
		userBalanceManager.ExchangeToBalance(tradePair.nCoinID1, cancelTrade.nUserPubKey, cancelTrade.nBalanceQty);
	}
}

void CUserTradeManager::InputUserTrade(const std::shared_ptr<CUserTrade>& userTrade)
{
	if (userTrade->nQuantity <= 0)
		return;

	if (!userTrade->VerifyUserSignature())
		return;

	CTradePair& tradePair = mapCompleteTradePair[userTrade->nTradePairID];
	if (tradePair.nTradePairID != userTrade->nTradePairID)
	{
		//request node setup
		return;
	}

	if (userBalanceManager.InChargeOfUserBalance(userTrade->nUserPubKey))
		if (userTrade->nMNBalancePubKey == "")
			if (!ProcessUserTradeRequest(userTrade, tradePair))
				return;

	if (userTrade->nMNBalancePubKey == "" || !userTrade->VerifyMNBalanceSignature())
		return;

	if (userBalanceManager.InChargeOfUserBalanceBackup(userTrade->nUserPubKey))
		SaveProcessedUserTrade(userTrade, tradePair);

	CUserTradeSetting& setting = mapUserTradeSetting[userTrade->nTradePairID];
	if (setting.nTradePairID != userTrade->nTradePairID)
		return;

	if (userTrade->nMNTradePubKey == "" && setting.nIsInChargeOfMatchUserTrade)
	{
		if (userTrade->nIsBid)
			InputMatchUserBuyRequest(userTrade, tradePair);
		else
			InputMatchUserSellRequest(userTrade, tradePair);
	}

	if (userTrade->nMNTradePubKey == "" || !userTrade->VerifyMNTradeSignature())
		return;

	//need to add 1 more check whether we process this trade request into order book before
	if (setting.nInChargeOfBidBroadcast && userTrade->nIsBid) 
	{
		orderBookManager.AdjustBidQuantity(tradePair.nTradePairID, userTrade->nPrice, userTrade->nBalanceQty);
	}
	if (setting.nInChargeOfAskBroadcast && !userTrade->nIsBid)
	{		
		orderBookManager.AdjustAskQuantity(tradePair.nTradePairID, userTrade->nPrice, userTrade->nBalanceQty);
	}
}

bool CUserTradeManager::ProcessUserTradeRequest(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& tradePair)
{
	int minTradeFee = (userTrade->nIsBid) ? tradePair.nBidTradeFee : tradePair.nAskTradeFee;
	if (minTradeFee > userTrade->nTradeFee)
		return false;

	if (IsUserTradeInList(userTrade->nTradePairID, userTrade->nUserHash))
		return false;

	if (userTrade->nIsBid)
	{
		if (!IsSubmittedBidAmountValid(userTrade, tradePair.nBidTradeFee))
			return false;
	}
	else
	{
		if (!IsSubmittedAskAmountValid(userTrade, tradePair.nBidTradeFee))
			return false;
	}

	int timeDiff = GetAdjustedTime() - userTrade->nTimeSubmit;
	if (timeDiff > 10000 || timeDiff < -10000)
		return false;

	std::shared_ptr<CUserBalance> userBalance;
	userBalanceManager.InitUserBalance(tradePair.nCoinID2, userTrade->nUserPubKey, userBalance);
	if (userBalance->nAvailableBalance < userTrade->nAmount)
		return false;

	userBalance->nAvailableBalance -= userTrade->nAmount;
	userBalance->nInExchangeBalance += userTrade->nAmount;

	if (!mapUserTrades.count(userTrade->nUserPubKey))
		mapUserTrades.insert(std::make_pair(userTrade->nUserPubKey, pULTIUTC()));

	auto& a = mapUserTrades[userTrade->nUserPubKey];
	userTrade->nUserTradeID = a.first + 1;
	userTrade->nBalanceAmount = userTrade->nAmount;
	userTrade->nBalanceQty = userTrade->nQuantity;
	userTrade->nMNBalancePubKey = ""; //to replace with actual MN pub key
	userTrade->nLastUpdate = GetAdjustedTime();
	if (!userTrade->MNBalanceSign())
		return false;
	a.second.insert(std::make_pair(userTrade->nUserTradeID, userTrade));
	AddToUserTradeList(userTrade->nTradePairID, userTrade->nUserHash);
	return true;
}

void CUserTradeManager::SaveProcessedUserTrade(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& tradePair)
{
	std::shared_ptr<CUserBalance> userBalance;
	userBalanceManager.InitUserBalance(tradePair.nCoinID2, userTrade->nUserPubKey, userBalance);
	userBalance->nAvailableBalance -= userTrade->nAmount;
	userBalance->nInExchangeBalance += userTrade->nAmount;

	if (!mapUserTrades.count(userTrade->nUserPubKey))
		mapUserTrades.insert(std::make_pair(userTrade->nUserPubKey, pULTIUTC()));	
	mapUserTrades[userTrade->nUserPubKey].second.insert(std::make_pair(userTrade->nUserTradeID, userTrade));
	AddToUserTradeList(userTrade->nTradePairID, userTrade->nUserHash);
}

void CUserTradeManager::InputMatchUserBuyRequest(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& tradePair)
{
	CActualTradeSetting& actualTradeSetting = mapActualTradeSetting[userTrade->nTradePairID];
	if (actualTradeSetting.nTradePairID != userTrade->nTradePairID)
		return;

	mUTPIUTV::iterator Sellers = mapAskUserTradeByPrice[userTrade->nTradePairID].begin();
	while (Sellers != mapAskUserTradeByPrice[userTrade->nTradePairID].end() && Sellers->first <= userTrade->nPrice)
	{
		mINTUT::iterator Seller = Sellers->second.begin();
		while (Seller != Sellers->second.end())
		{
			std::shared_ptr<CUserTrade> ExistingTrade = Seller->second;
			if (ExistingTrade->nBalanceQty <= 0)
				continue;

			int qty = 0;
			if (userTrade->nBalanceQty <= ExistingTrade->nBalanceQty)
				qty = userTrade->nBalanceQty;
			else
				qty = ExistingTrade->nBalanceQty;

			int bidTradeFee = (userTrade->nTradeFee < tradePair.nAskTradeFee) ? userTrade->nTradeFee : tradePair.nAskTradeFee;
			int askTradeFee = (ExistingTrade->nTradeFee < tradePair.nBidTradeFee) ? ExistingTrade->nTradeFee : tradePair.nBidTradeFee;
			uint64_t bidAmount = GetBidRequiredAmount(userTrade->nPrice, qty, bidTradeFee);
			uint64_t askAmount = GetAskExpectedAmount(ExistingTrade->nPrice, qty, askTradeFee);
			uint64_t tradeTime = GetAdjustedTime();

			std::shared_ptr<CActualTrade> actualTrade = std::make_shared<CActualTrade>(tradePair.nTradePairID, userTrade->nUserTradeID, ExistingTrade->nUserTradeID, userTrade->nPrice, qty, bidAmount, askAmount, userTrade->nUserPubKey, ExistingTrade->nUserPubKey, bidTradeFee, askTradeFee, GetAdjustedTime());			
			if (!actualTradeManager.GenerateActualTrade(actualTrade, actualTradeSetting))
				return;

			ExistingTrade->nBalanceQty -= qty;
			userTrade->nBalanceQty -= qty;
			actualTradeManager.InputActualTrade(actualTrade, actualTradeSetting, tradePair);
			if (actualTradeSetting.nInChargeOfAskBroadcast)
				orderBookManager.AdjustAskQuantity(actualTrade->nTradePairID, actualTrade->nTradePrice, (0 - actualTrade->nTradeQty));

			if (userTrade->nBalanceQty <= 0)
				return;

			++Seller;
		}
		++Sellers;
	}
}

void CUserTradeManager::InputMatchUserSellRequest(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& tradePair)
{
	mUTPIUTV::reverse_iterator Buyers = mapBidUserTradeByPrice[userTrade->nTradePairID].rbegin();
	while (Buyers != mapBidUserTradeByPrice[userTrade->nTradePairID].rend() && Buyers->first >= userTrade->nPrice)
	{
		mINTUT::iterator Buyer = Buyers->second.begin();
		while (Buyer != Buyers->second.end())
		{
			std::shared_ptr<CUserTrade> ExistingTrade = Buyer->second;
			if (ExistingTrade->nBalanceQty <= 0)
				continue;

			int qty = 0;
			if (userTrade->nBalanceQty <= ExistingTrade->nBalanceQty)
				qty = userTrade->nBalanceQty;
			else
				qty = ExistingTrade->nBalanceQty;

			int bidTradeFee = (ExistingTrade->nTradeFee < tradePair.nBidTradeFee) ? ExistingTrade->nTradeFee : tradePair.nBidTradeFee;
			int askTradeFee = (userTrade->nTradeFee < tradePair.nAskTradeFee) ? userTrade->nTradeFee : tradePair.nAskTradeFee;
			uint64_t bidAmount = GetBidRequiredAmount(ExistingTrade->nPrice, qty, bidTradeFee);
			uint64_t askAmount = GetAskExpectedAmount(userTrade->nPrice, qty, askTradeFee);			
			uint64_t tradeTime = GetAdjustedTime();
			
			std::shared_ptr<CActualTrade> actualTrade = std::make_shared<CActualTrade>(tradePair.nTradePairID, ExistingTrade->nUserTradeID, userTrade->nUserTradeID, userTrade->nPrice, qty, bidAmount, askAmount, ExistingTrade->nUserPubKey, userTrade->nUserPubKey, bidTradeFee, askTradeFee, GetAdjustedTime());
			CActualTradeSetting& actualTradeSetting = mapActualTradeSetting[userTrade->nTradePairID];
			if (actualTradeSetting.nTradePairID != userTrade->nTradePairID)
				return;
			if (!actualTradeManager.GenerateActualTrade(actualTrade, actualTradeSetting))
				return;

			ExistingTrade->nBalanceQty -= qty;
			userTrade->nBalanceQty -= qty;			
			actualTradeManager.InputActualTrade(actualTrade, actualTradeSetting, tradePair);
			if (actualTradeSetting.nInChargeOfBidBroadcast)
				orderBookManager.AdjustBidQuantity(actualTrade->nTradePairID, actualTrade->nTradePrice, (0 - actualTrade->nTradeQty));

			if (userTrade->nBalanceQty <= 0)
				return;

			++Buyer;
		}
		++Buyers;
	}
}

bool CActualTradeManager::GenerateActualTrade(std::shared_ptr<CActualTrade> actualTrade, CActualTradeSetting& actualTradeSetting)
{
	actualTrade->nActualTradeID = (actualTradeSetting.nLastActualTradeID + 1);
	actualTrade->nMasternodeInspector = actualTradeSetting.nMNPubKey;
	actualTradeSetting.nLastActualTradeTime = actualTrade->nTradeTime;
	actualTrade->nCurrentHash = actualTrade->GetHash();
	if (mapActualTradeHash[actualTrade->nTradePairID].count(actualTrade->nCurrentHash))
		return false;
	if (!actualTrade->Sign())
		return false;
	++actualTradeSetting.nLastActualTradeID;
	return true;
}

bool CActualTradeManager::InputActualTrade(std::shared_ptr<CActualTrade> actualTrade, CActualTradeSetting& setting, CTradePair& tradePair)
{
	mapActualTradeHash[actualTrade->nTradePairID].insert(actualTrade->GetHash());
	std::pair<int, std::shared_ptr<CActualTrade>> temp(std::make_pair(actualTrade->nActualTradeID, actualTrade));
	mapActualTradeByActualTradeID[actualTrade->nTradePairID].insert(temp);

	auto& a = mapActualTradeByUserTradeID[actualTrade->nTradePairID];
	if (a.count(actualTrade->nUserTrade1))
		a[actualTrade->nUserTrade1].insert(temp);
	else
	{
		mATIAT temp2;
		temp2.insert(temp);
		a.insert(std::make_pair(actualTrade->nUserTrade1, temp2));
	}

	if (a.count(actualTrade->nUserTrade2))
		a[actualTrade->nUserTrade2].insert(temp);
	else
	{
		mATIAT temp2;
		temp2.insert(temp);
		a.insert(std::make_pair(actualTrade->nUserTrade2, temp2));
	}
	
	if (setting.nIsInChargeOfChartData)
		ChartDataManager.InputNewTrade(actualTrade->nTradePairID, actualTrade->nTradePrice, actualTrade->nTradeQty, actualTrade->nTradeTime);
	if (setting.nIsInChargeOfMarketTradeHistory)
		userTradeHistoryManager.InputNewUserTradeHistory(CUserTradeHistory(actualTrade->nTradePairID, actualTrade->nUserPubKey1, actualTrade->nUserPubKey2, actualTrade->nTradePrice, actualTrade->nTradeQty, actualTrade->nTradeAmount, false, actualTrade->nTradeTime));
	
	actualTrade->Relay();
	return true;
}

void CActualTradeManager::InputActualTrade(std::shared_ptr<CActualTrade> actualTrade)
{
	if (!actualTrade->VerifySignature())
		return;

	CTradePair& tradePair = mapCompleteTradePair[actualTrade->nTradePairID];
	if (tradePair.nTradePairID != actualTrade->nTradePairID)
	{
		//request node setup
		return;
	}

	CActualTradeSetting& setting = mapActualTradeSetting[actualTrade->nTradePairID];
	if (setting.nTradePairID != actualTrade->nTradePairID)
		return;

	if (userBalanceManager.InChargeOfUserBalance(actualTrade->nUserPubKey1) || userBalanceManager.InChargeOfUserBalanceBackup(actualTrade->nUserPubKey1))
		userBalanceManager.UpdateAfterTradeBalance(actualTrade->nUserPubKey1, tradePair.nCoinID2, tradePair.nCoinID1, actualTrade->nBidAmount, actualTrade->nTradeQty);
	if (userBalanceManager.InChargeOfUserBalance(actualTrade->nUserPubKey2) || userBalanceManager.InChargeOfUserBalanceBackup(actualTrade->nUserPubKey2))
		userBalanceManager.UpdateAfterTradeBalance(actualTrade->nUserPubKey2, tradePair.nCoinID1, tradePair.nCoinID2, actualTrade->nTradeQty, actualTrade->nAskAmount);
	if (setting.nIsInChargeOfChartData)
		ChartDataManager.InputNewTrade(actualTrade->nTradePairID, actualTrade->nTradePrice, actualTrade->nTradeQty, actualTrade->nTradeTime);
	if (setting.nIsInChargeOfMarketTradeHistory)
		userTradeHistoryManager.InputNewUserTradeHistory(CUserTradeHistory(actualTrade->nTradePairID, actualTrade->nUserPubKey1, actualTrade->nUserPubKey2, actualTrade->nTradePrice, actualTrade->nTradeQty, actualTrade->nTradeAmount, false, actualTrade->nTradeTime));
}

bool CActualTradeManager::InputActualTradeFromNode(std::shared_ptr<CActualTrade> actualTrade, CActualTradeSetting& setting, CTradePair& tradePair)
{
	if (!actualTrade->VerifySignature())
		return false;

	if (tradePair.nTradePairID != actualTrade->nTradePairID)
		return false;

	if (setting.nTradePairID != actualTrade->nTradePairID)
		return false;

	if (IsActualTradeInList(actualTrade->nTradePairID, actualTrade->nActualTradeID, actualTrade->nCurrentHash))
		return false;

	int sequence = actualTrade->nActualTradeID - setting.nLastActualTradeID;
	if (sequence <= 0)
		return false;

	if (sequence > 1)
	{
		//need to do something more here
		return false;
	}

	if (mapActualTradeHash[actualTrade->nTradePairID].count(actualTrade->GetHash()))
		return false;

	userTradeManager.ReduceBalanceQty(actualTrade->nTradePairID, actualTrade->nUserTrade1, actualTrade->nUserTrade2, actualTrade->nTradeQty);
	if (!InputActualTrade(actualTrade, setting, tradePair))
		return false;

	return true;
}

uint64_t CActualTradeManager::GetTotalTradedQuantity(int TradePairID, int UserTradeID)
{
	mATIAT* temp = &mapActualTradeByUserTradeID[TradePairID][UserTradeID];
	mATIAT::iterator it = temp->begin();
	uint64_t qty = 0;
	while (it != temp->end())
	{
		qty += it->second->nTradeQty;
		it++;
	}
	return qty;
}

void CActualTradeManager::InputNewTradePair(int TradePairID)
{
	if (!mapActualTradeByActualTradeID.count(TradePairID))
	{
		mapActualTradeByActualTradeID.insert(std::make_pair(TradePairID, mATIAT()));
		mapActualTradeByUserTradeID.insert(std::make_pair(TradePairID, mUTImAT()));
		mapConflictTrade.insert(std::make_pair(TradePairID, std::vector<CActualTrade>()));
		mapActualTradeSetting.insert(std::make_pair(TradePairID, CActualTradeSetting(TradePairID, "")));
		mapActualTradeHash.insert(std::make_pair(TradePairID, std::set<std::string>()));
	}
}

void CUserTradeManager::AssignBidBroadcastRole(int TradePairID)
{
	InitTradePair(TradePairID);
	orderBookManager.InitTradePair(TradePairID);
	mapUserTradeSetting[TradePairID].nInChargeOfBidBroadcast = true;
	//request complete data from other node
}

void CUserTradeManager::AssignAskBroadcastRole(int TradePairID)
{
	InitTradePair(TradePairID);
	orderBookManager.InitTradePair(TradePairID);
	mapUserTradeSetting[TradePairID].nInChargeOfAskBroadcast = true;
	//request complete data from other node
}

void CUserTradeManager::InitTradePair(int TradePairID)
{
	if (mapUserTradeSetting.count(TradePairID))
		return;

	mapBidUserTradeByPrice.insert(std::make_pair(TradePairID, mUTPIUTV()));
	mapAskUserTradeByPrice.insert(std::make_pair(TradePairID, mUTPIUTV()));
	mapBidUserTradeByPubkey.insert(std::make_pair(TradePairID, mUTPKUTV()));
	mapAskUserTradeByPubkey.insert(std::make_pair(TradePairID, mUTPKUTV()));
	mapUserTradeSetting.insert(std::make_pair(TradePairID, CUserTradeSetting(TradePairID, "")));
	mapUserTradeHash.insert(std::make_pair(TradePairID, std::set<std::string>()));
}

bool CUserTradeManager::AssignNodeToProcessUserTrade(int TradePairID, bool toAssign)
{
	if (!mapUserTradeSetting.count(TradePairID))
		return false;

	mapUserTradeSetting[TradePairID].nIsInChargeOfProcessUserTrade = toAssign;
	return true;
}

bool CUserTradeManager::AssignNodeToMatchUserTrade(int TradePairID, bool toAssign)
{
	if (!mapUserTradeSetting.count(TradePairID))
		return false;

	mapUserTradeSetting[TradePairID].nIsInChargeOfMatchUserTrade = toAssign;
	return true;
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

bool CUserTradeManager::IsSubmittedBidAmountValid(const std::shared_ptr<CUserTrade>& userTrade, int nTradeFee)
{
	uint64_t ExpectedAmount = GetBidRequiredAmount(userTrade->nPrice, userTrade->nQuantity, nTradeFee);
	if ((ExpectedAmount - 1) <= userTrade->nAmount <= (ExpectedAmount + 1))
		return true;
	return false;
}

bool CUserTradeManager::IsSubmittedAskAmountValid(const std::shared_ptr<CUserTrade>& userTrade, int nTradeFee)
{
	uint64_t ExpectedAmount = GetAskExpectedAmount(userTrade->nPrice, userTrade->nQuantity, nTradeFee);
	if ((ExpectedAmount - 1) <= userTrade->nAmount <= (ExpectedAmount + 1))
		return true;
	return false;
}

//change to enum for more return info
bool CUserTradeManager::IsSubmittedBidValid(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& TradePair)
{
	if (userTrade->nAmount < TradePair.nMinimumTradeAmount)
		return false;

	if (userTrade->nAmount > TradePair.nMaximumTradeAmount)
		return false;

	if (!IsSubmittedBidAmountValid(userTrade, TradePair.nBidTradeFee))
		return false;

	return true;
}

//change to enum for more return info
bool CUserTradeManager::IsSubmittedAskValid(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& TradePair)
{
	if (userTrade->nAmount < TradePair.nMaximumTradeAmount)
		return false;

	if (userTrade->nAmount > TradePair.nMaximumTradeAmount)
		return false;

	if (!IsSubmittedAskAmountValid(userTrade, TradePair.nAskTradeFee))
		return false;

	return true;
}

bool CUserTradeManager::IsUserTradeInList(int TradePairID, std::string UserHash)
{
	return mapUserTradeHash[TradePairID].count(UserHash);
}

void CUserTradeManager::AddToUserTradeList(int TradePairID, std::string UserHash)
{
	mapUserTradeHash[TradePairID].insert(UserHash);
}

bool CUserTradeSetting::IsValidSubmissionTimeAndUpdate(uint64_t time)
{
	int diff = nLastUserTradeTime - time;
	if (diff > nMaxSubmissionTimeDiff)
		return false;

	nLastUserTradeTime = time;
	return true;
}

std::string CActualTrade::GetHash()
{
	return "";
}

bool CActualTrade::Sign()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nTradePrice) +
		boost::lexical_cast<std::string>(nTradeQty) + boost::lexical_cast<std::string>(nTradeAmount) + nUserPubKey1 + nUserPubKey2 + boost::lexical_cast<std::string>(nFee1) +
		boost::lexical_cast<std::string>(nFee2) + nMasternodeInspector + boost::lexical_cast<std::string>(nTradeTime);
	return true;
}

bool CActualTrade::VerifySignature()
{
	return true;
}

bool CActualTrade::Relay()
{
	if (!Sign())
	{
		return false;
	}
	return true;
}

bool CUserTradeManager::ReduceBalanceQty(int TradePairID, int UserTradeID1, int UserTradeID2, uint64_t Qty)
{
	return true;
}

int64_t CUserTradeManager::GetBalanceAmount(int TradePairID, uint64_t Price, int UserTradeID)
{
	return 0;
}

bool CActualTradeSetting::IsValidSubmissionTimeAndUpdate(uint64_t time)
{
	return true;
}

void CActualTradeManager::AssignUserHistoryProviderRole(int TradePairID)
{

}

void CActualTradeManager::AssignMarketHistoryProviderRole(int TradePairID)
{

}

void CActualTradeManager::AssignChartDataProviderRole(int TradePairID)
{

}

void CActualTradeManager::AssignTradeProcessorRole(int TradePairID)
{

}

bool CActualTrade::InformConflictTrade(CNode* node)
{
	return true;
}

bool CActualTradeManager::IsActualTradeInList(int TradePairID, int ActualTradeID, std::string Hash)
{
	return true;
}

bool CUserTrade::VerifyUserSignature()
{
	return true;
}

bool CUserTrade::VerifyMNBalanceSignature()
{
	return true;
}

bool CUserTrade::VerifyMNTradeSignature()
{
	return true;
}

bool CUserTrade::MNBalanceSign()
{
	return true;
}

bool CUserTrade::MNTradeSign()
{
	return true;
}

void CUserTrade::RelayTo(CUserConnection& conn)
{

}

void CUserTrade::RelayToHandler()
{

}

uint64_t CUserTradeManager::GetAdjustedTime()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

bool CCancelTrade::VerifyUserSignature()
{
	return true;
}

bool CCancelTrade::VerifyMNSignature()
{
	return true;
}

bool CCancelTrade::MNSign()
{
	return true;
}