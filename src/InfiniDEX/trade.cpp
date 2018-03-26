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

void CUserTrade::RelayTo(CNode* node, CConnman& connman)
{

}

void CUserTrade::RelayToHandler(CConnman& connman)
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

void CUserTradeManager::ProcessUserBuyRequest(CConnman& connman, CUserTrade& userTrade)
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

	AddToUserTradeList(userTrade.nTradePairID, userTrade.nUserHash);
	userTrade.nUserTradeID = ++mapUserTradeSetting[userTrade.nTradePairID].nLastUserTradeID;
	userTrade.nBalanceAmount = userTrade.nAmount;
	userTrade.nBalanceQty = userTrade.nQuantity;
	userTrade.nMNPubKey = mapUserTradeSetting[userTrade.nTradePairID].nMNPubKey;
	userTrade.nLastUpdate = GetAdjustedTime();
	userTrade.MNSign();	
	mapBidUserTradeByPrice[userTrade.nTradePairID][userTrade.nPrice].insert(std::make_pair(userTrade.nUserTradeID, &userTrade));
	mapBidUserTradeByPubkey[userTrade.nTradePairID][userTrade.nUserPubKey].insert(std::make_pair(userTrade.nUserTradeID, &userTrade));
	userTrade.RelayToHandler(connman);
}

void CUserTradeManager::ProcessUserSellRequest(CConnman& connman, CUserTrade& userTrade)
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

	AddToUserTradeList(userTrade.nTradePairID, userTrade.nUserHash);
	userTrade.nUserTradeID = ++mapUserTradeSetting[userTrade.nTradePairID].nLastUserTradeID;
	userTrade.nBalanceAmount = userTrade.nAmount;
	userTrade.nBalanceQty = userTrade.nQuantity;
	userTrade.nMNPubKey = mapUserTradeSetting[userTrade.nTradePairID].nMNPubKey;
	userTrade.nLastUpdate = GetAdjustedTime();
	userTrade.MNSign();
	mapAskUserTradeByPrice[userTrade.nTradePairID][userTrade.nPrice].insert(std::make_pair(userTrade.nUserTradeID, &userTrade));
	mapAskUserTradeByPubkey[userTrade.nTradePairID][userTrade.nUserPubKey].insert(std::make_pair(userTrade.nUserTradeID, &userTrade));
	userTrade.RelayToHandler(connman);
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
		return;
	}
}

void CUserTradeManager::InputMatchUserSellRequest(CUserTrade userTrade)
{

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

		userbalance_to_exchange_enum_t result = userBalanceManager.BalanceToExchange(tradePair.nCoinID1, cut->nUserPubKey, cut->nAmount);
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
					//userBalanceManager.UpdateAfterTradeBalance(ExistingTrade->nUserPubKey, cut->nUserPubKey, tradePair.nCoinInfoID1, tradePair.nCoinInfoID2, -bidAmount, qty, -qty, askAmount);
					orderBookManager.AdjustAskQuantity(tradePair.nTradePairID, ExistingTrade->nPrice, qty);
					//actualTradeManager.InputNewCompletedTrade(CActualTrade(ExistingTrade->nUserPubKey, cut->nUserPubKey, tradePair.nTradePairID, ExistingTrade->nPrice, qty, actualAmount, bidAmount - actualAmount, tradePair.nBidTradeFeeCoinID, actualAmount - askAmount, tradePair.nAskTradeFeeCoinID, "", GetAdjustedTime()));
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
			mapUserTradeRequest[cut->nTradePairID].second.first[cut->nPrice].push_back(std::make_pair(cut->nUserTradeID, cut));
		}
		else
		{
			std::vector<PairSignatureUserTrade> vecCut;
			vecCut.push_back(std::make_pair(cut->nUserTradeID, cut));
			mapUserTradeRequest[cut->nTradePairID].second.first.insert(std::make_pair(cut->nPrice, vecCut));
		}
		if (mucut.count(cut->nUserPubKey))
		{
			mapUserTradeRequest[cut->nTradePairID].second.second[cut->nUserPubKey].push_back(std::make_pair(cut->nUserTradeID, cut));
		}
		else
		{
			std::vector<PairSignatureUserTrade> vecCut;
			vecCut.push_back(std::make_pair(cut->nUserTradeID, cut));
			mapUserTradeRequest[cut->nTradePairID].second.second.insert(std::make_pair(cut->nUserPubKey, vecCut));
		}
	}
	else
	{
		//for testing purpose
		//to remove on actual implementation
		std::vector<PairSignatureUserTrade> vecCut;
		vecCut.push_back(std::make_pair(cut->nUserTradeID, cut));
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
	int BuySideCoinID = tradePairManager.GetBidSideCoinID(userTrade.nTradePairID);
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
			mapUserTradeRequest[cut->nTradePairID].first.first[cut->nPrice].push_back(std::make_pair(cut->nUserTradeID, cut));
		}
		else
		{
			std::vector<PairSignatureUserTrade> vecCut;
			vecCut.push_back(std::make_pair(cut->nUserTradeID, cut));
			mapUserTradeRequest[cut->nTradePairID].first.first.insert(std::make_pair(cut->nPrice, vecCut));
		}
		if (mucut.count(cut->nUserPubKey))
		{
			mapUserTradeRequest[cut->nTradePairID].first.second[cut->nUserPubKey].push_back(std::make_pair(cut->nUserTradeID, cut));
		}
		else
		{
			std::vector<PairSignatureUserTrade> vecCut;
			vecCut.push_back(std::make_pair(cut->nUserTradeID, cut));
			mapUserTradeRequest[cut->nTradePairID].first.second.insert(std::make_pair(cut->nUserPubKey, vecCut));
		}
	}
	else
	{
		//for testing purpose
		//to remove on actual implementation
		std::vector<PairSignatureUserTrade> vecCut;
		vecCut.push_back(std::make_pair(cut->nUserTradeID, cut));
		mpcut.insert(std::make_pair(cut->nPrice, vecCut));
		mucut.insert(std::make_pair(cut->nUserPubKey, vecCut));
		PairPricePubKeyCUserTrade buySide = std::make_pair(mpcut, mucut);

		MapPriceCUserTrade mpcut2;
		MapPubKeyCUserTrade mucut2;
		PairPricePubKeyCUserTrade sellSide = std::make_pair(mpcut2, mucut2);

		mapUserTradeRequest.insert(std::make_pair(cut->nTradePairID, std::make_pair(buySide, sellSide)));
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