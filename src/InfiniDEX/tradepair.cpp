// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "tradepair.h"

class CTradePair;
class CTradePairManager;

std::vector<CTradePair> vecCompleteTradePair;
CTradePairManager tradePairManager;

CTradePair CTradePairManager::GetTradePair(int TradePairID)
{
	for (int i = 0; i < vecCompleteTradePair.size(); i++)
	{
		if (vecCompleteTradePair[i].nTradePairID == TradePairID)
			return vecCompleteTradePair[i];
	}
	return CTradePair();
}

int CTradePairManager::GetAskSideCoinInfoID(int TradePairID)
{
	for(int i = 0; i < vecCompleteTradePair.size(); i++)
	{
		if (vecCompleteTradePair[i].nTradePairID == TradePairID)
			return vecCompleteTradePair[i].nCoinInfoID2;
	}
	return 0;
}

int CTradePairManager::GetBidSideCoinInfoID(int TradePairID)
{
	for (int i = 0; i < vecCompleteTradePair.size(); i++)
	{
		if (vecCompleteTradePair[i].nTradePairID == TradePairID)
			return vecCompleteTradePair[i].nCoinInfoID1;
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

void CTradePairManager::GetTradeFee(int TradePairID, int &BuyFee, int &SellFee)
{
	std::vector<CTradePair>::iterator it = vecCompleteTradePair.begin();
	while (it != vecCompleteTradePair.end())
	{
		if (it->nTradePairID == TradePairID)
		{
			BuyFee = it->nBidTradeFee;
			SellFee = it->nAskTradeFee;
			return;
		}
		++it;
	}
}

int CTradePairManager::GetBidTradeFee(int TradePairID)
{
	std::vector<CTradePair>::iterator it = vecCompleteTradePair.begin();
	while (it != vecCompleteTradePair.end())
	{
		if (it->nTradePairID == TradePairID)
		{			
			return it->nBidTradeFee;
		}
		++it;
	}
	return 0;
}

int CTradePairManager::GetAskTradeFee(int TradePairID)
{
	std::vector<CTradePair>::iterator it = vecCompleteTradePair.begin();
	while (it != vecCompleteTradePair.end())
	{
		if (it->nTradePairID == TradePairID)
		{
			return it->nAskTradeFee;
		}
		++it;
	}
	return 0;
}