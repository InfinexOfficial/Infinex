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

std::map<int, mATIAT> mapActualTradeByActualTradeID;
std::map<int, mUTImAT> mapActualTradeByUserTradeID;
std::map<int, std::vector<CActualTrade>> mapConflictTrade;
std::map<int, CActualTradeSetting> mapActualTradeSetting;
std::map<int, std::set<std::string>> mapActualTradeHash;
CActualTradeManager actualTradeManager;
CUserTradeManager userTradeManager;

bool CUserTradeSetting::IsValidSubmissionTimeAndUpdate(uint64_t time)
{
	int diff = nLastUserTradeTime - time;
	if (diff > nMaxSubmissionTimeDiff)
		return false;

	nLastUserTradeTime = time;
	return true;
}

bool CUserTrade::VerifyUserSignature()
{
	return true;
}

bool CUserTrade::VerifyMNSignature()
{
	return true;
}

bool CUserTrade::MNSign()
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

bool CUserTradeManager::InputUserBuyTrade(const std::shared_ptr<CUserTrade>& userTrade)
{
	userbalance_to_exchange_enum_t result = userBalanceManager.BalanceToExchange(userTrade->nTradePairID, userTrade->nUserPubKey, userTrade->nAmount);
	if (result != USER_BALANCE_DEDUCTED)
	{
		//we should not be here
		//for future user banning
		return false;
	}

	auto& a = mapUserTrades[userTrade->nUserPubKey];
	userTrade->nUserTradeID = a.first + 1;
	userTrade->nBalanceAmount = userTrade->nAmount;
	userTrade->nBalanceQty = userTrade->nQuantity;
	userTrade->nMNPubKey = ""; //to replace with actual MN pub key
	userTrade->nLastUpdate = GetAdjustedTime();
	userTrade->MNSign();
	a.second.insert(std::make_pair(userTrade->nUserTradeID, userTrade));
	return true;
}

bool CUserTradeManager::InputUserSellTrade(const std::shared_ptr<CUserTrade>& userTrade)
{
	userbalance_to_exchange_enum_t result = userBalanceManager.BalanceToExchange(userTrade->nTradePairID, userTrade->nUserPubKey, userTrade->nAmount);
	if (result != USER_BALANCE_DEDUCTED)
	{
		//we should not be here
		//for future user banning
		return false;
	}

	if (!mapAskUserTradeByPrice[userTrade->nTradePairID].count(userTrade->nPrice))
	{
		mINTUT temp;
		temp.insert(std::make_pair(userTrade->nUserTradeID, userTrade));
		mapAskUserTradeByPrice[userTrade->nTradePairID].insert(std::make_pair(userTrade->nPrice, temp));
	}
	else
		mapAskUserTradeByPrice[userTrade->nTradePairID][userTrade->nPrice].insert(std::make_pair(userTrade->nUserTradeID, userTrade));

	if (!mapAskUserTradeByPubkey[userTrade->nTradePairID].count(userTrade->nUserPubKey))
	{
		mINTUT temp;
		temp.insert(std::make_pair(userTrade->nUserTradeID, userTrade));
		mapAskUserTradeByPubkey[userTrade->nTradePairID].insert(std::make_pair(userTrade->nUserPubKey, temp));
	}
	else
		mapAskUserTradeByPubkey[userTrade->nTradePairID][userTrade->nUserPubKey].insert(std::make_pair(userTrade->nUserTradeID, userTrade));
	return true;
}

void CUserTradeManager::InputUserTrade(const std::shared_ptr<CUserTrade>& userTrade)
{
	if (userTrade->nQuantity <= 0)
		return;

	if (!userTrade->VerifyUserSignature())
		return;

	CUserTradeSetting& setting = mapUserTradeSetting[userTrade->nTradePairID];
	CTradePair& tradePair = mapCompleteTradePair[userTrade->nTradePairID];
	if (tradePair.nTradePairID != userTrade->nTradePairID)
	{
		//request node setup
		return;
	}

	if (userTrade->nIsBid)
	{
		ProcessUserBuyRequest(userTrade, setting, tradePair);
		InputMatchUserBuyRequest(userTrade, setting, tradePair);
	}
	else
	{
		ProcessUserSellRequest(userTrade, setting, tradePair);
		InputMatchUserSellRequest(userTrade, setting, tradePair);
	}
}

void CUserTradeManager::ProcessUserBuyRequest(const std::shared_ptr<CUserTrade>& userTrade, CUserTradeSetting& setting, CTradePair& tradePair)
{
	if (tradePair.nBidTradeFee > userTrade->nTradeFee)
		return;

	if (!userBalanceManager.InChargeOfUserBalance(userTrade->nUserPubKey))
		return;

	if (IsUserTradeInList(userTrade->nTradePairID, userTrade->nUserHash))
		return;

	if (!IsSubmittedBidAmountValid(userTrade, tradePair.nBidTradeFee))
		return;	

	std::shared_ptr<CUserBalance> userBalance;
	userBalanceManager.InitUserBalance(tradePair.nCoinID2, userTrade->nUserPubKey, userBalance);
	if (userBalance->nAvailableBalance < userTrade->nAmount)
		return;

	if (userTrade->nMNPubKey == "")
	{
		int timeDiff = GetAdjustedTime() - userTrade->nTimeSubmit;
		if (timeDiff > 10000 || timeDiff < -10000)
			return;	

		if (!InputUserBuyTrade(userTrade))
			return;
		
		AddToUserTradeList(userTrade->nTradePairID, userTrade->nUserHash);
		userTrade->RelayToHandler();
	}
}

void CUserTradeManager::ProcessUserSellRequest(const std::shared_ptr<CUserTrade>& userTrade, CUserTradeSetting& setting, CTradePair& tradePair)
{
	if (userTrade->nQuantity <= 0)
		return;

	if (tradePair.nTradePairID != userTrade->nTradePairID)
		return;

	if (tradePair.nAskTradeFee > userTrade->nTradeFee)
		return;

	if (setting.nTradePairID != userTrade->nTradePairID)
		return;

	if (setting.nSyncInProgress)
		return;

	if (!setting.nIsInChargeOfProcessUserTrade)
		return;

	if (!setting.IsValidSubmissionTimeAndUpdate(userTrade->nTimeSubmit))
		return;

	if (!userTrade->VerifyUserSignature())
		return;

	if (IsUserTradeInList(userTrade->nTradePairID, userTrade->nUserHash))
		return;

	if (!IsSubmittedAskAmountValid(userTrade, tradePair.nAskTradeFee))
		return;

	userTrade->nUserTradeID = (setting.nLastUserTradeID + 1);
	userTrade->nBalanceAmount = userTrade->nAmount;
	userTrade->nBalanceQty = userTrade->nQuantity;
	userTrade->nMNPubKey = setting.nMNPubKey;
	userTrade->nLastUpdate = GetAdjustedTime();
	userTrade->MNSign();

	if (!InputUserSellTrade(userTrade))
		return;

	++setting.nLastUserTradeID;
	AddToUserTradeList(userTrade->nTradePairID, userTrade->nUserHash);
	userTrade->RelayToHandler();

	if (setting.nIsInChargeOfMatchUserTrade)
		InputMatchUserBuyRequest(userTrade, setting, tradePair);
}

void CUserTradeManager::InputMatchUserBuyRequest(const std::shared_ptr<CUserTrade>& userTrade, CUserTradeSetting& setting, CTradePair& tradePair, bool InitialCheck)
{
	if (!InitialCheck)
	{
		if (userTrade->nBalanceQty <= 0)
			return;

		if (tradePair.nTradePairID != userTrade->nTradePairID)
			return;

		if (setting.nTradePairID != userTrade->nTradePairID)
			return;

		if (setting.nSyncInProgress)
			return;

		if (!setting.nIsInChargeOfMatchUserTrade)
			return;

		if (!userTrade->VerifyMNSignature())
			return;

		if (IsUserTradeInList(userTrade->nTradePairID, userTrade->nUserHash))
			return;

		int sequence = userTrade->nUserTradeID - setting.nLastUserTradeID;
		if (sequence <= 0)
			return;

		if (sequence > 1)
		{
			//need to do something more here
			return;
		}

		if (!InputUserBuyTrade(userTrade))
		{
			//need to know why its here
			return;
		}
	}

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
			{
				qty = userTrade->nBalanceQty;
			}
			else
			{
				qty = ExistingTrade->nBalanceQty;
			}

			int bidTradeFee = (userTrade->nTradeFee < tradePair.nAskTradeFee) ? userTrade->nTradeFee : tradePair.nAskTradeFee;
			int askTradeFee = (ExistingTrade->nTradeFee < tradePair.nBidTradeFee) ? ExistingTrade->nTradeFee : tradePair.nBidTradeFee;
			uint64_t bidAmount = GetBidRequiredAmount(userTrade->nPrice, qty, bidTradeFee);
			uint64_t askAmount = GetAskExpectedAmount(ExistingTrade->nPrice, qty, askTradeFee);
			uint64_t tradeTime = GetAdjustedTime();

			std::shared_ptr<CActualTrade> actualTrade = std::make_shared<CActualTrade>(tradePair.nTradePairID, userTrade->nUserTradeID, ExistingTrade->nUserTradeID, userTrade->nPrice, qty, bidAmount, askAmount, userTrade->nUserPubKey, ExistingTrade->nUserPubKey, bidTradeFee, askTradeFee, GetAdjustedTime());
			CActualTradeSetting& actualTradeSetting = mapActualTradeSetting[userTrade->nTradePairID];
			if (actualTradeSetting.nTradePairID != userTrade->nTradePairID)
				return;
			if (!actualTradeManager.GenerateActualTrade(actualTrade, actualTradeSetting))
				return;

			ExistingTrade->nBalanceQty -= qty;
			userTrade->nBalanceQty -= qty;
			actualTradeManager.InputActualTrade(actualTrade, actualTradeSetting, tradePair);

			if (userTrade->nBalanceQty <= 0)
				return;

			++Seller;
		}
		++Sellers;
	}
}

void CUserTradeManager::InputMatchUserSellRequest(const std::shared_ptr<CUserTrade>& userTrade, CUserTradeSetting& setting, CTradePair& tradePair, bool InitialCheck)
{
	if (!InitialCheck)
	{
		if (userTrade->nBalanceQty <= 0)
			return;

		if (tradePair.nTradePairID != userTrade->nTradePairID)
			return;

		if (setting.nTradePairID != userTrade->nTradePairID)
			return;

		if (setting.nSyncInProgress)
			return;

		if (!setting.nIsInChargeOfMatchUserTrade)
			return;

		if (!userTrade->VerifyMNSignature())
			return;

		if (IsUserTradeInList(userTrade->nTradePairID, userTrade->nUserHash))
			return;

		int sequence = userTrade->nUserTradeID - setting.nLastUserTradeID;
		if (sequence <= 0)
			return;

		if (sequence > 1)
		{
			//need to do something more here
			return;
		}

		if (!InputUserSellTrade(userTrade))
		{
			//need to know why its here
			return;
		}
	}

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
			{
				qty = userTrade->nBalanceQty;
			}
			else
			{
				qty = ExistingTrade->nBalanceQty;
			}			
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

			if (userTrade->nBalanceQty <= 0)
				return;

			++Buyer;
		}
		++Buyers;
	}
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

void CActualTradeManager::AssignWithdrawProcessorRole(int CoinID)
{

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

bool CActualTrade::InformConflictTrade(CNode* node)
{
	return true;
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

bool CActualTradeManager::GetActualTrade(CNode* node, int ActualTradeID, int TradePairID)
{
	if (!mapActualTradeByActualTradeID.count(TradePairID))
	{
		//inform other node that we don't carry this trade pair
		return false;
	}

	if (!mapActualTradeByActualTradeID[TradePairID].count(ActualTradeID))
	{
		//inform other node that we don't have this trade data		
		return false;
	}

	//send this trade data to other node
	CActualTrade temp = *mapActualTradeByActualTradeID[TradePairID][ActualTradeID];

	return true;
}

bool CActualTradeManager::AddNewActualTrade(CActualTrade ActualTrade)
{		
	
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

std::vector<std::string> CActualTradeManager::FindDuplicateTrade(int TradePairID)
{
	std::vector<std::string> DuplicatedTrade;
	std::vector<std::string> HashContainer;
	for (auto &a : mapActualTradeByActualTradeID[TradePairID])
	{
		bool Conflict = false;
		for (auto &b : HashContainer)
		{
			if (a.second->nCurrentHash == b)
			{
				DuplicatedTrade.push_back(b);
				Conflict = true;
				break;
			}
		}
		if (!Conflict)
			HashContainer.push_back(a.second->nCurrentHash);
	}
	return DuplicatedTrade;
}

bool CActualTradeManager::IsActualTradeInList(int TradePairID, int ActualTradeID, std::string Hash)
{
	return true;
}

bool CActualTradeManager::InputActualTrade(std::shared_ptr<CActualTrade> actualTrade, CActualTradeSetting& setting, CTradePair& tradePair)
{
	mapActualTradeHash[actualTrade->nTradePairID].insert(actualTrade->GetHash());
	std::pair<int, std::shared_ptr<CActualTrade>> temp(std::make_pair(actualTrade->nActualTradeID, actualTrade));
	mapActualTradeByActualTradeID[actualTrade->nTradePairID].insert(temp);

	if (mapActualTradeByUserTradeID[actualTrade->nTradePairID].count(actualTrade->nUserTrade1))
		mapActualTradeByUserTradeID[actualTrade->nTradePairID][actualTrade->nUserTrade1].insert(temp);
	else
	{
		mATIAT temp2;
		temp2.insert(temp);
		mapActualTradeByUserTradeID[actualTrade->nTradePairID].insert(std::make_pair(actualTrade->nUserTrade1, temp2));
	}

	if (mapActualTradeByUserTradeID[actualTrade->nTradePairID].count(actualTrade->nUserTrade2))
		mapActualTradeByUserTradeID[actualTrade->nTradePairID][actualTrade->nUserTrade2].insert(temp);
	else
	{
		mATIAT temp2;
		temp2.insert(temp);
		mapActualTradeByUserTradeID[actualTrade->nTradePairID].insert(std::make_pair(actualTrade->nUserTrade2, temp2));
	}

	if (setting.nIsInChargeOfOrderBook)
		orderBookManager.AdjustBidQuantity(actualTrade->nTradePairID, actualTrade->nTradePrice, actualTrade->nTradeQty);
	if (setting.nIsInChargeOfChartData)
		ChartDataManager.InputNewTrade(actualTrade->nTradePairID, actualTrade->nTradePrice, actualTrade->nTradeQty, actualTrade->nTradeTime);
	if (setting.nIsInChargeOfMarketTradeHistory)
		userTradeHistoryManager.InputNewUserTradeHistory(CUserTradeHistory(actualTrade->nTradePairID, actualTrade->nUserPubKey1, actualTrade->nUserPubKey2, actualTrade->nTradePrice, actualTrade->nTradeQty, actualTrade->nTradeAmount, false, actualTrade->nTradeTime));
	
	userBalanceManager.UpdateAfterTradeBalance(actualTrade->nUserPubKey1, actualTrade->nUserPubKey2, tradePair.nCoinID1, tradePair.nCoinID2, 0 - actualTrade->nBidAmount, actualTrade->nTradeQty, actualTrade->nTradeQty, actualTrade->nAskAmount);
	actualTrade->Relay();
	return true;
}

bool CActualTradeManager::InputActualTradeFromNode(std::shared_ptr<CActualTrade> actualTrade, CActualTradeSetting& setting, CTradePair& tradePair)
{
	if (!actualTrade->VerifySignature())
		return false;

	if (tradePair.nTradePairID != actualTrade->nTradePairID)
		return false;

	if (setting.nTradePairID != actualTrade->nTradePairID)
		return false;

	if (setting.nSyncInProgress)
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