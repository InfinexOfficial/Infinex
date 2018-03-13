// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERTRADE_H
#define USERTRADE_H

#include "hash.h"
#include "net.h"
#include "utilstrencodings.h"

class CUserTrade;
class CUserTradeManager;

typedef std::map<uint64_t, std::vector<CUserTrade*>> MapPriceCUserTrade; //price and user trade map
typedef std::map<std::string, std::vector<CUserTrade*>> MapPubKeyCUserTrade; //user public key and user trade map
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
	int64_t nBalanceQty;
	int64_t nBalanceAmount;
	uint64_t nLastUpdate;

	CUserTrade(int nTradePairID, uint64_t nPrice, uint64_t nQuantity, uint64_t nAmount, std::string nUserPubKey, uint64_t nTimeSubmit, int64_t nBalanceQty, int64_t nBalanceAmount, uint64_t nLastUpdate) :
		nTradePairID(nTradePairID),
		nPrice(nPrice),
		nQuantity(nQuantity),
		nAmount(nAmount),
		nUserPubKey(nUserPubKey),
		nTimeSubmit(nTimeSubmit),
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
		nBalanceQty(0),
		nBalanceAmount(0),
		nLastUpdate(0)
	{}
};

class CUserTradeManager
{
private:
	std::vector<unsigned char> vchSig;

public:
	void UserSellRequest(CUserTrade userTrade);
	void UserBuyRequest(CUserTrade userTrade);
};

#endif