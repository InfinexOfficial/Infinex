// Copyright (c) 2014-2017 The Dash Core developers
// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "validation.h"
#include "messagesigner.h"
#include "net_processing.h"
#include "spork.h"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

class CSporkMessage;
class CSporkManager;

CSporkManager sporkManager;

std::map<uint256, CSporkMessage> mapSporks;

void CSporkManager::ProcessSpork(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{
	if (fLiteMode) return; // disable all Infinex specific functionality

	if (strCommand == NetMsgType::SPORK) {

		CSporkMessage Spork;
		vRecv >> Spork;

		uint256 hash = Spork.GetHash();

		std::string strLogMsg;
		{
			LOCK(cs_main);
			pfrom->setAskFor.erase(hash);
			if (!chainActive.Tip()) return;
			strLogMsg = strprintf("SPORK -- hash: %s id: %d numeric value: %d text value: %s bestHeight: %d peer=%d", hash.ToString(), Spork.nSporkID, Spork.nNumericValue, Spork.nTextValue, chainActive.Height(), pfrom->id);
		}

		if (mapSporksActive.count(Spork.nSporkID)) {
			if (mapSporksActive[Spork.nSporkID].nTimeSigned >= Spork.nTimeSigned) {
				LogPrint("spork", "%s seen\n", strLogMsg);
				return;
			}
			else {
				LogPrintf("%s updated\n", strLogMsg);
			}
		}
		else {
			LogPrintf("%s new\n", strLogMsg);
		}

		if (!Spork.CheckSignature()) {
			LogPrintf("CSporkManager::ProcessSpork -- invalid signature\n");
			Misbehaving(pfrom->GetId(), 100);
			return;
		}

		mapSporks[hash] = Spork;
		mapSporksActive[Spork.nSporkID] = Spork;
		Spork.Relay(connman);
		
		int sporkType = GetSporkType(Spork.nSporkID);
		if (sporkType == SPORK_TEXT_TYPE) {
			ExecuteTextSpork(Spork.nSporkID, Spork.nTextValue);
		}
		else if (sporkType == SPORK_NUMERIC_TYPE || sporkType == SPORK_BOOL_TYPE || sporkType == SPORK_TIME_TYPE) {
			ExecuteNumericSpork(Spork.nSporkID, Spork.nNumericValue);
		}
	}
	else if (strCommand == NetMsgType::GETSPORKS) {

		std::map<int, CSporkMessage>::iterator it = mapSporksActive.begin();

		while (it != mapSporksActive.end()) {
			connman.PushMessage(pfrom, NetMsgType::SPORK, it->second);
			it++;
		}
	}
}

void CSporkManager::ExecuteNumericSpork(int nSporkID, int nValue)
{
	//correct fork via spork technology
	if (nSporkID == SPORK_RECONSIDER_BLOCKS && nValue > 0) {
		// allow to reprocess 24h of blocks max, which should be enough to resolve any issues
		int64_t nMaxBlocks = 960;
		// this potentially can be a heavy operation, so only allow this to be executed once per 10 minutes
		int64_t nTimeout = 10 * 60;

		static int64_t nTimeExecuted = 0; // i.e. it was never executed before

		if (GetTime() - nTimeExecuted < nTimeout) {
			LogPrint("spork", "CSporkManager::ExecuteSpork -- ERROR: Trying to reconsider blocks, too soon - %d/%d\n", GetTime() - nTimeExecuted, nTimeout);
			return;
		}

		if (nValue > nMaxBlocks) {
			LogPrintf("CSporkManager::ExecuteSpork -- ERROR: Trying to reconsider too many blocks %d/%d\n", nValue, nMaxBlocks);
			return;
		}


		LogPrintf("CSporkManager::ExecuteSpork -- Reconsider Last %d Blocks\n", nValue);

		ReprocessBlocks(nValue);
		nTimeExecuted = GetTime();
	}
}

void CSporkManager::ExecuteTextSpork(int nSporkID, std::string nValue)
{
	if (nSporkID == SPORK_BEST_BLOCK_HASH) 
	{
		if (nValue == "")
			return;

		if(!IsBestBlockHashFound(true))
		{
			ReprocessBlocks(960);
		}
	}
	else if (nSporkID == SPORK_MINIMUM_CHAIN_WORK)
	{
		if (nValue == "")
			return;

		!IsExceedMinimumChainWork(true);
	}
}

bool CSporkManager::UpdateNumericSpork(int nSporkID, int64_t nValue, CConnman& connman)
{
	CSporkMessage spork = CSporkMessage(nSporkID, nValue, "", GetAdjustedTime());

	if (!spork.Sign(strMasterPrivKey))
		return false;

	spork.Relay(connman);
	mapSporks[spork.GetHash()] = spork;
	mapSporksActive[nSporkID] = spork;
	return true;
}

bool CSporkManager::UpdateTextSpork(int nSporkID, std::string nValue, CConnman& connman)
{
	CSporkMessage spork = CSporkMessage(nSporkID, 0, nValue, GetAdjustedTime());

	if (!spork.Sign(strMasterPrivKey))
		return false;

	spork.Relay(connman);
	mapSporks[spork.GetHash()] = spork;
	mapSporksActive[nSporkID] = spork;
	return true;
}

bool CSporkManager::IsSporkActive(int nSporkID)
{
	int sporkType = GetSporkType(nSporkID);
	
	if (sporkType == SPORK_TEXT_TYPE || sporkType == SPORK_NUMERIC_TYPE)
		return true;

	if(sporkType == SPORK_BOOL_TYPE)
	{		
		if(GetNumericSporkValue(nSporkID) == SPORK_ENABLED)
			return true;
		return false;
	}

	if (GetSporkType(nSporkID) == SPORK_TIME_TYPE)
	{		
		return GetNumericSporkValue(nSporkID) > GetAdjustedTime();
	}

	return false;
}

int64_t CSporkManager::GetNumericSporkValue(int nSporkID)
{
	if (GetSporkType(nSporkID) != SPORK_NUMERIC_TYPE && GetSporkType(nSporkID) != SPORK_BOOL_TYPE && GetSporkType(nSporkID) != SPORK_TIME_TYPE)
	{
		LogPrint("spork", "CSporkManager::GetTextSporkValue -- Spork ID %d is not a numeric spork \n", nSporkID);
		return -1;
	}	

	if (mapSporksActive.count(nSporkID))
		return mapSporksActive[nSporkID].nNumericValue;

	switch (nSporkID)
	{
		case SPORK_INSTANTSEND_ENABLED:
			return SPORK_INSTANTSEND_ENABLED_VALUE;
		case SPORK_INSTANTSEND_BLOCK_FILTERING:
			return SPORK_INSTANTSEND_BLOCK_FILTERING_VALUE;
		case SPORK_INSTANTSEND_MAX_VALUE:
			return SPORK_INSTANTSEND_MAX_VALUE_VALUE;
		case SPORK_MASTERNODE_PAYMENT_ENFORCEMENT:
			return SPORK_MASTERNODE_PAYMENT_ENFORCEMENT_VALUE;
		case SPORK_RECONSIDER_BLOCKS:
			return SPORK_RECONSIDER_BLOCKS_VALUE;
		case SPORK_REQUIRE_SENTINEL_FLAG:
			return SPORK_REQUIRE_SENTINEL_FLAG_VALUE;
		default:
			LogPrint("spork", "CSporkManager::GetNumericSporkValue -- Unknown Spork ID %d\n", nSporkID);
			return -1;
	}
}

std::string CSporkManager::GetTextSporkValue(int nSporkID)
{
	if (GetSporkType(nSporkID) != SPORK_TEXT_TYPE) 
	{
		LogPrint("spork", "CSporkManager::GetTextSporkValue -- Spork ID %d is not a text spork \n", nSporkID);
		return "";
	}

	if (mapSporksActive.count(nSporkID))
		return mapSporksActive[nSporkID].nTextValue;

	switch (nSporkID)
	{
		case SPORK_MINIMUM_CHAIN_WORK:
			return SPORK_MINIMUM_CHAIN_WORK_VALUE;
		case SPORK_BEST_BLOCK_HASH:
			return SPORK_BEST_BLOCK_HASH_VALUE; 
		default:
			LogPrint("spork", "CSporkManager::GetTextSporkValue -- Unknown Spork ID %d\n", nSporkID);
			return "";
	}
}

int CSporkManager::GetSporkIDByName(std::string strName)
{
	if (strName == "SPORK_BEST_BLOCK_HASH") return SPORK_BEST_BLOCK_HASH;
	if (strName == "SPORK_MINIMUM_CHAIN_WORK") return SPORK_MINIMUM_CHAIN_WORK;
    if (strName == "SPORK_INSTANTSEND_ENABLED") return SPORK_INSTANTSEND_ENABLED;
    if (strName == "SPORK_INSTANTSEND_BLOCK_FILTERING") return SPORK_INSTANTSEND_BLOCK_FILTERING;
    if (strName == "SPORK_INSTANTSEND_MAX_VALUE") return SPORK_INSTANTSEND_MAX_VALUE;
    if (strName == "SPORK_MASTERNODE_PAYMENT_ENFORCEMENT") return SPORK_MASTERNODE_PAYMENT_ENFORCEMENT;
    if (strName == "SPORK_RECONSIDER_BLOCKS") return SPORK_RECONSIDER_BLOCKS;
    if (strName == "SPORK_REQUIRE_SENTINEL_FLAG") return SPORK_REQUIRE_SENTINEL_FLAG;
    LogPrint("spork", "CSporkManager::GetSporkIDByName -- Unknown Spork name '%s'\n", strName);
    return -1;
}

std::string CSporkManager::GetSporkNameByID(int nSporkID)
{
	switch (nSporkID) {
	case SPORK_BEST_BLOCK_HASH: return "SPORK_BEST_BLOCK_HASH";
	case SPORK_MINIMUM_CHAIN_WORK: return "SPORK_MINIMUM_CHAIN_WORK";
	case SPORK_INSTANTSEND_ENABLED: return "SPORK_INSTANTSEND_ENABLED";
	case SPORK_INSTANTSEND_BLOCK_FILTERING: return "SPORK_INSTANTSEND_BLOCK_FILTERING";
	case SPORK_INSTANTSEND_MAX_VALUE: return "SPORK_INSTANTSEND_MAX_VALUE";
	case SPORK_MASTERNODE_PAYMENT_ENFORCEMENT: return "SPORK_MASTERNODE_PAYMENT_ENFORCEMENT";
	case SPORK_RECONSIDER_BLOCKS: return "SPORK_RECONSIDER_BLOCKS";
	case SPORK_REQUIRE_SENTINEL_FLAG: return "SPORK_REQUIRE_SENTINEL_FLAG";
	default:
		LogPrint("spork", "CSporkManager::GetSporkNameByID -- Unknown Spork ID %d\n", nSporkID);
		return "Unknown";
	}
}

int CSporkManager::GetSporkType(int nSporkID)
{
	switch (nSporkID)
	{
	case SPORK_BEST_BLOCK_HASH:return SPORK_TEXT_TYPE;
	case SPORK_MINIMUM_CHAIN_WORK:return SPORK_TEXT_TYPE;
	case SPORK_INSTANTSEND_ENABLED: return SPORK_BOOL_TYPE;
	case SPORK_INSTANTSEND_BLOCK_FILTERING: return SPORK_BOOL_TYPE;
	case SPORK_INSTANTSEND_MAX_VALUE: return SPORK_NUMERIC_TYPE;
	case SPORK_MASTERNODE_PAYMENT_ENFORCEMENT: return SPORK_BOOL_TYPE;
	case SPORK_RECONSIDER_BLOCKS: return SPORK_NUMERIC_TYPE;
	case SPORK_REQUIRE_SENTINEL_FLAG: return SPORK_BOOL_TYPE;
	default:
		LogPrint("spork", "CSporkManager::GetSporkType -- Unknown Spork Type %d\n", nSporkID);
		return SPORK_UNKNOWN_TYPE;
	}
}

bool CSporkManager::SetPrivKey(std::string strPrivKey)
{
	CSporkMessage spork;

	spork.Sign(strPrivKey);

	if (spork.CheckSignature()) {
		// Test signing successful, proceed
		LogPrintf("CSporkManager::SetPrivKey -- Successfully initialized as spork signer\n");
		strMasterPrivKey = strPrivKey;
		return true;
	}
	else {
		return false;
	}
}

bool CSporkMessage::Sign(std::string strSignKey)
{
	CKey key;
	CPubKey pubkey;
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nSporkID) + boost::lexical_cast<std::string>(nNumericValue) + boost::lexical_cast<std::string>(nTimeSigned);

	if (!CMessageSigner::GetKeysFromSecret(strSignKey, key, pubkey)) {
		LogPrintf("CSporkMessage::Sign -- GetKeysFromSecret() failed, invalid spork key %s\n", strSignKey);
		return false;
	}

	if (!CMessageSigner::SignMessage(strMessage, vchSig, key)) {
		LogPrintf("CSporkMessage::Sign -- SignMessage() failed\n");
		return false;
	}

	if (!CMessageSigner::VerifyMessage(pubkey, vchSig, strMessage, strError)) {
		LogPrintf("CSporkMessage::Sign -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CSporkMessage::CheckSignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nSporkID) + boost::lexical_cast<std::string>(nNumericValue) + boost::lexical_cast<std::string>(nTimeSigned);
	CPubKey pubkey(ParseHex(Params().SporkPubKey()));

	if (!CMessageSigner::VerifyMessage(pubkey, vchSig, strMessage, strError)) {
		LogPrintf("CSporkMessage::CheckSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

void CSporkMessage::Relay(CConnman& connman)
{
	CInv inv(MSG_SPORK, GetHash());
	connman.RelayInv(inv);
}