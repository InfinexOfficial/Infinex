// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "orderbook.h"
#include "messagesigner.h"
#include "net_processing.h"

class COrderBook;
class COrderBookManager;

COrderBookManager orderBookManager;

std::map<int, std::vector<COrderBook>> orderBidBook; //map of CoinID and bid data list
std::map<int, std::vector<COrderBook>> orderAskBook; //map of CoinID and ask data list

void COrderBookManager::AddToAsk(uint256 CoinID, uint64_t OrderPrice, uint64_t Quantity, uint64_t Amount)
{
    
}

void COrderBookManager::AddToBid(uint256 CoinID, uint64_t OrderPrice, uint64_t Quantity, uint64_t Amount)
{
    
}