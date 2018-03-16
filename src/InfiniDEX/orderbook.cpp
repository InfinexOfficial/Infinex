// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "orderbook.h"
#include <chrono>

class COrderBook;
class COrderBookManager;

std::map<int, PriceOrderBook> orderBidBook; //trade pair and bid data
std::map<int, PriceOrderBook> orderAskBook; //trade pair and ask data
COrderBookManager orderBookManager;

uint64_t COrderBookManager::GetAdjustedTime()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();	
}

bool COrderBookManager::InsertNewBidOrder(int TradePairID, uint64_t Price, int64_t Qty)
{
	if (orderBidBook[TradePairID].count(Price))
		return false;

	COrderBook NewOrder(Price, Qty, Price*Qty, GetAdjustedTime());
	orderBidBook[TradePairID].insert(std::make_pair(Price, NewOrder));
	return true;
}

bool COrderBookManager::InsertNewAskOrder(int TradePairID, uint64_t Price, int64_t Qty)
{
	if (orderAskBook[TradePairID].count(Price))
		return false;

	COrderBook NewOrder(Price, Qty, Price*Qty, GetAdjustedTime());
	orderAskBook[TradePairID].insert(std::make_pair(Price, NewOrder));
	return true;
}

void COrderBookManager::AdjustBidQuantity(int TradePairID, uint64_t Price, int64_t Qty)
{
	if (!orderBidBook.count(TradePairID))
	{
		//need to sync with seed server or check node task
	}

	if (InsertNewBidOrder(TradePairID, Price, Qty))
		return;

	int finalQty = orderBidBook[TradePairID][Price].nQuantity + Qty;
	if (finalQty < 0)
	{
		//some check need to be done here
		finalQty = 0;
	}

	orderBidBook[TradePairID][Price].nQuantity = finalQty;
}

void COrderBookManager::AdjustAskQuantity(int TradePairID, uint64_t Price, int64_t Qty)
{
	if (!orderAskBook.count(TradePairID))
	{
		//need to sync with seed server or check node task
	}

	if (InsertNewAskOrder(TradePairID, Price, Qty))
		return;

	int finalQty = orderAskBook[TradePairID][Price].nQuantity + Qty;
	if (finalQty < 0)
	{
		//some check need to be done here
		finalQty = 0;
	}

	orderAskBook[TradePairID][Price].nQuantity = finalQty;
}

void COrderBookManager::UpdateBidOrder(int TradePairID, uint64_t Price, uint64_t Qty)
{
	if (!orderBidBook.count(TradePairID))
	{
		//need to sync with seed server or check node task
	}

	if (InsertNewBidOrder(TradePairID, Price, Qty))
		return;

	orderBidBook[TradePairID][Price].nQuantity = Qty;
}

void COrderBookManager::UpdateAskOrder(int TradePairID, uint64_t Price, uint64_t Qty)
{
	if (!orderAskBook.count(TradePairID))
	{
		//need to sync with seed server or check node task
	}

	if (InsertNewAskOrder(TradePairID, Price, Qty))
		return;

	orderAskBook[TradePairID][Price].nQuantity = Qty;
}

void ::COrderBookManager::CheckForTradePossibility(int TradePairID)
{
	std::map<int, PriceOrderBook>::iterator bidit = orderBidBook.find(TradePairID);
	std::map<int, PriceOrderBook>::iterator askit = orderAskBook.find(TradePairID);

	if (bidit != orderBidBook.end() && askit != orderAskBook.end())
	{

	}
}