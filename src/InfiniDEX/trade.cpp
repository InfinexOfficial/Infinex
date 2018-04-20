// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "activemasternode.h"
#include "chartdata.h"
#include "messagesigner.h"
#include "noderole.h"
#include "orderbook.h"
#include "timedata.h"
#include "trade.h"
#include "userbalance.h"
#include "userconnection.h"
#include "usertradehistory.h"

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/lexical_cast.hpp>

class CUserTrade;
class CUserTradeSetting;
class CUserTradeManager;

class CActualTrade;
class CActualTradeManager;

std::set<uint256> userTradesHash;
std::vector<CUserTrade> pendingProcessUserTrades;
std::map<std::string, pULTIUTC> mapUserTrades;

std::map<int, mUTPIUTV> mapBidUserTradeByPrice;
std::map<int, mUTPIUTV> mapAskUserTradeByPrice;

std::map<int, CUserTradeSetting> mapUserTradeSetting;

std::map<std::string, mUTImAT> mapUserActualTrades;
std::map<int, mATIAT> mapActualTradeByActualTradeID;
std::map<int, mUTImAT> mapActualTradeByUserTradeID;
std::map<int, std::vector<CActualTrade>> mapConflictTrade;
std::map<int, std::set<std::string>> mapActualTradeHash;
CActualTradeManager actualTradeManager;
CUserTradeManager userTradeManager;

void CUserTradeManager::ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{
	if (strCommand == NetMsgType::DEXUSERTRADES)
	{
		std::vector<CUserTrade> incomings;
		vRecv >> incomings;

		for (auto incoming : incomings)
		{
			if (!incoming.VerifyUserSignature()) {
				LogPrintf("CUserTradeManager::ProcessMessage -- invalid signature\n");
				Misbehaving(pfrom->GetId(), 100);
				return;
			}

			std::shared_ptr<CUserTrade> UserTrade = std::make_shared<CUserTrade>(incoming);
			InputUserTrade(UserTrade);
		}
	}
	else if (strCommand == NetMsgType::DEXUSERTRADE)
	{
		CUserTrade incoming;
		vRecv >> incoming;

		if (!incoming.VerifyUserSignature()) {
			LogPrintf("CUserTradeManager::ProcessMessage -- invalid signature\n");
			Misbehaving(pfrom->GetId(), 100);
			return;

			std::shared_ptr<CUserTrade> UserTrade = std::make_shared<CUserTrade>(incoming);
			InputUserTrade(UserTrade);
		}
	}
	else if (strCommand == NetMsgType::DEXACTUALTRADES)
	{
		std::vector<CActualTrade> incomings;
		vRecv >> incomings;

		for (auto incoming : incomings)
		{
			if (!incoming.VerifyUserSignature()) {
				LogPrintf("CUserTradeManager::ProcessMessage -- invalid signature\n");
				Misbehaving(pfrom->GetId(), 100);
				return;
			}

			std::shared_ptr<CActualTrade> actualTrade = std::make_shared<CActualTrade>(incoming);
			
		}
	}
	else if (strCommand == NetMsgType::DEXACTUALTRADE)
	{
		CActualTrade incoming;
		vRecv >> incoming;

		if (!incoming.VerifyUserSignature()) {
			LogPrintf("CUserTradeManager::ProcessMessage -- invalid signature\n");
			Misbehaving(pfrom->GetId(), 100);
			return;

			std::shared_ptr<CActualTrade> UserTrade = std::make_shared<CActualTrade>(incoming);
			
		}
	}
	else if (strCommand == NetMsgType::DEXCANCELTRADE)
	{
		CCancelTrade incoming;
		vRecv >> incoming;

		if (!incoming.VerifyUserSignature()) {
			LogPrintf("CUserTradeManager::ProcessMessage -- invalid signature\n");
			Misbehaving(pfrom->GetId(), 100);
			return;

			std::shared_ptr<CCancelTrade> UserTrade = std::make_shared<CCancelTrade>(incoming);
			InputTradeCancel(incoming);
		}
	}
}

void CUserTradeManager::InputUserTrade(CUserTrade& UserTrade, CNode* node, CConnman& connman)
{
	if (!UserTrade.VerifyUserSignature()) {
		LogPrintf("CUserTradeManager::InputUserTrade -- invalid user signature\n");
		Misbehaving(pfrom->GetId(), 100);
		return;
	}

	if (UserTrade.nTradePairID < 1)
	{
		LogPrintf("CUserTradeManager::InputUserTrade -- invalid trade pair\n");
		Misbehaving(pfrom->GetId(), 100);
		return;
	}

	CTradePair& tradePair = mapTradePair[UserTrade.nTradePairID];
	if (tradePair.nTradePairID != UserTrade.nTradePairID)
	{
		tradePairManager.RequestTradePairInfoFromNode(UserTrade.nTradePairID);
		pendingProcessUserTrades.push_back(UserTrade);
		pendingProcessStatus.UserTrade = true;
		return;
	}

	std::shared_ptr<CUserTrade> userTrade = std::make_shared<CUserTrade>(UserTrade);
	if (userBalanceManager.InChargeOfUserBalance(userTrade->nUserPubKey))
	{
		if (userTrade->nMNBalancePubKey == "")
		{
			if (userTradesHash.count(userTrade->nUserHash))
				return;
			userTradesHash.insert(userTrade->nUserHash);
			if (!ProcessUserTradeRequest(userTrade, tradePair))
			{
				connman.PushMessage(node, NetMsgType::DEXINVALIDUSERTRADE, userTrade->nUserHash);
				return;
			}
			mnBalanceTradesHash.insert(userTrade->nMNBalanceHash);
			userTrade->RelayTo(node, connman);
			userTrade->RelayToTradeMN(connman);
			userTrade->RelayToBackupMN(connman);
		}
		else if (!userTrade->VerifyMNBalanceSignature())
		{
			LogPrintf("CUserTradeManager::InputUserTrade -- invalid MN balance signature\n");
			Misbehaving(pfrom->GetId(), 100);
			return;
		}
		else if (!userTradesHash.count(userTrade->nMNBalanceHash))
		{
			if (!mapUserTrades.count(userTrade->nUserPubKey))
				mapUserTrades.insert(std::make_pair(userTrade->nUserPubKey, pULTIUTC()));
			auto& a = mapUserTrades[userTrade->nUserPubKey];
			if (!a.second.count(userTrade->nUserTradeID))
			{
				//check if the node processing the trade at that time is valid
				a.second.insert(std::make_pair(userTrade->nUserTradeID, userTrade));
			}
			else
			{
				auto& b = a.second[userTrade->nUserTradeID];
				if (b->nLastUpdate < userTrade->nLastUpdate)
					*b = *userTrade;
			}
		}
	}

	if (userTrade->nMNBalancePubKey == "")
	{		
		userTrade->RelayToBalanceMN(connman);
		return;
	}

	if (!userTrade->VerifyMNBalanceSignature())
	{
		LogPrintf("CUserTradeManager::InputUserTrade -- invalid MN balance signature\n");
		Misbehaving(pfrom->GetId(), 100);
		return;
	}

	if (userBalanceManager.InChargeOfBackup(userTrade->nUserPubKey))
		SaveProcessedUserTrade(userTrade, tradePair);

	CUserTradeSetting& setting = mapUserTradeSetting[userTrade->nTradePairID];
	if (setting.nTradePairID != userTrade->nTradePairID)
		return;

	if (userTrade->nMNTradePubKey == "")
	{
		if (setting.nInChargeOfMatchUserTrade)
		{
			if (userTrade->nIsBid)
				InputMatchUserBuyRequest(userTrade, tradePair);
			else
				InputMatchUserSellRequest(userTrade, tradePair);
		}
	}
	else if (userTrade->nBalanceQty > 0 && userTrade->VerifyMNTradeSignature())
	{
		if (userTrade->nIsBid)
		{
			if (setting.nInChargeOfBidBroadcast)
				orderBookManager.AdjustBidQuantity(userTrade->nTradePairID, userTrade->nPrice, userTrade->nBalanceQty);
		}
		else if (setting.nInChargeOfAskBroadcast)
			orderBookManager.AdjustAskQuantity(userTrade->nTradePairID, userTrade->nPrice, userTrade->nBalanceQty);
	}
}

void CUserTradeManager::InputTradeCancel(CCancelTrade& cancelTrade)
{
	if (!cancelTrade.VerifyUserSignature())
		return;

	CUserTradeSetting& setting = mapUserTradeSetting[cancelTrade.nTradePairID];
	if (setting.nTradePairID != cancelTrade.nTradePairID)
		return;

	if (cancelTrade.nMNTradePubKey != "")
	{
		if (!cancelTrade.VerifyMNSignature())
			return;

		if (userBalanceManager.InChargeOfUserBalance(cancelTrade.nUserPubKey))
			ReturnTradeCancelBalance(cancelTrade);

		if (setting.nInChargeOfBidBroadcast && cancelTrade.isBid)
		{
			orderBookManager.AdjustBidQuantity(cancelTrade.nTradePairID, cancelTrade.nPrice, (0 - cancelTrade.nBalanceQty));			
		}

		if (setting.nInChargeOfAskBroadcast && !cancelTrade.isBid)
		{
			orderBookManager.AdjustAskQuantity(cancelTrade.nTradePairID, cancelTrade.nPrice, (0 - cancelTrade.nBalanceQty));
		}
	}
	else if (setting.nInChargeOfMatchUserTrade)
		ProcessTradeCancelRequest(cancelTrade);
}

void CUserTradeManager::ProcessTradeCancelRequest(CCancelTrade& cancelTrade)
{
	std::shared_ptr<CUserTrade> existingUserTrade;
	if (cancelTrade.isBid)
	{
		if (mapBidUserTradeByPrice.count(cancelTrade.nTradePairID))
		{
			auto& a = mapBidUserTradeByPrice[cancelTrade.nTradePairID];
			if (a.count(cancelTrade.nPrice))
			{
				auto& b = a[cancelTrade.nPrice];
				if (b.count(cancelTrade.nPairTradeID))
				{
					existingUserTrade = b[cancelTrade.nPairTradeID];
					b.erase(cancelTrade.nPairTradeID);
				}
			}
		}
	}
	else if (mapAskUserTradeByPrice.count(cancelTrade.nTradePairID))
	{
		auto& a = mapAskUserTradeByPrice[cancelTrade.nTradePairID];
		if (a.count(cancelTrade.nPrice))
		{
			auto& b = a[cancelTrade.nPrice];
			if (b.count(cancelTrade.nPairTradeID))
			{
				existingUserTrade = b[cancelTrade.nPairTradeID];
				b.erase(cancelTrade.nPairTradeID);
			}
		}
	}

	cancelTrade.nBalanceQty = existingUserTrade->nBalanceQty;
	cancelTrade.nBalanceAmount = existingUserTrade->nBalanceAmount;
	cancelTrade.nMNProcessTime = GetAdjustedTime();
	if (!cancelTrade.MNSign())
		return;

	//broadcast to other node
}

void CUserTradeManager::ReturnTradeCancelBalance(CCancelTrade& cancelTrade)
{
	CTradePair tradePair = tradePairManager.GetTradePair(cancelTrade.nTradePairID);
	if (tradePair.nTradePairID != cancelTrade.nTradePairID)
		return;

	if (mapUserTrades.count(cancelTrade.nUserPubKey))
	{
		auto& a = mapUserTrades[cancelTrade.nUserPubKey];
		if (a.second.count(cancelTrade.nUserTradeID))
		{
			auto& b = a.second[cancelTrade.nUserTradeID];
			if (b->nBalanceAmount == cancelTrade.nBalanceAmount)
			{
				if (b->nIsBid)
					userBalanceManager.ExchangeToBalance(tradePair.nCoinID2, cancelTrade.nUserPubKey, cancelTrade.nBalanceAmount);
				else
					userBalanceManager.ExchangeToBalance(tradePair.nCoinID1, cancelTrade.nUserPubKey, cancelTrade.nBalanceQty);
				a.second.erase(cancelTrade.nUserTradeID);
			}
		}
	}
}

bool CUserTradeManager::ProcessUserTradeRequest(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& tradePair)
{
	int minTradeFee = (userTrade->nIsBid) ? tradePair.nBidTradeFee : tradePair.nAskTradeFee;
	if (minTradeFee > userTrade->nTradeFee)
		return false;

	if (userTrade->nIsBid)
	{
		if (!IsSubmittedBidAmountValid(userTrade, minTradeFee))
			return false;
	}
	else if (!IsSubmittedAskAmountValid(userTrade, minTradeFee))
		return false;

	if (tradePair.nMaximumTradeAmount < userTrade->nAmount || userTrade->nAmount < tradePair.nMinimumTradeAmount)
	{		
		return false;
	}

	int timeDiff = GetAdjustedTime() - userTrade->nTimeSubmit;
	if (timeDiff > 10000 && timeDiff < -10000)
		return false;

	std::shared_ptr<CUserBalance> userBalance;
	if (userTrade->nIsBid)
		userBalanceManager.InitUserBalance(tradePair.nCoinID2, userTrade->nUserPubKey, userBalance);
	else
		userBalanceManager.InitUserBalance(tradePair.nCoinID1, userTrade->nUserPubKey, userBalance);
	
	if (userBalance->nAvailableBalance < userTrade->nAmount)
	{
		return false;
	}

	userBalance->nAvailableBalance -= userTrade->nAmount;
	userBalance->nInExchangeBalance += userTrade->nAmount;

	if (!mapUserTrades.count(userTrade->nUserPubKey))
		mapUserTrades.insert(std::make_pair(userTrade->nUserPubKey, pULTIUTC()));

	auto& a = mapUserTrades[userTrade->nUserPubKey];
	userTrade->nUserTradeID = a.first + 1;
	userTrade->nBalanceAmount = userTrade->nAmount;
	userTrade->nBalanceQty = userTrade->nQuantity;
	userTrade->nMNBalancePubKey = MNPubKey;
	userTrade->nLastUpdate = GetAdjustedTime();
	if (!userTrade->MNBalanceSign())
		return false;
	a.second.insert(std::make_pair(userTrade->nUserTradeID, userTrade));

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
}

void CUserTradeManager::InputMatchUserBuyRequest(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& tradePair)
{	
	CUserTradeSetting& setting = mapUserTradeSetting[userTrade->nTradePairID];
	if (setting.nTradePairID != userTrade->nTradePairID)
		return;	

	mUTPIUTV::iterator Sellers = mapAskUserTradeByPrice[userTrade->nTradePairID].begin();
	while (Sellers != mapAskUserTradeByPrice[userTrade->nTradePairID].end() && Sellers->first <= userTrade->nPrice)
	{
		mINTUT::iterator Seller = Sellers->second.begin();
		while (Seller != Sellers->second.end())
		{
			auto& ExistingTrade = Seller->second;
			if (ExistingTrade->nBalanceQty <= 0)
			{
				if (ExistingTrade->nBalanceAmount <= 0)
				{
					if (ExistingTrade->nBalanceQty == 0)
						mapAskUserTradeByPrice[userTrade->nTradePairID][Sellers->first].erase(Seller->first);
					else
					{
						//not possible to be here but check for security purpose
					}
				}
				else
				{
					//not possible to be here but check for security purpose
				}
				++Seller;
				continue;
			}

			int qty = 0;
			if (userTrade->nBalanceQty <= ExistingTrade->nBalanceQty)
				qty = userTrade->nBalanceQty;
			else
				qty = ExistingTrade->nBalanceQty;

			int bidTradeFee = (userTrade->nTradeFee < tradePair.nAskTradeFee) ? userTrade->nTradeFee : tradePair.nAskTradeFee;
			int askTradeFee = (ExistingTrade->nTradeFee < tradePair.nBidTradeFee) ? ExistingTrade->nTradeFee : tradePair.nBidTradeFee;
			uint64_t bidAmount = GetBidRequiredAmount(ExistingTrade->nPrice, qty, bidTradeFee);
			uint64_t askAmount = GetAskExpectedAmount(ExistingTrade->nPrice, qty, askTradeFee);			
			std::shared_ptr<CActualTrade> actualTrade = std::make_shared<CActualTrade>(tradePair.nTradePairID, userTrade->nUserTradeID, ExistingTrade->nUserTradeID, ExistingTrade->nPrice, qty, bidAmount, askAmount, userTrade->nUserPubKey, ExistingTrade->nUserPubKey, bidTradeFee, askTradeFee, true, GetAdjustedTime());
			if (!actualTradeManager.GenerateActualTrade(actualTrade, setting))
				return;
			
			ExistingTrade->nBalanceQty -= qty;
			ExistingTrade->nBalanceAmount -= askAmount;			
			userTrade->nBalanceQty -= qty;
			userTrade->nBalanceAmount -= bidAmount;
			actualTradeManager.InputActualTrade(actualTrade, setting, tradePair);
			if (setting.nInChargeOfAskBroadcast)
				orderBookManager.AdjustAskQuantity(actualTrade->nTradePairID, actualTrade->nTradePrice, (0 - actualTrade->nTradeQty));
			
			if (ExistingTrade->nBalanceQty == 0)
			{				
				mapAskUserTradeByPrice[userTrade->nTradePairID][Sellers->first].erase(Seller->first);
			}
			++Seller;

			if (userTrade->nBalanceQty <= 0)
			{				
				if (userTrade->nBalanceAmount > 0)
				{
					if (userBalanceManager.InChargeOfUserBalance(userTrade->nUserPubKey))
						userBalanceManager.ExchangeToBalanceV2(tradePair.nCoinID2, userTrade->nUserPubKey, userTrade->nBalanceAmount);
				}
				return;
			}
		}
		++Sellers;
	}

	auto& a = mapBidUserTradeByPrice[userTrade->nTradePairID];
	if (!a.count(userTrade->nPrice))
	{
		mINTUT temp;
		temp.insert(std::make_pair(userTrade->nUserTradeID, userTrade));
		a.insert(std::make_pair(userTrade->nPrice, temp));
	}
	else
		a[userTrade->nPrice].insert(std::make_pair(userTrade->nUserTradeID, userTrade));

	if (setting.nInChargeOfBidBroadcast)
		orderBookManager.AdjustBidQuantity(userTrade->nTradePairID, userTrade->nPrice, userTrade->nBalanceQty);
}

void CUserTradeManager::InputMatchUserSellRequest(const std::shared_ptr<CUserTrade>& userTrade, CTradePair& tradePair)
{	
	CUserTradeSetting& actualTradeSetting = mapUserTradeSetting[userTrade->nTradePairID];
	if (actualTradeSetting.nTradePairID != userTrade->nTradePairID)
		return;

	mUTPIUTV::reverse_iterator Buyers = mapBidUserTradeByPrice[userTrade->nTradePairID].rbegin();
	while (Buyers != mapBidUserTradeByPrice[userTrade->nTradePairID].rend() && Buyers->first >= userTrade->nPrice)
	{
		mINTUT::iterator Buyer = Buyers->second.begin();
		while (Buyer != Buyers->second.end())
		{
			auto& ExistingTrade = Buyer->second;
			if (ExistingTrade->nBalanceQty <= 0)
			{
				if (ExistingTrade->nBalanceAmount <= 0)
				{
					if (ExistingTrade->nBalanceQty == 0)
						mapBidUserTradeByPrice[userTrade->nTradePairID][Buyers->first].erase(Buyer->first);
					else
					{
						//not possible to be here but check for security purpose
					}
				}
				else
				{
					//not possible to be here but check for security purpose
				}
				++Buyer;
				continue;
			}

			int qty = 0;
			if (userTrade->nBalanceQty <= ExistingTrade->nBalanceQty)
				qty = userTrade->nBalanceQty;
			else
				qty = ExistingTrade->nBalanceQty;

			int bidTradeFee = (ExistingTrade->nTradeFee < tradePair.nBidTradeFee) ? ExistingTrade->nTradeFee : tradePair.nBidTradeFee;
			int askTradeFee = (userTrade->nTradeFee < tradePair.nAskTradeFee) ? userTrade->nTradeFee : tradePair.nAskTradeFee;
			uint64_t bidAmount = GetBidRequiredAmount(ExistingTrade->nPrice, qty, bidTradeFee);
			uint64_t askAmount = GetAskExpectedAmount(ExistingTrade->nPrice, qty, askTradeFee);			
			std::shared_ptr<CActualTrade> actualTrade = std::make_shared<CActualTrade>(tradePair.nTradePairID, ExistingTrade->nUserTradeID, userTrade->nUserTradeID, ExistingTrade->nPrice, qty, bidAmount, askAmount, ExistingTrade->nUserPubKey, userTrade->nUserPubKey, bidTradeFee, askTradeFee, false, GetAdjustedTime());
			if (!actualTradeManager.GenerateActualTrade(actualTrade, actualTradeSetting))
				return;

			ExistingTrade->nBalanceQty -= qty;
			ExistingTrade->nBalanceAmount -= bidAmount;			
			userTrade->nBalanceQty -= qty;			
			userTrade->nBalanceAmount -= askAmount;
			actualTradeManager.InputActualTrade(actualTrade, actualTradeSetting, tradePair);
			if (actualTradeSetting.nInChargeOfBidBroadcast)
				orderBookManager.AdjustBidQuantity(actualTrade->nTradePairID, actualTrade->nTradePrice, (0 - actualTrade->nTradeQty));

			if (ExistingTrade->nBalanceQty == 0)
			{
				mapBidUserTradeByPrice[userTrade->nTradePairID][Buyers->first].erase(Buyer->first);
			}
			++Buyer;

			if (userTrade->nBalanceQty <= 0)
			{
				return;
			}
		}
		++Buyers;
	}

	auto& a = mapAskUserTradeByPrice[userTrade->nTradePairID];
	if (!a.count(userTrade->nPrice))
	{
		mINTUT temp;
		temp.insert(std::make_pair(userTrade->nUserTradeID, userTrade));
		a.insert(std::make_pair(userTrade->nPrice, temp));
	}
	else
		a[userTrade->nPrice].insert(std::make_pair(userTrade->nUserTradeID, userTrade));

	if (actualTradeSetting.nInChargeOfAskBroadcast)
		orderBookManager.AdjustAskQuantity(userTrade->nTradePairID, userTrade->nPrice, userTrade->nBalanceQty);
}

bool CActualTradeManager::GenerateActualTrade(std::shared_ptr<CActualTrade> actualTrade, CUserTradeSetting& setting)
{
	actualTrade->nActualTradeID = (setting.nLastActualTradeID + 1);
	actualTrade->nMasternodeInspector = MNPubKey;
	setting.nLastActualTradeTime = actualTrade->nTradeTime;
	actualTrade->nCurrentHash = actualTrade->GetHash();
	//to enable on actual implementation, testing phase disable
	//if (mapActualTradeHash[actualTrade->nTradePairID].count(actualTrade->nCurrentHash))
		//return false;
	if (!actualTrade->Sign())
		return false;
	++setting.nLastActualTradeID;
	return true;
}

bool CActualTradeManager::InputActualTrade(const std::shared_ptr<CActualTrade>& actualTrade, CUserTradeSetting& setting, CTradePair& tradePair)
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
	
	if (userBalanceManager.InChargeOfUserBalance(actualTrade->nUserPubKey1))
		userBalanceManager.UpdateAfterTradeBalance(actualTrade->nUserPubKey1, tradePair.nCoinID2, tradePair.nCoinID1, actualTrade->nBidAmount, actualTrade->nTradeQty);
	if (userBalanceManager.InChargeOfUserBalance(actualTrade->nUserPubKey2))
		userBalanceManager.UpdateAfterTradeBalance(actualTrade->nUserPubKey2, tradePair.nCoinID1, tradePair.nCoinID2, actualTrade->nTradeQty, actualTrade->nAskAmount);
	if (setting.nInChargeOfChartData)
		ChartDataManager.InputNewTrade(actualTrade->nTradePairID, actualTrade->nTradePrice, actualTrade->nTradeQty, actualTrade->nTradeTime);
	if (setting.nInChargeOfMarketTradeHistory || setting.nInChargeOfUserTradeHistory)
	{
		std::shared_ptr<CUserTradeHistory> tradeHistory = std::make_shared<CUserTradeHistory>(actualTrade->nTradePairID, actualTrade->nUserPubKey1, actualTrade->nUserPubKey2, actualTrade->nTradePrice, actualTrade->nTradeQty, actualTrade->nTradeAmount, false, actualTrade->nTradeTime);
		if (setting.nInChargeOfMarketTradeHistory)
			userTradeHistoryManager.InputMarketTradeHistory(tradeHistory);
		if (setting.nInChargeOfUserTradeHistory)
			userTradeHistoryManager.InputUserTradeHistory(tradeHistory);
	}
	
	actualTrade->Relay();
	return true;
}

bool CActualTradeManager::InputActualTradeFromNetwork(const std::shared_ptr<CActualTrade>& actualTrade, CUserTradeSetting& setting, CTradePair& tradePair)
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

	if (userBalanceManager.InChargeOfUserBalance(actualTrade->nUserPubKey1) || userBalanceManager.InChargeOfBackup(actualTrade->nUserPubKey1))
	{
		if (mapUserTrades.count(actualTrade->nUserPubKey1))
		{
			auto& a = mapUserTrades[actualTrade->nUserPubKey1];
			if (a.first >= actualTrade->nUserTrade1)
			{
				auto& b = a.second[actualTrade->nUserTrade1];
				if (b->nPrice >= actualTrade->nTradePrice)
				{
					b->nBalanceQty -= actualTrade->nTradeQty;
					b->nBalanceAmount -= actualTrade->nBidAmount;
					if (b->nBalanceQty == 0)
					{
						if (b->nBalanceAmount > 0)
							userBalanceManager.ExchangeToBalanceV2(tradePair.nCoinID2, b->nUserPubKey, b->nBalanceAmount);
					}
					else if (b->nBalanceQty < 0)
					{

					}
					userBalanceManager.UpdateAfterTradeBalance(actualTrade->nUserPubKey1, tradePair.nCoinID2, tradePair.nCoinID1, actualTrade->nBidAmount, actualTrade->nTradeQty);
				}
				else
				{

				}
			}
			else
			{
				//is it a newly assigned node?
			}
		}
	}

	if (userBalanceManager.InChargeOfUserBalance(actualTrade->nUserPubKey2) || userBalanceManager.InChargeOfBackup(actualTrade->nUserPubKey2))
	{
		if (mapUserTrades.count(actualTrade->nUserPubKey2))
		{
			auto& a = mapUserTrades[actualTrade->nUserPubKey2];
			if (a.first >= actualTrade->nUserTrade2)
			{
				auto& b = a.second[actualTrade->nUserTrade2];
				if (b->nPrice <= actualTrade->nTradePrice)
				{
					b->nBalanceQty -= actualTrade->nTradeQty;
					b->nBalanceAmount -= actualTrade->nAskAmount;
					if (b->nBalanceQty == 0)
					{
						if (b->nBalanceAmount > 0)
							userBalanceManager.ExchangeToBalanceV2(tradePair.nCoinID1, b->nUserPubKey, b->nBalanceAmount);
					}
					else if (b->nBalanceQty < 0)
					{

					}
					userBalanceManager.UpdateAfterTradeBalance(actualTrade->nUserPubKey2, tradePair.nCoinID1, tradePair.nCoinID2, actualTrade->nTradeQty, actualTrade->nAskAmount);
				}
				else
				{

				}
			}
			else
			{
				//is it a newly assigned node?
			}
		}
	}
	if (setting.nInChargeOfChartData)
		ChartDataManager.InputNewTrade(actualTrade->nTradePairID, actualTrade->nTradePrice, actualTrade->nTradeQty, actualTrade->nTradeTime);
	if (setting.nInChargeOfMarketTradeHistory)
	{
		std::shared_ptr<CUserTradeHistory> tradeHistory = std::make_shared<CUserTradeHistory>(actualTrade->nTradePairID, actualTrade->nUserPubKey1, actualTrade->nUserPubKey2, actualTrade->nTradePrice, actualTrade->nTradeQty, actualTrade->nTradeAmount, false, actualTrade->nTradeTime);
		userTradeHistoryManager.InputMarketTradeHistory(tradeHistory);
	}
	if (setting.nInChargeOfUserTradeHistory)
	{
		std::shared_ptr<CUserTradeHistory> tradeHistory = std::make_shared<CUserTradeHistory>(actualTrade->nTradePairID, actualTrade->nUserPubKey1, actualTrade->nUserPubKey2, actualTrade->nTradePrice, actualTrade->nTradeQty, actualTrade->nTradeAmount, false, actualTrade->nTradeTime);
		userTradeHistoryManager.InputUserTradeHistory(tradeHistory);
	}
	if (setting.nInChargeOfBidBroadcast)
	{
		if (!actualTrade->nFromBid)
		{
			orderBookManager.AdjustBidQuantity(actualTrade->nTradePairID, actualTrade->nTradePrice, actualTrade->nTradeQty);
			//broadcast
		}
	}
	if (setting.nInChargeOfAskBroadcast)
	{
		if (actualTrade->nFromBid)
		{
			orderBookManager.AdjustAskQuantity(actualTrade->nTradePairID, actualTrade->nTradePrice, actualTrade->nTradeQty);
			//broadcast
		}
	}

	return true;
}

void CUserTradeManager::InitTradePair(int TradePairID)
{
	if (mapUserTradeSetting.count(TradePairID))
		return;

	mapBidUserTradeByPrice.insert(std::make_pair(TradePairID, mUTPIUTV()));
	mapAskUserTradeByPrice.insert(std::make_pair(TradePairID, mUTPIUTV()));
	mapUserTradeSetting.insert(std::make_pair(TradePairID, CUserTradeSetting(TradePairID)));
	mapActualTradeByActualTradeID.insert(std::make_pair(TradePairID, mATIAT()));
	mapActualTradeByUserTradeID.insert(std::make_pair(TradePairID, mUTImAT()));
	mapConflictTrade.insert(std::make_pair(TradePairID, std::vector<CActualTrade>()));
	mapActualTradeHash.insert(std::make_pair(TradePairID, std::set<std::string>()));
}

void CUserTradeManager::AssignBidBroadcastRole(int TradePairID, bool toAssign)
{
	InitTradePair(TradePairID);
	mapUserTradeSetting[TradePairID].nInChargeOfBidBroadcast = toAssign;
}

void CUserTradeManager::AssignAskBroadcastRole(int TradePairID, bool toAssign)
{
	InitTradePair(TradePairID);
	mapUserTradeSetting[TradePairID].nInChargeOfAskBroadcast = toAssign;
}

void CUserTradeManager::AssignMatchUserTradeRole(int TradePairID, bool toAssign)
{
	InitTradePair(TradePairID);
	mapUserTradeSetting[TradePairID].nInChargeOfMatchUserTrade = toAssign;
}

void CUserTradeManager::AssignUserHistoryProviderRole(int TradePairID, bool toAssign)
{
	InitTradePair(TradePairID);
	mapUserTradeSetting[TradePairID].nInChargeOfUserTradeHistory = toAssign;
}

void CUserTradeManager::AssignMarketHistoryProviderRole(int TradePairID, bool toAssign)
{
	InitTradePair(TradePairID);
	mapUserTradeSetting[TradePairID].nInChargeOfMarketTradeHistory = toAssign;
}

void CUserTradeManager::AssignChartDataProviderRole(int TradePairID, bool toAssign)
{
	InitTradePair(TradePairID);
	mapUserTradeSetting[TradePairID].nInChargeOfChartData = toAssign;
}

uint64_t CUserTradeManager::GetBidRequiredAmount(uint64_t Price, uint64_t Qty, int TradeFee)
{
	//overflow prevention
	boost::multiprecision::uint128_t amount = Price * Qty * 10000 / (10000 - TradeFee);
	return (uint64_t)amount;
}

uint64_t CUserTradeManager::GetAskExpectedAmount(uint64_t Price, uint64_t Qty, int TradeFee)
{
	//overflow prevention
	boost::multiprecision::uint128_t amount = Price * Qty * 10000 / (10000 + TradeFee);
	return (uint64_t)amount;
}

bool CUserTradeManager::IsSubmittedBidAmountValid(const std::shared_ptr<CUserTrade>& userTrade, int nTradeFee)
{
	uint64_t ExpectedAmount = GetBidRequiredAmount(userTrade->nPrice, userTrade->nQuantity, nTradeFee);
	if ((ExpectedAmount - 1) <= userTrade->nAmount && userTrade->nAmount <= (ExpectedAmount + 1))
		return true;
	return false;
}

bool CUserTradeManager::IsSubmittedAskAmountValid(const std::shared_ptr<CUserTrade>& userTrade, int nTradeFee)
{
	uint64_t ExpectedAmount = GetAskExpectedAmount(userTrade->nPrice, userTrade->nQuantity, nTradeFee);
	if ((ExpectedAmount - 1) <= userTrade->nAmount && userTrade->nAmount <= (ExpectedAmount + 1))
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

bool CUserTradeSetting::IsValidSubmissionTimeAndUpdate(uint64_t time)
{
	int diff = nLastPairTradeTime - time;
	if (diff > nMaxSubmissionTimeDiff)
		return false;

	nLastPairTradeTime = time;
	return true;
}

std::string CActualTrade::GetHash()
{
	return "";
}

bool CUserTradeManager::ReduceBalanceQty(int TradePairID, int UserTradeID1, int UserTradeID2, uint64_t Qty)
{
	return true;
}

int64_t CUserTradeManager::GetBalanceAmount(int TradePairID, uint64_t Price, int UserTradeID)
{
	return 0;
}

bool CActualTradeManager::IsActualTradeInList(int TradePairID, int ActualTradeID, std::string Hash)
{
	return true;
}

bool CUserTrade::VerifyUserSignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nPrice) + boost::lexical_cast<std::string>(nQuantity)
		+ boost::lexical_cast<std::string>(nAmount) + boost::lexical_cast<std::string>(nIsBid)+ boost::lexical_cast<std::string>(nTradeFee) + nUserPubKey
		+ boost::lexical_cast<std::string>(nTimeSubmit) + nUserHash;
	CPubKey pubkey(ParseHex(nUserPubKey));

	if (!CMessageSigner::VerifyMessage(pubkey, userVchSig, strMessage, strError)) {
		LogPrintf("CUserTrade::VerifyUserSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CUserTrade::VerifyMNBalanceSignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nPrice) + boost::lexical_cast<std::string>(nQuantity)
		+ boost::lexical_cast<std::string>(nAmount) + boost::lexical_cast<std::string>(nIsBid)+ boost::lexical_cast<std::string>(nTradeFee) + nUserPubKey
		+ boost::lexical_cast<std::string>(nTimeSubmit) + nUserHash + boost::lexical_cast<std::string>(nUserTradeID) + nMNBalancePubKey
		+ boost::lexical_cast<std::string>(nBalanceQty) + boost::lexical_cast<std::string>(nBalanceAmount) + boost::lexical_cast<std::string>(nLastUpdate);
	CPubKey pubkey(ParseHex(nMNBalancePubKey));

	if (!CMessageSigner::VerifyMessage(pubkey, mnBalanceVchSig, strMessage, strError)) {
		LogPrintf("CUserTrade::VerifyUserSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CUserTrade::VerifyMNTradeSignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nPrice) + boost::lexical_cast<std::string>(nQuantity)
		+ boost::lexical_cast<std::string>(nAmount) + boost::lexical_cast<std::string>(nIsBid)+ boost::lexical_cast<std::string>(nTradeFee) + nUserPubKey
		+ boost::lexical_cast<std::string>(nTimeSubmit) + nUserHash + boost::lexical_cast<std::string>(nUserTradeID) + boost::lexical_cast<std::string>(nPairTradeID)
		+ nMNBalancePubKey + nMNTradePubKey	+ boost::lexical_cast<std::string>(nBalanceQty) + boost::lexical_cast<std::string>(nBalanceAmount) 
		+ boost::lexical_cast<std::string>(nLastUpdate);
	CPubKey pubkey(ParseHex(nMNTradePubKey));

	if (!CMessageSigner::VerifyMessage(pubkey, mnTradeVchSig, strMessage, strError)) {
		LogPrintf("CUserTrade::VerifyMNTradeSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CUserTrade::MNBalanceSign()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nPrice) + boost::lexical_cast<std::string>(nQuantity)
		+ boost::lexical_cast<std::string>(nAmount) + boost::lexical_cast<std::string>(nIsBid) + boost::lexical_cast<std::string>(nTradeFee) + nUserPubKey
		+ boost::lexical_cast<std::string>(nTimeSubmit) + nUserHash + boost::lexical_cast<std::string>(nUserTradeID) + nMNBalancePubKey
		+ boost::lexical_cast<std::string>(nBalanceQty) + boost::lexical_cast<std::string>(nBalanceAmount) + boost::lexical_cast<std::string>(nLastUpdate);

	if (!CMessageSigner::SignMessage(strMessage, mnBalanceVchSig, activeMasternode.keyMasternode))
	{
		LogPrintf("CUserTrade::MNBalanceSign -- SignMessage() failed\n");
		return false;
	}

	if (!CMessageSigner::VerifyMessage(activeMasternode.pubKeyMasternode, mnBalanceVchSig, strMessage, strError))
	{
		LogPrintf("CUserTrade::MNBalanceSign -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CUserTrade::MNTradeSign()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nPrice) + boost::lexical_cast<std::string>(nQuantity)
		+ boost::lexical_cast<std::string>(nAmount) + boost::lexical_cast<std::string>(nIsBid)+ boost::lexical_cast<std::string>(nTradeFee) + nUserPubKey
		+ boost::lexical_cast<std::string>(nTimeSubmit) + nUserHash + boost::lexical_cast<std::string>(nUserTradeID) + boost::lexical_cast<std::string>(nPairTradeID)
		+ nMNBalancePubKey + nMNTradePubKey	+ boost::lexical_cast<std::string>(nBalanceQty) + boost::lexical_cast<std::string>(nBalanceAmount) 
		+ boost::lexical_cast<std::string>(nLastUpdate);

	if (!CMessageSigner::SignMessage(strMessage, mnTradeVchSig, activeMasternode.keyMasternode)) 
	{
		LogPrintf("CUserTrade::MNTradeSign -- SignMessage() failed\n");
		return false;
	}

	if (!CMessageSigner::VerifyMessage(activeMasternode.pubKeyMasternode, mnTradeVchSig, strMessage, strError)) 
	{
		LogPrintf("CUserTrade::MNTradeSign -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CCancelTrade::MNSign()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nUserTradeID) + boost::lexical_cast<std::string>(nPairTradeID) + nUserPubKey + boost::lexical_cast<std::string>(isBid)
		+ boost::lexical_cast<std::string>(nUserSubmitTime) + boost::lexical_cast<std::string>(nPrice) + boost::lexical_cast<std::string>(nBalanceQty)
		+ boost::lexical_cast<std::string>(nBalanceAmount) + nMNTradePubKey + boost::lexical_cast<std::string>(nMNProcessTime);

	if (!CMessageSigner::SignMessage(strMessage, mnVchSig, activeMasternode.keyMasternode))
	{
		LogPrintf("CCancelTrade::MNSign -- SignMessage() failed\n");
		return false;
	}

	if (!CMessageSigner::VerifyMessage(activeMasternode.pubKeyMasternode, mnVchSig, strMessage, strError))
	{
		LogPrintf("CCancelTrade::MNSign -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CCancelTrade::VerifyUserSignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nUserTradeID) + boost::lexical_cast<std::string>(nPairTradeID) + nUserPubKey + boost::lexical_cast<std::string>(isBid)
		+ boost::lexical_cast<std::string>(nUserSubmitTime) + boost::lexical_cast<std::string>(nPrice);
	CPubKey pubkey(ParseHex(nUserPubKey));

	if (!CMessageSigner::VerifyMessage(pubkey, userVchSig, strMessage, strError)) {
		LogPrintf("CCancelTrade::VerifyUserSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CCancelTrade::VerifyMNSignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nUserTradeID) + boost::lexical_cast<std::string>(nPairTradeID) + nUserPubKey + boost::lexical_cast<std::string>(isBid)
		+ boost::lexical_cast<std::string>(nUserSubmitTime) + boost::lexical_cast<std::string>(nPrice)+ boost::lexical_cast<std::string>(nBalanceQty) 
		+ boost::lexical_cast<std::string>(nBalanceAmount) + nMNTradePubKey + boost::lexical_cast<std::string>(nMNProcessTime);
	CPubKey pubkey(ParseHex(MNPubKey));

	if (!CMessageSigner::VerifyMessage(pubkey, mnVchSig, strMessage, strError)) {
		LogPrintf("CCancelTrade::VerifyMNSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

void CUserTrade::RelayTo(CNode* node, CConnman& connman)
{

}

void CUserTrade::RelayToBalanceMN(CConnman& connman)
{
	
}

void CUserTrade::RelayToTradeMN(CConnman& connman)
{

}

void CUserTrade::RelayToBackupMN(CConnman& connman)
{

}

bool CActualTrade::TradeMNSign()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nActualTradeID) + boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nUserTrade1)
		+ boost::lexical_cast<std::string>(nUserTrade2) + boost::lexical_cast<std::string>(nTradePrice) + boost::lexical_cast<std::string>(nTradeQty)
		+ boost::lexical_cast<std::string>(nTradeAmount) + boost::lexical_cast<std::string>(nBidAmount) + boost::lexical_cast<std::string>(nAskAmount) + nUserPubKey1
		+ nUserPubKey2 + boost::lexical_cast<std::string>(nFee1) + boost::lexical_cast<std::string>(nFee2) + boost::lexical_cast<std::string>(nFromBid) + nTradeMNPubKey
		+ nCurrentHash + boost::lexical_cast<std::string>(nTradeTime);

	if (!CMessageSigner::SignMessage(strMessage, mnTradeVchSig, activeMasternode.keyMasternode))
	{
		LogPrintf("CActualTrade::TradeMNSign -- SignMessage() failed\n");
		return false;
	}

	if (!CMessageSigner::VerifyMessage(activeMasternode.pubKeyMasternode, mnTradeVchSig, strMessage, strError))
	{
		LogPrintf("CActualTrade::TradeMNSign -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CActualTrade::Balance1MNSign()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nActualTradeID) + boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nUserTrade1)
		+ boost::lexical_cast<std::string>(nUserTrade2) + boost::lexical_cast<std::string>(nTradePrice) + boost::lexical_cast<std::string>(nTradeQty)
		+ boost::lexical_cast<std::string>(nTradeAmount) + boost::lexical_cast<std::string>(nBidAmount) + boost::lexical_cast<std::string>(nAskAmount) + nUserPubKey1
		+ nUserPubKey2 + boost::lexical_cast<std::string>(nFee1) + boost::lexical_cast<std::string>(nFee2) + boost::lexical_cast<std::string>(nFromBid) + nTradeMNPubKey
		+ nBalance1MNPubKey + nCurrentHash + boost::lexical_cast<std::string>(nTradeTime);

	if (!CMessageSigner::SignMessage(strMessage, mnBalance1VchSig, activeMasternode.keyMasternode))
	{
		LogPrintf("CActualTrade::TradeMNSign -- SignMessage() failed\n");
		return false;
	}

	if (!CMessageSigner::VerifyMessage(activeMasternode.pubKeyMasternode, mnBalance1VchSig, strMessage, strError))
	{
		LogPrintf("CActualTrade::TradeMNSign -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CActualTrade::Balance2MNSign()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nActualTradeID) + boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nUserTrade1)
		+ boost::lexical_cast<std::string>(nUserTrade2) + boost::lexical_cast<std::string>(nTradePrice) + boost::lexical_cast<std::string>(nTradeQty)
		+ boost::lexical_cast<std::string>(nTradeAmount) + boost::lexical_cast<std::string>(nBidAmount) + boost::lexical_cast<std::string>(nAskAmount) + nUserPubKey1
		+ nUserPubKey2 + boost::lexical_cast<std::string>(nFee1) + boost::lexical_cast<std::string>(nFee2) + boost::lexical_cast<std::string>(nFromBid) + nTradeMNPubKey
		+ nBalance2MNPubKey + nCurrentHash + boost::lexical_cast<std::string>(nTradeTime);

	if (!CMessageSigner::SignMessage(strMessage, mnBalance2VchSig, activeMasternode.keyMasternode))
	{
		LogPrintf("CActualTrade::TradeMNSign -- SignMessage() failed\n");
		return false;
	}

	if (!CMessageSigner::VerifyMessage(activeMasternode.pubKeyMasternode, mnBalance2VchSig, strMessage, strError))
	{
		LogPrintf("CActualTrade::TradeMNSign -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CActualTrade::VerifyTradeMNSignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nActualTradeID) + boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nUserTrade1)
		+ boost::lexical_cast<std::string>(nUserTrade2) + boost::lexical_cast<std::string>(nTradePrice) + boost::lexical_cast<std::string>(nTradeQty)
		+ boost::lexical_cast<std::string>(nTradeAmount) + boost::lexical_cast<std::string>(nBidAmount) + boost::lexical_cast<std::string>(nAskAmount) + nUserPubKey1
		+ nUserPubKey2 + boost::lexical_cast<std::string>(nFee1) + boost::lexical_cast<std::string>(nFee2) + boost::lexical_cast<std::string>(nFromBid) + nTradeMNPubKey
		+ nCurrentHash + boost::lexical_cast<std::string>(nTradeTime);
	CPubKey pubkey(ParseHex(nTradeMNPubKey));

	if (!CMessageSigner::VerifyMessage(pubkey, mnTradeVchSig, strMessage, strError)) {
		LogPrintf("CActualTrade::VerifyTradeMNSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CActualTrade::VerifyBalance1MNSignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nActualTradeID) + boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nUserTrade1)
		+ boost::lexical_cast<std::string>(nUserTrade2) + boost::lexical_cast<std::string>(nTradePrice) + boost::lexical_cast<std::string>(nTradeQty)
		+ boost::lexical_cast<std::string>(nTradeAmount) + boost::lexical_cast<std::string>(nBidAmount) + boost::lexical_cast<std::string>(nAskAmount) + nUserPubKey1
		+ nUserPubKey2 + boost::lexical_cast<std::string>(nFee1) + boost::lexical_cast<std::string>(nFee2) + boost::lexical_cast<std::string>(nFromBid) + nTradeMNPubKey
		+ nBalance1MNPubKey + nCurrentHash + boost::lexical_cast<std::string>(nTradeTime);
	CPubKey pubkey(ParseHex(nBalance1MNPubKey));

	if (!CMessageSigner::VerifyMessage(pubkey, mnBalance1VchSig, strMessage, strError)) {
		LogPrintf("CActualTrade::VerifyTradeMNSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CActualTrade::VerifyBalance2MNSignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nActualTradeID) + boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nUserTrade1)
		+ boost::lexical_cast<std::string>(nUserTrade2) + boost::lexical_cast<std::string>(nTradePrice) + boost::lexical_cast<std::string>(nTradeQty)
		+ boost::lexical_cast<std::string>(nTradeAmount) + boost::lexical_cast<std::string>(nBidAmount) + boost::lexical_cast<std::string>(nAskAmount) + nUserPubKey1
		+ nUserPubKey2 + boost::lexical_cast<std::string>(nFee1) + boost::lexical_cast<std::string>(nFee2) + boost::lexical_cast<std::string>(nFromBid) + nTradeMNPubKey
		+ nBalance2MNPubKey + nCurrentHash + boost::lexical_cast<std::string>(nTradeTime);
	CPubKey pubkey(ParseHex(nBalance2MNPubKey));

	if (!CMessageSigner::VerifyMessage(pubkey, mnBalance1VchSig, strMessage, strError)) {
		LogPrintf("CActualTrade::VerifyTradeMNSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}