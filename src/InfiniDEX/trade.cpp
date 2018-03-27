// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "trade.h"
#include "orderbook.h"
#include "tradepair.h"
#include "userbalance.h"

#include <boost/multiprecision/cpp_int.hpp>
#include <chrono>

class CUserTrade;
class CUserTradeSetting;
class CUserTradeManager;

class CActualTrade;
class CActualTradeSetting;
class CActualTradeManager;

std::map<int, mUTPIUTV> mapBidUserTradeByPrice;
std::map<int, mUTPIUTV> mapAskUserTradeByPrice;
std::map<int, mUTPKUTV> mapBidUserTradeByPubkey;
std::map<int, mUTPKUTV> mapAskUserTradeByPubkey;
std::map<int, CUserTradeSetting> mapUserTradeSetting;
std::map<int, std::set<std::string>> mapUserTradeHash;
std::map<int, mATIAT> mapActualTradeByActualTradeID;
std::map<int, mUPKAT> mapActualTradeByUserPublicKey;
std::map<int, mUTIAT> mapActualTradeByUserTradeID;
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

bool CUserTradeManager::IsTradePairInList(int TradePairID)
{
	return mapUserTradeSetting.count(TradePairID);
}

bool CUserTradeManager::IsSyncInProgress(int TradePairID)
{
	return mapUserTradeSetting[TradePairID].nSyncInProgress;
}

bool CUserTradeManager::IsInChargeOfProcessUserTrade(int TradePairID)
{
	return mapUserTradeSetting[TradePairID].nIsInChargeOfProcessUserTrade;
}

bool CUserTradeManager::IsInChargeOfMatchUserTrade(int TradePairID)
{
	return mapUserTradeSetting[TradePairID].nIsInChargeOfMatchUserTrade;
}

bool CUserTradeManager::IsUserTradeInList(int TradePairID, std::string UserHash)
{
	return mapUserTradeHash[TradePairID].count(UserHash);
}

void CUserTradeManager::AddToUserTradeList(int TradePairID, std::string UserHash)
{
	mapUserTradeHash[TradePairID].insert(UserHash);
}

bool CUserTradeManager::AddUserBuyTradeIntoList(const std::shared_ptr<CUserTrade>& userTrade)
{
	userbalance_to_exchange_enum_t result = userBalanceManager.BalanceToExchange(userTrade->nTradePairID, userTrade->nUserPubKey, userTrade->nAmount);
	if (result != USER_BALANCE_DEDUCTED)
	{
		//we should not be here
		//for future user banning
		return false;
	}

	if (!mapBidUserTradeByPrice[userTrade->nTradePairID].count(userTrade->nPrice))
	{
		mUTIUT temp;
		temp.insert(std::make_pair(userTrade->nUserTradeID, userTrade));
		mapBidUserTradeByPrice[userTrade->nTradePairID].insert(std::make_pair(userTrade->nPrice, temp));
	}
	else
		mapBidUserTradeByPrice[userTrade->nTradePairID][userTrade->nPrice].insert(std::make_pair(userTrade->nUserTradeID, userTrade));

	if (!mapBidUserTradeByPubkey[userTrade->nTradePairID].count(userTrade->nUserPubKey))
	{
		mUTIUT temp;
		temp.insert(std::make_pair(userTrade->nUserTradeID, userTrade));
		mapBidUserTradeByPubkey[userTrade->nTradePairID].insert(std::make_pair(userTrade->nUserPubKey, temp));
	}
	else
		mapBidUserTradeByPubkey[userTrade->nTradePairID][userTrade->nUserPubKey].insert(std::make_pair(userTrade->nUserTradeID, userTrade));
	return true;
}

bool CUserTradeManager::AddUserSellTradeIntoList(const std::shared_ptr<CUserTrade>& userTrade)
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
		mUTIUT temp;
		temp.insert(std::make_pair(userTrade->nUserTradeID, userTrade));
		mapAskUserTradeByPrice[userTrade->nTradePairID].insert(std::make_pair(userTrade->nPrice, temp));
	}
	else
		mapAskUserTradeByPrice[userTrade->nTradePairID][userTrade->nPrice].insert(std::make_pair(userTrade->nUserTradeID, userTrade));

	if (!mapAskUserTradeByPubkey[userTrade->nTradePairID].count(userTrade->nUserPubKey))
	{
		mUTIUT temp;
		temp.insert(std::make_pair(userTrade->nUserTradeID, userTrade));
		mapAskUserTradeByPubkey[userTrade->nTradePairID].insert(std::make_pair(userTrade->nUserPubKey, temp));
	}
	else
		mapAskUserTradeByPubkey[userTrade->nTradePairID][userTrade->nUserPubKey].insert(std::make_pair(userTrade->nUserTradeID, userTrade));
	return true;
}

void CUserTradeManager::ProcessUserBuyRequest(CUserTrade userTrade)
{
	if (!IsTradePairInList(userTrade.nTradePairID))
		return;

	if (IsSyncInProgress(userTrade.nTradePairID))
		return;

	if (!IsInChargeOfProcessUserTrade(userTrade.nTradePairID))
		return;

	if (!userTrade.VerifyUserSignature())
		return;

	if (IsUserTradeInList(userTrade.nTradePairID, userTrade.nUserHash))
		return;

	if (!mapUserTradeSetting[userTrade.nTradePairID].IsValidSubmissionTimeAndUpdate(userTrade.nTimeSubmit))
		return;

	int TradeFee = tradePairManager.GetBidTradeFee(userTrade.nTradePairID);
	if (TradeFee > userTrade.nTradeFee)
		return;

	if (!IsSubmittedBidAmountValid(userTrade, TradeFee))
		return;

	userTrade.nUserTradeID = ++mapUserTradeSetting[userTrade.nTradePairID].nLastUserTradeID;
	userTrade.nBalanceAmount = userTrade.nAmount;
	userTrade.nBalanceQty = userTrade.nQuantity;
	userTrade.nMNPubKey = mapUserTradeSetting[userTrade.nTradePairID].nMNPubKey;
	userTrade.nLastUpdate = GetAdjustedTime();
	userTrade.MNSign();

	std::shared_ptr<CUserTrade> ut = std::make_shared<CUserTrade>(userTrade);
	if (!AddUserBuyTradeIntoList(ut))
		return;
	AddToUserTradeList(userTrade.nTradePairID, userTrade.nUserHash);
	ut->RelayToHandler();
}

void CUserTradeManager::ProcessUserSellRequest(CUserTrade userTrade)
{
	if (!IsTradePairInList(userTrade.nTradePairID))
		return;

	if (IsSyncInProgress(userTrade.nTradePairID))
		return;

	if (!IsInChargeOfProcessUserTrade(userTrade.nTradePairID))
		return;

	if (!userTrade.VerifyUserSignature())
		return;

	if (IsUserTradeInList(userTrade.nTradePairID, userTrade.nUserHash))
		return;

	if (!mapUserTradeSetting[userTrade.nTradePairID].IsValidSubmissionTimeAndUpdate(userTrade.nTimeSubmit))
		return;

	int TradeFee = tradePairManager.GetAskTradeFee(userTrade.nTradePairID);
	if (TradeFee > userTrade.nTradeFee)
		return;

	if (!IsSubmittedAskAmountValid(userTrade, TradeFee))
		return;

	userTrade.nUserTradeID = ++mapUserTradeSetting[userTrade.nTradePairID].nLastUserTradeID;
	userTrade.nBalanceAmount = userTrade.nAmount;
	userTrade.nBalanceQty = userTrade.nQuantity;
	userTrade.nMNPubKey = mapUserTradeSetting[userTrade.nTradePairID].nMNPubKey;
	userTrade.nLastUpdate = GetAdjustedTime();
	userTrade.MNSign();

	std::shared_ptr<CUserTrade> ut = std::make_shared<CUserTrade>(userTrade);
	if (!AddUserSellTradeIntoList(ut))
		return;
	AddToUserTradeList(userTrade.nTradePairID, userTrade.nUserHash);
	ut->RelayToHandler();
}

bool CUserTradeManager::IsProcessedUserTradeInList(CUserTrade UserTrade)
{
	return (mapUserTradeSetting[UserTrade.nUserTradeID].nLastUserTradeID >= UserTrade.nUserTradeID);
}

int CUserTradeManager::IsProcessedUserTradeInSequence(CUserTrade UserTrade)
{
	return UserTrade.nUserTradeID - mapUserTradeSetting[UserTrade.nUserTradeID].nLastUserTradeID;
}

void CUserTradeManager::InputMatchUserBuyRequest(CUserTrade userTrade)
{
	CTradePair tradePair = tradePairManager.GetTradePair(userTrade.nTradePairID);
	if (tradePair.nTradePairID != userTrade.nTradePairID)
		return;

	if (!IsTradePairInList(userTrade.nTradePairID))
		return;

	if (IsSyncInProgress(userTrade.nTradePairID))
		return;

	if (!IsInChargeOfMatchUserTrade(userTrade.nTradePairID))
		return;

	if (!userTrade.VerifyMNSignature())
		return;

	int sequence = IsProcessedUserTradeInSequence(userTrade);
	if (sequence <= 0)
		return;

	if (sequence > 1) 
	{
		//need to do something more here
		return;
	}

	std::shared_ptr<CUserTrade> ut = std::make_shared<CUserTrade>(userTrade);
	if (!AddUserBuyTradeIntoList(ut)) 
	{
		//need to know why its here
		return;
	}

	mUTPIUTV::iterator Sellers = mapAskUserTradeByPrice[ut->nTradePairID].begin();
	while (Sellers != mapAskUserTradeByPrice[ut->nTradePairID].end() && Sellers->first <= ut->nPrice)
	{
		mUTIUT::iterator Seller = Sellers->second.begin();
		while (Seller != Sellers->second.end())
		{
			std::shared_ptr<CUserTrade> ExistingTrade = Seller->second;
			int qty = 0;
			if (ut->nBalanceQty <= ExistingTrade->nBalanceQty)
			{
				qty = ut->nBalanceQty;
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
			ut->nBalanceQty -= qty;
			int bidTradeFee = (ut->nTradeFee < tradePair.nAskTradeFee) ? ut->nTradeFee : tradePair.nAskTradeFee;
			int askTradeFee = (ExistingTrade->nTradeFee < tradePair.nBidTradeFee) ? ExistingTrade->nTradeFee : tradePair.nBidTradeFee;
			uint64_t bidAmount = GetBidRequiredAmount(ut->nPrice, qty, bidTradeFee);
			uint64_t askAmount = GetAskExpectedAmount(ExistingTrade->nPrice, qty, askTradeFee);			
			uint64_t actualAmount = ut->nPrice*qty;
			//userBalanceManager.UpdateAfterTradeBalance(ExistingTrade->nUserPubKey, cut->nUserPubKey, tradePair.nCoinInfoID1, tradePair.nCoinInfoID2, -bidAmount, qty, -qty, askAmount);
			orderBookManager.AdjustAskQuantity(tradePair.nTradePairID, ExistingTrade->nPrice, qty);
			//actualTradeManager.InputNewCompletedTrade(CActualTrade(ExistingTrade->nUserPubKey, cut->nUserPubKey, tradePair.nTradePairID, ExistingTrade->nPrice, qty, actualAmount, bidAmount - actualAmount, tradePair.nBidTradeFeeCoinID, actualAmount - askAmount, tradePair.nAskTradeFeeCoinID, "", GetAdjustedTime()));
			if (ut->nBalanceQty == 0)
				return;

			++Seller;
		}
		++Sellers;
	}
}

void CUserTradeManager::InputMatchUserSellRequest(CUserTrade userTrade)
{
	CTradePair tradePair = tradePairManager.GetTradePair(userTrade.nTradePairID);
	if (tradePair.nTradePairID != userTrade.nTradePairID)
		return;

	if (!IsTradePairInList(userTrade.nTradePairID))
		return;

	if (IsSyncInProgress(userTrade.nTradePairID))
		return;

	if (!IsInChargeOfMatchUserTrade(userTrade.nTradePairID))
		return;

	if (!userTrade.VerifyMNSignature())
		return;

	int sequence = IsProcessedUserTradeInSequence(userTrade);
	if (sequence <= 0)
		return;

	if (sequence > 1)
	{
		//need to do something more here
		return;
	}

	std::shared_ptr<CUserTrade> ut = std::make_shared<CUserTrade>(userTrade);
	if (!AddUserSellTradeIntoList(ut))
	{
		//need to know why its here
		return;
	}

	mUTPIUTV::reverse_iterator Buyers = mapBidUserTradeByPrice[ut->nTradePairID].rbegin();
	while (Buyers != mapBidUserTradeByPrice[ut->nTradePairID].rend() && Buyers->first >= ut->nPrice)
	{
		mUTIUT::iterator Buyer = Buyers->second.begin();
		while (Buyer != Buyers->second.end())
		{
			std::shared_ptr<CUserTrade> ExistingTrade = Buyer->second;
			int qty = 0;
			if (ut->nBalanceQty <= ExistingTrade->nBalanceQty)
			{
				qty = ut->nBalanceQty;
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
			ut->nBalanceQty -= qty;
			int bidTradeFee = (ExistingTrade->nTradeFee < tradePair.nBidTradeFee) ? ExistingTrade->nTradeFee : tradePair.nBidTradeFee;
			int askTradeFee = (ut->nTradeFee < tradePair.nAskTradeFee) ? ut->nTradeFee : tradePair.nAskTradeFee;
			uint64_t bidAmount = GetBidRequiredAmount(ExistingTrade->nPrice, qty, bidTradeFee);
			uint64_t askAmount = GetAskExpectedAmount(ut->nPrice, qty, askTradeFee);

			uint64_t actualAmount = ut->nPrice*qty;
			//userBalanceManager.UpdateAfterTradeBalance(ExistingTrade->nUserPubKey, cut->nUserPubKey, tradePair.nCoinInfoID1, tradePair.nCoinInfoID2, -bidAmount, qty, -qty, askAmount);
			orderBookManager.AdjustAskQuantity(tradePair.nTradePairID, ExistingTrade->nPrice, qty);
			//actualTradeManager.InputNewCompletedTrade(CActualTrade(ExistingTrade->nUserPubKey, cut->nUserPubKey, tradePair.nTradePairID, ExistingTrade->nPrice, qty, actualAmount, bidAmount - actualAmount, tradePair.nBidTradeFeeCoinID, actualAmount - askAmount, tradePair.nAskTradeFeeCoinID, "", GetAdjustedTime()));
			if (ut->nBalanceQty == 0)
				return;

			++Buyer;
		}
		++Buyers;
	}
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

bool CActualTrade::CheckSignature()
{
	return true;
}

bool CActualTrade::InformActualTrade()
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

void CActualTradeManager::InputNewTradePair(int TradePairID, bool SecurityCheck)
{
	if (!mapTradePairActualTradeContainer.count(TradePairID))
	{
		pairSettingSecurity temp(std::make_pair(CActualTradeSetting(TradePairID, SecurityCheck), std::set<std::string>()));
		mapTradePairActualTradeContainer.insert(std::make_pair(TradePairID, std::make_pair(temp, mapActualTrade())));
	}
	if (!mapTradePairToResolveTrade.count(TradePairID))
	{
		mapTradePairToResolveTrade.insert(std::make_pair(TradePairID, std::vector<CActualTrade>()));
	}
}

bool CActualTradeManager::SetSecurityCheck(int TradePairID, bool SecurityCheck)
{
	if (!mapTradePairActualTradeContainer.count(TradePairID))
	{

	}

	if (mapTradePairActualTradeContainer[TradePairID].first.first.nSecurityCheck == SecurityCheck)
		return true;

	if (SecurityCheck)
	{
		for (auto &a : mapTradePairActualTradeContainer[TradePairID].second)
		{
			bool NoConflict = true;
			if (mapTradePairActualTradeContainer[TradePairID].first.second.count(a.second.nCurrentHash))
			{
				NoConflict = false;
				//to resolve duplicated trade
			}
			else
			{
				mapTradePairActualTradeContainer[TradePairID].first.second.insert(a.second.nCurrentHash);
			}
		}
		mapTradePairActualTradeContainer[TradePairID].first.first.nSecurityCheck = SecurityCheck;
	}
	else
	{
		mapTradePairActualTradeContainer[TradePairID].first.second.clear();
		mapTradePairActualTradeContainer[TradePairID].first.first.nSecurityCheck = SecurityCheck;
	}

	return false;
}

bool CActualTradeManager::GetActualTrade(CNode* node, int ActualTradeID, int TradePairID)
{
	if (!mapTradePairActualTradeContainer.count(TradePairID))
	{
		//inform other node that we don't carry this trade pair
		return false;
	}

	if (!mapTradePairActualTradeContainer[TradePairID].second.count(ActualTradeID))
	{
		//inform other node that we don't have this trade data		
		return false;
	}

	//send this trade data to other node
	CActualTrade temp(mapTradePairActualTradeContainer[TradePairID].second[ActualTradeID]);

	return true;
}

bool CActualTradeManager::AddNewActualTrade(CActualTrade ActualTrade)
{
	if (!mapTradePairActualTradeContainer.count(ActualTrade.nTradePairID))
	{
		//its not possible to be here
		//if here, we would need to switch this node to passive & resync
		return false;
	}

	ActualTrade.nCurrentHash = ActualTrade.GetHash();
	bool SecurityCheck = mapTradePairActualTradeContainer[ActualTrade.nTradePairID].first.first.nSecurityCheck;
	if (SecurityCheck)
	{
		if (!RunSecurityCheck(ActualTrade.nTradePairID, ActualTrade.nCurrentHash))
		{
			return false;
		}
		mapTradePairActualTradeContainer[ActualTrade.nTradePairID].first.second.insert(ActualTrade.nCurrentHash);
	}
	ActualTrade.nActualTradeID = (++mapTradePairActualTradeContainer[ActualTrade.nTradePairID].first.first.nLastActualTradeID);
	mapTradePairActualTradeContainer[ActualTrade.nTradePairID].second.insert(std::make_pair(ActualTrade.nActualTradeID, ActualTrade));
	mapTradePairActualTradeContainer[ActualTrade.nTradePairID].first.second.insert(ActualTrade.nCurrentHash);
	mapTradePairActualTradeContainer[ActualTrade.nTradePairID].first.first.nLastHash = ActualTrade.nCurrentHash;
	ActualTrade.InformActualTrade();
	return true;
}

bool CActualTradeManager::IsActualTradeInList(CActualTrade ActualTrade)
{
	if (!mapActualTradeByActualTradeID[ActualTrade.nTradePairID].count(ActualTrade.nActualTradeID))
		return false;

	if (mapActualTradeByActualTradeID[ActualTrade.nTradePairID][ActualTrade.nActualTradeID]->nCurrentHash != ActualTrade.nCurrentHash)
	{
		//inform of differences
		return false;
	}

	return true;
}

bool CActualTradeManager::AddNewActualTrade(CNode* node, CConnman& connman, CActualTrade ActualTrade)
{

	if (!mapTradePairActualTradeContainer.count(ActualTrade.nTradePairID))
	{
		//check whether current node is in charge of current trade pair
		if (true)
		{
			InputNewTradePair(ActualTrade.nTradePairID, true);
		}
		else
		{

		}
		return false;
	}

	mapActualTrade::iterator temp = mapTradePairActualTradeContainer[ActualTrade.nTradePairID].second.find(ActualTrade.nActualTradeID);
	if (temp != mapTradePairActualTradeContainer[ActualTrade.nTradePairID].second.end)
	{
		if (temp->second.nCurrentHash == ActualTrade.nCurrentHash)
			return true;
		else
		{
			//resync required to make sure network running the same data
		}
	}
	else if (mapTradePairActualTradeContainer[ActualTrade.nTradePairID].first.second.count(ActualTrade.nCurrentHash))
	{
		//resync required to make sure network running the same data
	}
	else
	{
		mapTradePairActualTradeContainer[ActualTrade.nTradePairID].second.insert(std::make_pair(ActualTrade.nActualTradeID, ActualTrade));
		mapTradePairActualTradeContainer[ActualTrade.nTradePairID].first.second.insert(ActualTrade.nCurrentHash);
		mapTradePairActualTradeContainer[ActualTrade.nTradePairID].first.first.nLastHash = ActualTrade.nCurrentHash;
	}
	return true;
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