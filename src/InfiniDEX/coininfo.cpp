// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "stdafx.h"
#include "coininfo.h"

class CCoinInfo;
class CCoinInfoManager;

std::map<int, std::shared_ptr<CCoinInfo>> mapCompleteCoinInfoWithID;
std::map<std::string, std::shared_ptr<CCoinInfo>> mapCompleteCoinInfoWithSymbol;
CCoinInfoManager coinInfoManager;

bool CCoinInfoManager::IsCoinInCompleteListByCoinID(int CoinID)
{
	return mapCompleteCoinInfoWithID.count(CoinID);
}

bool CCoinInfoManager::IsCoinInCompleteListBySymbol(std::string Symbol)
{
	return mapCompleteCoinInfoWithSymbol.count(Symbol);
}

bool CCoinInfoManager::GetCoinInfoByCoinID(int CoinID, CCoinInfo &CoinInfo)
{
	if (!IsCoinInCompleteListByCoinID(CoinID))
		return false;

	CoinInfo = *mapCompleteCoinInfoWithID[CoinID];
	return true;
}

bool CCoinInfoManager::GetCoinInfoBySymbol(std::string Symbol, CCoinInfo &CoinInfo)
{
	if (!IsCoinInCompleteListBySymbol(Symbol))
		return false;

	CoinInfo = *mapCompleteCoinInfoWithSymbol[Symbol];
	return true;
}

void CCoinInfoManager::UpdateCoinInfo(CCoinInfo CoinInfo)
{
	if (IsCoinInCompleteListByCoinID(CoinInfo.nCoinInfoID))
	{
		*mapCompleteCoinInfoWithID[CoinInfo.nCoinInfoID] = CoinInfo;
	}
	else
	{
		mapCompleteCoinInfoWithID.insert(std::make_pair(CoinInfo.nCoinInfoID, &CoinInfo));
		mapCompleteCoinInfoWithSymbol.insert(std::make_pair(CoinInfo.nSymbol, &CoinInfo));
	}
}