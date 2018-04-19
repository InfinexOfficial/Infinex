// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//to remove
#include "masternodeman.h"
#include "version.h"
#include "timedata.h"

#include "coininfo.h"
#include "messagesigner.h"
#include "net_processing.h"
#include "noderole.h"
#include "userconnection.h"
#include <boost/lexical_cast.hpp>

class CCoinInfo;
class CCoinInfoManager;

std::map<int, std::shared_ptr<CCoinInfo>> mapCompleteCoinInfoWithID;
std::map<std::string, std::shared_ptr<CCoinInfo>> mapCompleteCoinInfoWithSymbol;
std::vector<CCoinInfo> completeCoinInfo;
CCoinInfoManager coinInfoManager;

void CCoinInfoManager::ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{
	if (strCommand == NetMsgType::DEXCOININFO)
	{
		std::vector<CCoinInfo> incomingCoinInfo;
		vRecv >> incomingCoinInfo;

		for (auto coinInfo : incomingCoinInfo)
		{
			if (!coinInfo.VerifySignature()) {
				LogPrintf("CCoinInfoManager::ProcessMessage -- invalid signature\n");
				Misbehaving(pfrom->GetId(), 100);
				return;
			}
			InputCoinInfo(coinInfo);
		}
		BroadcastToConnectedNode(connman, incomingCoinInfo);
	}
	else if (strCommand == NetMsgType::DEXGETCOININFO)
	{
		std::string symbol;
		vRecv >> symbol;

		PushCoinInfoToNode(symbol, pfrom, connman);
	}
}

void CCoinInfoManager::BroadcastToConnectedNode(CConnman& connman, std::vector<CCoinInfo> coinInfo)
{
	for (auto a : mapMNConnection)
	{
		if (!a.second.first->fDisconnect)
			connman.PushMessage(a.second.first, NetMsgType::DEXCOININFO, coinInfo);
	}
	for (auto a : mapUserConnections)
	{
		for (auto b : a.second)
		{
			if (!b.first->fDisconnect)
				connman.PushMessage(b.first, NetMsgType::DEXCOININFO, coinInfo);
		}
	}
}

void CCoinInfoManager::PushCoinInfoToNode(std::string symbol, CNode* pnode, CConnman& connman)
{
	if (symbol == "")
		connman.PushMessage(pnode, NetMsgType::DEXCOININFO, completeCoinInfo);
	else
	{
		CCoinInfo temp;
		if (GetCoinInfoBySymbol(symbol, temp))
		{
			std::vector<CCoinInfo> vec;
			vec.push_back(temp);
			connman.PushMessage(pnode, NetMsgType::DEXCOININFO, vec);
		}
	}
}

void CCoinInfoManager::Broadcast()
{
	auto a = mnodeman.GetFullMasternodeMap();
	for(auto b: a)
	{
		if(b.second.nProtocolVersion >= INFINIDEX_MIN_VERSION)
		{
			std::cout<<"IP: " << b.second.addr.ToStringIPPort() << std::endl;
			CNode* pnode = g_connman->ConnectNode(CAddress(b.second.addr, NODE_NETWORK), NULL);
			g_connman->PushMessage(pnode, NetMsgType::DEXCOININFO, completeCoinInfo);
		}
	}
}

void CCoinInfo::Relay(CNode* node, CConnman& connman)
{	
	connman.PushMessage(node, NetMsgType::DEXCOININFO, *this);
}

bool CCoinInfoManager::AddCoinInfo(std::string coinID, std::string name, std::string symbol, std::string logoURL, std::string blockTime, std::string blockHeight, std::string walletVersion, std::string walletActive, std::string walletStatus)
{
	try
	{		
		int CoinID = boost::lexical_cast<int>(coinID);	
		int BlockTime = boost::lexical_cast<int>(blockTime);		
		int BlockHeight = boost::lexical_cast<int>(blockHeight);		
		bool WalletActive = boost::lexical_cast<bool>(walletActive);		
		CCoinInfo temp(CoinID, name, symbol, logoURL, BlockTime, BlockHeight, walletVersion, WalletActive, walletStatus, GetAdjustedTime());
		if (!temp.Sign())
			return false;
		InputCoinInfo(temp);
		return true;
	}	
	catch (boost::bad_lexical_cast& e)
	{
		return false;
	}
}

void CCoinInfoManager::InputCoinInfo(CCoinInfo CoinInfo)
{	
	if (!mapCompleteCoinInfoWithID.count(CoinInfo.nCoinInfoID))
	{
		mapCompleteCoinInfoWithID.insert(std::make_pair(CoinInfo.nCoinInfoID, std::make_shared<CCoinInfo>(CoinInfo)));
		completeCoinInfo.push_back(CoinInfo);
	}
	else
	{
		auto& a = mapCompleteCoinInfoWithID[CoinInfo.nCoinInfoID];
		if (a->nLastUpdate < CoinInfo.nLastUpdate)
		{
			std::shared_ptr<CCoinInfo> temp = std::make_shared<CCoinInfo>(CoinInfo);
			*a = *temp;
		}
	}

	if (!mapCompleteCoinInfoWithSymbol.count(CoinInfo.nSymbol))
		mapCompleteCoinInfoWithSymbol.insert(std::make_pair(CoinInfo.nSymbol, std::make_shared<CCoinInfo>(CoinInfo)));
	else
	{
		auto& a = mapCompleteCoinInfoWithSymbol[CoinInfo.nSymbol];
		if (a->nLastUpdate < CoinInfo.nLastUpdate)
		{
			std::shared_ptr<CCoinInfo> temp = std::make_shared<CCoinInfo>(CoinInfo);
			*a = *temp;
		}
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
	CKey key;
	CPubKey pubkey;
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nCoinInfoID) + nName + nSymbol + nLogoURL + boost::lexical_cast<std::string>(nBlockTime)
		+ boost::lexical_cast<std::string>(nBlockHeight) + nWalletVersion + boost::lexical_cast<std::string>(nWalletActive) + nWalletStatus + boost::lexical_cast<std::string>(nLastUpdate);
	if (!CMessageSigner::GetKeysFromSecret(dexMasterPrivKey, key, pubkey)) {
		LogPrintf("CCoinInfo::Sign -- GetKeysFromSecret() failed, invalid dex key %s\n", dexMasterPrivKey);
		return false;
	}
	if (!CMessageSigner::SignMessage(strMessage, vchSig, key))
	{
		LogPrintf("CCoinInfo::Sign -- SignMessage() failed\n");
		return false;
	}
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