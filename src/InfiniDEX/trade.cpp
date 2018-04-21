// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "activemasternode.h"
#include "messagesigner.h"
#include "trade.h"
#include "userconnection.h"
#include <boost/lexical_cast.hpp>

class CUserTrade;
class CUserTradeSetting;
class CActualTrade;

std::set<uint256> userTradesHash;
std::vector<CUserTrade> pendingProcessUserTrades;
std::vector<CCancelTrade> pendingProcessCancelTrades;
std::map<int, std::set<int>> mapCompletedUserTrade;
std::map<std::string, pULTIUTC> mapUserTrades;

std::map<int, mUTPIUTV> mapBidUserTradeByPrice;
std::map<int, mUTPIUTV> mapAskUserTradeByPrice;

std::map<int, CUserTradeSetting> mapUserTradeSetting;

std::map<std::string, mUTImAT> mapUserActualTrades;
std::map<int, std::set<int>> mapCompletedActualTradeID;
std::map<int, std::set<int>> mapApprovedActualTradeID;
std::map<int, mATIAT> mapActualTradeByActualTradeID;
std::map<int, mUTImAT> mapActualTradeByUserTradeID;
std::map<int, std::set<std::string>> mapActualTradeHash;

bool CUserTradeSetting::IsValidSubmissionTimeAndUpdate(uint64_t time)
{
	int diff = nLastPairTradeTime - time;
	if (diff > nMaxSubmissionTimeDiff)
		return false;

	nLastPairTradeTime = time;
	return true;
}

std::string CActualTrade::GetHash()
{
	return "";
}

bool CUserTrade::VerifyUserSignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nPrice) + boost::lexical_cast<std::string>(nQuantity)
		+ boost::lexical_cast<std::string>(nAmount) + boost::lexical_cast<std::string>(nIsBid)+ boost::lexical_cast<std::string>(nTradeFee) + nUserPubKey
		+ boost::lexical_cast<std::string>(nTimeSubmit) + nUserHash;
	CPubKey pubkey(ParseHex(nUserPubKey));

	if (!CMessageSigner::VerifyMessage(pubkey, userVchSig, strMessage, strError)) {
		LogPrintf("CUserTrade::VerifyUserSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CUserTrade::VerifyMNBalanceSignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nPrice) + boost::lexical_cast<std::string>(nQuantity)
		+ boost::lexical_cast<std::string>(nAmount) + boost::lexical_cast<std::string>(nIsBid)+ boost::lexical_cast<std::string>(nTradeFee) + nUserPubKey
		+ boost::lexical_cast<std::string>(nTimeSubmit) + nUserHash + boost::lexical_cast<std::string>(nUserTradeID) + nMNBalancePubKey
		+ boost::lexical_cast<std::string>(nBalanceQty) + boost::lexical_cast<std::string>(nBalanceAmount) + boost::lexical_cast<std::string>(nLastUpdate);
	CPubKey pubkey(ParseHex(nMNBalancePubKey));

	if (!CMessageSigner::VerifyMessage(pubkey, mnBalanceVchSig, strMessage, strError)) {
		LogPrintf("CUserTrade::VerifyUserSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CUserTrade::VerifyMNTradeSignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nPrice) + boost::lexical_cast<std::string>(nQuantity)
		+ boost::lexical_cast<std::string>(nAmount) + boost::lexical_cast<std::string>(nIsBid)+ boost::lexical_cast<std::string>(nTradeFee) + nUserPubKey
		+ boost::lexical_cast<std::string>(nTimeSubmit) + nUserHash + boost::lexical_cast<std::string>(nUserTradeID) + boost::lexical_cast<std::string>(nPairTradeID)
		+ nMNBalancePubKey + nMNTradePubKey	+ boost::lexical_cast<std::string>(nBalanceQty) + boost::lexical_cast<std::string>(nBalanceAmount) 
		+ boost::lexical_cast<std::string>(nLastUpdate);
	CPubKey pubkey(ParseHex(nMNTradePubKey));

	if (!CMessageSigner::VerifyMessage(pubkey, mnTradeVchSig, strMessage, strError)) {
		LogPrintf("CUserTrade::VerifyMNTradeSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CUserTrade::MNBalanceSign()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nPrice) + boost::lexical_cast<std::string>(nQuantity)
		+ boost::lexical_cast<std::string>(nAmount) + boost::lexical_cast<std::string>(nIsBid) + boost::lexical_cast<std::string>(nTradeFee) + nUserPubKey
		+ boost::lexical_cast<std::string>(nTimeSubmit) + nUserHash + boost::lexical_cast<std::string>(nUserTradeID) + nMNBalancePubKey
		+ boost::lexical_cast<std::string>(nBalanceQty) + boost::lexical_cast<std::string>(nBalanceAmount) + boost::lexical_cast<std::string>(nLastUpdate);

	if (!CMessageSigner::SignMessage(strMessage, mnBalanceVchSig, activeMasternode.keyMasternode))
	{
		LogPrintf("CUserTrade::MNBalanceSign -- SignMessage() failed\n");
		return false;
	}

	if (!CMessageSigner::VerifyMessage(activeMasternode.pubKeyMasternode, mnBalanceVchSig, strMessage, strError))
	{
		LogPrintf("CUserTrade::MNBalanceSign -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CUserTrade::MNTradeSign()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nPrice) + boost::lexical_cast<std::string>(nQuantity)
		+ boost::lexical_cast<std::string>(nAmount) + boost::lexical_cast<std::string>(nIsBid)+ boost::lexical_cast<std::string>(nTradeFee) + nUserPubKey
		+ boost::lexical_cast<std::string>(nTimeSubmit) + nUserHash + boost::lexical_cast<std::string>(nUserTradeID) + boost::lexical_cast<std::string>(nPairTradeID)
		+ nMNBalancePubKey + nMNTradePubKey	+ boost::lexical_cast<std::string>(nBalanceQty) + boost::lexical_cast<std::string>(nBalanceAmount) 
		+ boost::lexical_cast<std::string>(nLastUpdate);

	if (!CMessageSigner::SignMessage(strMessage, mnTradeVchSig, activeMasternode.keyMasternode)) 
	{
		LogPrintf("CUserTrade::MNTradeSign -- SignMessage() failed\n");
		return false;
	}

	if (!CMessageSigner::VerifyMessage(activeMasternode.pubKeyMasternode, mnTradeVchSig, strMessage, strError)) 
	{
		LogPrintf("CUserTrade::MNTradeSign -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CCancelTrade::MNSign()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nUserTradeID) + boost::lexical_cast<std::string>(nPairTradeID) + nUserPubKey + boost::lexical_cast<std::string>(isBid)
		+ boost::lexical_cast<std::string>(nUserSubmitTime) + boost::lexical_cast<std::string>(nPrice) + boost::lexical_cast<std::string>(nBalanceQty)
		+ boost::lexical_cast<std::string>(nBalanceAmount) + nMNTradePubKey + boost::lexical_cast<std::string>(nMNProcessTime);

	if (!CMessageSigner::SignMessage(strMessage, mnVchSig, activeMasternode.keyMasternode))
	{
		LogPrintf("CCancelTrade::MNSign -- SignMessage() failed\n");
		return false;
	}

	if (!CMessageSigner::VerifyMessage(activeMasternode.pubKeyMasternode, mnVchSig, strMessage, strError))
	{
		LogPrintf("CCancelTrade::MNSign -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CCancelTrade::VerifyUserSignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nUserTradeID) + boost::lexical_cast<std::string>(nPairTradeID) + nUserPubKey + boost::lexical_cast<std::string>(isBid)
		+ boost::lexical_cast<std::string>(nUserSubmitTime) + boost::lexical_cast<std::string>(nPrice);
	CPubKey pubkey(ParseHex(nUserPubKey));

	if (!CMessageSigner::VerifyMessage(pubkey, userVchSig, strMessage, strError)) {
		LogPrintf("CCancelTrade::VerifyUserSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CCancelTrade::VerifyMNSignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nUserTradeID) + boost::lexical_cast<std::string>(nPairTradeID) + nUserPubKey + boost::lexical_cast<std::string>(isBid)
		+ boost::lexical_cast<std::string>(nUserSubmitTime) + boost::lexical_cast<std::string>(nPrice)+ boost::lexical_cast<std::string>(nBalanceQty) 
		+ boost::lexical_cast<std::string>(nBalanceAmount) + nMNTradePubKey + boost::lexical_cast<std::string>(nMNProcessTime);
	CPubKey pubkey(ParseHex(MNPubKey));

	if (!CMessageSigner::VerifyMessage(pubkey, mnVchSig, strMessage, strError)) {
		LogPrintf("CCancelTrade::VerifyMNSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

void CUserTrade::RelayTo(CNode* node, CConnman& connman)
{

}

void CUserTrade::RelayToBalanceMN(CConnman& connman)
{
	
}

void CUserTrade::RelayToTradeMN(CConnman& connman)
{

}

void CUserTrade::RelayToBackupMN(CConnman& connman)
{

}

bool CActualTrade::TradeMNSign()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nActualTradeID) + boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nUserTrade1)
		+ boost::lexical_cast<std::string>(nUserTrade2) + boost::lexical_cast<std::string>(nTradePrice) + boost::lexical_cast<std::string>(nTradeQty)
		+ boost::lexical_cast<std::string>(nTradeAmount) + boost::lexical_cast<std::string>(nBidAmount) + boost::lexical_cast<std::string>(nAskAmount) + nUserPubKey1
		+ nUserPubKey2 + boost::lexical_cast<std::string>(nFee1) + boost::lexical_cast<std::string>(nFee2) + boost::lexical_cast<std::string>(nFromBid) + nTradeMNPubKey
		+ nCurrentHash + boost::lexical_cast<std::string>(nTradeTime);

	if (!CMessageSigner::SignMessage(strMessage, mnTradeVchSig, activeMasternode.keyMasternode))
	{
		LogPrintf("CActualTrade::TradeMNSign -- SignMessage() failed\n");
		return false;
	}

	if (!CMessageSigner::VerifyMessage(activeMasternode.pubKeyMasternode, mnTradeVchSig, strMessage, strError))
	{
		LogPrintf("CActualTrade::TradeMNSign -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CActualTrade::Balance1MNSign()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nActualTradeID) + boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nUserTrade1)
		+ boost::lexical_cast<std::string>(nUserTrade2) + boost::lexical_cast<std::string>(nTradePrice) + boost::lexical_cast<std::string>(nTradeQty)
		+ boost::lexical_cast<std::string>(nTradeAmount) + boost::lexical_cast<std::string>(nBidAmount) + boost::lexical_cast<std::string>(nAskAmount) + nUserPubKey1
		+ nUserPubKey2 + boost::lexical_cast<std::string>(nFee1) + boost::lexical_cast<std::string>(nFee2) + boost::lexical_cast<std::string>(nFromBid) + nTradeMNPubKey
		+ nBalance1MNPubKey + nCurrentHash + boost::lexical_cast<std::string>(nTradeTime);

	if (!CMessageSigner::SignMessage(strMessage, mnBalance1VchSig, activeMasternode.keyMasternode))
	{
		LogPrintf("CActualTrade::TradeMNSign -- SignMessage() failed\n");
		return false;
	}

	if (!CMessageSigner::VerifyMessage(activeMasternode.pubKeyMasternode, mnBalance1VchSig, strMessage, strError))
	{
		LogPrintf("CActualTrade::TradeMNSign -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CActualTrade::Balance2MNSign()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nActualTradeID) + boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nUserTrade1)
		+ boost::lexical_cast<std::string>(nUserTrade2) + boost::lexical_cast<std::string>(nTradePrice) + boost::lexical_cast<std::string>(nTradeQty)
		+ boost::lexical_cast<std::string>(nTradeAmount) + boost::lexical_cast<std::string>(nBidAmount) + boost::lexical_cast<std::string>(nAskAmount) + nUserPubKey1
		+ nUserPubKey2 + boost::lexical_cast<std::string>(nFee1) + boost::lexical_cast<std::string>(nFee2) + boost::lexical_cast<std::string>(nFromBid) + nTradeMNPubKey
		+ nBalance2MNPubKey + nCurrentHash + boost::lexical_cast<std::string>(nTradeTime);

	if (!CMessageSigner::SignMessage(strMessage, mnBalance2VchSig, activeMasternode.keyMasternode))
	{
		LogPrintf("CActualTrade::TradeMNSign -- SignMessage() failed\n");
		return false;
	}

	if (!CMessageSigner::VerifyMessage(activeMasternode.pubKeyMasternode, mnBalance2VchSig, strMessage, strError))
	{
		LogPrintf("CActualTrade::TradeMNSign -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CActualTrade::VerifyTradeMNSignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nActualTradeID) + boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nUserTrade1)
		+ boost::lexical_cast<std::string>(nUserTrade2) + boost::lexical_cast<std::string>(nTradePrice) + boost::lexical_cast<std::string>(nTradeQty)
		+ boost::lexical_cast<std::string>(nTradeAmount) + boost::lexical_cast<std::string>(nBidAmount) + boost::lexical_cast<std::string>(nAskAmount) + nUserPubKey1
		+ nUserPubKey2 + boost::lexical_cast<std::string>(nFee1) + boost::lexical_cast<std::string>(nFee2) + boost::lexical_cast<std::string>(nFromBid) + nTradeMNPubKey
		+ nCurrentHash + boost::lexical_cast<std::string>(nTradeTime);
	CPubKey pubkey(ParseHex(nTradeMNPubKey));

	if (!CMessageSigner::VerifyMessage(pubkey, mnTradeVchSig, strMessage, strError)) {
		LogPrintf("CActualTrade::VerifyTradeMNSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CActualTrade::VerifyBalance1MNSignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nActualTradeID) + boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nUserTrade1)
		+ boost::lexical_cast<std::string>(nUserTrade2) + boost::lexical_cast<std::string>(nTradePrice) + boost::lexical_cast<std::string>(nTradeQty)
		+ boost::lexical_cast<std::string>(nTradeAmount) + boost::lexical_cast<std::string>(nBidAmount) + boost::lexical_cast<std::string>(nAskAmount) + nUserPubKey1
		+ nUserPubKey2 + boost::lexical_cast<std::string>(nFee1) + boost::lexical_cast<std::string>(nFee2) + boost::lexical_cast<std::string>(nFromBid) + nTradeMNPubKey
		+ nBalance1MNPubKey + nCurrentHash + boost::lexical_cast<std::string>(nTradeTime);
	CPubKey pubkey(ParseHex(nBalance1MNPubKey));

	if (!CMessageSigner::VerifyMessage(pubkey, mnBalance1VchSig, strMessage, strError)) {
		LogPrintf("CActualTrade::VerifyTradeMNSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}

bool CActualTrade::VerifyBalance2MNSignature()
{
	std::string strError = "";
	std::string strMessage = boost::lexical_cast<std::string>(nActualTradeID) + boost::lexical_cast<std::string>(nTradePairID) + boost::lexical_cast<std::string>(nUserTrade1)
		+ boost::lexical_cast<std::string>(nUserTrade2) + boost::lexical_cast<std::string>(nTradePrice) + boost::lexical_cast<std::string>(nTradeQty)
		+ boost::lexical_cast<std::string>(nTradeAmount) + boost::lexical_cast<std::string>(nBidAmount) + boost::lexical_cast<std::string>(nAskAmount) + nUserPubKey1
		+ nUserPubKey2 + boost::lexical_cast<std::string>(nFee1) + boost::lexical_cast<std::string>(nFee2) + boost::lexical_cast<std::string>(nFromBid) + nTradeMNPubKey
		+ nBalance2MNPubKey + nCurrentHash + boost::lexical_cast<std::string>(nTradeTime);
	CPubKey pubkey(ParseHex(nBalance2MNPubKey));

	if (!CMessageSigner::VerifyMessage(pubkey, mnBalance1VchSig, strMessage, strError)) {
		LogPrintf("CActualTrade::VerifyTradeMNSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}

	return true;
}