// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "orderbook.h"
#include "messagesigner.h"
#include "net_processing.h"

class COrderBook;
class COrderBookManager;

std::map<int, PriceOrderBook> orderBidBook; //trade pair and bid data
std::map<int, PriceOrderBook> orderAskBook; //trade pair and ask data
COrderBookManager orderBookManager;

void COrderBookManager::AddToAsk(int TradePairID, uint64_t OrderPrice, uint64_t Quantity)
{
    std::map<int, PriceOrderBook>::iterator it = orderAskBook.find(TradePairID);
    if (it != orderAskBook.end()) {
        PriceOrderBook::iterator it2 = it->second.find(OrderPrice);
        if (it2 != it->second.end()) {
            it2->second.nQuantity += Quantity;
            it2->second.nAmount = OrderPrice * it2->second.nQuantity;
            it2->second.LastUpdateTime = GetAdjustedTime();
        } else {
            orderAskBook[TradePairID][OrderPrice] = COrderBook(OrderPrice, Quantity, OrderPrice * Quantity, GetAdjustedTime());
        }
    } else {
		//node need to sync for updated trade pair before proceed as below
        PriceOrderBook NewTradePair;
        NewTradePair.insert(std::make_pair(OrderPrice, COrderBook(OrderPrice, Quantity, OrderPrice * Quantity, GetAdjustedTime())));
        orderAskBook.insert(std::make_pair(TradePairID, NewTradePair));
    }
}

void COrderBookManager::AddToBid(int TradePairID, uint64_t OrderPrice, uint64_t Quantity)
{
    std::map<int, PriceOrderBook>::iterator it = orderBidBook.find(TradePairID);
    if (it != orderBidBook.end()) {
        PriceOrderBook::iterator it2 = it->second.find(OrderPrice);
        if (it2 != it->second.end()) {
            it2->second.nQuantity += Quantity;
            it2->second.nAmount = OrderPrice * it2->second.nQuantity;
        } else {
            orderBidBook[TradePairID][OrderPrice] = COrderBook(OrderPrice, Quantity, OrderPrice * Quantity, GetAdjustedTime());
        }
    } else {
		//node need to sync for updated trade pair before proceed as below
        PriceOrderBook NewTradePair;
        NewTradePair.insert(std::make_pair(OrderPrice, COrderBook(OrderPrice, Quantity, OrderPrice * Quantity, GetAdjustedTime())));
        orderBidBook.insert(std::make_pair(TradePairID, NewTradePair));
    }
}

void ::COrderBookManager::CheckForTradePossibility(int TradePairID)
{
	std::map<int, PriceOrderBook>::iterator bidit = orderBidBook.find(TradePairID);
	std::map<int, PriceOrderBook>::iterator askit = orderAskBook.find(TradePairID);

	if(bidit != orderBidBook.end() && askit != orderAskBook.end())
	{
		
	}
}