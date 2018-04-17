// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COININFO_H
#define COININFO_H

#include <vector>
#include <map>
#include <memory>
#include "net.h"
#include "utilstrencodings.h"

class CCoinInfo;
class CCoinInfoManager;
class CCoinInfoSync;

extern std::map<int, std::shared_ptr<CCoinInfo>> mapCompleteCoinInfoWithID;
extern std::map<std::string, std::shared_ptr<CCoinInfo>> mapCompleteCoinInfoWithSymbol;
extern CCoinInfoManager coinInfoManager;
extern CCoinInfoSync coinInfoSync;

class CCoinInfo
{
private:
	std::vector<unsigned char> vchSig;

public:
	int nCoinInfoID;
	std::string nName;
	std::string nSymbol;
	std::string nLogoURL;
	int nBlockTime;
	int nBlockHeight;
	std::string nWalletVersion;
	bool nWalletActive;
	std::string nWalletStatus;
	uint64_t nLastUpdate;

	CCoinInfo(int nCoinInfoID, std::string nName, std::string nSymbol, std::string nLogoURL, int nBlockTime,
		int nBlockHeight, std::string nWalletVersion, bool nWalletActive, std::string nWalletStatus, uint64_t nLastUpdate) :
		nCoinInfoID(nCoinInfoID),
		nName(nName),
		nSymbol(nSymbol),
		nLogoURL(nLogoURL),
		nBlockTime(nBlockTime),
		nBlockHeight(nBlockHeight),
		nWalletVersion(nWalletVersion),
		nWalletActive(nWalletActive),
		nWalletStatus(nWalletStatus),
		nLastUpdate(nLastUpdate)
	{}

	CCoinInfo() :
		nCoinInfoID(0),
		nName(""),
		nSymbol(""),
		nLogoURL(""),
		nBlockTime(0),
		nBlockHeight(0),
		nWalletVersion(""),
		nWalletActive(false),
		nWalletStatus(""),
		nLastUpdate(0)
	{}

	ADD_SERIALIZE_METHODS;
	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
		READWRITE(nCoinInfoID);
		READWRITE(nName);
		READWRITE(nSymbol);
		READWRITE(nLogoURL);
		READWRITE(nBlockTime);
		READWRITE(nBlockHeight);
		READWRITE(nWalletVersion);
		READWRITE(nWalletActive);
		READWRITE(nWalletStatus);
		READWRITE(nLastUpdate);
		READWRITE(vchSig);
	}

	bool VerifySignature();
	bool Sign();
};

class CCoinInfoSync
{
public:
	std::vector<CCoinInfo> CompleteCoinInfo;

	CCoinInfoSync(std::vector<CCoinInfo> CompleteCoinInfo) :
		CompleteCoinInfo(CompleteCoinInfo)
	{}

	CCoinInfoSync() :
		CompleteCoinInfo()
	{}

	// ADD_SERIALIZE_METHODS;
	// template <typename Stream, typename Operation>
	// inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
	// 	READWRITE(CompleteCoinInfo);
	// }

	void Relay(CNode* node, CConnman& connman);
};

class CCoinInfoManager
{
public:
	CCoinInfoManager() {}
	void ProcessMessage(CNode* pfrom, std::string& strCommand, CDataStream& vRecv, CConnman& connman);
	bool AddCoinInfo(std::string coinID, std::string name, std::string symbol, std::string logoURL, std::string blockTime, std::string blockHeight, std::string walletVersion, std::string walletActive, std::string walletStatus);
	void InputCoinInfo(const std::shared_ptr<CCoinInfo>& CoinInfo);
	bool IsCoinInCompleteListByCoinID(int CoinID);
	bool IsCoinInCompleteListBySymbol(std::string Symbol);
	bool GetCoinInfoByCoinID(int CoinID, CCoinInfo &CoinInfo);
	bool GetCoinInfoBySymbol(std::string Symbol, CCoinInfo &CoinInfo);
};

#endif