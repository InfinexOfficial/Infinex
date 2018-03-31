// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "userbalance.h"
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

class CUserBalance;
class CUserBalanceManager;

std::map<std::string, mapUserBalanceByCoinID> mapGlobalUserBalance;
std::map<int, mapUserBalanceByPubKey> mapUserBalance;
std::map<int, CUserBalanceSetting> mapUserBalanceSetting;
CGlobalUserBalanceHandler globalUserBalanceHandler;
CUserBalanceManager userBalanceManager;

bool CUserBalance::VerifySignature()
{
	return true;
}

void CUserBalanceManager::InitCoin(int CoinID)
{
	if (mapUserBalance.count(CoinID))
		return;

	mapUserBalanceSetting.insert(std::make_pair(CoinID, CUserBalanceSetting(CoinID)));
	mapUserBalance.insert(std::make_pair(CoinID, mapUserBalanceByPubKey()));
}

bool CUserBalanceManager::AssignNodeToHandleGlobalBalance(bool toAssign = true)
{
	globalUserBalanceHandler.nIsInChargeOfGlobalUserBalance = true;
}

int CUserBalanceManager::GetLastDepositID(int CoinID, std::string UserPubKey)
{
	return mapUserBalance[CoinID][UserPubKey].nLastDepositID;
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
	return mapUserBalance.count(CoinID);
}

bool CUserBalanceManager::IsFurtherInTime(int CoinID, std::string UserPubKey, uint64_t time)
{
	return (time > mapUserBalance[CoinID][UserPubKey].nInSyncTime);
}

bool CUserBalanceManager::IsUserBalanceExist(int CoinID, std::string UserPubKey)
{
	return (mapUserBalance[CoinID].count(UserPubKey));
}

bool CUserBalanceManager::UpdateUserBalance(CUserBalance UserBalance)
{
	if (!UserBalance.VerifySignature())
		return false;

	if (IsInChargeOfCoinBalance(UserBalance.nCoinID))
	{
		mapUserBalanceByPubKey& temp = mapUserBalance[UserBalance.nCoinID];
		if (!temp.count(UserBalance.nUserPubKey))
			temp.insert(std::make_pair(UserBalance.nUserPubKey, UserBalance));
		else if (temp[UserBalance.nUserPubKey].nLastUpdateTime < UserBalance.nLastUpdateTime)
			temp[UserBalance.nUserPubKey] = UserBalance;
	}

	if (IsInChargeOfGlobalCoinBalance())
	{
		if (!mapGlobalUserBalance.count(UserBalance.nUserPubKey))
			mapGlobalUserBalance.insert(std::make_pair(UserBalance.nUserPubKey, mapUserBalanceByCoinID()));

		mapUserBalanceByCoinID& temp = mapGlobalUserBalance[UserBalance.nUserPubKey];
		if (!temp.count(UserBalance.nCoinID))
			temp.insert(std::make_pair(UserBalance.nCoinID, UserBalance));
		else if (temp[UserBalance.nCoinID].nLastUpdateTime < UserBalance.nLastUpdateTime)
			temp[UserBalance.nCoinID] = UserBalance;
	}

	return true;
}

userbalance_to_exchange_enum_t CUserBalanceManager::BalanceToExchange(int CoinID, std::string UserPubKey, uint64_t amount)
{
	CUserBalanceSetting& setting = mapUserBalanceSetting[CoinID];
	if (setting.nCoinID != CoinID)
		return USER_BALANCE_INVALID_NODE;

	mapUserBalanceByPubKey& temp = mapUserBalance[CoinID];
	if (!temp.count(UserPubKey))
		return USER_ACCOUNT_NOT_FOUND;

	CUserBalance& temp2 = temp[UserPubKey];

	if (setting.nVerifyUserBalance)
	{
		if (temp2.nAvailableBalance < amount)
			return USER_BALANCE_NOT_ENOUGH;
	}

	temp2.nAvailableBalance -= amount;
	temp2.nInExchangeBalance += amount;
	return USER_BALANCE_DEDUCTED;
}

exchange_to_userbalance_enum_t CUserBalanceManager::ExchangeToBalance(int CoinID, std::string UserPubKey, uint64_t amount)
{
	CUserBalanceSetting& setting = mapUserBalanceSetting[CoinID];
	if (setting.nCoinID != CoinID)
		return EXCHANGE_INVALID_NODE;

	mapUserBalanceByPubKey& temp = mapUserBalance[CoinID];
	if (!temp.count(UserPubKey))
		return EXCHANGE_ACCOUNT_NOT_FOUND;

	CUserBalance& temp2 = temp[UserPubKey];

	if (setting.nVerifyUserBalance)
	{
		if (temp2.nInExchangeBalance < amount)
			return EXCHANGE_BALANCE_NOT_ENOUGH;
	}

	temp2.nAvailableBalance += amount;
	temp2.nInExchangeBalance -= amount;
	return EXCHANGE_BALANCE_RETURNED;
}

int64_t CUserBalanceManager::GetUserAvailableBalance(int CoinID, std::string UserPubKey)
{
	if (IsUserBalanceExist(CoinID, UserPubKey))
		return mapUserBalance[CoinID][UserPubKey].nAvailableBalance;
	return 0;
}

int64_t CUserBalanceManager::GetUserInExchangeBalance(int CoinID, std::string UserPubKey)
{
	if (IsUserBalanceExist(CoinID, UserPubKey))
		return mapUserBalance[CoinID][UserPubKey].nInExchangeBalance;
	return 0;
}

int64_t CUserBalanceManager::GetUserPendingBalance(int CoinID, std::string UserPubKey)
{
	if (IsUserBalanceExist(CoinID, UserPubKey))
		return mapUserBalance[CoinID][UserPubKey].nPendingBalance;
	return 0;
}

void CUserBalanceManager::AdjustUserAvailableBalance(int CoinID, std::string UserPubKey, int64_t amount)
{
	if (IsUserBalanceExist(CoinID, UserPubKey))
		mapUserBalance[CoinID][UserPubKey].nAvailableBalance += amount;
}

void CUserBalanceManager::AdjustUserInExchangeBalance(int CoinID, std::string UserPubKey, int64_t amount)
{
	if (IsUserBalanceExist(CoinID, UserPubKey))
		mapUserBalance[CoinID][UserPubKey].nInExchangeBalance += amount;
}

void CUserBalanceManager::AdjustUserPendingBalance(int CoinID, std::string UserPubKey, int64_t amount)
{
	if (IsUserBalanceExist(CoinID, UserPubKey))
		mapUserBalance[CoinID][UserPubKey].nPendingBalance += amount;
}

bool CUserBalanceManager::UpdateAfterTradeBalance(std::string User1PubKey, std::string User2PubKey, int CoinID1, int CoinID2, int64_t User1EAdjDown, int64_t User1BAdjUp, int64_t User2EAdjDown, int64_t User2BAdjUp)
{
	if (!IsInChargeOfCoinBalance(CoinID1) || !IsInChargeOfCoinBalance(CoinID2))
		return false;

	mapUserBalanceByPubKey& temp1 = mapUserBalance[CoinID1];
	mapUserBalanceByPubKey& temp2 = mapUserBalance[CoinID2];

	if (!temp1.count(User1PubKey) || !temp1.count(User2PubKey) || !temp2.count(User1PubKey) || !temp2.count(User2PubKey))
		return false;

	temp1[User1PubKey].nAvailableBalance += User1BAdjUp;
	temp1[User2PubKey].nInExchangeBalance -= User2EAdjDown;
	temp2[User1PubKey].nInExchangeBalance -= User1EAdjDown;
	temp2[User2PubKey].nAvailableBalance += User2BAdjUp;
	return true;
}