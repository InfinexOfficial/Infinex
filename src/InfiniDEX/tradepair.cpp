// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "tradepair.h"

class CTradePair;
class CTradePairManager;

std::vector<CTradePair> vecCompleteTradePair;
CTradePairManager tradePairManager;

int CTradePairManager::GetAskSideCoinInfoID(int TradePairID)
{
	std::vector<CTradePair>::iterator it = vecCompleteTradePair.begin();
	while (it != vecCompleteTradePair.end())
	{
		if (it->nTradePairID == TradePairID)
			return it->nCoinInfoID2;
	}
	return 0;
}

int CTradePairManager::GetBidSideCoinInfoID(int TradePairID)
{
	std::vector<CTradePair>::iterator it = vecCompleteTradePair.begin();
	while (it != vecCompleteTradePair.end())
	{
		if (it->nTradePairID == TradePairID)
			return it->nCoinInfoID1;
	}
	return 0;
}

new_tradepair_enum CTradePairManager::ProcessNewTradePair(CTradePair TradePair)
{
	bool sameTradePairID = false;
	bool sameCoinID = false;
	bool sameSymbol = false;
	bool sameReverseCoinID = false;
	bool sameReverseSymbol = false;

	std::vector<CTradePair>::iterator it = vecCompleteTradePair.begin();
	while (it != vecCompleteTradePair.end())
	{
		if (sameTradePairID && sameCoinID && sameSymbol)
			return NEW_TRADEPAIR_EXIST;

		if (it->nTradePairID == TradePair.nTradePairID)
		{
			sameTradePairID = true;
		}
		else if (it->nCoinInfoID1 == TradePair.nCoinInfoID1 && it->nCoinInfoID2 == TradePair.nCoinInfoID2)
		{
			sameCoinID = true;
		}
		else if (it->nCoinInfoID1 == TradePair.nCoinInfoID2 && it->nCoinInfoID2 == TradePair.nCoinInfoID1)
		{
			sameReverseCoinID = true;
		}
		else if (it->nSymbol1 == TradePair.nSymbol1 && it->nSymbol2 == TradePair.nSymbol2)
		{
			sameSymbol = true;
		}
		else if (it->nSymbol1 == TradePair.nSymbol2 && it->nSymbol2 == TradePair.nSymbol1)
		{
			sameReverseSymbol = true;
		}
		++it;
	}

	vecCompleteTradePair.push_back(TradePair);
	return NEW_TRADEPAIR_ADDED;
}