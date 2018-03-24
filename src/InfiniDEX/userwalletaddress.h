// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef USERWALLETADDRESS_H
#define USERWALLETADDRESS_H

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include "userconnection.h"

class CUserWalletAddress;
class CUserWalletAddressSetting;
class CUserWalletAddressRequest;
class CUserWalletAddressManager;

typedef std::map<std::string, CUserWalletAddress> mapPubKeyWalletAddress;
typedef std::pair<CUserWalletAddressSetting, mapPubKeyWalletAddress> pairSettingPubKeyAddress;
extern std::map<int, pairSettingPubKeyAddress> mapWalletAddressByCoinID;
extern CUserWalletAddressManager userWalletAddressManager;

class CUserWalletAddressSetting
{
public:
	bool ProvideUserWalletInfo;
	bool SyncInProgress;

	CUserWalletAddressSetting() :
		ProvideUserWalletInfo(false),
		SyncInProgress(false)
	{}
};

class CUserWalletAddress
{
private:
	std::vector<unsigned char> vchSig;

public:
	int nCoinID;
	std::string nUserPubKey;
	std::string nWalletAddress;
	bool nWithdrawalStatus;
	bool nDepositStatus;
	std::string nHash;
	uint64_t nLastUpdateTime;

	CUserWalletAddress(int nCoinID, std::string nUserPubKey, std::string nWalletAddress, bool nWithdrawalStatus, bool nDepositStatus, std::string nHash, uint64_t nLastUpdateTime) :
		nCoinID(nCoinID),
		nUserPubKey(nUserPubKey),
		nWalletAddress(nWalletAddress),
		nWithdrawalStatus(nWithdrawalStatus),
		nDepositStatus(nDepositStatus),
		nHash(nHash),
		nLastUpdateTime(nLastUpdateTime)
	{}

	CUserWalletAddress() :
		nCoinID(0),
		nUserPubKey(""),
		nWalletAddress(""),
		nWithdrawalStatus(false),
		nDepositStatus(false),
		nHash(""),
		nLastUpdateTime(0)
	{}

	bool Verify();
	void RelayTo(CNode* node, CConnman& connman);
};

class CUserWalletAddressRequest
{
private:
	std::vector<unsigned char> vchSig;

public:
	std::string nUserPubKey;
	int nCoinID;
	uint64_t nRequestTime;
	std::string nHash;

	CUserWalletAddressRequest(std::string nUserPubKey, int nCoinID, uint64_t nRequestTime, std::string nHash):
		nUserPubKey(nUserPubKey),
		nCoinID(nCoinID),
		nRequestTime(nRequestTime),
		nHash(nHash)
	{}

	bool Verify();
};

class CUserWalletAddressManager
{
private:
	std::vector<unsigned char> vchSig;

public:
	CUserWalletAddressManager() {}
	void ProcessMessage(CNode* node, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
	bool IsCoinInList(int CoinID);
	void SetupNewCoinContainer(int CoinID);
	void SendWalletAddress(CNode* node, CConnman& connman, CUserWalletAddress WalletAddress);
	bool UpdateWalletAddress(CUserWalletAddress WalletAddress);
	CUserWalletAddress GetWalletAddress(int CoinID, std::string UserPubKey);
	void RequestWalletAddress(int CoinID, std::string UserPubKey);
};

#endif