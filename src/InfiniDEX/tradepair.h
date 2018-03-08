// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRADEPAIR_H
#define TRADEPAIR_H

#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"

class CTradePair;
class CTradePairManager;

extern std::vector<CTradePair> vecCompleteTradePair;
extern std::vector<CTradePair> vecTradePairInNode;
extern CTradePairManager tradePairManager;

class CTradePair
{
private:
	std::vector<unsigned char> vchSig;

public:
	int nTradePairID;
	std::string nName;
    int nCoinInfoID1;
	std::string nSymbol1;
    int nCoinInfoID2;
    std::string nSymbol2;
    bool nTradeEnabled;
    uint64_t nMinimumTradeQuantity;
    uint64_t nMaximumTradeQuantity;
	std::string nStatus;
    uint64_t nLastUpdate;

	CTradePair(int nTradePairID, std::string nName, int nCoinInfoID1, std::string nSymbol1, int nCoinInfoID2, std::string nSymbol2, bool nTradeEnabled, uint64_t nMinimumTradeQuantity, uint64_t nMaximumTradeQuantity, std::string nStatus, uint64_t nLastUpdate) :
		nTradePairID(nTradePairID),
		nName(nName),
		nCoinInfoID1(nCoinInfoID1),
		nSymbol1(nSymbol1),
        nCoinInfoID2(nCoinInfoID2),
        nSymbol2(nSymbol2),
        nTradeEnabled(nTradeEnabled),
        nMinimumTradeQuantity(nMinimumTradeQuantity),
        nMaximumTradeQuantity(nMaximumTradeQuantity),
        nStatus(nStatus),
        nLastUpdate(nLastUpdate)
	{}

	CTradePair() :
		nTradePairID(0),
		nName(""),
		nCoinInfoID1(0),
		nSymbol1(""),
        nCoinInfoID2(0),
        nSymbol2(""),
        nTradeEnabled(false),
        nMinimumTradeQuantity(0),
        nMaximumTradeQuantity(0),
        nStatus(""),
        nLastUpdate(0)
	{}


	ADD_SERIALIZE_METHODS;

	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
		READWRITE(nTradePairID);
		READWRITE(nName);
		READWRITE(nCoinInfoID1);
		READWRITE(nSymbol1);
        READWRITE(nCoinInfoID2);
		READWRITE(nSymbol2);
		READWRITE(nTradeEnabled);
		READWRITE(nMinimumTradeQuantity);
        READWRITE(nMaximumTradeQuantity);
        READWRITE(nStatus);
        READWRITE(nLastUpdate);
		READWRITE(vchSig);
	}

	uint256 GetHash() const
	{
		CHashWriter ss(SER_GETHASH, PROTOCOL_VERSION);
		ss << nTradePairID;
		ss << nName;
		ss << nCoinInfoID1;
		ss << nSymbol1;
        ss << nCoinInfoID2;
		ss << nSymbol2;
		ss << nTradeEnabled;
		ss << nMinimumTradeQuantity;        
		ss << nMaximumTradeQuantity;
        ss << nStatus;
		ss << nLastUpdate;
		return ss.GetHash();
	}

	bool Sign(std::string strSignKey);
	bool CheckSignature();
	void Relay(CConnman& connman);
};

class CTradePairManager
{
private:
    std::vector<unsigned char> vchSig;

public:

    CTradePairManager() {}

    void ProcessTradePairMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
};

#endif