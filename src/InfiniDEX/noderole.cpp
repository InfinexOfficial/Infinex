// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "noderole.h"
#include "messagesigner.h"
#include "net_processing.h"
#include <boost/lexical_cast.hpp>

class CNodeRole;

std::map<infinidex_node_role_enum, std::vector<CNodeRole>> mapGlobalNodeRoles;
std::map<infinidex_node_role_enum, std::vector<CNodeRole>> mapNodeRole;

bool CNodeRole::VerifySignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(NodeRoleID) + boost::lexical_cast<std::string>(TradePairID) + boost::lexical_cast<std::string>(CoinID)
		+ Char + boost::lexical_cast<std::string>(NodeRole) + NodeIP + NodePubKey + boost::lexical_cast<std::string>(IsValid)+ boost::lexical_cast<std::string>(ToReplaceNodeRoleID)
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
		+ Char + boost::lexical_cast<std::string>(NodeRole) + NodeIP + NodePubKey + boost::lexical_cast<std::string>(IsValid) + boost::lexical_cast<std::string>(ToReplaceNodeRoleID)
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