// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "userbalance.h"
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

class CUserBalance;
class CUserBalanceManager;

std::map<std::string, mapUserBalanceWithCoinID> mapUserBalanceByPubKey;
std::map<int, mapUserBalanceWithPubKey> mapUserBalanceByCoinID;
std::map<int, CUserBalanceSetting> mapUserBalanceSetting;
CGlobalUserBalanceHandler globalUserBalanceHandler;
CUserBalanceManager userBalanceManager;
std::set<char> globalInChargeDeposit;

bool CUserBalance::VerifySignature()
{
	return true;
}

void CUserBalanceManager::InitCoin(int CoinID)
{
	if (mapUserBalanceByCoinID.count(CoinID))
		return;

	mapUserBalanceSetting.insert(std::make_pair(CoinID, CUserBalanceSetting(CoinID)));
	mapUserBalanceByCoinID.insert(std::make_pair(CoinID, mapUserBalanceWithPubKey()));
}

bool CUserBalanceManager::AssignUserBalanceRole(char Char1, char Char2, bool toAssign)
{
	globalUserBalanceHandler.nIsInChargeOfGlobalUserBalance = toAssign;
	return true;
}

bool CUserBalanceManager::UpdateUserBalance(CUserBalance UserBalance)
{
	if (!UserBalance.VerifySignature())
		return false;

	if (!IsInChargeOfGlobalCoinBalance())
		return false;

	std::shared_ptr<CUserBalance> ub = std::make_shared<CUserBalance>(UserBalance);

	if(!mapUserBalanceByCoinID.count(ub->nCoinID))
		mapUserBalanceByCoinID.insert(std::make_pair(UserBalance.nCoinID, mapUserBalanceWithPubKey()));
	mapUserBalanceWithPubKey& temp1 = mapUserBalanceByCoinID[UserBalance.nCoinID];
	if (!temp1.count(UserBalance.nUserPubKey))
		temp1.insert(std::make_pair(UserBalance.nUserPubKey, ub));
	else if (temp1[UserBalance.nUserPubKey]->nLastUpdateTime < UserBalance.nLastUpdateTime)
		temp1[UserBalance.nUserPubKey] = ub;

	if (!mapUserBalanceByPubKey.count(UserBalance.nUserPubKey))
		mapUserBalanceByPubKey.insert(std::make_pair(UserBalance.nUserPubKey, mapUserBalanceWithCoinID()));
	mapUserBalanceWithCoinID& temp2 = mapUserBalanceByPubKey[UserBalance.nUserPubKey];
	if (!temp2.count(UserBalance.nCoinID))
		temp2.insert(std::make_pair(UserBalance.nCoinID, ub));
	else if (temp2[UserBalance.nCoinID]->nLastUpdateTime < UserBalance.nLastUpdateTime)
		temp2[UserBalance.nCoinID] = ub;

	return true;
}

bool CUserBalanceManager::GetUserBalance(int CoinID, std::string UserPubKey, std::shared_ptr<CUserBalance>& UserBalance)
{
	if (!mapUserBalanceByCoinID.count(CoinID))
		return false;

	mapUserBalanceWithPubKey& temp = mapUserBalanceByCoinID[CoinID];
	if (!temp.count(UserPubKey))
		return false;

	UserBalance = temp[UserPubKey];
	return true;
}

int CUserBalanceManager::GetLastDepositID(int CoinID, std::string UserPubKey)
{
	return mapUserBalanceByCoinID[CoinID][UserPubKey]->nLastDepositID;
}

bool CUserBalanceManager::IsInChargeOfGlobalCoinBalance()
{
	return globalUserBalanceHandler.nIsInChargeOfGlobalUserBalance;
}

bool CUserBalanceManager::IsInChargeOfCoinBalance(int CoinID)
{
	return mapUserBalanceSetting[CoinID].nIsInChargeOfUserBalance;
}

bool CUserBalanceManager::VerifyUserBalance(int CoinID)
{
	return mapUserBalanceSetting[CoinID].nVerifyUserBalance;
}

bool CUserBalanceManager::IsCoinInList(int CoinID)
{
	return mapUserBalanceByCoinID.count(CoinID);
}

bool CUserBalanceManager::IsFurtherInTime(int CoinID, std::string UserPubKey, uint64_t time)
{
	return (time > mapUserBalanceByCoinID[CoinID][UserPubKey]->nInSyncTime);
}

bool CUserBalanceManager::IsUserBalanceExist(int CoinID, std::string UserPubKey)
{
	return (mapUserBalanceByCoinID[CoinID].count(UserPubKey));
}

userbalance_to_exchange_enum_t CUserBalanceManager::BalanceToExchange(int CoinID, std::string UserPubKey, uint64_t amount)
{
	CUserBalanceSetting& setting = mapUserBalanceSetting[CoinID];
	if (setting.nCoinID != CoinID)
		return USER_BALANCE_INVALID_NODE;

	mapUserBalanceWithPubKey& temp = mapUserBalanceByCoinID[CoinID];
	if (!temp.count(UserPubKey))
		return USER_ACCOUNT_NOT_FOUND;

	auto& temp2 = temp[UserPubKey];

	if (setting.nVerifyUserBalance)
	{
		if (temp2->nAvailableBalance < amount)
			return USER_BALANCE_NOT_ENOUGH;
	}

	temp2->nAvailableBalance -= amount;
	temp2->nInExchangeBalance += amount;
	return USER_BALANCE_DEDUCTED;
}

exchange_to_userbalance_enum_t CUserBalanceManager::ExchangeToBalance(int CoinID, std::string UserPubKey, uint64_t amount)
{
	CUserBalanceSetting& setting = mapUserBalanceSetting[CoinID];
	if (setting.nCoinID != CoinID)
		return EXCHANGE_INVALID_NODE;

	mapUserBalanceWithPubKey& temp = mapUserBalanceByCoinID[CoinID];
	if (!temp.count(UserPubKey))
		return EXCHANGE_ACCOUNT_NOT_FOUND;

	auto& temp2 = temp[UserPubKey];

	if (setting.nVerifyUserBalance)
	{
		if (temp2->nInExchangeBalance < amount)
			return EXCHANGE_BALANCE_NOT_ENOUGH;
	}

	temp2->nAvailableBalance += amount;
	temp2->nInExchangeBalance -= amount;
	return EXCHANGE_BALANCE_RETURNED;
}

int64_t CUserBalanceManager::GetUserAvailableBalance(int CoinID, std::string UserPubKey)
{
	if (IsUserBalanceExist(CoinID, UserPubKey))
		return mapUserBalanceByCoinID[CoinID][UserPubKey]->nAvailableBalance;
	return 0;
}

int64_t CUserBalanceManager::GetUserInExchangeBalance(int CoinID, std::string UserPubKey)
{
	if (IsUserBalanceExist(CoinID, UserPubKey))
		return mapUserBalanceByCoinID[CoinID][UserPubKey]->nInExchangeBalance;
	return 0;
}

int64_t CUserBalanceManager::GetUserPendingBalance(int CoinID, std::string UserPubKey)
{
	if (IsUserBalanceExist(CoinID, UserPubKey))
		return mapUserBalanceByCoinID[CoinID][UserPubKey]->nPendingBalance;
	return 0;
}

void CUserBalanceManager::AdjustUserAvailableBalance(int CoinID, std::string UserPubKey, int64_t amount)
{
	if (IsUserBalanceExist(CoinID, UserPubKey))
		mapUserBalanceByCoinID[CoinID][UserPubKey]->nAvailableBalance += amount;
}

void CUserBalanceManager::AdjustUserInExchangeBalance(int CoinID, std::string UserPubKey, int64_t amount)
{
	if (IsUserBalanceExist(CoinID, UserPubKey))
		mapUserBalanceByCoinID[CoinID][UserPubKey]->nInExchangeBalance += amount;
}

void CUserBalanceManager::AdjustUserPendingBalance(int CoinID, std::string UserPubKey, int64_t amount)
{
	if (IsUserBalanceExist(CoinID, UserPubKey))
		mapUserBalanceByCoinID[CoinID][UserPubKey]->nPendingBalance += amount;
}

bool CUserBalanceManager::UpdateAfterTradeBalance(std::string User1PubKey, std::string User2PubKey, int CoinID1, int CoinID2, int64_t User1EAdjDown, int64_t User1BAdjUp, int64_t User2EAdjDown, int64_t User2BAdjUp)
{
	if (!IsInChargeOfCoinBalance(CoinID1) || !IsInChargeOfCoinBalance(CoinID2))
		return false;

	auto& temp1 = mapUserBalanceByCoinID[CoinID1];
	auto& temp2 = mapUserBalanceByCoinID[CoinID2];

	if (!temp1.count(User1PubKey) || !temp1.count(User2PubKey) || !temp2.count(User1PubKey) || !temp2.count(User2PubKey))
		return false;

	temp1[User1PubKey]->nAvailableBalance += User1BAdjUp;
	temp1[User2PubKey]->nInExchangeBalance -= User2EAdjDown;
	temp2[User1PubKey]->nInExchangeBalance -= User1EAdjDown;
	temp2[User2PubKey]->nAvailableBalance += User2BAdjUp;
	return true;
}