// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "coininfo.h"
#include "tradepair.h"
#include "messagesigner.h"
#include "net_processing.h"
#include "noderole.h"
#include "nodesetup.h"
#include "userconnection.h"
#include <boost/lexical_cast.hpp>

class CTradePair;
class CTradePairManager;

std::map<int, CTradePair> mapTradePair;
std::vector<CTradePair> completeTradePair;
CTradePairManager tradePairManager;

void CTradePairManager::ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{
	if (strCommand == NetMsgType::DEXTRADEPAIRS)
	{
		std::vector<CTradePair> incomings;
		vRecv >> incomings;

		std::vector<CTradePair> toBroadcast;
		for (auto incoming : incomings)
		{
			if (!incoming.VerifySignature())
			{
				LogPrintf("CTradePairManager::ProcessMessage -- invalid signature\n");
				Misbehaving(pfrom->GetId(), 100);
				return;
			}
			if (InputTradePair(incoming) && !nodeSetup.TradePairSyncInProgress)
				toBroadcast.push_back(incoming);
		}
		if (toBroadcast.size() > 0)
			BroadcastToConnectedNode(connman, toBroadcast);

		if (nodeSetup.TradePairSyncInProgress)
		{
			bool stillSync = false;

			if (completeTradePair.size() < nodeSetup.TotalTradePairCount)
				stillSync = true;

			std::map<int, CTradePair>::reverse_iterator it = mapTradePair.rbegin();
			if (it == mapTradePair.rend())
				stillSync = true;
			else if (it->first < nodeSetup.LastTradePairID)
				stillSync = true;

			if (!stillSync)
				nodeSetup.TradePairSyncInProgress = false;
		}
	}
	else if (strCommand == NetMsgType::DEXTRADEPAIR)
	{
		CTradePair incoming;
		vRecv >> incoming;

		if (!incoming.VerifySignature())
		{
			LogPrintf("CTradePairManager::ProcessMessage -- invalid signature\n");
			Misbehaving(pfrom->GetId(), 100);
			return;

			if (InputTradePair(incoming))
				incoming.BroadcastToConnectedNode(connman);
		}
	}
}

void CTradePair::BroadcastToConnectedNode(CConnman& connman)
{
	for (auto a : mapMNConnection)
	{
		if (!a.second.first->fDisconnect)
			connman.PushMessage(a.second.first, NetMsgType::DEXTRADEPAIR, *this);
	}
	for (auto a : mapUserConnections)
	{
		for (auto b : a.second)
		{
			if (!b.first->fDisconnect)
				connman.PushMessage(b.first, NetMsgType::DEXTRADEPAIR, *this);
		}
	}
}

void CTradePairManager::BroadcastToConnectedNode(CConnman& connman, std::vector<CTradePair> tradePairs)
{
	for (auto a : mapMNConnection)
	{
		if (!a.second.first->fDisconnect)
			connman.PushMessage(a.second.first, NetMsgType::DEXTRADEPAIRS, tradePairs);
	}
	for (auto a : mapUserConnections)
	{
		for (auto b : a.second)
		{
			if (!b.first->fDisconnect)
				connman.PushMessage(b.first, NetMsgType::DEXTRADEPAIRS, tradePairs);
		}
	}
}

bool CTradePair::VerifySignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nTradePairID) + nName + boost::lexical_cast<std::string>(nCoinID1) + nSymbol1 + boost::lexical_cast<std::string>(nCoinID2)
		+ nSymbol2 + boost::lexical_cast<std::string>(nTradeEnabled) + boost::lexical_cast<std::string>(nMinimumTradeQuantity)+ boost::lexical_cast<std::string>(nMaximumTradeQuantity)
		+ boost::lexical_cast<std::string>(nMinimumTradeAmount) + boost::lexical_cast<std::string>(nMaximumTradeAmount) + boost::lexical_cast<std::string>(nBidTradeFee)
		+ boost::lexical_cast<std::string>(nAskTradeFee) + nStatus + boost::lexical_cast<std::string>(nLastUpdate);
	CPubKey pubkey(ParseHex(DEXKey));

	if (!CMessageSigner::VerifyMessage(pubkey, vchSig, strMessage, strError)) {
		LogPrintf("CTradePair::VerifySignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CTradePairManager::InputTradePair(CTradePair &tradePair)
{
	if (!mapTradePair.count(tradePair.nTradePairID))
		mapTradePair.insert(std::make_pair(tradePair.nTradePairID, tradePair));
	else if (tradePair.nLastUpdate > mapTradePair[tradePair.nTradePairID].nLastUpdate)
		mapTradePair[tradePair.nTradePairID] = tradePair;

	return true;
}

void CTradePairManager::SendCompleteTradePairs(CNode* node, CConnman& connman)
{
	connman.PushMessage(node, NetMsgType::DEXTRADEPAIR, completeTradePair);
}

void CTradePairManager::SendTradePair(CTradePair TradePair, CNode* node, CConnman& connman)
{
	std::vector<CTradePair> tradePairs;
	tradePairs.push_back(TradePair);
	connman.PushMessage(node, NetMsgType::DEXTRADEPAIR, tradePairs);
}

CTradePair CTradePairManager::GetTradePair(int TradePairID)
{
	if (IsValidTradePairID(TradePairID))
		return mapTradePair[TradePairID];

	return CTradePair();
}

int CTradePairManager::GetAskSideCoinID(int TradePairID)
{
	if (IsValidTradePairID(TradePairID))
		return mapTradePair[TradePairID].nCoinID2;
	return 0;
}

int CTradePairManager::GetBidSideCoinID(int TradePairID)
{
	if (IsValidTradePairID(TradePairID))
		return mapTradePair[TradePairID].nCoinID1;
	return 0;
}

tradepair_enum CTradePairManager::ProcessTradePair(CTradePair TradePair)
{
	if (!TradePair.VerifySignature())
		return TRADEPAIR_INVALID;

	CTradePair temp = GetTradePair(TradePair.nTradePairID);
	if (temp.nTradePairID == TradePair.nTradePairID)
	{
		if (TradePair.nLastUpdate > temp.nLastUpdate)
			temp = TradePair;
		return TRADEPAIR_UPDATED;
	}

	std::map<int, CTradePair>::iterator it = mapTradePair.begin();
	while (it != mapTradePair.end())
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

	mapTradePair.insert(std::make_pair(TradePair.nTradePairID, TradePair));
	return TRADEPAIR_ADDED;
}

void CTradePairManager::GetTradeFee(int TradePairID, int &BuyFee, int &SellFee)
{	
	if (IsValidTradePairID(TradePairID))
	{
		BuyFee = mapTradePair[TradePairID].nBidTradeFee;
		SellFee = mapTradePair[TradePairID].nAskTradeFee;
		return;
	}
	BuyFee = 0;
	SellFee = 0;
}

int CTradePairManager::GetBidTradeFee(int TradePairID)
{
	if (IsValidTradePairID(TradePairID))
	{
		return mapTradePair[TradePairID].nBidTradeFee;
	}
	return 0;
}

int CTradePairManager::GetAskTradeFee(int TradePairID)
{
	if (IsValidTradePairID(TradePairID))
	{
		return mapTradePair[TradePairID].nAskTradeFee;
	}
	return 0;
}

bool CTradePairManager::IsValidTradePairID(int TradePairID)
{
	return mapTradePair.count(TradePairID);
}

std::string CTradePairManager::GetTradePairStatus(int TradePairID)
{
	if (IsValidTradePairID(TradePairID))
		return mapTradePair[TradePairID].nStatus;
	return "";
}

bool CTradePairManager::IsTradeEnabled(int TradePairID)
{
	if (IsValidTradePairID(TradePairID))
		return mapTradePair[TradePairID].nTradeEnabled;
	return false;
}