// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "userdeposit.h"

class CUserDeposit;
class CUserDepositManager;

std::map<pairCoinIDUserPubKey, mapHashUserDeposit> mapCoinUserDeposit;
CUserDepositManager userDepositManager;

bool CUserDepositManager::IsUserDepositInList(std::string UserPubKey, int CoinID)
{
	return mapCoinUserDeposit.count(std::make_pair(CoinID, UserPubKey));
}

bool CUserDepositManager::AddNewDeposit(CUserDeposit UserDeposit)
{
	if (!IsUserDepositInList(UserDeposit.nUserPubKey, UserDeposit.nCoinID))
	{
		pairCoinIDUserPubKey temp = std::make_pair(UserDeposit.nCoinID, UserDeposit.nUserPubKey);
		mapHashUserDeposit temp2;
		temp2.insert(std::make_pair(UserDeposit.nHash, UserDeposit));
		mapCoinUserDeposit.insert(std::make_pair(temp, temp2));
	}
	else
	{
		pairCoinIDUserPubKey temp = std::make_pair(UserDeposit.nCoinID, UserDeposit.nUserPubKey);
		mapHashUserDeposit UserDeposits = mapCoinUserDeposit[temp];
		mapHashUserDeposit::reverse_iterator rit = UserDeposits.rbegin();
		if (rit == UserDeposits.rend())
		{
			if (UserDeposit.nUserDepositID == 1)
			{
				mapCoinUserDeposit[temp].insert(std::make_pair(UserDeposit.nHash, UserDeposit));
			}
			else
			{
				//need to sync from seed server to download previous deposit
			}
		}
		else
		{
			if (rit->second.nUserDepositID == (UserDeposit.nUserDepositID - 1))
			{
				mapCoinUserDeposit[temp].insert(std::make_pair(UserDeposit.nHash, UserDeposit));
			}
			else
			{
				//need to sync from seed server to download previous deposit
			}
		}
	}
}

bool CUserDepositManager::DepositConfirmation(std::string UserPubKey, int CoinID, std::string Hash, uint64_t LastUpdateTime)
{
	if (!IsUserDepositInList(UserPubKey, CoinID))
	{
		//need to sync from seed server
		return false;
	}

	pairCoinIDUserPubKey temp = std::make_pair(CoinID, UserPubKey);
	if (!mapCoinUserDeposit[temp].count(Hash))
	{
		//need to sync from seed server
		return false;
	}

	CUserDeposit* temp2 = &mapCoinUserDeposit[temp][Hash];
	temp2->nValidDeposit = true;
	temp2->nLastUpdateTime = LastUpdateTime;	
}