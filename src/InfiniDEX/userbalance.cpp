// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "userbalance.h"
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

class CUserBalance;
class CUserBalanceManager;

std::map<int, mapUserBalanceByPubKey> mapUserBalance;
std::map<int, CUserBalanceSetting> mapUserBalanceSetting;
CUserBalanceManager userBalanceManager;

bool CUserBalance::VerifySignature()
{
	return true;
}

bool CUserBalanceManager::IsInChargeOfCoinBalance(int CoinID)
{
	if (mapUserBalanceSetting[CoinID].nIsInChargeOfUserBalance)
	{
		if (!IsCoinInList(CoinID))
		{
			//time to sync
		}
		return true;
	}
	return false;
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

//change to enum
bool CUserBalanceManager::AddNewUserBalance(CUserBalance NewUserBalance)
{
	if (!IsInChargeOfCoinBalance(NewUserBalance.nCoinID))
		return false;	

	if (!NewUserBalance.VerifySignature())
		return false;

	if (!IsUserBalanceExist(NewUserBalance.nCoinID, NewUserBalance.nUserPubKey))
		mapUserBalance[NewUserBalance.nCoinID].insert(std::make_pair(NewUserBalance.nUserPubKey, NewUserBalance));
	else if (IsFurtherInTime(NewUserBalance.nCoinID, NewUserBalance.nUserPubKey, NewUserBalance.nLastUpdateTime))
		mapUserBalance[NewUserBalance.nCoinID][NewUserBalance.nUserPubKey] = NewUserBalance;	
	return true;
}

userbalance_to_exchange_enum_t CUserBalanceManager::BalanceToExchange(int CoinID, std::string UserPubKey, uint64_t amount)
{
	if (!IsInChargeOfCoinBalance(CoinID))
		return USER_BALANCE_INVALID_NODE;

	if (!IsUserBalanceExist(CoinID, UserPubKey))
	{
		//to resync from seed server for latest user balance list
		return USER_ACCOUNT_NOT_FOUND;
	}

	if (VerifyUserBalance(CoinID))
	{
		if (GetUserAvailableBalance(CoinID, UserPubKey) < amount)
		{
			//to inform nodes of invalid balance issue;
			return USER_BALANCE_NOT_ENOUGH;
		}
	}

	mapUserBalance[CoinID][UserPubKey].nAvailableBalance -= amount;
	mapUserBalance[CoinID][UserPubKey].nInExchangeBalance += amount;
	return USER_BALANCE_DEDUCTED;
}

exchange_to_userbalance_enum_t CUserBalanceManager::ExchangeToBalance(int CoinID, std::string UserPubKey, uint64_t amount)
{
	if (!IsInChargeOfCoinBalance(CoinID))
		return EXCHANGE_INVALID_NODE;

	if (!IsUserBalanceExist(CoinID, UserPubKey))
	{
		//to resync from seed server for latest user balance list
		return EXCHANGE_ACCOUNT_NOT_FOUND;
	}

	if (VerifyUserBalance(CoinID))
	{
		if (GetUserInExchangeBalance(CoinID, UserPubKey) < amount)
		{
			//to inform nodes of invalid balance issue;
			return EXCHANGE_BALANCE_NOT_ENOUGH;
		}
	}

	mapUserBalance[CoinID][UserPubKey].nAvailableBalance += amount;
	mapUserBalance[CoinID][UserPubKey].nInExchangeBalance -= amount;
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

bool CUserBalanceManager::UpdateAfterTradeBalance(std::string User1PubKey, std::string User2PubKey, int CoinID1, int CoinID2, int64_t User1EAdj, int64_t User1BAdj, int64_t User2EAdj, int64_t User2BAdj)
{
	if (!IsInChargeOfCoinBalance(CoinID1) || !IsInChargeOfCoinBalance(CoinID2))
		return false;

	if (!IsUserBalanceExist(CoinID1, User1PubKey) || !IsUserBalanceExist(CoinID1, User2PubKey) || !IsUserBalanceExist(CoinID2, User1PubKey) || !IsUserBalanceExist(CoinID2, User2PubKey))
		return false;

	AdjustUserInExchangeBalance(CoinID2, User1PubKey, User1EAdj);
	AdjustUserAvailableBalance(CoinID1, User1PubKey, User1BAdj);
	AdjustUserInExchangeBalance(CoinID1, User2PubKey, User2EAdj);
	AdjustUserAvailableBalance(CoinID2, User2PubKey, User2BAdj);
	return true;
}