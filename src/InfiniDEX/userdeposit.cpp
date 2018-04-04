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

std::map<int, mapUserDepositWithPubKey> mapUserDepositByCoinID;
std::map<std::string, mapUserDepositWithCoinID> mapUserDepositByPubKey;
std::map<int, mapLastRequestTimeByPubKey> mapUserLastRequestTime;
std::map<int, CUserDepositSetting> mapUserDepositSetting;
CUserDepositManager userDepositManager;

bool CUserDeposit::Verify()
{
	return true;
}

bool CUserDepositManager::InChargeOfGlobalBalance(std::string pubKey)
{
	char c = pubKey[2];
	if (globalInChargeDeposit.count(c))
		return true;
	return false;
}

void CUserDepositManager::InputUserDeposit(std::shared_ptr<CUserDeposit> UserDeposit)
{
	if (!UserDeposit->Verify())
		return;

	if (UserDeposit->nDepositStatus != USER_DEPOSIT_CONFIRMED && UserDeposit->nDepositStatus != USER_DEPOSIT_PENDING)
		return;

	if (InChargeOfGlobalBalance(UserDeposit->nUserPubKey))
	{
		if (!mapUserDepositByPubKey.count(UserDeposit->nUserPubKey))
			mapUserDepositByPubKey.insert(std::make_pair(UserDeposit->nUserPubKey, mapUserDepositWithCoinID()));
		mapUserDepositWithCoinID& temp = mapUserDepositByPubKey[UserDeposit->nUserPubKey];
		if (!temp.count(UserDeposit->nCoinID))
			temp.insert(std::make_pair(UserDeposit->nCoinID, mapUserDepositWithID()));
		mapUserDepositWithID& temp2 = temp[UserDeposit->nCoinID];
		if (!temp2.count(UserDeposit->nUserDepositID))
		{
			temp2.insert(std::make_pair(UserDeposit->nUserDepositID, UserDeposit));
			if (UserDeposit->nDepositStatus == USER_DEPOSIT_PENDING)
				userBalanceManager.AdjustUserPendingBalance(UserDeposit->nCoinID, UserDeposit->nUserPubKey, UserDeposit->nDepositAmount);
			else if (UserDeposit->nDepositStatus == USER_DEPOSIT_CONFIRMED)
				userBalanceManager.AdjustUserAvailableBalance(UserDeposit->nCoinID, UserDeposit->nUserPubKey, UserDeposit->nDepositAmount);
		}
		else
		{
			auto& temp3 = temp2[UserDeposit->nUserDepositID];
			if (temp3->nDepositStatus == USER_DEPOSIT_CONFIRMED)
				return;

			if (UserDeposit->nDepositStatus == USER_DEPOSIT_PENDING)
				return;

			temp3 = UserDeposit;
			userBalanceManager.ConfirmPendingBalance(UserDeposit->nCoinID, UserDeposit->nUserPubKey, UserDeposit->nDepositAmount);
		}
	}

	CUserDepositSetting& setting = mapUserDepositSetting[UserDeposit->nCoinID];
	if (setting.CoinID != UserDeposit->nCoinID)
		return;

	if (!setting.ProvideUserDepositInfo)
		return;

	mapUserDepositWithPubKey& temp4 = mapUserDepositByCoinID[UserDeposit->nCoinID];
	if (!temp4.count(UserDeposit->nUserPubKey))
		temp4.insert(std::make_pair(UserDeposit->nUserPubKey, mapUserDepositWithID()));

	mapUserDepositWithID& temp5 = temp4[UserDeposit->nUserPubKey];
	if (!temp5.count(UserDeposit->nUserDepositID))
	{
		int lastPendingID = 0;
		mapUserDepositWithID::reverse_iterator rit = temp5.rbegin();
		if (rit != temp5.rend())
			lastPendingID = rit->second->nUserDepositID;

		if (lastPendingID != (UserDeposit->nUserDepositID - 1))
		{
			SetSyncInProgress(UserDeposit->nCoinID, true);
			//need to request data from other node
			return;
		}
		temp5.insert(std::make_pair(UserDeposit->nUserDepositID, UserDeposit));
	}
	else
	{
		auto& temp6 = temp5[UserDeposit->nUserDepositID];
		if (temp6->nDepositStatus == USER_DEPOSIT_CONFIRMED)
			return;

		if (UserDeposit->nDepositStatus == USER_DEPOSIT_PENDING)
			return;

		temp6 = UserDeposit;
	}
}

void CUserDepositManager::AssignDepositInfoRole(int TradePairID)
{

}

void CUserDepositManager::ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{

}

bool CUserDepositManager::IsInChargeOfUserDepositInfo(int CoinID)
{
	return mapUserDepositSetting[CoinID].ProvideUserDepositInfo;
}

void CUserDepositManager::ToProvideUserDepositInfo(int CoinID, bool toProcess)
{
	mapUserDepositSetting[CoinID].ProvideUserDepositInfo = toProcess;
}

void CUserDepositManager::SetSyncInProgress(int CoinID, bool status)
{
	mapUserDepositSetting[CoinID].SyncInProgress = status;
}

void CUserDepositManager::AddCoinToList(int CoinID)
{
	mapUserDepositSetting.insert(std::make_pair(CoinID, CUserDepositSetting()));
	mapUserLastRequestTime.insert(std::make_pair(CoinID, mapLastRequestTimeByPubKey()));
	mapUserDepositByCoinID.insert(std::make_pair(CoinID, mapUserDepositWithPubKey()));
}

bool CUserDepositManager::IsCoinInList(int CoinID)
{
	return mapUserDepositByCoinID.count(CoinID);
}

void CUserDepositManager::AddNewUser(std::string UserPubKey, int CoinID)
{
	mapUserDepositByCoinID[CoinID].insert(std::make_pair(UserPubKey, mapUserDepositWithID()));
	mapUserLastRequestTime[CoinID].insert(std::make_pair(UserPubKey, 0));
}

bool CUserDepositManager::IsUserInList(std::string UserPubKey, int CoinID)
{
	return mapUserDepositByCoinID[CoinID].count(UserPubKey);
}

void CUserDepositManager::RequestPendingDepositData(int StartingID)
{

}

void CUserDepositManager::AddPendingDeposit(std::shared_ptr<CUserDeposit> UserDeposit)
{
	CUserDepositSetting& setting = mapUserDepositSetting[UserDeposit->nCoinID];
	if (setting.CoinID != UserDeposit->nCoinID)
		return;

	if (!setting.ProvideUserDepositInfo)
		return;

	if (UserDeposit->nDepositStatus != USER_DEPOSIT_PENDING)
		return;

	if (!UserDeposit->Verify())
		return;

	mapUserDepositWithPubKey& temp = mapUserDepositByCoinID[UserDeposit->nCoinID];
	if (!temp.count(UserDeposit->nUserPubKey))
		temp.insert(std::make_pair(UserDeposit->nUserPubKey, mapUserDepositWithID()));
	
	mapUserDepositWithID& temp2 = temp[UserDeposit->nUserPubKey];
	if (temp2.count(UserDeposit->nUserDepositID))
		return;	

	int lastPendingID = 0;
	mapUserDepositWithID::reverse_iterator rit = temp2.rbegin();
	if (rit != temp2.rend())
		lastPendingID = rit->second->nUserDepositID;

	if (lastPendingID != (UserDeposit->nUserDepositID - 1))
	{
		SetSyncInProgress(UserDeposit->nCoinID, true);
		//need to request data from other node
		return;
	}

	temp2.insert(std::make_pair(UserDeposit->nUserDepositID, UserDeposit));
}

bool CUserDepositManager::IsUserDepositInList(std::shared_ptr<CUserDeposit> UserDeposit)
{
	return mapUserDepositByCoinID[UserDeposit->nCoinID][UserDeposit->nUserPubKey].count(UserDeposit->nUserDepositID);
}

int CUserDepositManager::GetLastUserPendingDepositID(std::string UserPubKey, int CoinID)
{
	mapUserDepositWithID::reverse_iterator it = mapUserDepositByCoinID[CoinID][UserPubKey].rbegin();
	if (it == mapUserDepositByCoinID[CoinID][UserPubKey].rend())
		return 0;

	return it->second->nUserDepositID;
}

void CUserDepositManager::RequestConfirmDepositData(int StartingID)
{

}

void CUserDepositManager::AddConfirmDeposit(std::shared_ptr<CUserDeposit> UserDeposit)
{
	if (!UserDeposit->Verify())
		return;

	if (UserDeposit->nDepositStatus != USER_DEPOSIT_CONFIRMED)
		return;

	if (!IsUserInList(UserDeposit->nUserPubKey, UserDeposit->nCoinID))
	{
		AddNewUser(UserDeposit->nUserPubKey, UserDeposit->nCoinID);
		if (UserDeposit->nUserDepositID != 1)
		{
			SetSyncInProgress(UserDeposit->nCoinID, true);
			//need to request data from other node
			return;
		}
	}
	else if (IsUserDepositInList(UserDeposit))
	{
		if (mapUserDepositByCoinID[UserDeposit->nCoinID][UserDeposit->nUserPubKey][UserDeposit->nUserDepositID]->nDepositStatus == USER_DEPOSIT_CONFIRMED)
			return;
	}

	int UserLastConfirmDepositID = GetLastUserConfirmDepositID(UserDeposit->nUserPubKey, UserDeposit->nCoinID);
	if (UserLastConfirmDepositID != (UserDeposit->nUserDepositID - 1))
	{
		SetSyncInProgress(UserDeposit->nCoinID, true);
		//need to request data from other node
		return;
	}

	mapUserDepositByCoinID[UserDeposit->nCoinID][UserDeposit->nUserPubKey].insert(std::make_pair(UserDeposit->nUserDepositID, UserDeposit));
	if (userBalanceManager.GetLastDepositID(UserDeposit->nCoinID, UserDeposit->nUserPubKey) > UserDeposit->nUserDepositID)
		return;

	userBalanceManager.AdjustUserAvailableBalance(UserDeposit->nCoinID, UserDeposit->nUserPubKey, UserDeposit->nDepositAmount);
}

int CUserDepositManager::GetLastUserConfirmDepositID(std::string UserPubKey, int CoinID)
{
	mapUserDepositWithID::reverse_iterator it = mapUserDepositByCoinID[CoinID][UserPubKey].rbegin();
	if (it == mapUserDepositByCoinID[CoinID][UserPubKey].rend())
		return 0;

	return it->second->nUserDepositID;
}