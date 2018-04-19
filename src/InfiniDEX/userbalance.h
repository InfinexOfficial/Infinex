// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERBALANCE_H
#define USERBALANCE_H

#include <vector>
#include <map>
#include <memory>
#include "net.h"
#include "utilstrencodings.h"

class CUserBalance;
class CUserBalanceSetting;
class CUserBalanceManager;
class CGlobalUserBalanceHandler;
class CGlobalUserSetting;

typedef std::map<std::string, std::shared_ptr<CUserBalance>> mapUserBalanceWithPubKey;
typedef std::map<int, std::shared_ptr<CUserBalance>> mapUserBalanceWithCoinID;
extern std::map<std::string, mapUserBalanceWithCoinID> mapUserBalanceByPubKey;
extern std::map<int, mapUserBalanceWithPubKey> mapUserBalanceByCoinID;
extern std::map<std::string, mapUserBalanceWithCoinID> mapVerifiedUserBalanceByPubKey;
extern std::map<int, mapUserBalanceWithPubKey> mapVerifiedUserBalanceByCoinID;
extern std::map<int, CUserBalanceSetting> mapUserBalanceSetting;
extern CGlobalUserBalanceHandler globalUserBalanceHandler;
extern CUserBalanceManager userBalanceManager;
extern std::map<char, CGlobalUserSetting> mapGlobalUserSetting;

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
	char Char1;
	char Char2;
	bool nIsInChargeOfGlobalUserBalance;

	CGlobalUserBalanceHandler(char Char1, char Char2, bool nIsInChargeOfGlobalUserBalance):
		Char1(Char1),
		Char2(Char2),
		nIsInChargeOfGlobalUserBalance(nIsInChargeOfGlobalUserBalance)
	{}

	CGlobalUserBalanceHandler():
		Char1(),
		Char2(),
		nIsInChargeOfGlobalUserBalance(false)
	{}
};

class CGlobalUserSetting
{
public:
	char nChar;
	bool nInChargeUserBalance;
	bool nInChargeBackup;

	CGlobalUserSetting(char nChar) :
		nChar(nChar),
		nInChargeUserBalance(false),
		nInChargeBackup(false)
	{}

	CGlobalUserSetting() :
		nChar(),
		nInChargeUserBalance(false),
		nInChargeBackup(false)
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
	std::vector<unsigned char> mnVchSig;
	std::vector<unsigned char> finalVchSig;

public:
	std::string nUserPubKey;
	int nCoinID;
	int64_t nAvailableBalance;
	int64_t nInExchangeBalance;
	int64_t nInDisputeBalance;
	int64_t nPendingDepositBalance;
	int64_t nPendingWithdrawalBalance;
	int64_t nTotalBalance;
	int nLastDepositID;
	int nLastWithdrawID;
	int nLastActualTradeID;
	int nLastUserTradeID;
	std::string nMNPubKey;
	uint64_t nLastUpdateTime;

	CUserBalance(std::string nUserPubKey, int nCoinID) :
		nUserPubKey(nUserPubKey),
		nCoinID(nCoinID),
		nAvailableBalance(0),
		nInExchangeBalance(0),
		nInDisputeBalance(0),
		nPendingDepositBalance(0),
		nPendingWithdrawalBalance(0),
		nTotalBalance(0),
		nLastDepositID(0),
		nLastWithdrawID(0),
		nLastActualTradeID(0),
		nLastUserTradeID(0),
		nMNPubKey(nMNPubKey),
		nLastUpdateTime(0)
	{}

	CUserBalance() :
		nUserPubKey(""),
		nCoinID(0),
		nAvailableBalance(0),
		nInExchangeBalance(0),
		nInDisputeBalance(0),
		nPendingDepositBalance(0),
		nPendingWithdrawalBalance(0),
		nTotalBalance(0),
		nLastDepositID(0),
		nLastWithdrawID(0),
		nLastActualTradeID(0),
		nLastUserTradeID(0),
		nMNPubKey(""),
		nLastUpdateTime(0)
	{}

	bool VerifyMNSignature();
	bool VerifyFinalSignature();
	bool MNSign();
	void RelayToNode(CNode* node, CConnman& connman);
	void RelayToUser(CConnman& connman);
};

class CUserBalanceManager
{
private:
	std::vector<unsigned char> vchSig;

public:
	CUserBalanceManager() {}
	void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
	void InputUserBalance(CUserBalance userBalance);
	void InputVerifiedUserBalance(CConnman& connman, CUserBalance userBalance);
	void InitCoin(int CoinID);
	void InitGlobalUserSetting(char Char);
	bool AssignUserBalanceRole(char Char, bool toAssign = true);
	bool AssignBackupRole(char Char, bool toAssign = true);
	bool UpdateUserBalance(CUserBalance UserBalance);
	void InitUserBalance(int CoinID, std::string UserPubKey, std::shared_ptr<CUserBalance>& UserBalance);
	int GetLastDepositID(int CoinID, std::string UserPubKey);
	bool InChargeOfUserBalance(std::string pubKey);
	bool InChargeOfBackup(std::string pubKey);
	bool IsInChargeOfCoinBalance(int CoinID);
	bool IsCoinInList(int CoinID);
	bool IsUserBalanceExist(int CoinID, std::string UserPubKey);	
	bool VerifyUserBalance(int CoinID);
	userbalance_to_exchange_enum_t BalanceToExchange(int CoinID, std::string UserPubKey, uint64_t amount);
	exchange_to_userbalance_enum_t ExchangeToBalance(int CoinID, std::string UserPubKey, uint64_t amount);
	int64_t GetUserAvailableBalance(int CoinID, std::string UserPubKey);
	int64_t GetUserInExchangeBalance(int CoinID, std::string UserPubKey);
	int64_t GetUserPendingDepositBalance(int CoinID, std::string UserPubKey);
	int64_t GetUserPendingWithdrawalBalance(int CoinID, std::string UserPubKey);
	void AdjustUserAvailableBalance(int CoinID, std::string UserPubKey, int64_t amount);
	void AdjustUserInExchangeBalance(int CoinID, std::string UserPubKey, int64_t amount);
	void AdjustUserPendingDepositBalance(int CoinID, std::string UserPubKey, int64_t amount);
	void AdjustUserPendingWithdrawalBalance(int CoinID, std::string UserPubKey, int64_t amount);
	bool UpdateAfterTradeBalance(std::string User1PubKey, std::string User2PubKey, int CoinID1, int CoinID2, int64_t User1EAdjDown, int64_t User1BAdjUp, int64_t User2EAdjDown, int64_t User2BAdjUp);

	bool BalanceToExchangeV2(int CoinID, std::string UserPubKey, uint64_t amount);
	bool ExchangeToBalanceV2(int CoinID, std::string UserPubKey, uint64_t amount);
	bool PendingToAvailable(int CoinID, std::string UserPubKey, uint64_t PendingAmount, uint64_t AvailableAmount);
	bool AvailableToPending(int CoinID, std::string UserPubKey, uint64_t AvailableAmount, uint64_t PendingAmount);
	bool UpdateAfterTradeBalance(std::string UserPubKey, int ExchangeCoinID, int BalanceCoinID, int64_t ExchangeAdjDown, int64_t BalanceAdjUp);
};

#endif
