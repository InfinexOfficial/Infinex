// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERDEPOSIT_H
#define USERDEPOSIT_H

#include <iostream>
#include <vector>
#include <map>

class CUserDeposit;
class CUserDepositManager;

typedef std::pair<int, std::string> pairCoinIDUserPubKey; //pair of coin id & user public key
typedef std::map<std::string, CUserDeposit> mapHashUserDeposit;
extern std::map<pairCoinIDUserPubKey, mapHashUserDeposit> mapCoinUserDeposit;
extern CUserDepositManager userDepositManager;

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
};

class CUserDepositManager
{
private:
	std::vector<unsigned char> vchSig;

public:
	CUserDepositManager() {}
	bool IsUserDepositInList(std::string UserPubKey, int CoinID);
	bool AddNewDeposit(CUserDeposit UserDeposit);
	bool DepositConfirmation(std::string UserPubKey, int CoinID, std::string Hash, uint64_t LastUpdateTime);
};

#endif