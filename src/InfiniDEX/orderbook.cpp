// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "activemasternode.h"
#include "messagesigner.h"
#include "timedata.h"
#include "orderbook.h"
#include <boost/lexical_cast.hpp>

class COrderBook;
class COrderBookManager;
class COrderBookSetting;

std::map<int, PriceOrderBook> mapOrderBidBook;
std::map<int, PriceOrderBook> mapOrderAskBook;
COrderBookManager orderBookManager;

bool COrderBook::VerifySignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nIsBid) + boost::lexical_cast<std::string>(nPrice)
		+ boost::lexical_cast<std::string>(nQty) + boost::lexical_cast<std::string>(nLastUpdateTime) + nMNPubKey;
	CPubKey pubkey(ParseHex(nMNPubKey));
	if (!CMessageSigner::VerifyMessage(pubkey, vchSig, strMessage, strError)) {
		LogPrintf("COrderBook::VerifySignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}
	return true;
}

bool COrderBook::Sign()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nIsBid) + boost::lexical_cast<std::string>(nPrice)
		+ boost::lexical_cast<std::string>(nQty) + boost::lexical_cast<std::string>(nLastUpdateTime) + nMNPubKey;
	if (!CMessageSigner::SignMessage(strMessage, vchSig, activeMasternode.keyMasternode)) {
		LogPrintf("COrderBook::Sign -- SignMessage() failed\n");
		return false;
	}
	if (!CMessageSigner::VerifyMessage(activeMasternode.pubKeyMasternode, vchSig, strMessage, strError)) {
		LogPrintf("COrderBook::Sign -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}
	return true;
}

void COrderBook::Broadcast()
{

}

void COrderBookManager::AdjustBidQuantity(int TradePairID, uint64_t Price, int64_t Qty)
{
	if (!mapOrderBidBook.count(TradePairID))
	{
		//need to sync with seed server or check node task
	}

	auto& a = mapOrderBidBook[TradePairID];
	if (!a.count(Price))
	{
		COrderBook temp(TradePairID, true, Price, Qty, GetAdjustedTime(), MNPubKey);
		if (!temp.Sign())
			return;
		a.insert(std::make_pair(Price, temp));
		temp.Broadcast();
	}
	else
	{
		auto& b = a[Price];
		b.nQty += Qty;
		b.nLastUpdateTime = GetAdjustedTime();
		if (!b.Sign())
			return;
		b.Broadcast();
	}
}

void COrderBookManager::AdjustAskQuantity(int TradePairID, uint64_t Price, int64_t Qty)
{
	if (!mapOrderAskBook.count(TradePairID))
	{
		//need to sync with seed server or check node task
	}

	auto& a = mapOrderAskBook[TradePairID];
	if (!a.count(Price))
	{
		COrderBook temp(TradePairID, false, Price, Qty, GetAdjustedTime(), MNPubKey);
		if (!temp.Sign())
			return;
		a.insert(std::make_pair(Price, temp));
		temp.Broadcast();
	}
	else
	{
		auto& b = a[Price];
		b.nQty += Qty;
		b.nLastUpdateTime = GetAdjustedTime();
		if (!b.Sign())
			return;
		b.Broadcast();
	}
}

void COrderBookManager::UpdateBidQuantity(int TradePairID, uint64_t Price, uint64_t Qty)
{
	if (!mapOrderBidBook.count(TradePairID))
	{
		//need to sync with seed server or check node task
	}

	auto& a = mapOrderBidBook[TradePairID];
	if (!a.count(Price))
	{
		COrderBook temp(TradePairID, true, Price, Qty, GetAdjustedTime(), MNPubKey);
		if (!temp.Sign())
			return;
		a.insert(std::make_pair(Price, temp));
		temp.Broadcast();
	}
	else
	{
		auto& b = a[Price];
		b.nQty = Qty;
		b.nLastUpdateTime = GetAdjustedTime();
		if (!b.Sign())
			return;
		b.Broadcast();
	}
}

void COrderBookManager::UpdateAskQuantity(int TradePairID, uint64_t Price, uint64_t Qty)
{
	if (!mapOrderAskBook.count(TradePairID))
	{
		//need to sync with seed server or check node task
	}

	auto& a = mapOrderAskBook[TradePairID];
	if (!a.count(Price))
	{
		COrderBook temp(TradePairID, false, Price, Qty, GetAdjustedTime(), MNPubKey);
		if (!temp.Sign())
			return;
		a.insert(std::make_pair(Price, temp));
		temp.Broadcast();
	}
	else
	{
		auto& b = a[Price];
		b.nQty = Qty;
		b.nLastUpdateTime = GetAdjustedTime();
		if (!b.Sign())
			return;
		b.Broadcast();
	}
}

void COrderBookManager::InitTradePair(int TradePairID)
{
	if (mapOrderBidBook.count(TradePairID))
		return;

	mapOrderBidBook.insert(std::make_pair(TradePairID, PriceOrderBook()));
	mapOrderAskBook.insert(std::make_pair(TradePairID, PriceOrderBook()));
}