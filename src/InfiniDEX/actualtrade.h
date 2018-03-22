// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACTUALTRADE_H
#define ACTUALTRADE_H

#include <iostream>
#include <vector>
#include <set>
#include <map>
#include "userconnection.h"

class CActualTrade;
class CActualTradeSetting;
class CActualTradeManager;

typedef std::map<int, CActualTrade> mapActualTrade; //trade id & detail details;
typedef std::pair<CActualTradeSetting, std::set<std::string>> pairSettingSecurity; //trade pair setting & trade hash record
typedef std::pair<pairSettingSecurity, mapActualTrade> ActualTradeContainer;
extern std::map<int, ActualTradeContainer> mapTradePairActualTradeContainer;
extern std::map<int, std::vector<CActualTrade>> mapTradePairConflictTrade;
extern CActualTradeManager actualTradeManager;

class CActualTradeSetting
{
public:
	int nTradePairID;
	bool nSecurityCheck;
	bool nSyncInProgress;
	int nLastTradeMaxPreTimeDistance;
	int nToStoreLowerLimit;
	int nToStoreUpperLimit;
	int nLastActualTradeID;
	std::string nLastHash;

	CActualTradeSetting(int nTradePairID, bool nSecurityCheck) :
		nTradePairID(nTradePairID),
		nSecurityCheck(nSecurityCheck),
		nSyncInProgress(false),
		nLastTradeMaxPreTimeDistance(3000),
		nToStoreLowerLimit(50),
		nToStoreUpperLimit(100),
		nLastActualTradeID(0),
		nLastHash("")
	{}

	bool GetSecurityCheck() { return nSecurityCheck; }
};

class CActualTrade
{
private:
	std::vector<unsigned char> vchSig;

public:
	int nActualTradeID;
	int nTradePairID;
    uint64_t nTradePrice;
    uint64_t nTradeQty;
    uint64_t nTradeAmount;
	std::string nUserPubKey1;
	std::string nUserPubKey2;
    int64_t nFee1; //during promo period, we can provide rebate instead of trade fee to user
    int nFee1CoinID;
    int64_t nFee2; //during promo period, we can provide rebate instead of trade fee to user
    int nFee2CoinID;
    std::string nMasternodeInspector;
	std::string nCurrentHash;
	uint64_t nTradeTime;

	CActualTrade(int nActualTradeID) :
		nActualTradeID(nActualTradeID)
	{}

	CActualTrade(std::string nUserPubKey1, std::string nUserPubKey2, int nTradePairID, uint64_t nTradePrice, uint64_t nTradeQty, 
        uint64_t nTradeAmount, int64_t nFee1, int nFee1CoinID, int64_t nFee2, int nFee2CoinID, 
        std::string nMasternodeInspector, uint64_t nTradeTime) :
            nUserPubKey1(nUserPubKey1),
            nUserPubKey2(nUserPubKey2),
            nTradePairID(nTradePairID),
            nTradePrice(nTradePrice),
            nTradeQty(nTradeQty),
            nTradeAmount(nTradeAmount),
            nFee1(nFee1),
            nFee1CoinID(nFee1CoinID),
            nFee2(nFee2),
            nFee2CoinID(nFee2CoinID),
            nMasternodeInspector(nMasternodeInspector),
            nTradeTime(nTradeTime)
	{}

	CActualTrade() :
		nUserPubKey1(""),
        nUserPubKey2(""),
        nTradePairID(0),
        nTradePrice(0),
        nTradeQty(0),
        nTradeAmount(0),
        nFee1(0),
        nFee1CoinID(0),
        nFee2(0),
        nFee2CoinID(0),
        nMasternodeInspector(""),
        nTradeTime(0)
	{}

	std::string GetHash();
	bool CheckSignature();
	bool Sign();
	bool InformActualTrade();
	bool InformConflictTrade(CNode* node);
};

class CActualTradeManager
{
private:
	bool RunSecurityCheck(int TradePairID, std::string Hash);

public:
    CActualTradeManager() {}
	bool SetSecurityCheck(int TradePairID, bool SecurityCheck);
	bool GetActualTrade(CNode* node, int ActualTradeID, int TradePairID);
	bool AddNewActualTrade(CActualTrade ActualTrade); //process by same node
	bool AddNewActualTrade(CNode* node, CActualTrade ActualTrade); //data from other node
	std::vector<std::string> FindDuplicateTrade(int TradePairID);
	void InitiateCompleteResync(int TradePairID);
	void InitialSync(int TradePairID);
	void InputNewTradePair(int TradePairID, bool SecurityCheck);
};

#endif
