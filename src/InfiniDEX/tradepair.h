// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRADEPAIR_H
#define TRADEPAIR_H

#include <iostream>
#include <vector>
#include <map>

class CTradePair;
class CTradePairManager;

extern std::vector<CTradePair> vecCompleteTradePair;
extern CTradePairManager tradePairManager;

enum new_tradepair_enum {
	NEW_TRADEPAIR_ADDED = 0,
	NEW_TRADEPAIR_EXIST = 1,
	NEW_TRADEPAIR_COINID_EXIST = 2,
	NEW_TRADEPAIR_REVERSE_COINID_EXIST = 3,
	NEW_TRADEPAIR_SYMBOL_EXIST = 4,
	NEW_TRADEPAIR_REVERSE_SYMBOL_EXIST = 5
};

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
	uint64_t nMinimumTradeAmount;
	uint64_t nMaximumTradeAmount;
	int nBidTradeFee; //regular trade fee in percentage for bid, can be negative as form of incentive
	int nBidTradeFeeCoinID; //trade fee coin type payout for bid
	int nAskTradeFee; //regular trade fee in percentage for ask, can be negative as form of incentive
	int nAskTradeFeeCoinID; //trade fee coin type payout for ask
	std::string nStatus;
	uint64_t nLastUpdate;

	CTradePair(int nTradePairID, std::string nName, int nCoinInfoID1, std::string nSymbol1, int nCoinInfoID2, std::string nSymbol2,
		bool nTradeEnabled, uint64_t nMinimumTradeQuantity, uint64_t nMaximumTradeQuantity, uint64_t nMinimumTradeAmount, uint64_t nMaximumTradeAmount,
		int nBidTradeFee, int nBidTradeFeeCoinID, int nAskTradeFee, int nAskTradeFeeCoinID, std::string nStatus, uint64_t nLastUpdate) :
		nTradePairID(nTradePairID),
		nName(nName),
		nCoinInfoID1(nCoinInfoID1),
		nSymbol1(nSymbol1),
		nCoinInfoID2(nCoinInfoID2),
		nSymbol2(nSymbol2),
		nTradeEnabled(nTradeEnabled),
		nMinimumTradeQuantity(nMinimumTradeQuantity),
		nMaximumTradeQuantity(nMaximumTradeQuantity),
		nMinimumTradeAmount(nMinimumTradeAmount),
		nMaximumTradeAmount(nMaximumTradeAmount),
		nBidTradeFee(nBidTradeFee),
		nBidTradeFeeCoinID(nBidTradeFeeCoinID),
		nAskTradeFee(nAskTradeFee),
		nAskTradeFeeCoinID(nAskTradeFeeCoinID),
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
		nMinimumTradeAmount(0),
		nMaximumTradeAmount(0),
		nBidTradeFee(0),
		nBidTradeFeeCoinID(0),
		nAskTradeFee(0),
		nAskTradeFeeCoinID(0),
		nStatus(""),
		nLastUpdate(0)
	{}
};

class CTradePairManager
{
private:
	std::vector<unsigned char> vchSig;

public:
	CTradePairManager() {}
	new_tradepair_enum ProcessNewTradePair(CTradePair TradePair);
	bool IsValidTradePair(int TradePairID);
	CTradePair GetTradePair(int TradePairID);
	int GetAskSideCoinInfoID(int TradePairID);
	int GetBidSideCoinInfoID(int TradePairID);
	void GetTradeFee(int TradePairID, int &BuyFee, int &SellFee);
	int GetBidTradeFee(int TradePairID);
	int GetAskTradeFee(int TradePairID);
};

#endif