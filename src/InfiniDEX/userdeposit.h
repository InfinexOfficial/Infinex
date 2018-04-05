// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERDEPOSIT_H
#define USERDEPOSIT_H

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include "userconnection.h"

enum userdeposit_status_enum {
	USER_DEPOSIT_EMPTY = 0,
	USER_DEPOSIT_INVALID = 1,
	USER_DEPOSIT_PENDING = 2,
	USER_DEPOSIT_CONFIRMED = 3
};

class CUserDeposit;
class CUserDepositSetting;
class CUserDepositManager;

typedef std::map<std::string, uint64_t> mapLastRequestTimeByPubKey;
typedef std::map<int, std::shared_ptr<CUserDeposit>> mapUserDepositWithID;
typedef std::map<std::string, mapUserDepositWithID> mapUserDepositWithPubKey;
typedef std::map<int, mapUserDepositWithID> mapUserDepositWithCoinID;
extern std::map<int, mapUserDepositWithPubKey> mapUserDepositByCoinID;
extern std::map<std::string, mapUserDepositWithCoinID> mapUserDepositByPubKey;
extern std::map<int, mapLastRequestTimeByPubKey> mapUserLastRequestTime;
extern std::map<int, CUserDepositSetting> mapUserDepositSetting;
extern CUserDepositManager userDepositManager;

class CUserDepositSetting
{
public:
	int CoinID;
	bool ProvideUserDepositInfo;
	bool SyncInProgress;
	int MaxNoOfUserDepositInfo;
	int MinTimeToLastRequest;
	int LastDepositID;

	CUserDepositSetting(int CoinID) :
		CoinID(CoinID),
		ProvideUserDepositInfo(false),
		SyncInProgress(false),
		MaxNoOfUserDepositInfo(100),
		MinTimeToLastRequest(3000),
		LastDepositID(0)
	{}

	CUserDepositSetting() :
		CoinID(0),
		ProvideUserDepositInfo(false),
		SyncInProgress(false),
		MaxNoOfUserDepositInfo(100),
		MinTimeToLastRequest(3000),
		LastDepositID(0)
	{}
};

class CUserDeposit
{
private:
	std::vector<unsigned char> vchSig;

public:
	int nUserDepositID;
	std::string nUserPubKey;
	int nCoinID;
	uint64_t nDepositAmount;
	uint64_t nBlockNumber;
	uint64_t nDepositTime;
	userdeposit_status_enum nDepositStatus;
	std::string nRemark;
	uint64_t nLastUpdateTime;

	CUserDeposit(int nUserDepositID, std::string nUserPubKey, int nCoinID, uint64_t nDepositAmount, uint64_t nBlockNumber, uint64_t nDepositTime, userdeposit_status_enum nDepositStatus, std::string nRemark, uint64_t nLastUpdateTime) :
		nUserDepositID(nUserDepositID),
		nUserPubKey(nUserPubKey),
		nCoinID(nCoinID),
		nDepositAmount(nDepositAmount),
		nBlockNumber(nBlockNumber),
		nDepositTime(nDepositTime),
		nDepositStatus(nDepositStatus),
		nRemark(nRemark),
		nLastUpdateTime(nLastUpdateTime)
	{}

	CUserDeposit() :
		nUserDepositID(0),
		nUserPubKey(""),
		nCoinID(0),
		nDepositAmount(0),
		nBlockNumber(0),
		nDepositTime(0),
		nDepositStatus(USER_DEPOSIT_EMPTY),
		nRemark(""),
		nLastUpdateTime(0)
	{}

	bool Verify();
	void RelayTo(CNode* node, CConnman& connman);
	void RelayToCoOpNode(CConnman& connman);
};

class CUserDepositManager
{
private:
	std::vector<unsigned char> vchSig;

public:
	CUserDepositManager() {}
	void InputUserDeposit(std::shared_ptr<CUserDeposit> UserDeposit);
	void AssignDepositInfoRole(int TradePairID);
	void ProcessMessage(CNode* node, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
	bool IsInChargeOfUserDepositInfo(int CoinID);
	void ToProvideUserDepositInfo(int CoinID, bool toProcess);
	void SetSyncInProgress(int CoinID, bool status);
	void AddCoinToList(int CoinID);
	bool IsCoinInList(int CoinID);
	void AddNewUser(std::string UserPubKey, int CoinID);
	bool IsUserInList(std::string UserPubKey, int CoinID);
	void RequestPendingDepositData(int StartingID);
	void AddPendingDeposit(std::shared_ptr<CUserDeposit> UserDeposit);
	int GetLastUserPendingDepositID(std::string UserPubKey, int CoinID);
	void RequestConfirmDepositData(int StartingID);
	void AddConfirmDeposit(std::shared_ptr<CUserDeposit> UserDeposit);
	bool IsUserDepositInList(std::shared_ptr<CUserDeposit> UserDeposit);
	int GetLastUserConfirmDepositID(std::string UserPubKey, int CoinID);
};

#endif