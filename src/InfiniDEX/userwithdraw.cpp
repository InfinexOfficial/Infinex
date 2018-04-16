// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "activemasternode.h"
#include "messagesigner.h"
#include "net_processing.h"
#include "noderole.h"
#include "timedata.h"
#include "userbalance.h"
#include "userconnection.h"
#include "userwithdraw.h"
#include <boost/lexical_cast.hpp>

class CUserWithdraw;
class CUserWithdrawManager;

std::map<std::string, mapCoinUserWithdraw> mapUserWithdraw;
CUserWithdrawManager userWithdrawManager;

void CUserWithdrawManager::ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{
	if (strCommand == NetMsgType::DEXUSERWITHDRAW) {

		CUserWithdraw userWithdraw;
		vRecv >> userWithdraw;

		if (!userWithdraw.VerifyUserSignature()) {
			LogPrintf("CUserWithdrawManager::ProcessMessage -- invalid user signature\n");
			Misbehaving(pfrom->GetId(), 100);
			return;
		}

		if (userWithdraw.WithdrawProcessTime > 0)
		{
			if (!userWithdraw.VerifyFinalSignature())
			{
				LogPrintf("CUserWithdrawManager::ProcessMessage -- invalid masternode signature\n");
				Misbehaving(pfrom->GetId(), 100);
				return;
			}
			else
			{

			}
		}
		else if (userWithdraw.MNPubKey != "")
		{
			if (!userWithdraw.VerifyMNSignature())
			{
				LogPrintf("CUserWithdrawManager::ProcessMessage -- invalid masternode signature\n");
				Misbehaving(pfrom->GetId(), 100);
				return;
			}
			else
			{

			}
		}
		else
		{
			InputUserWithdraw(userWithdraw);
		}
	}
	else if (strCommand == NetMsgType::DEXGETUSERWITHDRAW)
	{
	}
}

void CUserWithdrawManager::InputUserWithdraw(CUserWithdraw userWithdraw)
{
	if (userBalanceManager.InChargeOfUserBalance(userWithdraw.UserPubKey))
	{
		if (mapUserBalanceByPubKey.count(userWithdraw.UserPubKey))
		{
			auto& a = mapUserBalanceByPubKey[userWithdraw.UserPubKey];
			if (a.count(userWithdraw.CoinID))
			{
				auto& b = a[userWithdraw.CoinID];

				if (!mapUserWithdraw.count(userWithdraw.UserPubKey))
					mapUserWithdraw.insert(std::make_pair(userWithdraw.UserPubKey, mapCoinUserWithdraw()));
				auto& c = mapUserWithdraw[userWithdraw.UserPubKey];
				if (!c.count(userWithdraw.CoinID))					
					c.insert(std::make_pair(userWithdraw.CoinID, mapUserWithdrawWithID()));
				auto& d = c[userWithdraw.CoinID];
				if (d.first == 0)
					d.first = 1;

				if (b->nAvailableBalance >= userWithdraw.WithdrawAmount)
				{
					b->nAvailableBalance -= userWithdraw.WithdrawAmount;
					b->nPendingWithdrawalBalance += userWithdraw.WithdrawAmount;
					userWithdraw.MNPubKey = MNPubKey;
					userWithdraw.UserWithdrawID = d.first;
					userWithdraw.WithdrawCheckTime = GetAdjustedTime();
					if (!userWithdraw.MNSign())
						return;

					d.second.insert(std::make_pair(userWithdraw.UserWithdrawID, userWithdraw));
					++d.first;
				}
			}
		}
	}
}

void CUserWithdrawManager::InputUserWithdrawRecord(CUserWithdraw userWithdraw)
{
	if (InChargeOfUserWithdrawRecord(userWithdraw.UserPubKey))
	{
		if (!mapUserWithdraw.count(userWithdraw.UserPubKey))
			mapUserWithdraw.insert(std::make_pair(userWithdraw.UserPubKey, mapCoinUserWithdraw()));

		auto& a = mapUserWithdraw[userWithdraw.UserPubKey];
		if (!a.count(userWithdraw.CoinID))
			a.insert(std::make_pair(userWithdraw.CoinID, mapUserWithdrawWithID()));

		auto& b = a[userWithdraw.CoinID];
		int lastID = b.first;
		if (lastID >= userWithdraw.UserWithdrawID)
		{
			auto& c = b.second[userWithdraw.UserWithdrawID];
			if (c.LastUpdateTime < userWithdraw.LastUpdateTime)
			{
				c = userWithdraw;
				userWithdraw.RelayToUser();
			}
		}
		else if (lastID == (userWithdraw.UserWithdrawID - 1))
		{			
			b.second.insert(std::make_pair(userWithdraw.UserWithdrawID, userWithdraw));
			++b.first;
			userWithdraw.RelayToUser();
		}
		else
		{
			//need to sync
		}
	}
}

bool CUserWithdrawManager::InChargeOfUserWithdrawRecord(std::string UserPubKey)
{
	char c = UserPubKey[2];	
	return mapUserWithdrawSetting[c].InChargeOfWithdrawRecord;
}

bool CUserWithdraw::VerifyUserSignature()
{
	std::string strError = "";
	std::string strMessage = UserPubKey + boost::lexical_cast<std::string>(CoinID) + boost::lexical_cast<std::string>(WithdrawAmount)
		+ boost::lexical_cast<std::string>(WithdrawRequestTime) + UserHash;
	CPubKey pubkey(ParseHex(UserPubKey));
	if (!CMessageSigner::VerifyMessage(pubkey, userVchSig, strMessage, strError)) {
		LogPrintf("CUserWithdraw::VerifyUserSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}
	return true;
}

bool CUserWithdraw::VerifyMNSignature()
{
	std::string strError = "";
	std::string strMessage = UserPubKey + boost::lexical_cast<std::string>(CoinID) + boost::lexical_cast<std::string>(WithdrawAmount)
		+ boost::lexical_cast<std::string>(WithdrawRequestTime) + UserHash + boost::lexical_cast<std::string>(UserWithdrawID) + MNPubKey
		+ boost::lexical_cast<std::string>(ValidWithdraw) + boost::lexical_cast<std::string>(WithdrawCheckTime);
	CPubKey pubkey(ParseHex(MNPubKey));
	if (!CMessageSigner::VerifyMessage(pubkey, mnVchSig, strMessage, strError)) {
		LogPrintf("CUserWithdraw::VerifyMNSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}
	return true;
}

bool CUserWithdraw::VerifyFinalSignature()
{
	std::string strError = "";
	std::string strMessage = UserPubKey + boost::lexical_cast<std::string>(CoinID) + boost::lexical_cast<std::string>(WithdrawAmount)
		+ boost::lexical_cast<std::string>(WithdrawRequestTime) + UserHash + boost::lexical_cast<std::string>(UserWithdrawID) + MNPubKey
		+ boost::lexical_cast<std::string>(ValidWithdraw) + boost::lexical_cast<std::string>(WithdrawCheckTime) + boost::lexical_cast<std::string>(WithdrawProcessTime)
		+ TransactionID + Remark + boost::lexical_cast<std::string>(LastUpdateTime);
	CPubKey pubkey(ParseHex(DEXKey));
	if (!CMessageSigner::VerifyMessage(pubkey, finalVchSig, strMessage, strError)) {
		LogPrintf("CUserWithdraw::VerifyFinalSignature -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}
	return true;
}

bool CUserWithdraw::MNSign()
{
	std::string strError = "";
	std::string strMessage = UserPubKey + boost::lexical_cast<std::string>(CoinID) + boost::lexical_cast<std::string>(WithdrawAmount)
		+ boost::lexical_cast<std::string>(WithdrawRequestTime) + UserHash + boost::lexical_cast<std::string>(UserWithdrawID) + MNPubKey
		+ boost::lexical_cast<std::string>(ValidWithdraw) + boost::lexical_cast<std::string>(WithdrawCheckTime);
	if (!CMessageSigner::SignMessage(strMessage, mnVchSig, activeMasternode.keyMasternode))
	{
		LogPrintf("CUserWithdraw::MNSign -- SignMessage() failed\n");
		return false;
	}
	if (!CMessageSigner::VerifyMessage(activeMasternode.pubKeyMasternode, mnVchSig, strMessage, strError))
	{
		LogPrintf("CUserWithdraw::MNSign -- VerifyMessage() failed, error: %s\n", strError);
		return false;
	}
	return true;
}

void CUserWithdraw::RelayToUser()
{

}

void CUserWithdraw::RelayToMN()
{

}

void CUserWithdraw::RelayToNetwork()
{

}

void CUserWithdrawManager::AssignWithdrawProcessorRole(int CoinID)
{

}

void CUserWithdrawManager::AssignWithdrawInfoRole(int CoinID)
{

}

void CUserWithdrawManager::ProcessUserWithdrawRequest(CUserWithdraw UserWithdrawRequest)
{

}

void CUserWithdrawManager::SendUserWithdrawalRecords(std::string UserPubKey, int CoinID)
{

}

void CUserWithdrawManager::SendUsersWithdrawalRecords(CNode* node, CConnman& connman, int CoinID)
{

}