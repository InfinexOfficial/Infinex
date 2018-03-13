// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "messagesigner.h"
#include "net_processing.h"
#include "userbalance.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

std::map<int, mapUserBalance> mapCoinUserBalance; //coin ID & map user balance
CUserBalanceManager userBalanceManager;

userbalance_to_exchange_enum_t CUserBalanceManager::BalanceToExchange(int CoinID, uint64_t amount)
{
	return USER_BALANCE_DEDUCTED;
}

exchange_to_userbalance_enum_t CUserBalanceManager::ExchangeToBalance(int CoinID, uint64_t amount)
{
	return EXCHANGE_BALANCE_RETURNED;
}

int64_t CUserBalanceManager::GetUserAvailableBalance(int CoinID, std::string UserPubKey)
{
	return 0;
}

int64_t CUserBalanceManager::GetUserInExchangeBalance(int CoinID, std::string UserPubKey)
{
	return 0;
}

int64_t CUserBalanceManager::GetUserPendingBalance(int CoinID, std::string UserPubKey)
{
	return 0;
}

void CUserBalanceManager::UpdateUserAvailableBalance()
{

}

void CUserBalanceManager::UpdateUserPendingBalance()
{

}

void CUserBalance::RelayTo(CNode* pnode, CConnman& connman)
{
    CInv inv(MSG_INFINIDEX_USERBALANCE, GetHash());
    connman.PushMessage(pnode, NetMsgType::DEXUSERBALANCE, inv);
}