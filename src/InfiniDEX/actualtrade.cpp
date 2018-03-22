// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "actualtrade.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

class CActualTrade;
class CActualTradeManager;

std::map<int, ActualTradeContainer> mapTradePairActualTradeContainer;
std::map<int, std::vector<CActualTrade>> mapTradePairToResolveTrade;

std::map<int, mapActualTrade> mapTradePairActualTrade;
std::map<int, std::pair<bool, std::vector<std::string>>> mapActualTradeChecker;
CActualTradeManager actualTradeManager;

std::string CActualTrade::GetHash()
{
	return "";
}

bool CActualTrade::Sign()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nTradePrice) +
		boost::lexical_cast<std::string>(nTradeQty) + boost::lexical_cast<std::string>(nTradeAmount) + nUserPubKey1 + nUserPubKey2 + boost::lexical_cast<std::string>(nFee1) +
		boost::lexical_cast<std::string>(nFee2) + nMasternodeInspector + boost::lexical_cast<std::string>(nTradeTime);
	return true;
}

bool CActualTrade::CheckSignature() 
{
	return true;
}

bool CActualTrade::InformActualTrade() 
{
	if (!Sign())
	{
		return false;
	}
	return true;
}

bool CActualTrade::InformConflictTrade(CNode* node)
{
	return true;
}

void CActualTradeManager::InputNewTradePair(int TradePairID, bool SecurityCheck)
{
	if (!mapTradePairActualTradeContainer.count(TradePairID))
	{
		pairSettingSecurity temp(std::make_pair(CActualTradeSetting(TradePairID, SecurityCheck), std::set<std::string>()));
		mapTradePairActualTradeContainer.insert(std::make_pair(TradePairID, std::make_pair(temp, mapActualTrade())));
	}
	if (!mapTradePairToResolveTrade.count(TradePairID))
	{
		mapTradePairToResolveTrade.insert(std::make_pair(TradePairID, std::vector<CActualTrade>()));
	}
}

bool CActualTradeManager::SetSecurityCheck(int TradePairID, bool SecurityCheck)
{
	if (!mapTradePairActualTradeContainer.count(TradePairID))
	{
		
	}

	if (mapTradePairActualTradeContainer[TradePairID].first.first.GetSecurityCheck() == SecurityCheck)
		return true;

	if (SecurityCheck)
	{
		for (int i = 0; i < mapTradePairActualTradeContainer[TradePairID].second.size(); i++)
		{
			bool NoConflict = true;
			for (int j = 0; j < mapTradePairActualTradeContainer[TradePairID].first.second.size(); j++)
			{
				if (mapTradePairActualTradeContainer[TradePairID].first.second.count(mapTradePairActualTradeContainer[TradePairID].second[i].nCurrentHash))
				{
					NoConflict = false;
					//to resolve duplicated trade
				}
			}
			if (NoConflict)
				mapTradePairActualTradeContainer[TradePairID].first.second.insert(mapTradePairActualTradeContainer[TradePairID].second[i].nCurrentHash);
		}
		mapTradePairActualTradeContainer[TradePairID].first.first = SecurityCheck;
	}
	else
	{
		mapTradePairActualTradeContainer[TradePairID].first.second.clear();
		mapTradePairActualTradeContainer[TradePairID].first.first = SecurityCheck;
	}

	return false;
}

bool CActualTradeManager::GetActualTrade(CNode* node, int ActualTradeID, int TradePairID)
{
	if (!mapTradePairActualTradeContainer.count(TradePairID))
	{
		//inform other node that we don't carry this trade pair
		return false;
	}

	if (!mapTradePairActualTradeContainer[TradePairID].second.count(ActualTradeID))
	{
		//inform other node that we don't have this trade data		
		return false;
	}

	//send this trade data to other node
	CActualTrade temp(mapTradePairActualTradeContainer[TradePairID].second[ActualTradeID]);
	
	return true;
}

bool CActualTradeManager::AddNewActualTrade(CActualTrade ActualTrade)
{
	if (!mapTradePairActualTradeContainer.count(ActualTrade.nTradePairID))
	{
		//its not possible to be here
		//if here, we would need to switch this node to passive & resync
		return false;
	}

	ActualTrade.nCurrentHash = ActualTrade.GetHash();
	bool SecurityCheck = mapTradePairActualTradeContainer[ActualTrade.nTradePairID].first;
	if (SecurityCheck)
	{
		if (!RunSecurityCheck(ActualTrade.nTradePairID, ActualTrade.nCurrentHash))
		{
			return false;
		}
		mapActualTradeChecker[ActualTrade.nTradePairID].second.push_back(ActualTrade.nCurrentHash);
	}
	ActualTrade.nActualTradeID = (++nLastActualTradeID);
	mapTradePairActualTrade[ActualTrade.nTradePairID].insert(std::make_pair(ActualTrade.nActualTradeID, ActualTrade));
	nLastHash = ActualTrade.nCurrentHash;
	ActualTrade.InformActualTrade();
	return true;
}

bool CActualTradeManager::AddNewActualTrade(CNode* node, CActualTrade ActualTrade)
{
	return true;
}

bool CActualTradeManager::RunSecurityCheck(int TradePairID, std::string Hash)
{
	for (int j = 0; j < mapActualTradeChecker[TradePairID].second.size(); j++)
	{
		if (mapActualTradeChecker[TradePairID].second[j] == Hash)
			return false;
	}
	return true;
}

std::vector<std::string> CActualTradeManager::FindDuplicateTrade(int TradePairID)
{
	std::vector<std::string> DuplicatedTrade;
	std::vector<std::string> HashContainer;
	for (int i = 0; i < mapTradePairActualTrade[TradePairID].size(); i++)
	{
		bool Conflict = false;
		for (int j = 0; j < HashContainer.size(); j++)
		{
			if (mapTradePairActualTrade[TradePairID][i].nCurrentHash == HashContainer[j])
			{
				DuplicatedTrade.push_back(HashContainer[j]);
				Conflict = true;
				break;
			}
		}
		if (!Conflict)
			HashContainer.push_back(mapTradePairActualTrade[TradePairID][i].nCurrentHash);
	}
	return DuplicatedTrade;
}