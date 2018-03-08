// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "messagesigner.h"
#include "net_processing.h"
#include "userbalance.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

std::map<int, mapUserBalance> mapCoinUserBalance; //coin ID & map user balance
CUserBalanceManager userBalanceManager;

bool CUserBalance::Sign(std::string strSignKey)
{
	CKey key;
	CPubKey pubkey;
	std::string strError = "";
	std::string strMessage = GetMessage();

	if (!CMessageSigner::GetKeysFromSecret(strSignKey, key, pubkey)) {
		LogPrintf("CUserBalance::Sign -- GetKeysFromSecret() failed, invalid spork key %s\n", strSignKey);
		return false;
	}

	if (!CMessageSigner::SignMessage(strMessage, vchSig, key)) {
		LogPrintf("CUserBalance::Sign -- SignMessage() failed\n");
		return false;
	}

	if (!CMessageSigner::VerifyMessage(pubkey, vchSig, strMessage, strError)) {
		LogPrintf("CUserBalance::Sign -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CUserBalance::CheckSignature()
{
	std::string strError = "";
	std::string strMessage = GetMessage();
	CPubKey pubkey(ParseHex(Params().SporkPubKey()));

	if (!CMessageSigner::VerifyMessage(pubkey, vchSig, strMessage, strError)) {
		LogPrintf("CUserBalance::CheckSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

std::string CUserBalance::GetMessage() 
{
    return nUserPubKey + boost::lexical_cast<std::string>(nCoinID) + boost::lexical_cast<std::string>(nAvailableBalance) + boost::lexical_cast<std::string>(nLastUpdateTime);
}

void CUserBalance::RelayTo(CNode* pnode, CConnman& connman)
{
    CInv inv(MSG_INFINIDEX_USERBALANCE, GetHash());
    connman.PushMessage(pnode, NetMsgType::DEXUSERBALANCE, inv);
}