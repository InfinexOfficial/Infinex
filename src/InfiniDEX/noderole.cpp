// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "noderole.h"
#include "messagesigner.h"
#include "net_processing.h"
#include <boost/lexical_cast.hpp>

class CNodeRole;

std::map<int, pairIPPubKeyNodeRole> mapTradePairGlobalRoleByIPPubKey;
std::map<int, pairNodeRoleByRole> mapTradePairGlobalRoleByRole;
std::map<int, std::vector<std::shared_ptr<CNodeRole>>> mapTradePairNodeRole;
std::map<int, std::shared_ptr<CNodeRole>> mapNodeRoleByID; 
std::map<int, std::shared_ptr<CNodeRole>> mapNodeRole;

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