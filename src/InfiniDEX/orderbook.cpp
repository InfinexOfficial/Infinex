// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "orderbook.h"
#include <chrono>

class COrderBook;
class COrderBookManager;

std::map<int, OrderBookInfo> mapOrderBook;
COrderBookManager orderBookManager;

uint64_t COrderBookManager::GetAdjustedTime()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();	
}

bool COrderBookManager::InsertNewBidOrder(int TradePairID, uint64_t Price, int64_t Qty)
{
	if (!mapOrderBook.count(TradePairID))
		return false;

	if (mapOrderBook[TradePairID].second.first.count(Price))
		return false;
	
	COrderBook NewOrder(Price, Qty, Price*Qty, "", GetAdjustedTime());
	mapOrderBook[TradePairID].second.first.insert(std::make_pair(Price, NewOrder));
	return true;
}

bool COrderBookManager::InsertNewAskOrder(int TradePairID, uint64_t Price, int64_t Qty)
{
	if (!mapOrderBook.count(TradePairID))
		return false;

	if (mapOrderBook[TradePairID].second.second.count(Price))
		return false;

	COrderBook NewOrder(Price, Qty, Price*Qty, "", GetAdjustedTime());
	mapOrderBook[TradePairID].second.second.insert(std::make_pair(Price, NewOrder));
	return true;
}

void COrderBookManager::AdjustBidQuantity(int TradePairID, uint64_t Price, int64_t Qty)
{
	if (!mapOrderBook.count(TradePairID))
	{
		//need to sync with seed server or check node task
	}

	if (InsertNewBidOrder(TradePairID, Price, Qty))
		return;

	int finalQty = mapOrderBook[TradePairID].second.first[Price].nQuantity + Qty;
	if (finalQty < 0)
	{
		//some check need to be done here
		finalQty = 0;
	}

	mapOrderBook[TradePairID].second.first[Price].nQuantity = finalQty;
}

void COrderBookManager::AdjustAskQuantity(int TradePairID, uint64_t Price, int64_t Qty)
{
	if (!mapOrderBook.count(TradePairID))
	{
		//need to sync with seed server or check node task
	}

	if (InsertNewAskOrder(TradePairID, Price, Qty))
		return;

	int finalQty = mapOrderBook[TradePairID].second.second[Price].nQuantity + Qty;
	if (finalQty < 0)
	{
		//some check need to be done here
		finalQty = 0;
	}

	mapOrderBook[TradePairID].second.second[Price].nQuantity = finalQty;
}

void COrderBookManager::UpdateBidOrder(int TradePairID, uint64_t Price, uint64_t Qty)
{
	if (!mapOrderBook.count(TradePairID))
	{
		//need to sync with seed server or check node task
	}

	if (InsertNewBidOrder(TradePairID, Price, Qty))
		return;

	mapOrderBook[TradePairID].second.first[Price].nQuantity = Qty;
}

void COrderBookManager::UpdateAskOrder(int TradePairID, uint64_t Price, uint64_t Qty)
{
	if (!mapOrderBook.count(TradePairID))
	{
		//need to sync with seed server or check node task
	}

	if (InsertNewAskOrder(TradePairID, Price, Qty))
		return;

	mapOrderBook[TradePairID].second.second[Price].nQuantity = Qty;
}

void COrderBookManager::AssignRole(int TradePairID)
{
	if (!mapOrderBook.count(TradePairID))
	{
		
	}
}