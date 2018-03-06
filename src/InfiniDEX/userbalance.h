// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERBALANCE_H
#define USERBALANCE_H

#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"

class CUserBalance;
class CUserBalanceManager;

extern std::map<std::string, CUserBalance> mapUserBalance; //user public key & userbalance
extern CUserBalanceManager userBalanceManager;

class CUserBalance
{
private:
	std::vector<unsigned char> vchSig;

public:
    std::string nUserPubKey;
	uint256 nCoinID;    
    uint64_t nAvailableBalance;
	uint64_t nLastUpdateTime;

	CUserBalance(std::string nUserPubKey, uint256 nCoinID, uint64_t nAvailableBalance, uint64_t nLastUpdateTime) :
		nUserPubKey(nUserPubKey),
        nTradePairID(nCoinID),
        nAvailableBalance(nAvailableBalance),
        nLastUpdateTime(nLastUpdateTime)
	{}

	CUserBalance() :
		nUserPubKey(""),
        nTradePairID(0),
        nAvailableBalance(0),
        nLastUpdateTime(0)
	{}

	ADD_SERIALIZE_METHODS;

	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
		READWRITE(nUserPubKey);
		READWRITE(nCoinID);
		READWRITE(nAvailableBalance);
		READWRITE(nLastUpdateTime);
	}

	uint256 GetHash() const
	{
		CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
		ss << nUserPubKey;
		ss << nCoinID;
		ss << nAvailableBalance;
		ss << nLastUpdateTime;
		return ss.GetHash();
	}

	bool Sign(std::string strSignKey);
	bool CheckSignature();
	void RelayTo(CNode* pnode, CConnman& connman);
};

class CUserBalanceManager
{
private:
    std::vector<unsigned char> vchSig;

public:

    CUserBalanceManager() {}

    void ProcessUserBalanceMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);    
};

#endif
