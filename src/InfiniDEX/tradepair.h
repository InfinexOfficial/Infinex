// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRADEPAIR_H
#define TRADEPAIR_H

#include <iostream>
#include <vector>
#include <map>
#include "userconnection.h"
#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"

class CTradePair;
class CTradePairManager;

extern std::map<int, CTradePair> mapCompleteTradePair;
extern CTradePairManager tradePairManager;

enum tradepair_enum {
	TRADEPAIR_INVALID = 0,
	TRADEPAIR_EXIST = 1,
	TRADEPAIR_ADDED = 2,
	TRADEPAIR_UPDATED = 3,
	TRADEPAIR_COINID_EXIST = 4,
	TRADEPAIR_REVERSE_COINID_EXIST = 5,
	TRADEPAIR_SYMBOL_EXIST = 6,
	TRADEPAIR_REVERSE_SYMBOL_EXIST = 7
};

class CTradePair
{
private:
	std::vector<unsigned char> vchSig;

public:
	int nTradePairID;
	std::string nName;
	int nCoinID1;
	std::string nSymbol1;
	int nCoinID2;
	std::string nSymbol2;
	bool nTradeEnabled;
	uint64_t nMinimumTradeQuantity;
	uint64_t nMaximumTradeQuantity;
	uint64_t nMinimumTradeAmount;
	uint64_t nMaximumTradeAmount;
	int nBidTradeFee; //regular trade fee in percentage for bid, can be negative as form of incentive
	int nAskTradeFee; //regular trade fee in percentage for ask, can be negative as form of incentive
	std::string nStatus;
	uint64_t nLastUpdate;

	CTradePair(int nTradePairID, std::string nName, int nCoinID1, std::string nSymbol1, int nCoinID2, std::string nSymbol2,
		bool nTradeEnabled, uint64_t nMinimumTradeQuantity, uint64_t nMaximumTradeQuantity, uint64_t nMinimumTradeAmount, uint64_t nMaximumTradeAmount,
		int nBidTradeFee, int nAskTradeFee, std::string nStatus, uint64_t nLastUpdate) :
		nTradePairID(nTradePairID),
		nName(nName),
		nCoinID1(nCoinID1),
		nSymbol1(nSymbol1),
		nCoinID2(nCoinID2),
		nSymbol2(nSymbol2),
		nTradeEnabled(nTradeEnabled),
		nMinimumTradeQuantity(nMinimumTradeQuantity),
		nMaximumTradeQuantity(nMaximumTradeQuantity),
		nMinimumTradeAmount(nMinimumTradeAmount),
		nMaximumTradeAmount(nMaximumTradeAmount),
		nBidTradeFee(nBidTradeFee),
		nAskTradeFee(nAskTradeFee),
		nStatus(nStatus),
		nLastUpdate(nLastUpdate)
	{}

	CTradePair() :
		nTradePairID(0),
		nName(""),
		nCoinID1(0),
		nSymbol1(""),
		nCoinID2(0),
		nSymbol2(""),
		nTradeEnabled(false),
		nMinimumTradeQuantity(0),
		nMaximumTradeQuantity(0),
		nMinimumTradeAmount(0),
		nMaximumTradeAmount(0),
		nBidTradeFee(0),
		nAskTradeFee(0),
		nStatus(""),
		nLastUpdate(0)
	{}

	ADD_SERIALIZE_METHODS;

	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
		READWRITE(nTradePairID);
		READWRITE(nName);
		READWRITE(nCoinID1);
		READWRITE(nSymbol1);
		READWRITE(nCoinID2);
		READWRITE(nSymbol2);
		READWRITE(nTradeEnabled);
		READWRITE(nMinimumTradeQuantity);
		READWRITE(nMaximumTradeQuantity);
		READWRITE(nMinimumTradeAmount);
		READWRITE(nMaximumTradeAmount);
		READWRITE(nBidTradeFee);
		READWRITE(nAskTradeFee);
		READWRITE(nStatus);
		READWRITE(nLastUpdate);
		READWRITE(vchSig);
	}

	bool VerifySignature();
	bool RelayTo(CNode* node, CConnman& connman);
};

class CTradePairManager
{
private:
	std::vector<unsigned char> vchSig;

public:
	CTradePairManager() {}
	void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
<<<<<<< HEAD
	bool InputTradePair(CTradePair &tradePair);	
=======
	bool InputTradePair(CTradePair &tradePair);
>>>>>>> InfiniDEX-dev
	void SendCompleteTradePairs(CNode* node, CConnman& connman);
	void SendTradePair(CTradePair TradePair, CNode* node, CConnman& connman);
	tradepair_enum ProcessTradePair(CTradePair TradePair);
	bool IsValidTradePairID(int TradePairID);
	CTradePair GetTradePair(int TradePairID);
	bool IsTradeEnabled(int TradePairID);
	std::string GetTradePairStatus(int TradePairID);
	int GetAskSideCoinID(int TradePairID);
	int GetBidSideCoinID(int TradePairID);
	void GetTradeFee(int TradePairID, int &BuyFee, int &SellFee);
	int GetBidTradeFee(int TradePairID);
	int GetAskTradeFee(int TradePairID);
};

#endif