// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "orderbook.h"
#include <chrono>

class COrderBook;
class COrderBookManager;
class COrderBookSetting;

std::map<int, PriceOrderBook> mapOrderBidBook;
std::map<int, PriceOrderBook> mapOrderAskBook;
COrderBookManager orderBookManager;

bool COrderBook::VerifySignature()
{
	return true;
}

bool Sign()
{
	return true;
}

uint64_t COrderBookManager::GetAdjustedTime()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();	
}

bool COrderBookManager::InsertNewBidOrder(int TradePairID, uint64_t Price, int64_t Qty)
{
	if (!mapOrderBidBook.count(TradePairID))
		return false;

	if (mapOrderBidBook[TradePairID].count(Price))
		return false;
	
	COrderBook NewOrder(Price, Qty, Price*Qty, "", GetAdjustedTime());
	mapOrderBidBook[TradePairID].insert(std::make_pair(Price, NewOrder));
	return true;
}

bool COrderBookManager::InsertNewAskOrder(int TradePairID, uint64_t Price, int64_t Qty)
{
	if (!mapOrderAskBook.count(TradePairID))
		return false;

	if (mapOrderAskBook[TradePairID].count(Price))
		return false;

	COrderBook NewOrder(Price, Qty, Price*Qty, "", GetAdjustedTime());
	mapOrderAskBook[TradePairID].insert(std::make_pair(Price, NewOrder));
	return true;
}

void COrderBookManager::AdjustBidQuantity(int TradePairID, uint64_t Price, int64_t Qty)
{
	if (!mapOrderBidBook.count(TradePairID))
	{
		//need to sync with seed server or check node task
	}

	if (InsertNewBidOrder(TradePairID, Price, Qty))
		return;

	int finalQty = mapOrderBidBook[TradePairID][Price].nQuantity + Qty;
	if (finalQty < 0)
	{
		//some check need to be done here
		finalQty = 0;
	}

	mapOrderBidBook[TradePairID][Price].nQuantity = finalQty;
}

void COrderBookManager::AdjustAskQuantity(int TradePairID, uint64_t Price, int64_t Qty)
{
	if (!mapOrderAskBook.count(TradePairID))
	{
		//need to sync with seed server or check node task
	}

	if (InsertNewAskOrder(TradePairID, Price, Qty))
		return;

	int finalQty = mapOrderAskBook[TradePairID][Price].nQuantity + Qty;
	if (finalQty < 0)
	{
		//some check need to be done here
		finalQty = 0;
	}

	mapOrderAskBook[TradePairID][Price].nQuantity = finalQty;
}

void COrderBookManager::UpdateBidOrder(int TradePairID, uint64_t Price, uint64_t Qty)
{
	if (!mapOrderBidBook.count(TradePairID))
	{
		//need to sync with seed server or check node task
	}

	if (InsertNewBidOrder(TradePairID, Price, Qty))
		return;

	mapOrderBidBook[TradePairID][Price].nQuantity = Qty;
}

void COrderBookManager::UpdateAskOrder(int TradePairID, uint64_t Price, uint64_t Qty)
{
	if (!mapOrderAskBook.count(TradePairID))
	{
		//need to sync with seed server or check node task
	}

	if (InsertNewAskOrder(TradePairID, Price, Qty))
		return;

	mapOrderAskBook[TradePairID][Price].nQuantity = Qty;
}

void COrderBookManager::InitTradePair(int TradePairID)
{
	if (mapOrderBidBook.count(TradePairID))
		return;

	mapOrderBidBook.insert(std::make_pair(TradePairID, PriceOrderBook()));
	mapOrderAskBook.insert(std::make_pair(TradePairID, PriceOrderBook()));
}

void COrderBookManager::BroadcastBidOrder(int TradePairID, uint64_t Price)
{

}

void COrderBookManager::BroadcastAskOrder(int TradePairID, uint64_t Price)
{

}

void COrderBookManager::BroadcastBidOrders(int TradePairID)
{

}

void COrderBookManager::BroadcastAskOrders(int TradePairID)
{

}