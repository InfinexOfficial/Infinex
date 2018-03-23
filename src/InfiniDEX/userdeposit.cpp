// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "userdeposit.h"
#include "userbalance.h"

class CUserDeposit;
class CUserDepositManager;

std::map<pairCoinIDUserPubKey, pairConfirmPendingDeposit> mapCoinUserDeposit;
CUserDepositManager userDepositManager;

std::string CUserDeposit::GetHash()
{
	return "";
}

bool CUserDeposit::Verify()
{
	return true;
}

bool CUserDepositManager::AddNewUserDepositIntoList(std::string UserPubKey, int CoinID)
{
	pairConfirmPendingDeposit temp = std::make_pair(mapHashUserDeposit(), mapHashUserDeposit());
	pairCoinIDUserPubKey temp2 = std::make_pair(CoinID, UserPubKey);
	mapCoinUserDeposit.insert(std::make_pair(temp2, temp));
	return true;
}

bool CUserDepositManager::IsUserDepositInList(std::string UserPubKey, int CoinID)
{
	return mapCoinUserDeposit.count(std::make_pair(CoinID, UserPubKey));
}

bool CUserDepositManager::AddNewPendingDeposit(CUserDeposit UserDeposit)
{
	if (!UserDeposit.Verify())
	{
		return false;
	}

	if (!IsUserDepositInList(UserDeposit.nUserPubKey, UserDeposit.nCoinID))
	{
		if (!AddNewUserDepositIntoList(UserDeposit.nUserPubKey, UserDeposit.nCoinID))
			return false;
	}	

	pairCoinIDUserPubKey temp = std::make_pair(UserDeposit.nCoinID, UserDeposit.nUserPubKey);
	if (mapCoinUserDeposit[temp].second.count(UserDeposit.nUserDepositID))
	{
		return true;
	}
	
	userBalanceManager.UpdateUserPendingBalance(UserDeposit.nCoinID, UserDeposit.nUserPubKey, UserDeposit.nDepositAmount);
	mapCoinUserDeposit[temp].second.insert(std::make_pair(UserDeposit.nUserDepositID, UserDeposit));
	return true;
}

bool CUserDepositManager::AddNewConfirmDeposit(CUserDeposit UserDeposit)
{
	if (!UserDeposit.Verify())
	{
		return false;
	}

	if (!IsUserDepositInList(UserDeposit.nUserPubKey, UserDeposit.nCoinID))
	{
		if (!AddNewUserDepositIntoList(UserDeposit.nUserPubKey, UserDeposit.nCoinID))
			return false;
	}

	pairCoinIDUserPubKey temp = std::make_pair(UserDeposit.nCoinID, UserDeposit.nUserPubKey);
	if (mapCoinUserDeposit[temp].first.count(UserDeposit.nUserDepositID))
	{
		return true;
	}

	userBalanceManager.UpdateUserAvailableBalance(UserDeposit.nCoinID, UserDeposit.nUserPubKey, UserDeposit.nDepositAmount);
	mapCoinUserDeposit[temp].first.insert(std::make_pair(UserDeposit.nUserDepositID, UserDeposit));
	return true;
}

bool CUserDepositManager::DepositConfirmation(std::string UserPubKey, int CoinID, std::string Hash, uint64_t LastUpdateTime)
{
	return true;
}