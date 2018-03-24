// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "tradepair.h"

class CTradePair;
class CTradePairManager;

std::map<int, CTradePair> mapCompleteTradePair;
CTradePairManager tradePairManager;

bool CTradePair::Verify()
{
	return true;
}

bool RelayTo(CNode* node, CConnman& connman)
{
	return true;
}

void CTradePairManager::ProcessMessage(CNode* node, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{

}

void CTradePairManager::SendCompleteTradePairs(CNode* node, CConnman& connman)
{
	std::map<int, CTradePair>::iterator it = mapCompleteTradePair.begin();
	while (it != mapCompleteTradePair.end())
	{
		SendTradePair(it->second, node, connman);
		++it;
	}
}

void CTradePairManager::SendTradePair(CTradePair TradePair, CNode* node, CConnman& connman)
{

}

CTradePair CTradePairManager::GetTradePair(int TradePairID)
{
	if (IsValidTradePairID(TradePairID))
		return mapCompleteTradePair[TradePairID];

	return CTradePair();
}

int CTradePairManager::GetAskSideCoinID(int TradePairID)
{
	if (IsValidTradePairID(TradePairID))
		return mapCompleteTradePair[TradePairID].nCoinID2;
	return 0;
}

int CTradePairManager::GetBidSideCoinID(int TradePairID)
{
	if (IsValidTradePairID(TradePairID))
		return mapCompleteTradePair[TradePairID].nCoinID1;
	return 0;
}

tradepair_enum CTradePairManager::ProcessTradePair(CTradePair TradePair)
{
	if (!TradePair.Verify())
		return TRADEPAIR_INVALID;

	CTradePair temp = GetTradePair(TradePair.nTradePairID);
	if (temp.nTradePairID == TradePair.nTradePairID)
	{
		if (temp.nHash == TradePair.nHash)
			return TRADEPAIR_EXIST;
		else
		{
			return TRADEPAIR_UPDATED;
		}
	}

	std::map<int, CTradePair>::iterator it = mapCompleteTradePair.begin();
	while (it != mapCompleteTradePair.end())
	{
		if (it->second.nCoinID1 == TradePair.nCoinID1 && it->second.nCoinID2 == TradePair.nCoinID2)
		{
			return TRADEPAIR_COINID_EXIST;
		}
		else if (it->second.nCoinID1 == TradePair.nCoinID2 && it->second.nCoinID2 == TradePair.nCoinID1)
		{
			return TRADEPAIR_REVERSE_COINID_EXIST;
		}
		else if (it->second.nSymbol1 == TradePair.nSymbol1 && it->second.nSymbol2 == TradePair.nSymbol2)
		{
			return TRADEPAIR_SYMBOL_EXIST;
		}
		else if (it->second.nSymbol1 == TradePair.nSymbol2 && it->second.nSymbol2 == TradePair.nSymbol1)
		{
			return TRADEPAIR_REVERSE_SYMBOL_EXIST;
		}
		++it;
	}

	mapCompleteTradePair.insert(std::make_pair(TradePair.nTradePairID, TradePair));
	return TRADEPAIR_ADDED;
}

void CTradePairManager::GetTradeFee(int TradePairID, int &BuyFee, int &SellFee)
{	
	if (IsValidTradePairID(TradePairID))
	{
		BuyFee = mapCompleteTradePair[TradePairID].nBidTradeFee;
		SellFee = mapCompleteTradePair[TradePairID].nAskTradeFee;
		return;
	}
	BuyFee = 0;
	SellFee = 0;
}

int CTradePairManager::GetBidTradeFee(int TradePairID)
{
	if (IsValidTradePairID(TradePairID))
	{
		return mapCompleteTradePair[TradePairID].nBidTradeFee;
	}
	return 0;
}

int CTradePairManager::GetAskTradeFee(int TradePairID)
{
	if (IsValidTradePairID(TradePairID))
	{
		return mapCompleteTradePair[TradePairID].nAskTradeFee;
	}
	return 0;
}

bool CTradePairManager::IsValidTradePairID(int TradePairID)
{
	return mapCompleteTradePair.count(TradePairID);
}

std::string CTradePairManager::GetTradePairStatus(int TradePairID)
{
	if (IsValidTradePairID(TradePairID))
		return mapCompleteTradePair[TradePairID].nStatus;
	return "";
}

bool CTradePairManager::IsTradeEnabled(int TradePairID)
{
	if (IsValidTradePairID(TradePairID))
		return mapCompleteTradePair[TradePairID].nTradeEnabled;
	return false;
}