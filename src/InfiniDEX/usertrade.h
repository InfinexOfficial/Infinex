// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERTRADE_H
#define USERTRADE_H

#include <iostream>
#include <vector>
#include <map>
#include <memory>

class CUserTrade;
class CUserTradeManager;

typedef std::pair<std::string, std::shared_ptr<CUserTrade>> PairSignatureUserTrade; //user signature (change to uint256 on actual implementation) and trade details
typedef std::map<uint64_t, std::vector<PairSignatureUserTrade>> MapPriceCUserTrade; //price and user trade map
typedef std::map<std::string, std::vector<PairSignatureUserTrade>> MapPubKeyCUserTrade; //user public key and user trade map
typedef std::pair<MapPriceCUserTrade, MapPubKeyCUserTrade> PairPricePubKeyCUserTrade;
typedef std::pair<PairPricePubKeyCUserTrade, PairPricePubKeyCUserTrade> PairBidAskCUserTrade; //bid & ask
extern std::map<int, PairBidAskCUserTrade> mapUserTradeRequest; //trade pair and bid ask data
extern CUserTradeManager userTradeManager;

class CUserTrade
{
private:
	std::vector<unsigned char> vchSig;

public:
	int nTradePairID;
	uint64_t nPrice;
	uint64_t nQuantity;
	uint64_t nAmount;
	std::string nUserPubKey;
	uint64_t nTimeSubmit;
	std::string nUserSignature; //change to uint256 on actual implementation
	int nTradeFee; //this is here to compare with trade pair fee & apply to whichever lower fee (benefit user)	
	int64_t nBalanceQty;
	int64_t nBalanceAmount;
	uint64_t nLastUpdate;

	CUserTrade(int nTradePairID, uint64_t nPrice, uint64_t nQuantity, std::string nUserPubKey, uint64_t nTimeSubmit, std::string nUserSignature) :
		nTradePairID(nTradePairID),
		nPrice(nPrice),
		nQuantity(nQuantity),
		nAmount(nPrice*nQuantity),
		nUserPubKey(nUserPubKey),
		nTimeSubmit(nTimeSubmit),
		nUserSignature(nUserSignature),
		nTradeFee(0),
		nBalanceQty(nQuantity),
		nBalanceAmount(nAmount),
		nLastUpdate(nTimeSubmit)
	{}

	CUserTrade(int nTradePairID, uint64_t nPrice, uint64_t nQuantity, uint64_t nAmount, std::string nUserPubKey, uint64_t nTimeSubmit, std::string nUserSignature, int nTradeFee, int64_t nBalanceQty, int64_t nBalanceAmount, uint64_t nLastUpdate) :
		nTradePairID(nTradePairID),
		nPrice(nPrice),
		nQuantity(nQuantity),
		nAmount(nAmount),
		nUserPubKey(nUserPubKey),
		nTimeSubmit(nTimeSubmit),
		nUserSignature(nUserSignature),
		nTradeFee(nTradeFee),
		nBalanceQty(nBalanceQty),
		nBalanceAmount(nBalanceAmount),
		nLastUpdate(nLastUpdate)
	{}

	CUserTrade() :
		nTradePairID(0),
		nPrice(0),
		nQuantity(0),
		nAmount(0),
		nUserPubKey(""),
		nTimeSubmit(0),
		nUserSignature(""),
		nTradeFee(0),
		nBalanceQty(0),
		nBalanceAmount(0),
		nLastUpdate(0)
	{}
};

class CUserTradeManager
{
private:
	std::vector<unsigned char> vchSig;
	uint64_t GetAdjustedTime();

public:
	bool IsSubmittedBidValid(CUserTrade UserTrade, CTradePair TradePair);
	bool IsSubmittedAskValid(CUserTrade UserTrade, CTradePair TradePair);
	bool IsSubmittedBidAmountValid(CUserTrade userTrade, int nTradeFee);
	bool IsSubmittedAskAmountValid(CUserTrade userTrade, int nTradeFee);
	void UserSellRequest(CUserTrade userTrade);
	void UserBuyRequest(CUserTrade userTrade);
	uint64_t GetBidRequiredAmount(uint64_t Price, uint64_t Qty, int TradeFee);
	uint64_t GetAskExpectedAmount(uint64_t Price, uint64_t Qty, int TradeFee);
};

#endif