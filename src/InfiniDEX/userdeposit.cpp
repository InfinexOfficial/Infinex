// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "validation.h"
#include "messagesigner.h"
#include "net_processing.h"
#include "userdeposit.h"
#include "userbalance.h"
#include "noderole.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

class CUserDeposit;
class CUserDepositSetting;
class CUserDepositManager;

std::map<int, mapUserDepositWithPubKey> mapUserDepositByCoinID;
std::map<std::string, mapUserDepositWithCoinID> mapUserDepositByPubKey;
std::map<int, mapLastRequestTimeByPubKey> mapUserLastRequestTime;
std::map<int, CUserDepositSetting> mapUserDepositSetting;
CUserDepositManager userDepositManager;

void CUserDepositManager::ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{
	if (strCommand == NetMsgType::DEXUSERDEPOSIT) {

		CUserDeposit userDeposit;
		vRecv >> userDeposit;

		if (!userDeposit.VerifySignature()) {
			LogPrintf("CUserDepositManager::ProcessMessage -- invalid signature\n");
			Misbehaving(pfrom->GetId(), 100);
			return;
		}
		
		std::shared_ptr<CUserDeposit> UserDeposit = std::make_shared<CUserDeposit>(userDeposit);
		InputUserDeposit(UserDeposit);
	}
}

bool CUserDeposit::VerifySignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nUserDepositID) + nUserPubKey + boost::lexical_cast<std::string>(nCoinID) + boost::lexical_cast<std::string>(nDepositAmount)
		+ boost::lexical_cast<std::string>(nBlockNumber)+ boost::lexical_cast<std::string>(nDepositTime)+ boost::lexical_cast<std::string>(nDepositStatus)
		+ nRemark + boost::lexical_cast<std::string>(nLastUpdateTime);
	CPubKey pubkey(ParseHex(DEXKey));

	if (!CMessageSigner::VerifyMessage(pubkey, vchSig, strMessage, strError)) {
		LogPrintf("CUserDeposit::VerifySignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

void CUserDepositManager::InputUserDeposit(const std::shared_ptr<CUserDeposit>& UserDeposit)
{	
	if (userBalanceManager.InChargeOfUserBalance(UserDeposit->nUserPubKey))
	{		
		if (!mapUserDepositByPubKey.count(UserDeposit->nUserPubKey))
			mapUserDepositByPubKey.insert(std::make_pair(UserDeposit->nUserPubKey, mapUserDepositWithCoinID()));
		mapUserDepositWithCoinID& temp = mapUserDepositByPubKey[UserDeposit->nUserPubKey];
		if (!temp.count(UserDeposit->nCoinID))
			temp.insert(std::make_pair(UserDeposit->nCoinID, UserDepositInfo()));
		UserDepositInfo& temp2 = temp[UserDeposit->nCoinID];
		if (temp2.first == (UserDeposit->nUserDepositID - 1))
		{
			temp2.second.insert(std::make_pair(UserDeposit->nUserDepositID, UserDeposit));
			temp2.first++;
			if (UserDeposit->nDepositStatus == USER_DEPOSIT_PENDING)
				userBalanceManager.AdjustUserPendingDepositBalance(UserDeposit->nCoinID, UserDeposit->nUserPubKey, UserDeposit->nDepositAmount);
			else if (UserDeposit->nDepositStatus == USER_DEPOSIT_CONFIRMED)
				userBalanceManager.AdjustUserAvailableBalance(UserDeposit->nCoinID, UserDeposit->nUserPubKey, UserDeposit->nDepositAmount);
		}
		else if (temp2.first >= UserDeposit->nUserDepositID)
		{
			auto& temp3 = temp2.second[UserDeposit->nUserDepositID];
			if (UserDeposit->nLastUpdateTime > temp3->nLastUpdateTime)
			{				
				if (UserDeposit->nDepositStatus == USER_DEPOSIT_CONFIRMED)
				{
					if (temp3->nDepositStatus == USER_DEPOSIT_PENDING)
						userBalanceManager.PendingToAvailable(UserDeposit->nCoinID, UserDeposit->nUserPubKey, temp3->nDepositAmount, UserDeposit->nDepositAmount);
					else if (temp3->nDepositStatus == USER_DEPOSIT_INVALID)
						userBalanceManager.AdjustUserAvailableBalance(UserDeposit->nCoinID, UserDeposit->nUserPubKey, UserDeposit->nDepositAmount);
					else if (UserDeposit->nDepositAmount != temp3->nDepositAmount)
					{
						int64_t difference = UserDeposit->nDepositAmount - temp3->nDepositAmount;
						userBalanceManager.AdjustUserAvailableBalance(UserDeposit->nCoinID, UserDeposit->nUserPubKey, difference);
					}
				}
				else if (UserDeposit->nDepositStatus == USER_DEPOSIT_PENDING)
				{
					if (temp3->nDepositStatus == USER_DEPOSIT_CONFIRMED)
						userBalanceManager.AvailableToPending(UserDeposit->nCoinID, UserDeposit->nUserPubKey, temp3->nDepositAmount, UserDeposit->nDepositAmount);
					else if (temp3->nDepositStatus == USER_DEPOSIT_INVALID)
						userBalanceManager.AdjustUserPendingDepositBalance(UserDeposit->nCoinID, UserDeposit->nUserPubKey, UserDeposit->nDepositAmount);
					else if (UserDeposit->nDepositAmount != temp3->nDepositAmount)
					{
						int64_t difference = UserDeposit->nDepositAmount - temp3->nDepositAmount;
						userBalanceManager.AdjustUserPendingDepositBalance(UserDeposit->nCoinID, UserDeposit->nUserPubKey, difference);
					}
				}
				else if (UserDeposit->nDepositStatus == USER_DEPOSIT_INVALID)
				{
					if (temp3->nDepositStatus == USER_DEPOSIT_CONFIRMED)
						userBalanceManager.AdjustUserAvailableBalance(UserDeposit->nCoinID, UserDeposit->nUserPubKey, (0 - UserDeposit->nDepositAmount));
					else if (temp3->nDepositStatus == USER_DEPOSIT_PENDING)
						userBalanceManager.AdjustUserPendingDepositBalance(UserDeposit->nCoinID, UserDeposit->nUserPubKey, (0 - UserDeposit->nDepositAmount));
				}
				temp3 = UserDeposit;
			}
		}
		else if (temp2.first < UserDeposit->nUserDepositID)
		{
			//to request range data
		}
	}

	CUserDepositSetting& setting = mapUserDepositSetting[UserDeposit->nCoinID];
	if (setting.CoinID != UserDeposit->nCoinID)
		return;

	if (!setting.ProvideUserDepositInfo)
		return;

	mapUserDepositWithPubKey& temp4 = mapUserDepositByCoinID[UserDeposit->nCoinID];
	if (!temp4.count(UserDeposit->nUserPubKey))
		temp4.insert(std::make_pair(UserDeposit->nUserPubKey, UserDepositInfo()));

	mapUserDepositWithID& temp5 = temp4[UserDeposit->nUserPubKey].second;
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
	mapUserDepositByCoinID[CoinID].insert(std::make_pair(UserPubKey, UserDepositInfo()));
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

	mapUserDepositWithPubKey& temp = mapUserDepositByCoinID[UserDeposit->nCoinID];
	if (!temp.count(UserDeposit->nUserPubKey))
		temp.insert(std::make_pair(UserDeposit->nUserPubKey, UserDepositInfo()));
	
	mapUserDepositWithID& temp2 = temp[UserDeposit->nUserPubKey].second;
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
	return mapUserDepositByCoinID[UserDeposit->nCoinID][UserDeposit->nUserPubKey].second.count(UserDeposit->nUserDepositID);
}

int CUserDepositManager::GetLastUserPendingDepositID(std::string UserPubKey, int CoinID)
{
	mapUserDepositWithID::reverse_iterator it = mapUserDepositByCoinID[CoinID][UserPubKey].second.rbegin();
	if (it == mapUserDepositByCoinID[CoinID][UserPubKey].second.rend())
		return 0;

	return it->second->nUserDepositID;
}

void CUserDepositManager::RequestConfirmDepositData(int StartingID)
{

}

void CUserDepositManager::AddConfirmDeposit(std::shared_ptr<CUserDeposit> UserDeposit)
{
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
		if (mapUserDepositByCoinID[UserDeposit->nCoinID][UserDeposit->nUserPubKey].second[UserDeposit->nUserDepositID]->nDepositStatus == USER_DEPOSIT_CONFIRMED)
			return;
	}

	int UserLastConfirmDepositID = GetLastUserConfirmDepositID(UserDeposit->nUserPubKey, UserDeposit->nCoinID);
	if (UserLastConfirmDepositID != (UserDeposit->nUserDepositID - 1))
	{
		SetSyncInProgress(UserDeposit->nCoinID, true);
		//need to request data from other node
		return;
	}

	mapUserDepositByCoinID[UserDeposit->nCoinID][UserDeposit->nUserPubKey].second.insert(std::make_pair(UserDeposit->nUserDepositID, UserDeposit));
	if (userBalanceManager.GetLastDepositID(UserDeposit->nCoinID, UserDeposit->nUserPubKey) > UserDeposit->nUserDepositID)
		return;

	userBalanceManager.AdjustUserAvailableBalance(UserDeposit->nCoinID, UserDeposit->nUserPubKey, UserDeposit->nDepositAmount);
}

int CUserDepositManager::GetLastUserConfirmDepositID(std::string UserPubKey, int CoinID)
{
	mapUserDepositWithID::reverse_iterator it = mapUserDepositByCoinID[CoinID][UserPubKey].second.rbegin();
	if (it == mapUserDepositByCoinID[CoinID][UserPubKey].second.rend())
		return 0;

	return it->second->nUserDepositID;
}