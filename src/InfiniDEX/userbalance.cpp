// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "messagesigner.h"
#include "net_processing.h"
#include "noderole.h"
#include "userbalance.h"
#include "userconnection.h"
#include <boost/lexical_cast.hpp>

class CUserBalance;
class CUserBalanceManager;
class CGlobalUserSetting;

std::map<std::string, mapUserBalanceWithCoinID> mapUserBalanceByPubKey;
std::map<int, mapUserBalanceWithPubKey> mapUserBalanceByCoinID;
std::map<int, CUserBalanceSetting> mapUserBalanceSetting;
CUserBalanceManager userBalanceManager;
std::map<char, CGlobalUserSetting> mapGlobalUserSetting;

void CUserBalanceManager::ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{
	if (strCommand == NetMsgType::DEXUSERBALANCES)
	{
		std::vector<CUserBalance> incomingUserBalances;
		vRecv >> incomingUserBalances;

		for (auto userBalance : incomingUserBalances)
		{
			if (userBalance.nMNPubKey == "")
			{
				if (!userBalance.VerifySignature()) {
					LogPrintf("CUserBalanceManager::ProcessMessage -- invalid signature\n");
					Misbehaving(pfrom->GetId(), 100);
					return;
				}


			}
			else
			{
				if (!userBalance.VerifySignature()) {
					LogPrintf("CUserBalanceManager::ProcessMessage -- invalid signature\n");
					Misbehaving(pfrom->GetId(), 100);
					return;
				}
			}
		}
	}
}

void CUserBalanceManager::InputUserBalance(CUserBalance userBalance)
{
	if (!InChargeOfUserBalance(userBalance.nUserPubKey) && !InChargeOfBackup(userBalance.nUserPubKey))
		return;

	if (!mapUserBalanceByPubKey.count(userBalance.nUserPubKey))
		mapUserBalanceByPubKey.insert(std::make_pair(userBalance.nUserPubKey, mapUserBalanceWithCoinID()));

	auto& a = mapUserBalanceByPubKey[userBalance.nUserPubKey];
	if (!a.count(userBalance.nCoinID))
	{
		std::shared_ptr<CUserBalance> temp = std::make_shared<CUserBalance>(userBalance);
		a.insert(std::make_pair(userBalance.nCoinID, temp));
	}
	else
	{
		auto& b = a[userBalance.nCoinID];
		if (b->nLastUpdateTime < userBalance.nLastUpdateTime)
		{
			std::shared_ptr<CUserBalance> temp = std::make_shared<CUserBalance>(userBalance);
			*b = *temp;
		}
	}
}

void CUserBalanceManager::InputVerifiedUserBalance(CConnman& connman, CUserBalance userBalance)
{
	if (!InChargeOfUserBalance(userBalance.nUserPubKey) && !InChargeOfBackup(userBalance.nUserPubKey))
		return;

	if (!mapVerifiedUserBalanceByPubKey.count(userBalance.nUserPubKey))
		mapVerifiedUserBalanceByPubKey.insert(std::make_pair(userBalance.nUserPubKey, mapUserBalanceWithCoinID()));

	auto& a = mapVerifiedUserBalanceByPubKey[userBalance.nUserPubKey];
	if (!a.count(userBalance.nCoinID))
	{
		std::shared_ptr<CUserBalance> temp = std::make_shared<CUserBalance>(userBalance);
		a.insert(std::make_pair(userBalance.nCoinID, temp));
		userBalance.RelayToUser(connman);
	}
	else
	{
		auto& b = a[userBalance.nCoinID];
		if (b->nLastUpdateTime < userBalance.nLastUpdateTime)
		{
			std::shared_ptr<CUserBalance> temp = std::make_shared<CUserBalance>(userBalance);
			*b = *temp;
			userBalance.RelayToUser(connman);
		}
	}

	InputUserBalance(userBalance);
}

bool CUserBalance::VerifySignature()
{
	std::string strError = "";
	std::string strMessage = nUserPubKey + boost::lexical_cast<std::string>(nCoinID) + boost::lexical_cast<std::string>(nAvailableBalance) + boost::lexical_cast<std::string>(nInExchangeBalance)
		+ boost::lexical_cast<std::string>(nInDisputeBalance) + boost::lexical_cast<std::string>(nPendingDepositBalance) + boost::lexical_cast<std::string>(nPendingWithdrawalBalance)
		+ boost::lexical_cast<std::string>(nTotalBalance) + boost::lexical_cast<std::string>(nLastDepositID) + boost::lexical_cast<std::string>(nLastWithdrawID)
		+ boost::lexical_cast<std::string>(nLastActualTradeID) + boost::lexical_cast<std::string>(nLastUserTradeID) + boost::lexical_cast<std::string>(nLastUpdateTime);
	CPubKey pubkey(ParseHex(DEXKey));
	if (!CMessageSigner::VerifyMessage(pubkey, vchSig, strMessage, strError)) {
		LogPrintf("CUserBalance::VerifyFinalSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}
	return true;
}

void CUserBalance::RelayToNode(CNode* node, CConnman& connman)
{
	connman.PushMessage(node, NetMsgType::DEXUSERBALANCE, *this);
}

void CUserBalance::RelayToUser(CConnman& connman)
{
	if (!mapUserConnections.count(nUserPubKey))
		return;
	
	auto& a = mapUserConnections[nUserPubKey];
	for (auto b : a)
		connman.PushMessage(b.first, NetMsgType::DEXUSERBALANCE, *this);
}

void CUserBalanceManager::InitCoin(int CoinID)
{
	if (mapUserBalanceByCoinID.count(CoinID))
		return;

	mapUserBalanceSetting.insert(std::make_pair(CoinID, CUserBalanceSetting(CoinID)));
	mapUserBalanceByCoinID.insert(std::make_pair(CoinID, mapUserBalanceWithPubKey()));
}

void CUserBalanceManager::InitGlobalUserSetting(char Char)
{
	if (mapGlobalUserSetting.count(Char))
		return;

	mapGlobalUserSetting.insert(std::make_pair(Char, CGlobalUserSetting(Char)));
}

bool CUserBalanceManager::AssignUserBalanceRole(char Char, bool toAssign)
{
	InitGlobalUserSetting(Char);
	auto& a = mapGlobalUserSetting[Char];
	a.nInChargeUserBalance = toAssign;
	return true;
}

bool CUserBalanceManager::AssignBackupRole(char Char, bool toAssign)
{
	InitGlobalUserSetting(Char);
	auto& a = mapGlobalUserSetting[Char];
	a.nInChargeBackup = toAssign;
	return true;
}

bool CUserBalanceManager::UpdateUserBalance(CUserBalance UserBalance)
{
	if (!UserBalance.VerifySignature())
		return false;

	if (!InChargeOfUserBalance(UserBalance.nUserPubKey))
		return false;

	std::shared_ptr<CUserBalance> ub1;
	InitUserBalance(UserBalance.nCoinID, UserBalance.nUserPubKey, ub1);

	std::shared_ptr<CUserBalance> ub = std::make_shared<CUserBalance>(UserBalance);
	if (ub1->nLastUpdateTime < ub->nLastUpdateTime)
		ub1 = ub;

	return true;
}

void CUserBalanceManager::InitUserBalance(int CoinID, std::string UserPubKey, std::shared_ptr<CUserBalance>& UserBalance)
{	
	if (!mapUserBalanceByPubKey.count(UserPubKey))
		mapUserBalanceByPubKey.insert(std::make_pair(UserPubKey, mapUserBalanceWithCoinID()));

	mapUserBalanceWithCoinID& temp = mapUserBalanceByPubKey[UserPubKey];
	if (!temp.count(CoinID)) 
	{
		UserBalance = std::make_shared<CUserBalance>(UserPubKey, CoinID);
		temp.insert(std::make_pair(CoinID, UserBalance));
	}

	if (!mapUserBalanceByCoinID.count(CoinID))
		mapUserBalanceByCoinID.insert(std::make_pair(CoinID, mapUserBalanceWithPubKey()));

	mapUserBalanceWithPubKey& temp2 = mapUserBalanceByCoinID[CoinID];
	if (!temp2.count(UserPubKey))
		temp2.insert(std::make_pair(UserPubKey, UserBalance));

	UserBalance = temp[CoinID];
}

int CUserBalanceManager::GetLastDepositID(int CoinID, std::string UserPubKey)
{
	std::shared_ptr<CUserBalance> ub;
	InitUserBalance(CoinID, UserPubKey, ub);
	return ub->nLastDepositID;
}

bool CUserBalanceManager::InChargeOfUserBalance(std::string pubKey)
{
	char c = pubKey[2];
	return mapGlobalUserSetting[c].nInChargeUserBalance;	
}

bool CUserBalanceManager::InChargeOfBackup(std::string pubKey)
{
	char c = pubKey[2];
	return mapGlobalUserSetting[c].nInChargeBackup;
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
	std::shared_ptr<CUserBalance> ub;
	InitUserBalance(CoinID, UserPubKey, ub);
	return ub->nAvailableBalance;
}

int64_t CUserBalanceManager::GetUserInExchangeBalance(int CoinID, std::string UserPubKey)
{
	std::shared_ptr<CUserBalance> ub;
	InitUserBalance(CoinID, UserPubKey, ub);
	return ub->nInExchangeBalance;
}

int64_t CUserBalanceManager::GetUserPendingDepositBalance(int CoinID, std::string UserPubKey)
{
	std::shared_ptr<CUserBalance> ub;
	InitUserBalance(CoinID, UserPubKey, ub);
	return ub->nPendingDepositBalance;
}

int64_t CUserBalanceManager::GetUserPendingWithdrawalBalance(int CoinID, std::string UserPubKey)
{
	std::shared_ptr<CUserBalance> ub;
	InitUserBalance(CoinID, UserPubKey, ub);
	return ub->nPendingWithdrawalBalance;
}

void CUserBalanceManager::AdjustUserAvailableBalance(int CoinID, std::string UserPubKey, int64_t amount)
{
	std::shared_ptr<CUserBalance> ub;
	InitUserBalance(CoinID, UserPubKey, ub);
	ub->nAvailableBalance += amount;
}

void CUserBalanceManager::AdjustUserInExchangeBalance(int CoinID, std::string UserPubKey, int64_t amount)
{
	std::shared_ptr<CUserBalance> ub;
	InitUserBalance(CoinID, UserPubKey, ub);
	ub->nInExchangeBalance += amount;
}

void CUserBalanceManager::AdjustUserPendingDepositBalance(int CoinID, std::string UserPubKey, int64_t amount)
{
	std::shared_ptr<CUserBalance> ub;
	InitUserBalance(CoinID, UserPubKey, ub);
	ub->nPendingDepositBalance += amount;
}

void CUserBalanceManager::AdjustUserPendingWithdrawalBalance(int CoinID, std::string UserPubKey, int64_t amount)
{
	std::shared_ptr<CUserBalance> ub;
	InitUserBalance(CoinID, UserPubKey, ub);
	ub->nPendingWithdrawalBalance += amount;
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

bool CUserBalanceManager::UpdateAfterTradeBalance(std::string UserPubKey, int ExchangeCoinID, int BalanceCoinID, int64_t ExchangeAdjDown, int64_t BalanceAdjUp)
{
	auto& a = mapUserBalanceByPubKey[UserPubKey];
	if (!a.count(ExchangeCoinID))
	{
		std::shared_ptr<CUserBalance> temp = std::make_shared<CUserBalance>(UserPubKey, BalanceCoinID);
		temp->nAvailableBalance += BalanceAdjUp;
		a.insert(std::make_pair(BalanceCoinID, temp));
	}
	else
		a[BalanceCoinID]->nAvailableBalance += BalanceAdjUp;
	a[ExchangeCoinID]->nInExchangeBalance -= ExchangeAdjDown;
}

bool CUserBalanceManager::BalanceToExchangeV2(int CoinID, std::string UserPubKey, uint64_t amount)
{
	std::shared_ptr<CUserBalance> ub;
	InitUserBalance(CoinID, UserPubKey, ub);
	if (ub->nAvailableBalance < amount)
		return false;

	ub->nAvailableBalance -= amount;
	ub->nInExchangeBalance += amount;
	return true;
}

bool CUserBalanceManager::ExchangeToBalanceV2(int CoinID, std::string UserPubKey, uint64_t amount)
{
	std::shared_ptr<CUserBalance> ub;
	InitUserBalance(CoinID, UserPubKey, ub);
	if (ub->nInExchangeBalance < amount)
		return false;

	ub->nInExchangeBalance -= amount;
	ub->nAvailableBalance += amount;
	return true;
}

bool CUserBalanceManager::PendingToAvailable(int CoinID, std::string UserPubKey, uint64_t PendingAmount, uint64_t AvailableAmount)
{
	std::shared_ptr<CUserBalance> ub;
	InitUserBalance(CoinID, UserPubKey, ub);
	if (ub->nPendingDepositBalance < PendingAmount)
		return false;

	ub->nPendingDepositBalance -= PendingAmount;
	ub->nAvailableBalance += AvailableAmount;
	return true;
}

bool CUserBalanceManager::AvailableToPending(int CoinID, std::string UserPubKey, uint64_t AvailableAmount, uint64_t PendingAmount)
{
	std::shared_ptr<CUserBalance> ub;
	InitUserBalance(CoinID, UserPubKey, ub);
	if (ub->nAvailableBalance < AvailableAmount)
		return false;

	ub->nAvailableBalance -= AvailableAmount;
	ub->nPendingDepositBalance += PendingAmount;
	return true;
}