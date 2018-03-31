// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERBALANCE_H
#define USERBALANCE_H

#include <iostream>
#include <vector>
#include <map>

class CUserBalance;
class CUserBalanceSetting;
class CUserBalanceManager;
class CGlobalUserBalanceHandler;

typedef std::map<std::string, CUserBalance> mapUserBalanceByPubKey;
typedef std::map<int, CUserBalance> mapUserBalanceByCoinID;
extern std::map<std::string, mapUserBalanceByCoinID> mapGlobalUserBalance;
extern std::map<int, mapUserBalanceByPubKey> mapUserBalance;
extern std::map<int, CUserBalanceSetting> mapUserBalanceSetting;
extern CGlobalUserBalanceHandler globalUserBalanceHandler;
extern CUserBalanceManager userBalanceManager;

enum userbalance_to_exchange_enum_t {
	USER_ACCOUNT_NOT_FOUND = -1,
	USER_BALANCE_NOT_ENOUGH = 0,
	USER_BALANCE_DEDUCTED = 1,
	USER_BALANCE_INVALID_NODE = 2
};

enum exchange_to_userbalance_enum_t {
	EXCHANGE_ACCOUNT_NOT_FOUND = -1,
	EXCHANGE_BALANCE_NOT_ENOUGH = 0,
	EXCHANGE_BALANCE_RETURNED = 1,
	EXCHANGE_INVALID_NODE = 2
};

class CGlobalUserBalanceHandler
{
public:
	bool nIsInChargeOfGlobalUserBalance;

	CGlobalUserBalanceHandler(bool nIsInChargeOfGlobalUserBalance):
		nIsInChargeOfGlobalUserBalance(nIsInChargeOfGlobalUserBalance)
	{}

	CGlobalUserBalanceHandler():
		nIsInChargeOfGlobalUserBalance(false)
	{}
};

class CUserBalanceSetting
{
public:
	int nCoinID;
	bool nIsInChargeOfUserBalance;
	bool nVerifyUserBalance;

	CUserBalanceSetting(int nCoinID, bool nIsInChargeOfUserBalance, bool nVerifyUserBalance) :
		nCoinID(nCoinID),
		nIsInChargeOfUserBalance(nIsInChargeOfUserBalance),
		nVerifyUserBalance(nVerifyUserBalance)
	{}

	CUserBalanceSetting(int nCoinID) :
		nCoinID(nCoinID),
		nIsInChargeOfUserBalance(false),
		nVerifyUserBalance(false)
	{}

	CUserBalanceSetting() :
		nCoinID(0),
		nIsInChargeOfUserBalance(false),
		nVerifyUserBalance(false)
	{}
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
	uint64_t nInSyncTime;
	int nLastDepositID;
	int nLastWithdrawID;
	int nLastActualTradeID;
	int nLastUserTradeID;
	uint64_t nLastUpdateTime;

	CUserBalance(std::string nUserPubKey, int nCoinID, int64_t nAvailableBalance, int64_t nInExchangeBalance, int64_t nInDisputeBalance, 
		int64_t nPendingBalance, int64_t nTotalBalance, uint64_t nInSyncTime, int nLastDepositID, int nLastWithdrawID, 
		int nLastActualTradeID, int nLastUserTradeID, uint64_t nLastUpdateTime) :
		nUserPubKey(nUserPubKey),
		nCoinID(nCoinID),
		nAvailableBalance(nAvailableBalance),
		nInExchangeBalance(nInExchangeBalance),
		nInDisputeBalance(nInDisputeBalance),
		nPendingBalance(nPendingBalance),
		nTotalBalance(nTotalBalance),
		nInSyncTime(nInSyncTime),
		nLastDepositID(nLastDepositID),
		nLastWithdrawID(nLastWithdrawID),
		nLastActualTradeID(nLastActualTradeID),
		nLastUserTradeID(nLastUserTradeID),
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
		nInSyncTime(0),
		nLastDepositID(0),
		nLastWithdrawID(0),
		nLastActualTradeID(0),
		nLastUserTradeID(0),
		nLastUpdateTime(0)
	{}

	bool VerifySignature();
};

class CUserBalanceManager
{
private:
	std::vector<unsigned char> vchSig;

public:
	CUserBalanceManager() {}
	void InitCoin(int CoinID);
	bool AssignNodeToHandleGlobalBalance(bool toAssign = true);
	int GetLastDepositID(int CoinID, std::string UserPubKey);
	bool IsInChargeOfGlobalCoinBalance();
	bool IsInChargeOfCoinBalance(int CoinID);
	bool IsCoinInList(int CoinID);
	bool IsFurtherInTime(int CoinID, std::string UserPubKey, uint64_t time);
	bool IsUserBalanceExist(int CoinID, std::string UserPubKey);
	bool UpdateUserBalance(CUserBalance UserBalance);
	bool VerifyUserBalance(int CoinID);
	userbalance_to_exchange_enum_t BalanceToExchange(int CoinID, std::string UserPubKey, uint64_t amount);
	exchange_to_userbalance_enum_t ExchangeToBalance(int CoinID, std::string UserPubKey, uint64_t amount);
	int64_t GetUserAvailableBalance(int CoinID, std::string UserPubKey);
	int64_t GetUserInExchangeBalance(int CoinID, std::string UserPubKey);
	int64_t GetUserPendingBalance(int CoinID, std::string UserPubKey);
	void AdjustUserAvailableBalance(int CoinID, std::string UserPubKey, int64_t amount);
	void AdjustUserInExchangeBalance(int CoinID, std::string UserPubKey, int64_t amount);
	void AdjustUserPendingBalance(int CoinID, std::string UserPubKey, int64_t amount);
	bool UpdateAfterTradeBalance(std::string User1PubKey, std::string User2PubKey, int CoinID1, int CoinID2, int64_t User1EAdjDown, int64_t User1BAdjUp, int64_t User2EAdjDown, int64_t User2BAdjUp);
};

#endif
