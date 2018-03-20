// Copyright (c) 2017-2018 The Infinex Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COININFO_H
#define COININFO_H

#include <iostream>
#include <vector>
#include <map>
#include <memory>

class CCoinInfo;
class CCoinInfoManager;

extern std::map<int, std::shared_ptr<CCoinInfo>> mapCompleteCoinInfoWithID;
extern std::map<std::string, std::shared_ptr<CCoinInfo>> mapCompleteCoinInfoWithSymbol;
extern CCoinInfoManager coinInfoManager;

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
};

class CCoinInfoManager
{
private:
	std::vector<unsigned char> vchSig;

public:
	CCoinInfoManager() {}
	bool IsCoinInCompleteListByCoinID(int CoinID);
	bool IsCoinInCompleteListBySymbol(std::string Symbol);
	bool GetCoinInfoByCoinID(int CoinID, CCoinInfo &CoinInfo);
	bool GetCoinInfoBySymbol(std::string Symbol, CCoinInfo &CoinInfo);
	void UpdateCoinInfo(CCoinInfo CoinInfo);
};

#endif