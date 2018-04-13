// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "userwithdraw.h"

class CUserWithdraw;
class CUserWithdrawManager;

std::map<int, PairPubKeyUserWithdraw> mapCoinUserWithdraw; //coin ID and users withdraw details
CUserWithdrawManager userWithdrawManager;

void CUserWithdrawManager::ProcessUserWithdraw(CNode* node, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{

}

void CUserWithdrawManager::InputUserWithdraw(std::shared_ptr<CUserWithdraw> userWithdraw)
{
	if (!userWithdraw->VerifyWithdrawalSignature())
		return;

	if (userWithdraw->UserWithdrawID != 0 && !userWithdraw->VerifyMasternodeSignature())
		return;

	
}

bool CUserWithdraw::VerifyWithdrawalSignature()
{
	return true;
}

bool CUserWithdraw::VerifyMasternodeSignature()
{
	return true;
}

bool CUserWithdraw::VerifyFinalSignature()
{
	return true;
}

bool CUserWithdraw::MNSign()
{
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

void CUserWithdrawManager::ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
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