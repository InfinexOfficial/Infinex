// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERDEPOSIT_H
#define USERDEPOSIT_H

#include <iostream>
#include <vector>
#include <map>
#include "userconnection.h"

class CUserDeposit;
class CUserDepositSetting;
class CUserDepositManager;

typedef std::map<std::string, uint64_t> mapLastRequestTimeByPubKey;
typedef std::map<int, CUserDeposit> mapUserDepositByID;
typedef std::map<std::string, mapUserDepositByID> mapUserDepositByPubKey;
extern std::map<int, mapUserDepositByPubKey> mapUserDeposit;
extern std::map<int, mapLastRequestTimeByPubKey> mapUserLastRequestTime;
extern std::map<int, CUserDepositSetting> mapUserDepositSetting;
extern CUserDepositManager userDepositManager;

class CUserDepositSetting
{
public:
	bool ProvideUserDepositInfo;
	bool SyncInProgress;
	int MaxNoOfUserDepositInfo;
	int MinTimeToLastRequest;
	int LastDepositID;

	CUserDepositSetting() :
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
	bool nValidDeposit;
	std::string nRemark;
	uint64_t nLastUpdateTime;

	CUserDeposit(int nUserDepositID, std::string nUserPubKey, int nCoinID, uint64_t nDepositAmount, uint64_t nBlockNumber, uint64_t nDepositTime, bool nValidDeposit, std::string nRemark, uint64_t nLastUpdateTime) :
		nUserDepositID(nUserDepositID),
		nUserPubKey(nUserPubKey),
		nCoinID(nCoinID),
		nDepositAmount(nDepositAmount),
		nBlockNumber(nBlockNumber),
		nDepositTime(nDepositTime),
		nValidDeposit(nValidDeposit),
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
		nValidDeposit(false),
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
	void ProcessMessage(CNode* node, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
	void ToProvideUserDepositInfo(int CoinID, bool toProcess);
	void SetSyncInProgress(int CoinID, bool status);
	void AddCoinToList(int CoinID);
	bool IsCoinInList(int CoinID);
	void AddNewUser(std::string UserPubKey, int CoinID);
	bool IsUserInList(std::string UserPubKey, int CoinID);
	void RequestPendingDepositData(int StartingID);
	void AddNewPendingDeposit(CUserDeposit UserDeposit);	
	bool IsUserPendingDepositInList(CUserDeposit UserDeposit);
	int GetLastUserPendingDepositID(std::string UserPubKey, int CoinID);
	void RequestConfirmDepositData(int StartingID);
	void AddNewConfirmDeposit(CUserDeposit UserDeposit);
	bool IsUserConfirmDepositInList(CUserDeposit UserDeposit);
	int GetLastUserConfirmDepositID(std::string UserPubKey, int CoinID);
};

#endif