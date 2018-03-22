// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "actualtrade.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

class CActualTrade;
class CActualTradeSetting;
class CActualTradeManager;

std::map<int, ActualTradeContainer> mapTradePairActualTradeContainer;
std::map<int, std::vector<CActualTrade>> mapTradePairToResolveTrade;
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

	if (mapTradePairActualTradeContainer[TradePairID].first.first.nSecurityCheck == SecurityCheck)
		return true;

	if (SecurityCheck)
	{
		for (auto &a : mapTradePairActualTradeContainer[TradePairID].second)
		{
			bool NoConflict = true;
			if (mapTradePairActualTradeContainer[TradePairID].first.second.count(a.second.nCurrentHash))
			{
				NoConflict = false;
				//to resolve duplicated trade
			}
			else
			{
				mapTradePairActualTradeContainer[TradePairID].first.second.insert(a.second.nCurrentHash);
			}
		}
		mapTradePairActualTradeContainer[TradePairID].first.first.nSecurityCheck = SecurityCheck;
	}
	else
	{
		mapTradePairActualTradeContainer[TradePairID].first.second.clear();
		mapTradePairActualTradeContainer[TradePairID].first.first.nSecurityCheck = SecurityCheck;
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
	bool SecurityCheck = mapTradePairActualTradeContainer[ActualTrade.nTradePairID].first.first.nSecurityCheck;
	if (SecurityCheck)
	{
		if (!RunSecurityCheck(ActualTrade.nTradePairID, ActualTrade.nCurrentHash))
		{
			return false;
		}
		mapTradePairActualTradeContainer[ActualTrade.nTradePairID].first.second.insert(ActualTrade.nCurrentHash);
	}
	ActualTrade.nActualTradeID = (++mapTradePairActualTradeContainer[ActualTrade.nTradePairID].first.first.nLastActualTradeID);
	mapTradePairActualTradeContainer[ActualTrade.nTradePairID].second.insert(std::make_pair(ActualTrade.nActualTradeID, ActualTrade));
	mapTradePairActualTradeContainer[ActualTrade.nTradePairID].first.second.insert(ActualTrade.nCurrentHash);
	mapTradePairActualTradeContainer[ActualTrade.nTradePairID].first.first.nLastHash = ActualTrade.nCurrentHash;
	ActualTrade.InformActualTrade();
	return true;
}

bool CActualTradeManager::AddNewActualTrade(CNode* node, CActualTrade ActualTrade)
{
	if (!mapTradePairActualTradeContainer.count(ActualTrade.nTradePairID))
	{
		//check whether current node is in charge of current trade pair
		if (true)
		{
			InputNewTradePair(ActualTrade.nTradePairID, true);
		}
		else
		{

		}
		return false;
	}

	mapActualTrade::iterator temp = mapTradePairActualTradeContainer[ActualTrade.nTradePairID].second.find(ActualTrade.nActualTradeID);
	if (temp != mapTradePairActualTradeContainer[ActualTrade.nTradePairID].second.end)
	{
		if (temp->second.nCurrentHash == ActualTrade.nCurrentHash)
			return true;
		else
		{
			//resync required to make sure network running the same data
		}
	}
	else if (mapTradePairActualTradeContainer[ActualTrade.nTradePairID].first.second.count(ActualTrade.nCurrentHash))
	{
		//resync required to make sure network running the same data
	}
	else
	{
		mapTradePairActualTradeContainer[ActualTrade.nTradePairID].second.insert(std::make_pair(ActualTrade.nActualTradeID, ActualTrade));
		mapTradePairActualTradeContainer[ActualTrade.nTradePairID].first.second.insert(ActualTrade.nCurrentHash);
		mapTradePairActualTradeContainer[ActualTrade.nTradePairID].first.first.nLastHash = ActualTrade.nCurrentHash;
	}
	return true;
}

bool CActualTradeManager::RunSecurityCheck(int TradePairID, std::string Hash)
{
	for(auto const& a: mapTradePairActualTradeContainer[TradePairID].first.second)
	{
		if (a == Hash)
			return false;
	}
	return true;
}

std::vector<std::string> CActualTradeManager::FindDuplicateTrade(int TradePairID)
{
	std::vector<std::string> DuplicatedTrade;
	std::vector<std::string> HashContainer;
	for (auto &a : mapTradePairActualTradeContainer[TradePairID].second)
	{
		bool Conflict = false;
		for (auto &b : HashContainer)
		{
			if (a.second.nCurrentHash == b)
			{
				DuplicatedTrade.push_back(b);
				Conflict = true;
				break;
			}
		}
		if (!Conflict)
			HashContainer.push_back(a.second.nCurrentHash);
	}
	return DuplicatedTrade;
}