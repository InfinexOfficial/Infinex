// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "orderbook.h"
#include "messagesigner.h"
#include "net_processing.h"

class COrderBook;
class COrderBookManager;

COrderBookManager orderBookManager;

extern std::map<int, PriceOrderBook> orderBidBook; //trade pair and bid data
extern std::map<int, PriceOrderBook> orderAskBook; //trade pair and ask data
extern COrderBookManager orderBookManager;

void COrderBookManager::AddToAsk(int CoinID, uint64_t OrderPrice, uint64_t Quantity, uint64_t Amount)
{
    std::map<int, PriceOrderBook>::iterator it = orderBidBook.find(CoinID);
    if(it != orderBidBook.end())
    {
        PriceOrderBook::iterator it2 = it->second.find(OrderPrice);
        if(it2 != it->second.end())
        {
        }
        else
        {            
        }
    }
}

void COrderBookManager::AddToBid(int CoinID, uint64_t OrderPrice, uint64_t Quantity, uint64_t Amount)
{
    std::map<int, PriceOrderBook>::iterator it = orderAskBook.find(CoinID);
    if(it != orderAskBook.end())
    {
        PriceOrderBook::iterator it2 = it->second.find(OrderPrice);
        if(it2 != it->second.end())
        {            
        }
        else
        {            
        }
    }
}