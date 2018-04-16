// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "noderole.h"
#include "userconnection.h"
#include "userwalletaddress.h"

class CUserWalletAddress;
class CUserWalletAddressSetting;
class CUserWalletAddressRequest;
class CUserWalletAddressManager;

std::map<int, pairSettingPubKeyAddress> mapWalletAddressByCoinID;
CUserWalletAddressManager userWalletAddressManager;

bool CUserWalletAddress::Verify()
{
	return true;
}

void CUserWalletAddress::RelayTo(CNode* node, CConnman& connman)
{

}

bool CUserWalletAddressRequest::Verify()
{
	return true;
}

void CUserWalletAddressManager::AssignDepositInfoRole(int CoinID)
{

}

void CUserWalletAddressManager::ProcessMessage(CNode* node, std::string& strCommand, CDataStream& vRecv, CConnman& connman)
{

}

bool CUserWalletAddressManager::IsCoinInList(int CoinID)
{
	return mapWalletAddressByCoinID.count(CoinID);
}

void CUserWalletAddressManager::SetupNewCoinContainer(int CoinID)
{
	if (IsCoinInList(CoinID))
		return;

	mapWalletAddressByCoinID.insert(std::make_pair(CoinID, pairSettingPubKeyAddress()));
}

void CUserWalletAddressManager::SendWalletAddress(CNode* node, CConnman& connman, CUserWalletAddress WalletAddress)
{

}

//change to enum
bool CUserWalletAddressManager::UpdateWalletAddress(CUserWalletAddress WalletAddress)
{
	if (!IsCoinInList(WalletAddress.nCoinID))
	{
		SetupNewCoinContainer(WalletAddress.nCoinID);
		return false;
	}

	CUserWalletAddress temp(GetWalletAddress(WalletAddress.nCoinID, WalletAddress.nUserPubKey));
	if (temp.nCoinID == 0)
	{
		mapWalletAddressByCoinID[WalletAddress.nCoinID].second.insert(std::make_pair(WalletAddress.nUserPubKey, WalletAddress));
	}
	else if (temp.nLastUpdateTime < WalletAddress.nLastUpdateTime)
	{
		mapWalletAddressByCoinID[WalletAddress.nCoinID].second[WalletAddress.nUserPubKey] = WalletAddress;
	}
	return true;
}

CUserWalletAddress CUserWalletAddressManager::GetWalletAddress(int CoinID, std::string UserPubKey)
{
	return mapWalletAddressByCoinID[CoinID].second[UserPubKey];
}

void CUserWalletAddressManager::RequestWalletAddress(int CoinID, std::string UserPubKey)
{

}