// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERWITHDRAW_H
#define USERWITHDRAW_H

#include <iostream>
#include <vector>
#include <map>
#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"

class CUserWithdraw;
class CUserWithdrawManager;

typedef std::map<int, CUserWithdraw> mapUserWithdrawByID;
typedef std::pair<int, std::vector<mapUserWithdrawByID>> pairCoinUserWithdraw;
extern std::map<std::string, pairCoinUserWithdraw> mapUserWithdraw;
extern CUserWithdrawManager userWithdrawManager;

class CUserWithdraw
{
private:
	std::vector<unsigned char> userVchSig;
	std::vector<unsigned char> mnVchSig;
	std::vector<unsigned char> finalVchSig;

public:
    std::string UserPubKey;
	int CoinID;
    uint64_t WithdrawAmount;
	uint64_t WithdrawRequestTime;
	std::string UserHash;
	int UserWithdrawID;
	std::string MNPubKey;
	bool ValidWithdraw;
	uint64_t WithdrawCheckTime;	
	uint64_t WithdrawProcessTime;
    std::string TransactionID;    
    std::string Remark;
    uint64_t LastUpdateTime;

	CUserWithdraw():
		UserPubKey(""),
		CoinID(0),
		WithdrawAmount(0),
		WithdrawRequestTime(0),
		UserHash(""),
		UserWithdrawID(0),
		MNPubKey(""),
		ValidWithdraw(false),
		WithdrawCheckTime(0),
		WithdrawProcessTime(0),
		TransactionID(""),
		Remark(""),
		LastUpdateTime(0)
	{}
		
	bool VerifyUserSignature();
	bool VerifyMNSignature();
	bool VerifyFinalSignature();
	bool MNSign();
	void RelayToUser();
	void RelayToMN();
	void RelayToNetwork();
};

class CUserWithdrawManager
{
private:
    std::vector<unsigned char> vchSig;

public:
    CUserWithdrawManager() {}
	void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
	void InputUserWithdraw(CUserWithdraw userWithdraw);
	void AssignWithdrawProcessorRole(int CoinID);
	void AssignWithdrawInfoRole(int CoinID);	
	void ProcessUserWithdrawRequest(CUserWithdraw UserWithdrawRequest);
	void SendUserWithdrawalRecords(std::string UserPubKey, int CoinID);
	void SendUsersWithdrawalRecords(CNode* node, CConnman& connman, int CoinID);
};

#endif