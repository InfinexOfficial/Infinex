// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "masternodeman.h"
#include "noderole.h"
#include "messagesigner.h"
#include "net_processing.h"
#include "userconnection.h"
#include <boost/lexical_cast.hpp>

class CNodeRole;
class CNodeRoleManager;
class CPendingProcess;

std::map<int, mapGlobalNodeRolesByRole> mapGlobalNodeRolesByTradePairID;
std::map<char, mapGlobalNodeRolesByRole> mapGlobalNodeRolesByChar;
NodeRoleWithID mapGlobalNodeRoles;
std::map<int, NodeRoleWithID> mapLocalNodeRoles;
std::vector<CNodeRole> completeNodeRoles;
CNodeRoleManager nodeRoleManager;
std::string MNPubKey;
std::string DEXKey = "028afd3503f2aaa0898b853e1b28cdcb5fd422b5dc6426c92cf2b14c4b4ebeb969";
std::string dexMasterPrivKey;
CPendingProcess pendingProcessStatus;

void CNodeRoleManager::InitialInit()
{
	//to update
	auto a = mnodeman.GetFullMasternodeMap();
	int counter = 0;
	for (auto b : a)
	{
		if (b.second.nProtocolVersion >= INFINIDEX_MIN_VERSION)
		{
			if (++counter == 5)
				return;
			CNode* pnode = g_connman->ConnectNode(CAddress(b.second.addr, NODE_NETWORK), NULL);
			g_connman->PushMessage(pnode, NetMsgType::DEXINITIALSYNC);
		}
	}
}

void CNodeRole::Broadcast(CConnman& connman)
{
	for (auto a : mapMNConnection)
	{
		if (!a.second.first->fDisconnect)
			connman.PushMessage(a.second.first, NetMsgType::DEXNODEROLE, *this);
	}
	for (auto a : mapUserConnections)
	{
		for (auto b : a.second)
		{
			if (!b.first->fDisconnect)
				connman.PushMessage(b.first, NetMsgType::DEXNODEROLE, *this);
		}
	}
}

bool CNodeRole::VerifySignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(NodeRoleID) + boost::lexical_cast<std::string>(TradePairID) + boost::lexical_cast<std::string>(CoinID)
		+ Char + boost::lexical_cast<std::string>(NodeRole) + NodeIP + NodePubKey + boost::lexical_cast<std::string>(StartTime) + boost::lexical_cast<std::string>(EndTime)
		+ boost::lexical_cast<std::string>(IsActive) + boost::lexical_cast<std::string>(IsBackup) + boost::lexical_cast<std::string>(ToReplaceNodeRoleID)
		+ boost::lexical_cast<std::string>(LastUpdateTime);
	CPubKey pubkey(ParseHex(DEXKey));
	if (!CMessageSigner::VerifyMessage(pubkey, vchSig, strMessage, strError)) {
		LogPrintf("CNodeRole::VerifySignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}
	return true;
}

bool CNodeRole::DEXSign(std::string dexSignKey)
{
	CKey key;
	CPubKey pubkey;
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(NodeRoleID) + boost::lexical_cast<std::string>(TradePairID) + boost::lexical_cast<std::string>(CoinID)
		+ Char + boost::lexical_cast<std::string>(NodeRole) + NodeIP + NodePubKey + boost::lexical_cast<std::string>(StartTime) + boost::lexical_cast<std::string>(EndTime)
		+ boost::lexical_cast<std::string>(IsActive) + boost::lexical_cast<std::string>(IsBackup) + boost::lexical_cast<std::string>(ToReplaceNodeRoleID)
		+ boost::lexical_cast<std::string>(LastUpdateTime);
	if (!CMessageSigner::GetKeysFromSecret(dexSignKey, key, pubkey)) {
		LogPrintf("CNodeRole::DEXSign -- GetKeysFromSecret() failed, invalid DEX key %s\n", dexSignKey);
		return false;
	}
	if (!CMessageSigner::SignMessage(strMessage, vchSig, key)) {
		LogPrintf("CNodeRole::DEXSign -- SignMessage() failed\n");
		return false;
	}
	if (!CMessageSigner::VerifyMessage(pubkey, vchSig, strMessage, strError)) {
		LogPrintf("CNodeRole::DEXSign -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}
	return true;
}

bool CNodeRoleManager::SetDEXPrivKey(std::string dexPrivKey)
{
	CNodeRole role;
	role.DEXSign(dexPrivKey);
	if (role.VerifySignature()) {
		LogPrintf("CUserConnectionManager::SetDEXPrivKey -- Successfully initialized as dex signer\n");
		dexMasterPrivKey = dexPrivKey;
		return true;
	}
	else {
		return false;
	}
}

bool CNodeRoleManager::IsValidInChargeOfUserTrade(std::string MNPubkey, uint64_t time, int TradePairID)
{
	auto& a = mapGlobalNodeRolesByTradePairID[TradePairID];
	auto& b = a[INFINIDEX_TRADE_PROCESSOR];
	NodeRoleWithID::reverse_iterator it = b.rbegin();
	while (it != b.rend())
	{
		auto& c = it->second;
		if (c->NodePubKey == MNPubKey && c->StartTime <= time && (c->EndTime >= time) || c->EndTime == 0)
			return true;
	}
	return false;
}

bool CNodeRoleManager::IsValidInChargeOfUserBalance(std::string MNPubKey, uint64_t time, std::string UserPubKey)
{
	char Char = UserPubKey[2];
	auto& a = mapGlobalNodeRolesByChar[Char];
	auto& b = a[INFINIDEX_BALANCE_HANDLER];
	NodeRoleWithID::reverse_iterator it = b.rbegin();
	while (it != b.rend())
	{
		auto& c = it->second;
		if (c->NodePubKey == MNPubKey && c->StartTime <= time && (c->EndTime >= time) || c->EndTime == 0)
			return true;
	}
	return false;
}