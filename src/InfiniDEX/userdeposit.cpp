// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "userdeposit.h"
#include "userbalance.h"
#include "noderole.h"

class CUserDeposit;
class CUserDepositSetting;
class CUserDepositManager;

std::map<int, pairSettingUserDeposit> mapUserDeposit;
CUserDepositManager userDepositManager;

std::string CUserDeposit::GetHash()
{
	return "";
}

bool CUserDeposit::Verify()
{
	return true;
}

void CUserDepositManager::ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{

}

void CUserDepositManager::ToProvideUserDepositInfo(int CoinID, bool toProcess)
{
	mapUserDeposit[CoinID].first.ProvideUserDepositInfo = toProcess;
}

void CUserDepositManager::SetSyncInProgress(int CoinID, bool status)
{
	mapUserDeposit[CoinID].first.SyncInProgress = status;
}

void CUserDepositManager::AddCoinToList(int CoinID)
{
	mapUserDeposit.insert(std::make_pair(CoinID, pairSettingUserDeposit()));
}

bool CUserDepositManager::IsCoinInList(int CoinID)
{
	return mapUserDeposit.count(CoinID);
}

void CUserDepositManager::AddNewUser(std::string UserPubKey, int CoinID)
{
	mapUserDeposit[CoinID].second.insert(std::make_pair(UserPubKey, pairConfirmPendingDeposit()));
}

bool CUserDepositManager::IsUserInList(std::string UserPubKey, int CoinID)
{
	return mapUserDeposit[CoinID].second.count(UserPubKey);
}

void CUserDepositManager::RequestPendingDepositData(int StartingID)
{

}

void CUserDepositManager::AddNewPendingDeposit(CUserDeposit UserDeposit)
{
	if (!UserDeposit.Verify())
	{
		return;
	}

	if (!IsUserInList(UserDeposit.nUserPubKey, UserDeposit.nCoinID))
	{
		AddNewUser(UserDeposit.nUserPubKey, UserDeposit.nCoinID);
		if (UserDeposit.nUserDepositID != 1)
		{
			SetSyncInProgress(UserDeposit.nCoinID, true);
			//need to request data from other node
			return;
		}
	}
	else if (IsUserPendingDepositInList(UserDeposit))
	{
		return;
	}

	int UserLastPendingDepositID = GetLastUserPendingDepositID(UserDeposit.nUserPubKey, UserDeposit.nCoinID);
	if (UserLastPendingDepositID != (UserDeposit.nUserDepositID - 1))
	{
		SetSyncInProgress(UserDeposit.nCoinID, true);
		//need to request data from other node
		return;
	}

	userBalanceManager.UpdateUserPendingBalance(UserDeposit.nCoinID, UserDeposit.nUserPubKey, UserDeposit.nDepositAmount);
	mapUserDeposit[UserDeposit.nCoinID].second[UserDeposit.nUserPubKey].second.insert(std::make_pair(UserDeposit.nUserDepositID, UserDeposit));
}

bool CUserDepositManager::IsUserPendingDepositInList(CUserDeposit UserDeposit)
{
	return mapUserDeposit[UserDeposit.nCoinID].second[UserDeposit.nUserPubKey].second.count(UserDeposit.nUserDepositID);
}

int CUserDepositManager::GetLastUserPendingDepositID(std::string UserPubKey, int CoinID)
{
	mapUserDepositWithID::reverse_iterator it = mapUserDeposit[CoinID].second[UserPubKey].second.rbegin();
	if (it == mapUserDeposit[CoinID].second[UserPubKey].second.rend())
		return 0;

	return it->second.nUserDepositID;
}

void CUserDepositManager::RequestConfirmDepositData(int StartingID)
{

}

void CUserDepositManager::AddNewConfirmDeposit(CUserDeposit UserDeposit)
{
	if (!UserDeposit.Verify())
	{
		return;
	}

	if (!IsUserInList(UserDeposit.nUserPubKey, UserDeposit.nCoinID))
	{
		AddNewUser(UserDeposit.nUserPubKey, UserDeposit.nCoinID);
		if (UserDeposit.nUserDepositID != 1)
		{
			SetSyncInProgress(UserDeposit.nCoinID, true);
			//need to request data from other node
			return;
		}
	}
	else if (IsUserConfirmDepositInList(UserDeposit))
	{
		return;
	}

	int UserLastConfirmDepositID = GetLastUserConfirmDepositID(UserDeposit.nUserPubKey, UserDeposit.nCoinID);
	if (UserLastConfirmDepositID != (UserDeposit.nUserDepositID - 1))
	{
		SetSyncInProgress(UserDeposit.nCoinID, true);
		//need to request data from other node
		return;
	}

	userBalanceManager.UpdateUserAvailableBalance(UserDeposit.nCoinID, UserDeposit.nUserPubKey, UserDeposit.nDepositAmount);
	mapUserDeposit[UserDeposit.nCoinID].second[UserDeposit.nUserPubKey].first.insert(std::make_pair(UserDeposit.nUserDepositID, UserDeposit));
}

bool CUserDepositManager::IsUserConfirmDepositInList(CUserDeposit UserDeposit)
{
	return mapUserDeposit[UserDeposit.nCoinID].second[UserDeposit.nUserPubKey].first.count(UserDeposit.nUserDepositID);
}

int CUserDepositManager::GetLastUserConfirmDepositID(std::string UserPubKey, int CoinID)
{
	mapUserDepositWithID::reverse_iterator it = mapUserDeposit[CoinID].second[UserPubKey].first.rbegin();
	if (it == mapUserDeposit[CoinID].second[UserPubKey].first.rend())
		return 0;

	return it->second.nUserDepositID;
}

void CUserDepositManager::DepositConfirmation(std::string UserPubKey, int CoinID, std::string Hash, uint64_t LastUpdateTime)
{
	
}