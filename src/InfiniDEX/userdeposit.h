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

typedef std::map<int, CUserDeposit> mapUserDepositWithID;
typedef std::pair<mapUserDepositWithID, mapUserDepositWithID> pairConfirmPendingDeposit;
typedef std::map<std::string, pairConfirmPendingDeposit> mapPubKeyConfirmPendingDeposit;
typedef std::pair<CUserDepositSetting, mapPubKeyConfirmPendingDeposit> pairSettingUserDeposit;
extern std::map<int, pairSettingUserDeposit> mapUserDeposit;
extern CUserDepositManager userDepositManager;

class CUserDepositSetting
{
public:
	bool ProvideUserDepositInfo;
	bool SyncInProgress;

	CUserDepositSetting() :
		ProvideUserDepositInfo(false),
		SyncInProgress(false)
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
	std::string nHash;
	uint64_t nLastUpdateTime;

	CUserDeposit(int nUserDepositID, std::string nUserPubKey, int nCoinID, uint64_t nDepositAmount, uint64_t nBlockNumber, uint64_t nDepositTime, bool nValidDeposit, std::string nRemark, std::string nHash, uint64_t nLastUpdateTime) :
		nUserDepositID(nUserDepositID),
		nUserPubKey(nUserPubKey),
		nCoinID(nCoinID),
		nDepositAmount(nDepositAmount),
		nBlockNumber(nBlockNumber),
		nDepositTime(nDepositTime),
		nValidDeposit(nValidDeposit),
		nRemark(nRemark),
		nHash(nHash),
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
		nHash(""),
		nLastUpdateTime(0)
	{}

	std::string GetHash();
	bool Verify();
};

class CUserDepositManager
{
private:
	std::vector<unsigned char> vchSig;

public:
	CUserDepositManager() {}
	void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
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
	void DepositConfirmation(std::string UserPubKey, int CoinID, std::string Hash, uint64_t LastUpdateTime);
};

#endif