// Copyright (c) 2014-2017 The Dash Core developers
// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SPORK_H
#define SPORK_H

#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"

class CSporkMessage;
class CSporkManager;

static const int SPORK_ENABLED = 1;
static const int SPORK_DISABLED = 0;

static const int SPORK_START = 10000;
static const int SPORK_END = 10014;

static const int SPORK_BEST_BLOCK_HASH = 10000;
static const int SPORK_MINIMUM_CHAIN_WORK = 10001;

static const int SPORK_INSTANTSEND_ENABLED = 10007;
static const int SPORK_INSTANTSEND_BLOCK_FILTERING = 10008;
static const int SPORK_INSTANTSEND_MAX_VALUE = 10009;
static const int SPORK_MASTERNODE_PAYMENT_ENFORCEMENT = 10010;
static const int SPORK_RECONSIDER_BLOCKS = 10012;
static const int SPORK_REQUIRE_SENTINEL_FLAG = 10013;

static const std::string SPORK_BEST_BLOCK_HASH_VALUE = "0000030a46501e7b1f930175c4c8489aeb1ecb25babc41aa205077caa963b61d";
static const std::string SPORK_MINIMUM_CHAIN_WORK_VALUE = "000000000000000000000000000000000000000000000000000000000000000f";

static const int64_t SPORK_INSTANTSEND_ENABLED_VALUE = SPORK_DISABLED;
static const int64_t SPORK_INSTANTSEND_BLOCK_FILTERING_VALUE = SPORK_DISABLED;
static const int64_t SPORK_INSTANTSEND_MAX_VALUE_VALUE = 1000;
static const int64_t SPORK_MASTERNODE_PAYMENT_ENFORCEMENT_VALUE = SPORK_ENABLED;
static const int64_t SPORK_RECONSIDER_BLOCKS_VALUE = 0;
static const int64_t SPORK_REQUIRE_SENTINEL_FLAG_VALUE = SPORK_DISABLED;

static const int SPORK_UNKNOWN_TYPE = -1;
static const int SPORK_NUMERIC_TYPE = 0;
static const int SPORK_TEXT_TYPE = 1;
static const int SPORK_BOOL_TYPE = 2;
static const int SPORK_TIME_TYPE = 3;

extern std::map<uint256, CSporkMessage> mapSporks;
extern CSporkManager sporkManager;

//
// Spork classes
// Keep track of all of the network spork settings
//

class CSporkMessage
{
private:
	std::vector<unsigned char> vchSig;

public:
	int nSporkID;
	int64_t nNumericValue;
	std::string nTextValue;
	int64_t nTimeSigned;

	CSporkMessage(int nSporkID, int64_t nNumericValue, std::string nTextValue, int64_t nTimeSigned) :
		nSporkID(nSporkID),
		nNumericValue(nNumericValue),
		nTextValue(nTextValue),
		nTimeSigned(nTimeSigned)
	{}

	CSporkMessage() :
		nSporkID(0),
		nNumericValue(0),
		nTextValue(""),
		nTimeSigned(0)
	{}


	ADD_SERIALIZE_METHODS;

	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
		READWRITE(nSporkID);
		READWRITE(nNumericValue);
		READWRITE(nTextValue);
		READWRITE(nTimeSigned);
		READWRITE(vchSig);
	}

	uint256 GetHash() const
	{
		CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
		ss << nSporkID;
		ss << nNumericValue;
		ss << nTextValue;
		ss << nTimeSigned;
		return ss.GetHash();
	}

	bool Sign(std::string strSignKey);
	bool CheckSignature();
	void Relay(CConnman& connman);
};

class CSporkManager
{
private:
    std::vector<unsigned char> vchSig;
    std::string strMasterPrivKey;
    std::map<int, CSporkMessage> mapSporksActive;

public:

    CSporkManager() {}

    void ProcessSpork(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
    void ExecuteNumericSpork(int nSporkID, int nValue);
	void ExecuteTextSpork(int nSporkID, std::string nValue);
    bool UpdateNumericSpork(int nSporkID, int64_t nValue, CConnman& connman);
	bool UpdateTextSpork(int nSporkID, std::string nValue, CConnman& connman);

    bool IsSporkActive(int nSporkID);
    int64_t GetNumericSporkValue(int nSporkID);
	std::string GetTextSporkValue(int nSporkID);
    int GetSporkIDByName(std::string strName);
    std::string GetSporkNameByID(int nSporkID);
	int GetSporkType(int nSporkID);

    bool SetPrivKey(std::string strPrivKey);
};

#endif
