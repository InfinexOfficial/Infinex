// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERBALANCE_H
#define USERBALANCE_H

#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"

class CUserBalance;
class CUserBalanceManager;

typedef std::map<std::string, CUserBalance> mapUserBalance; //user public key & userbalance
extern std::map<int, mapUserBalance> mapCoinUserBalance; //coin ID & map user balance
extern CUserBalanceManager userBalanceManager;

enum userbalance_to_exchange_enum_t {
	USER_ACCOUNT_NOT_FOUND = -1,
	USER_BALANCE_NOT_ENOUGH = 0,
	USER_BALANCE_DEDUCTED = 1
};

enum exchange_to_userbalance_enum_t {
	EXCHANGE_ACCOUNT_NOT_FOUND = -1,
	EXCHANGE_BALANCE_NOT_ENOUGH = 0,
	EXCHANGE_BALANCE_RETURNED = 1
};

class CUserBalance
{
private:
	std::vector<unsigned char> vchSig;

public:
	std::string nUserPubKey;
	int nCoinID;
	int64_t nAvailableBalance;
	int64_t nInExchangeBalance;
	int64_t nInDisputeBalance;
	int64_t nPendingBalance;
	int64_t nTotalBalance;
	uint64_t nLastUpdateTime;

	CUserBalance(std::string nUserPubKey, int nCoinID, int64_t nAvailableBalance, int64_t nInExchangeBalance, int64_t nInDisputeBalance, int64_t nPendingBalance, int64_t nTotalBalance, uint64_t nLastUpdateTime) :
		nUserPubKey(nUserPubKey),
		nCoinID(nCoinID),
		nAvailableBalance(nAvailableBalance),
		nInExchangeBalance(nInExchangeBalance),
		nInDisputeBalance(nInDisputeBalance),
		nPendingBalance(nPendingBalance),
		nTotalBalance(nTotalBalance),
		nLastUpdateTime(nLastUpdateTime)
	{}

	CUserBalance() :
		nUserPubKey(""),
		nCoinID(0),
		nAvailableBalance(0),
		nInExchangeBalance(0),
		nInDisputeBalance(0),
		nPendingBalance(0),
		nTotalBalance(0),
		nLastUpdateTime(0)
	{}
};

class CUserBalanceManager
{
private:
	std::vector<unsigned char> vchSig;

public:
	CUserBalanceManager() {}
	userbalance_to_exchange_enum_t BalanceToExchange(int CoinID, uint64_t amount);
	exchange_to_userbalance_enum_t ExchangeToBalance(int CoinID, uint64_t amount);
	int64_t GetUserAvailableBalance(int CoinID, std::string UserPubKey);
	int64_t GetUserInExchangeBalance(int CoinID, std::string UserPubKey);
	int64_t GetUserPendingBalance(int CoinID, std::string UserPubKey);
	void UpdateUserAvailableBalance();
	void UpdateUserPendingBalance();
};

#endif
