// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "userbalance.h"
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

class CUserBalance;
class CUserBalanceManager;

std::map<pairCoinIDUserPubKey, CUserBalance> mapCoinUserBalance;
CUserBalanceManager userBalanceManager;

bool CUserBalanceManager::IsUserBalanceExist(int CoinID, std::string UserPubKey)
{
	pairCoinIDUserPubKey p1(std::make_pair(CoinID, UserPubKey));
	return mapCoinUserBalance.count(p1);
}

//change to enum
bool CUserBalanceManager::AddNewUserBalance(CUserBalance NewUserBalance)
{
	if (!IsUserBalanceExist(NewUserBalance.nCoinID, NewUserBalance.nUserPubKey))
	{
		mapCoinUserBalance.insert(std::make_pair(std::make_pair(NewUserBalance.nCoinID, NewUserBalance.nUserPubKey), NewUserBalance));
		return true;
	}
	return false;
}

userbalance_to_exchange_enum_t CUserBalanceManager::BalanceToExchange(int CoinID, std::string UserPubKey, uint64_t amount)
{
	pairCoinIDUserPubKey p1(std::make_pair(CoinID, UserPubKey));
	if (!mapCoinUserBalance.count(p1))
	{
		//to resync from seed server for latest user balance list
		
		//if still not found
		return USER_ACCOUNT_NOT_FOUND;

		//if found but no balance
		return USER_BALANCE_NOT_ENOUGH;
	}

	mapCoinUserBalance[p1].nAvailableBalance -= amount;
	mapCoinUserBalance[p1].nInExchangeBalance += amount;
	return USER_BALANCE_DEDUCTED;
}

exchange_to_userbalance_enum_t CUserBalanceManager::ExchangeToBalance(int CoinID, std::string UserPubKey, uint64_t amount)
{
	pairCoinIDUserPubKey p1(std::make_pair(CoinID, UserPubKey));
	if (!mapCoinUserBalance.count(p1))
	{
		//to resync from seed server for latest user balance list

		//if still not found
		return EXCHANGE_ACCOUNT_NOT_FOUND;

		//if found but no balance
		return EXCHANGE_BALANCE_NOT_ENOUGH;
	}

	mapCoinUserBalance[p1].nInExchangeBalance -= amount;
	mapCoinUserBalance[p1].nAvailableBalance += amount;	
	return EXCHANGE_BALANCE_RETURNED;
}

int64_t CUserBalanceManager::GetUserAvailableBalance(int CoinID, std::string UserPubKey)
{
	pairCoinIDUserPubKey p1(std::make_pair(CoinID, UserPubKey));
	if (mapCoinUserBalance.count(p1))
	{
		return mapCoinUserBalance[p1].nAvailableBalance;
	}
	return 0;
}

int64_t CUserBalanceManager::GetUserInExchangeBalance(int CoinID, std::string UserPubKey)
{
	pairCoinIDUserPubKey p1(std::make_pair(CoinID, UserPubKey));
	if (mapCoinUserBalance.count(p1))
	{
		return mapCoinUserBalance[p1].nInExchangeBalance;
	}
	return 0;
}

int64_t CUserBalanceManager::GetUserPendingBalance(int CoinID, std::string UserPubKey)
{
	pairCoinIDUserPubKey p1(std::make_pair(CoinID, UserPubKey));
	if (mapCoinUserBalance.count(p1))
	{
		return mapCoinUserBalance[p1].nPendingBalance;
	}
	return 0;
}

void CUserBalanceManager::UpdateUserAvailableBalance(int CoinID, std::string UserPubKey, int64_t amount)
{
	pairCoinIDUserPubKey p1(std::make_pair(CoinID, UserPubKey));
	if (mapCoinUserBalance.count(p1))
	{
		mapCoinUserBalance[p1].nAvailableBalance = amount;
	}
}

void CUserBalanceManager::UpdateUserPendingBalance(int CoinID, std::string UserPubKey, int64_t amount)
{
	pairCoinIDUserPubKey p1(std::make_pair(CoinID, UserPubKey));
	if (mapCoinUserBalance.count(p1))
	{
		mapCoinUserBalance[p1].nPendingBalance = amount;
	}
}

void CUserBalanceManager::AdjustUserAvailableBalance(int CoinID, std::string UserPubKey, int64_t amount)
{
	pairCoinIDUserPubKey p1(std::make_pair(CoinID, UserPubKey));
	if (mapCoinUserBalance.count(p1))
	{
		mapCoinUserBalance[p1].nAvailableBalance += amount;
	}
}

void CUserBalanceManager::AdjustUserPendingBalance(int CoinID, std::string UserPubKey, int64_t amount)
{
	pairCoinIDUserPubKey p1(std::make_pair(CoinID, UserPubKey));
	if (mapCoinUserBalance.count(p1))
	{
		mapCoinUserBalance[p1].nPendingBalance += amount;
	}
}

bool CUserBalanceManager::UpdateAfterTradeBalance(std::string User1PubKey, std::string User2PubKey, int CoinID1, int CoinID2, int64_t User1EAdj, int64_t User1BAdj, int64_t User2EAdj, int64_t User2BAdj)
{
	pairCoinIDUserPubKey p1(std::make_pair(CoinID1, User1PubKey));
	pairCoinIDUserPubKey p2(std::make_pair(CoinID2, User1PubKey));
	pairCoinIDUserPubKey p3(std::make_pair(CoinID1, User2PubKey));
	pairCoinIDUserPubKey p4(std::make_pair(CoinID2, User2PubKey));
	bool l1 = mapCoinUserBalance.count(p1);
	if (!l1)
	{
		//to resync from seed server for latest user balance list
	}

	bool l2 = mapCoinUserBalance.count(p2);
	if (!l2)
	{
		//to resync from seed server for latest user balance list
	}

	bool l3 = mapCoinUserBalance.count(p3);
	if (!l3)
	{
		//to resync from seed server for latest user balance list
	}

	bool l4 = mapCoinUserBalance.count(p4);
	if (!l4)
	{
		//to resync from seed server for latest user balance list
	}	
	
	if (l1 && l2 && l3 && l4)
	{
		mapCoinUserBalance[p1].nInExchangeBalance += User1EAdj;
		mapCoinUserBalance[p2].nAvailableBalance += User1BAdj;
		mapCoinUserBalance[p3].nInExchangeBalance += User2BAdj;
		mapCoinUserBalance[p4].nAvailableBalance += User2EAdj;
	}
	return true;
}

void CUserBalanceManager::VerifyUserBalance(CUserBalance UserBalance)
{
	if (!IsUserBalanceExist(UserBalance.nCoinID,UserBalance.nUserPubKey))
	{
		return;
	}


}