// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERWITHDRAW_H
#define USERWITHDRAW_H

#include <iostream>
#include <vector>
#include <map>
#include <userconnection.h>

class CUserWithdraw;
class CUserWithdrawManager;

typedef std::pair<std::string, std::shared_ptr<CUserWithdraw>> PairHashUserWithdraw; //node signature and user withdraw request
typedef std::pair<std::string, PairHashUserWithdraw> PairPubKeyUserWithdraw;
extern std::map<int, PairPubKeyUserWithdraw> mapCoinUserWithdraw; //coin ID and users withdraw details
extern CUserWithdrawManager userWithdrawManager;

class CUserWithdraw
{
private:
	std::vector<unsigned char> vchSig;

public:
    std::string nUserPubKey;
	int nCoinID;
    uint64_t nWithdrawAmount;
	uint64_t nWithdrawRequestTime;
	std::string nUserHash;
	int nUserWithdrawID;
	std::string nMNPubKey;
	bool nValidWithdraw;
	uint64_t nWithdrawCheckTime;	
	std::string nMNHash;
	uint64_t nWithdrawProcessTime;
    std::string nTransactionID;    
    std::string nRemark;
	std::string nFinalHash;
    uint64_t nLastUpdateTime;
		
	bool VerifyWithdrawalSignature();
	bool VerifyMasternodeSignature();
	bool VerifyFinalSignature();
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
	void ProcessUserWithdrawRequest(CUserWithdraw UserWithdrawRequest);
	void SendUserWithdrawalRecords(std::string UserPubKey, int CoinID);
	void SendUsersWithdrawalRecords(CNode* node, CConnman& connman, int CoinID);
};

#endif