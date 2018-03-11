// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "actualtrade.h"
#include "orderbook.h"
#include "messagesigner.h"
#include "net_processing.h"

class CActualTrade;
class CActualTradeManager;

std::map<int, std::vector<CActualTrade>> mapActualTrade; //trade pair ID & actual trade list
CActualTradeManager actualTradeManager;

void CActualTradeManager::TradeMatchingEngine() 
{
    
}