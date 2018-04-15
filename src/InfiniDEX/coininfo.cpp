// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "coininfo.h"
#include "messagesigner.h"
#include "net_processing.h"
#include <boost/lexical_cast.hpp>

class CCoinInfo;
class CCoinInfoManager;
class CCoinInfoSync;

std::map<int, std::shared_ptr<CCoinInfo>> mapCompleteCoinInfoWithID;
std::map<std::string, std::shared_ptr<CCoinInfo>> mapCompleteCoinInfoWithSymbol;
CCoinInfoManager coinInfoManager;
CCoinInfoSync coinInfoSync;

void CCoinInfoManager::ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{
	if (strCommand == NetMsgType::DEXCOININFO)
	{
		CCoinInfo coinInfo;
		vRecv >> coinInfo;

		if (!coinInfo.VerifySignature()) {
			LogPrintf("CCoinInfoManager::ProcessMessage -- invalid signature\n");
			Misbehaving(pfrom->GetId(), 100);
			return;
		}

		std::shared_ptr<CCoinInfo> CoinInfo = std::make_shared<CCoinInfo>(coinInfo);
		InputCoinInfo(CoinInfo);
	}
	else if (strCommand == NetMsgType::DEXCOMPLETECOININFO)
	{
		CCoinInfoSync completeCoinInfo;
		vRecv >> completeCoinInfo;
		for (int i = 0; i < completeCoinInfo.CompleteCoinInfo.size(); i++)
		{
			if (completeCoinInfo.CompleteCoinInfo[i].VerifySignature())
			{
				std::shared_ptr<CCoinInfo> info = std::make_shared<CCoinInfo>(completeCoinInfo.CompleteCoinInfo[i]);
				InputCoinInfo(info);
			}
		}
	}
	else if (strCommand == NetMsgType::DEXGETCOININFO)
	{
		coinInfoSync.Relay(pfrom, connman);
	}
}

void CCoinInfoSync::Relay(CNode* node, CConnman& connman)
{
	connman.PushMessage(node, NetMsgType::DEXCOININFO, *this);
}

void CCoinInfoManager::AddCoinInfo(CCoinInfo CoinInfo)
{
	std::shared_ptr<CCoinInfo> temp = std::make_shared<CCoinInfo>(CoinInfo);
	InputCoinInfo(temp);
}

void CCoinInfoManager::InputCoinInfo(const std::shared_ptr<CCoinInfo>& CoinInfo)
{
	if (!mapCompleteCoinInfoWithID.count(CoinInfo->nCoinInfoID))
		mapCompleteCoinInfoWithID.insert(std::make_pair(CoinInfo->nCoinInfoID, CoinInfo));
	else
	{
		auto& a = mapCompleteCoinInfoWithID[CoinInfo->nCoinInfoID];
		if (a->nLastUpdate < CoinInfo->nLastUpdate)
			*a = *CoinInfo;
	}

	if (!mapCompleteCoinInfoWithSymbol.count(CoinInfo->nSymbol))
		mapCompleteCoinInfoWithSymbol.insert(std::make_pair(CoinInfo->nSymbol, CoinInfo));
	else
	{
		auto& a = mapCompleteCoinInfoWithSymbol[CoinInfo->nSymbol];
		if (a->nLastUpdate < CoinInfo->nLastUpdate)
			*a = *CoinInfo;
	}
}

bool CCoinInfo::VerifySignature()
{	
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nCoinInfoID) + nName + nSymbol + nLogoURL + boost::lexical_cast<std::string>(nBlockTime) 
		+ boost::lexical_cast<std::string>(nBlockHeight) + nWalletVersion + boost::lexical_cast<std::string>(nWalletActive) + nWalletStatus + boost::lexical_cast<std::string>(nLastUpdate);
	CPubKey pubkey(ParseHex(DEXKey));
	if (!CMessageSigner::VerifyMessage(pubkey, vchSig, strMessage, strError)) {
		LogPrintf("CCoinInfo::VerifySignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}
	return true;
}

bool CCoinInfo::Sign()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nCoinInfoID) + nName + nSymbol + nLogoURL + boost::lexical_cast<std::string>(nBlockTime)
		+ boost::lexical_cast<std::string>(nBlockHeight) + nWalletVersion + boost::lexical_cast<std::string>(nWalletActive) + nWalletStatus + boost::lexical_cast<std::string>(nLastUpdate);
	if (!CMessageSigner::SignMessage(strMessage, vchSig, dexMasterPrivKey))
	{
		LogPrintf("CCoinInfo::Sign -- SignMessage() failed\n");
		return false;
	}
	CPubKey pubkey(ParseHex(DEXKey));
	if (!CMessageSigner::VerifyMessage(pubkey, vchSig, strMessage, strError))
	{
		LogPrintf("CCoinInfo::Sign -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}
	return true;
}

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