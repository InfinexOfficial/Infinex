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

typedef std::map<std::string, CUserBalance> mapUserBalance; //user public key & userbalance
extern std::map<int, mapUserBalance> mapCoinUserBalance; //coin ID & map user balance
extern CUserBalanceManager userBalanceManager;

enum userbalance_to_exchange_enum_t {
	USERACCOUNT_NOT_FOUND = -1,
	USERBALANCE_NOT_ENOUGH = 0,
	USERBALANCE_DEDUCTED = 1
};

enum exchange_to_userbalance_enum_t {
	USERACCOUNT_NOT_FOUND = -1,
	EXCHANGEBALANCE_NOT_ENOUGH = 0,
	EXCHANGEBALANCE_RETURNED = 1
};

class CUserBalance
{
private:
	std::vector<unsigned char> vchSig;

public:
    std::string nUserPubKey;
	uint256 nCoinID;    
    int64_t nAvailableBalance;
	int64_t nPendingBalance;
	uint64_t nLastUpdateTime;

	CUserBalance(std::string nUserPubKey, uint256 nCoinID, int64_t nAvailableBalance, int64_t nPendingBalance, uint64_t nLastUpdateTime) :
		nUserPubKey(nUserPubKey),
        nCoinID(nCoinID),
        nAvailableBalance(nAvailableBalance),
		nPendingBalance(nPendingBalance),
        nLastUpdateTime(nLastUpdateTime)
	{}

	CUserBalance() :
		nUserPubKey(""),
        nCoinID(0),
        nAvailableBalance(0),
		nPendingBalance(0),
        nLastUpdateTime(0)
	{}

	ADD_SERIALIZE_METHODS;

	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
		READWRITE(nUserPubKey);
		READWRITE(nCoinID);
		READWRITE(nAvailableBalance);
		READWRITE(nPendingBalance);
		READWRITE(nLastUpdateTime);
        READWRITE(vchSig);
	}

	uint256 GetHash() const
	{
		CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
		ss << nUserPubKey;
		ss << nCoinID;
		ss << nAvailableBalance;
		ss << nPendingBalance;
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
	userbalance_to_exchange_enum_t BalanceToExchange(int CoinID, uint64_t amount);
	exchange_to_userbalance_enum_t ExchangeToBalance(int CoinID, uint64_t amount);
	int64_t GetUserAvailableBalance(int CoinID, std::string UserPubKey);
	int64_t GetUserInExchangeBalance(int CoinID, std::string UserPubKey);
	int64_t GetUserPendingBalance(int CoinID, std::string UserPubKey);
	void UpdateUserAvailableBalance();
	void UpdateUserPendingBalance();
};

#endif
