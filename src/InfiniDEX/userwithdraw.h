// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERWITHDRAW_H
#define USERWITHDRAW_H

#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"

class CUserWithdraw;
class CUserWithdrawManager;

class CUserWithdraw
{
private:
	std::vector<unsigned char> vchSig;

public:
    std::string nUserPubKey;
	uint256 nCoinID;
    uint64_t nWithdrawAmount;
	uint64_t nWithdrawBlock;
	uint64_t nWithdrawTime;
    std::string nTransactionID;
    bool nValidWithdraw;
    std::string nRemark;
    uint64_t nLastUpdateTime;

	bool Sign(std::string strSignKey);
	bool CheckSignature();
	void RelayTo(CNode* pnode, CConnman& connman);
};

class CUserWithdrawManager
{
private:
    std::vector<unsigned char> vchSig;

public:
    CUserWithdrawManager() {}
};

#endif