// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERDEPOSIT_H
#define USERDEPOSIT_H

#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"
#include "userconnection.h"

class CUserDeposit;
class CUserDepositManager;



class CUserDeposit
{
private:
	std::vector<unsigned char> vchSig;

public:
    std::string nUserPubKey;
	uint256 nCoinID;
    uint64_t nDepositAmount;
	uint64_t nBlockNumber;
	uint64_t nDepositTime;
    bool nValidDeposit;
    std::string nRemark;
    uint64_t nLastUpdateTime;

	CUserDeposit(std::string nUserPubKey, uint256 nCoinID, uint64_t nDepositAmount, uint64_t nBlockNumber, uint64_t nDepositTime, bool nValidDeposit, std::string nRemark, uint64_t nLastUpdateTime) :
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
		nUserPubKey(""),
        nCoinID(0),
        nDepositAmount(0),
		nBlockNumber(0),
        nDepositTime(0),
        nValidDeposit(false),
        nRemark(""),
        nLastUpdateTime(0)
	{}

	ADD_SERIALIZE_METHODS;

	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
		READWRITE(nUserPubKey);
		READWRITE(nCoinID);
		READWRITE(nDepositAmount);
		READWRITE(nBlockNumber);
		READWRITE(nDepositTime);
        READWRITE(nValidDeposit);
        READWRITE(nRemark);
        READWRITE(nLastUpdateTime);
        READWRITE(vchSig);
	}

	uint256 GetHash() const
	{
		CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
		ss << nUserPubKey;
		ss << nCoinID;
		ss << nDepositAmount;
		ss << nBlockNumber;
		ss << nDepositTime;
        ss << nValidDeposit;
        ss << nRemark;
        ss << nLastUpdateTime;
		return ss.GetHash();
	}
	
	bool CheckSignature();
	void RelayTo(CUserConnection user);
};

class CUserDepositManager
{
private:
    std::vector<unsigned char> vchSig;

public:
    CUserDepositManager() {}
	bool AddNewDeposit(CUserDeposit UserDeposit);
	bool ReflectDepositToBalance(std::string DepositHash);
	bool UpdateDepositStatus(std::string DepositHash, bool ValidDeposit, std::string Remark);
};

#endif